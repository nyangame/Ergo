#include "editor_renderer.hpp"
#include "engine/math/mat4.hpp"
#include "engine/math/vec3.hpp"

#include <unordered_map>
#include <vector>
#include <cmath>
#include <cstdio>

// ===================================================================
// Vulkan real implementation
// ===================================================================
#if ERGO_HAS_VULKAN

#include <vulkan/vulkan.h>

#ifdef _WIN32
#include <vulkan/vulkan_win32.h>
#elif defined(__linux__)
#include <vulkan/vulkan_xlib.h>
#endif

namespace {

// Per-frame synchronisation constants
constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

// ------------------------------------------------------------------
// Per-render-target GPU resources
// ------------------------------------------------------------------
struct RenderTargetGPU {
    uint64_t    id          = 0;
    RenderMode  mode        = RenderMode::Scene;
    uint32_t    width       = 0;
    uint32_t    height      = 0;
    void*       native_handle = nullptr;
    EditorCamera camera;

    // Surface & swapchain
    VkSurfaceKHR           surface        = VK_NULL_HANDLE;
    VkSwapchainKHR         swapchain      = VK_NULL_HANDLE;
    VkFormat               swapchain_fmt  = VK_FORMAT_B8G8R8A8_SRGB;
    VkExtent2D             swapchain_ext  = {};
    std::vector<VkImage>     swapchain_images;
    std::vector<VkImageView> swapchain_views;

    // Depth buffer
    VkImage        depth_image  = VK_NULL_HANDLE;
    VkDeviceMemory depth_memory = VK_NULL_HANDLE;
    VkImageView    depth_view   = VK_NULL_HANDLE;

    // Render pass & framebuffers
    VkRenderPass                render_pass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer>  framebuffers;

    // Command recording
    VkCommandPool                         command_pool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer>          command_buffers; // one per frame-in-flight

    // Synchronisation (per frame-in-flight)
    std::vector<VkSemaphore> image_available;
    std::vector<VkSemaphore> render_finished;
    std::vector<VkFence>     in_flight;
    uint32_t current_frame = 0;
};

} // anonymous namespace

// ------------------------------------------------------------------
// Impl
// ------------------------------------------------------------------
struct EditorRenderer::Impl {
    bool initialized = false;

    // Device
    VkInstance       instance        = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice         device          = VK_NULL_HANDLE;
    uint32_t         gfx_queue_family = 0;
    VkQueue          gfx_queue       = VK_NULL_HANDLE;

    // Render targets
    uint64_t next_rt_id = 1;
    std::unordered_map<uint64_t, RenderTargetGPU> targets;

    // ----- helpers -----

    bool create_instance();
    bool pick_physical_device();
    bool create_device();

    bool create_surface(RenderTargetGPU& rt);
    bool create_swapchain(RenderTargetGPU& rt);
    bool create_depth_resources(RenderTargetGPU& rt);
    bool create_render_pass(RenderTargetGPU& rt);
    bool create_framebuffers(RenderTargetGPU& rt);
    bool create_command_resources(RenderTargetGPU& rt);
    bool create_sync_objects(RenderTargetGPU& rt);

    void destroy_swapchain_resources(RenderTargetGPU& rt);
    void destroy_render_target_full(RenderTargetGPU& rt);
    bool recreate_swapchain(RenderTargetGPU& rt, uint32_t w, uint32_t h);

    uint32_t find_memory_type(uint32_t filter, VkMemoryPropertyFlags props);
    VkFormat find_depth_format();

    void record_commands(RenderTargetGPU& rt, uint32_t image_index);
    void record_grid(VkCommandBuffer cmd, const EditorCamera& cam,
                     uint32_t w, uint32_t h);
};

// ===================================================================
// Instance / device setup
// ===================================================================

bool EditorRenderer::Impl::create_instance() {
    VkApplicationInfo app_info{};
    app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName   = "Ergo Editor";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName        = "Ergo";
    app_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion         = VK_API_VERSION_1_2;

    std::vector<const char*> extensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _WIN32
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(__linux__)
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#endif
    };

    VkInstanceCreateInfo ci{};
    ci.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pApplicationInfo        = &app_info;
    ci.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
    ci.ppEnabledExtensionNames = extensions.data();

#ifndef NDEBUG
    const char* validation[] = {"VK_LAYER_KHRONOS_validation"};
    ci.enabledLayerCount   = 1;
    ci.ppEnabledLayerNames = validation;
#endif

    return vkCreateInstance(&ci, nullptr, &instance) == VK_SUCCESS;
}

bool EditorRenderer::Impl::pick_physical_device() {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    if (count == 0) return false;

    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(instance, &count, devices.data());

    // Prefer discrete GPU
    for (auto& d : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(d, &props);
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            physical_device = d;
            return true;
        }
    }
    physical_device = devices[0];
    return true;
}

bool EditorRenderer::Impl::create_device() {
    // Find graphics queue family
    uint32_t qf_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &qf_count, nullptr);
    std::vector<VkQueueFamilyProperties> qf_props(qf_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &qf_count,
                                              qf_props.data());

    gfx_queue_family = UINT32_MAX;
    for (uint32_t i = 0; i < qf_count; ++i) {
        if (qf_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            gfx_queue_family = i;
            break;
        }
    }
    if (gfx_queue_family == UINT32_MAX) return false;

    float priority = 1.0f;
    VkDeviceQueueCreateInfo queue_ci{};
    queue_ci.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_ci.queueFamilyIndex = gfx_queue_family;
    queue_ci.queueCount       = 1;
    queue_ci.pQueuePriorities = &priority;

    const char* dev_exts[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceCreateInfo dev_ci{};
    dev_ci.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dev_ci.queueCreateInfoCount    = 1;
    dev_ci.pQueueCreateInfos       = &queue_ci;
    dev_ci.enabledExtensionCount   = 1;
    dev_ci.ppEnabledExtensionNames = dev_exts;

    if (vkCreateDevice(physical_device, &dev_ci, nullptr, &device) != VK_SUCCESS)
        return false;

    vkGetDeviceQueue(device, gfx_queue_family, 0, &gfx_queue);
    return true;
}

// ===================================================================
// Per-render-target setup
// ===================================================================

bool EditorRenderer::Impl::create_surface(RenderTargetGPU& rt) {
#ifdef _WIN32
    VkWin32SurfaceCreateInfoKHR ci{};
    ci.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    ci.hwnd      = static_cast<HWND>(rt.native_handle);
    ci.hinstance = GetModuleHandle(nullptr);
    return vkCreateWin32SurfaceKHR(instance, &ci, nullptr, &rt.surface) == VK_SUCCESS;
#elif defined(__linux__)
    VkXlibSurfaceCreateInfoKHR ci{};
    ci.sType  = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    // native_handle assumed to be packed {Display*, Window}
    // For now simplified: store Display* and Window separately in a real impl
    ci.dpy    = nullptr;  // TODO: pass display connection
    ci.window = reinterpret_cast<Window>(rt.native_handle);
    return vkCreateXlibSurfaceKHR(instance, &ci, nullptr, &rt.surface) == VK_SUCCESS;
#else
    (void)rt;
    return false;
#endif
}

bool EditorRenderer::Impl::create_swapchain(RenderTargetGPU& rt) {
    // Query surface capabilities
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, rt.surface, &caps);

    // Choose extent
    if (caps.currentExtent.width != UINT32_MAX) {
        rt.swapchain_ext = caps.currentExtent;
    } else {
        rt.swapchain_ext = {rt.width, rt.height};
    }

    uint32_t image_count = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && image_count > caps.maxImageCount)
        image_count = caps.maxImageCount;

    // Choose format (prefer B8G8R8A8_SRGB)
    uint32_t fmt_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, rt.surface, &fmt_count, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(fmt_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, rt.surface, &fmt_count,
                                          formats.data());

    rt.swapchain_fmt = formats[0].format;
    VkColorSpaceKHR color_space = formats[0].colorSpace;
    for (auto& f : formats) {
        if (f.format == VK_FORMAT_B8G8R8A8_SRGB &&
            f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            rt.swapchain_fmt = f.format;
            color_space = f.colorSpace;
            break;
        }
    }

    // Choose present mode (prefer mailbox for low-latency)
    uint32_t pm_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, rt.surface,
                                               &pm_count, nullptr);
    std::vector<VkPresentModeKHR> present_modes(pm_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, rt.surface,
                                               &pm_count, present_modes.data());

    VkPresentModeKHR chosen_pm = VK_PRESENT_MODE_FIFO_KHR;
    for (auto pm : present_modes) {
        if (pm == VK_PRESENT_MODE_MAILBOX_KHR) { chosen_pm = pm; break; }
    }

    VkSwapchainCreateInfoKHR sc_ci{};
    sc_ci.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    sc_ci.surface          = rt.surface;
    sc_ci.minImageCount    = image_count;
    sc_ci.imageFormat      = rt.swapchain_fmt;
    sc_ci.imageColorSpace  = color_space;
    sc_ci.imageExtent      = rt.swapchain_ext;
    sc_ci.imageArrayLayers = 1;
    sc_ci.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    sc_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    sc_ci.preTransform     = caps.currentTransform;
    sc_ci.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    sc_ci.presentMode      = chosen_pm;
    sc_ci.clipped          = VK_TRUE;
    sc_ci.oldSwapchain     = rt.swapchain; // for recreation

    if (vkCreateSwapchainKHR(device, &sc_ci, nullptr, &rt.swapchain) != VK_SUCCESS)
        return false;

    // Retrieve images
    uint32_t sc_img_count = 0;
    vkGetSwapchainImagesKHR(device, rt.swapchain, &sc_img_count, nullptr);
    rt.swapchain_images.resize(sc_img_count);
    vkGetSwapchainImagesKHR(device, rt.swapchain, &sc_img_count,
                             rt.swapchain_images.data());

    // Create image views
    rt.swapchain_views.resize(sc_img_count);
    for (uint32_t i = 0; i < sc_img_count; ++i) {
        VkImageViewCreateInfo iv_ci{};
        iv_ci.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        iv_ci.image    = rt.swapchain_images[i];
        iv_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        iv_ci.format   = rt.swapchain_fmt;
        iv_ci.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        iv_ci.subresourceRange.baseMipLevel   = 0;
        iv_ci.subresourceRange.levelCount     = 1;
        iv_ci.subresourceRange.baseArrayLayer = 0;
        iv_ci.subresourceRange.layerCount     = 1;
        if (vkCreateImageView(device, &iv_ci, nullptr, &rt.swapchain_views[i])
                != VK_SUCCESS)
            return false;
    }
    return true;
}

VkFormat EditorRenderer::Impl::find_depth_format() {
    VkFormat candidates[] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
    };
    for (auto fmt : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physical_device, fmt, &props);
        if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            return fmt;
    }
    return VK_FORMAT_D32_SFLOAT;
}

uint32_t EditorRenderer::Impl::find_memory_type(uint32_t filter,
                                                  VkMemoryPropertyFlags props) {
    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_props);
    for (uint32_t i = 0; i < mem_props.memoryTypeCount; ++i) {
        if ((filter & (1 << i)) &&
            (mem_props.memoryTypes[i].propertyFlags & props) == props)
            return i;
    }
    return 0;
}

bool EditorRenderer::Impl::create_depth_resources(RenderTargetGPU& rt) {
    VkFormat depth_fmt = find_depth_format();

    VkImageCreateInfo img_ci{};
    img_ci.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    img_ci.imageType     = VK_IMAGE_TYPE_2D;
    img_ci.format        = depth_fmt;
    img_ci.extent.width  = rt.swapchain_ext.width;
    img_ci.extent.height = rt.swapchain_ext.height;
    img_ci.extent.depth  = 1;
    img_ci.mipLevels     = 1;
    img_ci.arrayLayers   = 1;
    img_ci.samples       = VK_SAMPLE_COUNT_1_BIT;
    img_ci.tiling        = VK_IMAGE_TILING_OPTIMAL;
    img_ci.usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    if (vkCreateImage(device, &img_ci, nullptr, &rt.depth_image) != VK_SUCCESS)
        return false;

    VkMemoryRequirements mem_req;
    vkGetImageMemoryRequirements(device, rt.depth_image, &mem_req);

    VkMemoryAllocateInfo alloc{};
    alloc.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc.allocationSize  = mem_req.size;
    alloc.memoryTypeIndex = find_memory_type(
        mem_req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(device, &alloc, nullptr, &rt.depth_memory) != VK_SUCCESS)
        return false;
    vkBindImageMemory(device, rt.depth_image, rt.depth_memory, 0);

    VkImageViewCreateInfo view_ci{};
    view_ci.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_ci.image    = rt.depth_image;
    view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_ci.format   = depth_fmt;
    view_ci.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
    view_ci.subresourceRange.baseMipLevel   = 0;
    view_ci.subresourceRange.levelCount     = 1;
    view_ci.subresourceRange.baseArrayLayer = 0;
    view_ci.subresourceRange.layerCount     = 1;

    return vkCreateImageView(device, &view_ci, nullptr, &rt.depth_view) == VK_SUCCESS;
}

bool EditorRenderer::Impl::create_render_pass(RenderTargetGPU& rt) {
    VkAttachmentDescription color_att{};
    color_att.format         = rt.swapchain_fmt;
    color_att.samples        = VK_SAMPLE_COUNT_1_BIT;
    color_att.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_att.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    color_att.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_att.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    color_att.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depth_att{};
    depth_att.format         = find_depth_format();
    depth_att.samples        = VK_SAMPLE_COUNT_1_BIT;
    depth_att.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_att.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_att.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_att.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_att.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_ref{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference depth_ref{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &color_ref;
    subpass.pDepthStencilAttachment = &depth_ref;

    VkSubpassDependency dep{};
    dep.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dep.dstSubpass    = 0;
    dep.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dep.srcAccessMask = 0;
    dep.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription attachments[] = {color_att, depth_att};

    VkRenderPassCreateInfo rp_ci{};
    rp_ci.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_ci.attachmentCount = 2;
    rp_ci.pAttachments    = attachments;
    rp_ci.subpassCount    = 1;
    rp_ci.pSubpasses      = &subpass;
    rp_ci.dependencyCount = 1;
    rp_ci.pDependencies   = &dep;

    return vkCreateRenderPass(device, &rp_ci, nullptr, &rt.render_pass) == VK_SUCCESS;
}

bool EditorRenderer::Impl::create_framebuffers(RenderTargetGPU& rt) {
    rt.framebuffers.resize(rt.swapchain_views.size());
    for (size_t i = 0; i < rt.swapchain_views.size(); ++i) {
        VkImageView attachments[] = {rt.swapchain_views[i], rt.depth_view};

        VkFramebufferCreateInfo fb_ci{};
        fb_ci.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fb_ci.renderPass      = rt.render_pass;
        fb_ci.attachmentCount = 2;
        fb_ci.pAttachments    = attachments;
        fb_ci.width           = rt.swapchain_ext.width;
        fb_ci.height          = rt.swapchain_ext.height;
        fb_ci.layers          = 1;

        if (vkCreateFramebuffer(device, &fb_ci, nullptr, &rt.framebuffers[i])
                != VK_SUCCESS)
            return false;
    }
    return true;
}

bool EditorRenderer::Impl::create_command_resources(RenderTargetGPU& rt) {
    VkCommandPoolCreateInfo pool_ci{};
    pool_ci.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_ci.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_ci.queueFamilyIndex = gfx_queue_family;

    if (vkCreateCommandPool(device, &pool_ci, nullptr, &rt.command_pool)
            != VK_SUCCESS)
        return false;

    rt.command_buffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo alloc{};
    alloc.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc.commandPool        = rt.command_pool;
    alloc.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    return vkAllocateCommandBuffers(device, &alloc, rt.command_buffers.data())
            == VK_SUCCESS;
}

bool EditorRenderer::Impl::create_sync_objects(RenderTargetGPU& rt) {
    rt.image_available.resize(MAX_FRAMES_IN_FLIGHT);
    rt.render_finished.resize(MAX_FRAMES_IN_FLIGHT);
    rt.in_flight.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo sem_ci{};
    sem_ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_ci{};
    fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        if (vkCreateSemaphore(device, &sem_ci, nullptr, &rt.image_available[i])
                != VK_SUCCESS)
            return false;
        if (vkCreateSemaphore(device, &sem_ci, nullptr, &rt.render_finished[i])
                != VK_SUCCESS)
            return false;
        if (vkCreateFence(device, &fence_ci, nullptr, &rt.in_flight[i])
                != VK_SUCCESS)
            return false;
    }
    return true;
}

// ------------------------------------------------------------------
// Teardown helpers
// ------------------------------------------------------------------

void EditorRenderer::Impl::destroy_swapchain_resources(RenderTargetGPU& rt) {
    for (auto fb : rt.framebuffers) vkDestroyFramebuffer(device, fb, nullptr);
    rt.framebuffers.clear();

    if (rt.depth_view)   vkDestroyImageView(device, rt.depth_view, nullptr);
    if (rt.depth_image)  vkDestroyImage(device, rt.depth_image, nullptr);
    if (rt.depth_memory) vkFreeMemory(device, rt.depth_memory, nullptr);
    rt.depth_view   = VK_NULL_HANDLE;
    rt.depth_image  = VK_NULL_HANDLE;
    rt.depth_memory = VK_NULL_HANDLE;

    for (auto iv : rt.swapchain_views) vkDestroyImageView(device, iv, nullptr);
    rt.swapchain_views.clear();
    rt.swapchain_images.clear();
}

void EditorRenderer::Impl::destroy_render_target_full(RenderTargetGPU& rt) {
    vkDeviceWaitIdle(device);

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        if (i < rt.image_available.size())
            vkDestroySemaphore(device, rt.image_available[i], nullptr);
        if (i < rt.render_finished.size())
            vkDestroySemaphore(device, rt.render_finished[i], nullptr);
        if (i < rt.in_flight.size())
            vkDestroyFence(device, rt.in_flight[i], nullptr);
    }

    if (rt.command_pool) vkDestroyCommandPool(device, rt.command_pool, nullptr);

    destroy_swapchain_resources(rt);
    if (rt.render_pass) vkDestroyRenderPass(device, rt.render_pass, nullptr);
    if (rt.swapchain)   vkDestroySwapchainKHR(device, rt.swapchain, nullptr);
    if (rt.surface)     vkDestroySurfaceKHR(instance, rt.surface, nullptr);
}

bool EditorRenderer::Impl::recreate_swapchain(RenderTargetGPU& rt,
                                               uint32_t w, uint32_t h) {
    vkDeviceWaitIdle(device);
    rt.width  = w;
    rt.height = h;

    destroy_swapchain_resources(rt);

    VkSwapchainKHR old = rt.swapchain;
    if (!create_swapchain(rt)) return false;
    if (old) vkDestroySwapchainKHR(device, old, nullptr);

    if (!create_depth_resources(rt)) return false;
    if (!create_framebuffers(rt)) return false;
    return true;
}

// ===================================================================
// Command recording
// ===================================================================

void EditorRenderer::Impl::record_commands(RenderTargetGPU& rt,
                                            uint32_t image_index) {
    auto cmd = rt.command_buffers[rt.current_frame];
    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(cmd, &begin_info);

    // Clear values: dark gray for scene, sky-blue for game
    VkClearValue clear_values[2];
    if (rt.mode == RenderMode::Scene) {
        clear_values[0].color = {{0.12f, 0.12f, 0.12f, 1.0f}};
    } else {
        clear_values[0].color = {{0.30f, 0.55f, 0.85f, 1.0f}};
    }
    clear_values[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo rp_begin{};
    rp_begin.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rp_begin.renderPass        = rt.render_pass;
    rp_begin.framebuffer       = rt.framebuffers[image_index];
    rp_begin.renderArea.extent = rt.swapchain_ext;
    rp_begin.clearValueCount   = 2;
    rp_begin.pClearValues      = clear_values;

    vkCmdBeginRenderPass(cmd, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

    // Set viewport and scissor
    VkViewport viewport{};
    viewport.width    = static_cast<float>(rt.swapchain_ext.width);
    viewport.height   = static_cast<float>(rt.swapchain_ext.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.extent = rt.swapchain_ext;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    // TODO: Draw scene objects (meshes, sprites)
    // TODO: Scene mode → draw grid + gizmos via a dedicated pipeline

    vkCmdEndRenderPass(cmd);
    vkEndCommandBuffer(cmd);
}

// ===================================================================
// EditorRenderer public API (Vulkan path)
// ===================================================================

EditorRenderer::EditorRenderer()  = default;
EditorRenderer::~EditorRenderer() { shutdown(); }

bool EditorRenderer::initialize() {
    impl_ = std::make_unique<Impl>();

    if (!impl_->create_instance()) {
        std::fprintf(stderr, "[EditorRenderer] Failed to create VkInstance\n");
        return false;
    }
    if (!impl_->pick_physical_device()) {
        std::fprintf(stderr, "[EditorRenderer] No suitable GPU found\n");
        return false;
    }
    if (!impl_->create_device()) {
        std::fprintf(stderr, "[EditorRenderer] Failed to create VkDevice\n");
        return false;
    }

    impl_->initialized = true;
    std::fprintf(stdout, "[EditorRenderer] Vulkan initialized\n");
    return true;
}

void EditorRenderer::shutdown() {
    if (!impl_) return;
    if (impl_->device) {
        vkDeviceWaitIdle(impl_->device);
        for (auto& [id, rt] : impl_->targets)
            impl_->destroy_render_target_full(rt);
        impl_->targets.clear();
        vkDestroyDevice(impl_->device, nullptr);
    }
    if (impl_->instance)
        vkDestroyInstance(impl_->instance, nullptr);
    impl_.reset();
}

bool EditorRenderer::is_initialized() const {
    return impl_ && impl_->initialized;
}

uint64_t EditorRenderer::create_render_target(void* native_window_handle,
                                               uint32_t width, uint32_t height,
                                               RenderMode mode) {
    if (!impl_ || !impl_->initialized) return 0;

    uint64_t id = impl_->next_rt_id++;
    RenderTargetGPU rt;
    rt.id            = id;
    rt.mode          = mode;
    rt.width         = width;
    rt.height        = height;
    rt.native_handle = native_window_handle;

    if (!impl_->create_surface(rt))           { return 0; }
    if (!impl_->create_swapchain(rt))         { return 0; }
    if (!impl_->create_depth_resources(rt))   { return 0; }
    if (!impl_->create_render_pass(rt))       { return 0; }
    if (!impl_->create_framebuffers(rt))      { return 0; }
    if (!impl_->create_command_resources(rt)) { return 0; }
    if (!impl_->create_sync_objects(rt))      { return 0; }

    impl_->targets[id] = std::move(rt);
    return id;
}

void EditorRenderer::destroy_render_target(uint64_t id) {
    if (!impl_) return;
    auto it = impl_->targets.find(id);
    if (it == impl_->targets.end()) return;
    impl_->destroy_render_target_full(it->second);
    impl_->targets.erase(it);
}

void EditorRenderer::resize_render_target(uint64_t id,
                                           uint32_t width, uint32_t height) {
    if (!impl_) return;
    auto it = impl_->targets.find(id);
    if (it == impl_->targets.end()) return;
    if (width == 0 || height == 0) return;
    impl_->recreate_swapchain(it->second, width, height);
}

void EditorRenderer::set_camera(uint64_t id, const EditorCamera& camera) {
    if (!impl_) return;
    auto it = impl_->targets.find(id);
    if (it != impl_->targets.end())
        it->second.camera = camera;
}

bool EditorRenderer::render_frame(uint64_t id) {
    if (!impl_) return false;
    auto it = impl_->targets.find(id);
    if (it == impl_->targets.end()) return false;

    auto& rt = it->second;
    auto frame = rt.current_frame;

    // Wait for this frame's previous submission
    vkWaitForFences(impl_->device, 1, &rt.in_flight[frame],
                    VK_TRUE, UINT64_MAX);

    // Acquire next swapchain image
    uint32_t image_index = 0;
    VkResult result = vkAcquireNextImageKHR(
        impl_->device, rt.swapchain, UINT64_MAX,
        rt.image_available[frame], VK_NULL_HANDLE, &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        impl_->recreate_swapchain(rt, rt.width, rt.height);
        return false;
    }

    vkResetFences(impl_->device, 1, &rt.in_flight[frame]);

    // Record commands
    impl_->record_commands(rt, image_index);

    // Submit
    VkSemaphore wait_sems[]   = {rt.image_available[frame]};
    VkSemaphore signal_sems[] = {rt.render_finished[frame]};
    VkPipelineStageFlags wait_stages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submit{};
    submit.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.waitSemaphoreCount   = 1;
    submit.pWaitSemaphores      = wait_sems;
    submit.pWaitDstStageMask    = wait_stages;
    submit.commandBufferCount   = 1;
    submit.pCommandBuffers      = &rt.command_buffers[frame];
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores    = signal_sems;

    if (vkQueueSubmit(impl_->gfx_queue, 1, &submit, rt.in_flight[frame])
            != VK_SUCCESS)
        return false;

    // Present
    VkPresentInfoKHR present{};
    present.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores    = signal_sems;
    present.swapchainCount     = 1;
    present.pSwapchains        = &rt.swapchain;
    present.pImageIndices      = &image_index;

    result = vkQueuePresentKHR(impl_->gfx_queue, &present);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        impl_->recreate_swapchain(rt, rt.width, rt.height);
    }

    rt.current_frame = (frame + 1) % MAX_FRAMES_IN_FLIGHT;
    return true;
}

// ===================================================================
// Stub implementation (no Vulkan)
// ===================================================================
#else // !ERGO_HAS_VULKAN

struct EditorRenderer::Impl {
    bool initialized = false;
    uint64_t next_rt_id = 1;

    struct StubTarget {
        uint64_t   id     = 0;
        uint32_t   width  = 0;
        uint32_t   height = 0;
        RenderMode mode   = RenderMode::Scene;
        EditorCamera camera;
    };
    std::unordered_map<uint64_t, StubTarget> targets;
};

EditorRenderer::EditorRenderer()  = default;
EditorRenderer::~EditorRenderer() { shutdown(); }

bool EditorRenderer::initialize() {
    impl_ = std::make_unique<Impl>();
    impl_->initialized = true;
    std::fprintf(stdout, "[EditorRenderer] Stub renderer (no Vulkan)\n");
    return true;
}

void EditorRenderer::shutdown() { impl_.reset(); }

bool EditorRenderer::is_initialized() const {
    return impl_ && impl_->initialized;
}

uint64_t EditorRenderer::create_render_target(void* /*native_window_handle*/,
                                               uint32_t width, uint32_t height,
                                               RenderMode mode) {
    if (!impl_) return 0;
    uint64_t id = impl_->next_rt_id++;
    impl_->targets[id] = {id, width, height, mode, {}};
    return id;
}

void EditorRenderer::destroy_render_target(uint64_t id) {
    if (impl_) impl_->targets.erase(id);
}

void EditorRenderer::resize_render_target(uint64_t id,
                                           uint32_t w, uint32_t h) {
    if (!impl_) return;
    auto it = impl_->targets.find(id);
    if (it != impl_->targets.end()) {
        it->second.width  = w;
        it->second.height = h;
    }
}

void EditorRenderer::set_camera(uint64_t id, const EditorCamera& camera) {
    if (!impl_) return;
    auto it = impl_->targets.find(id);
    if (it != impl_->targets.end()) it->second.camera = camera;
}

bool EditorRenderer::render_frame(uint64_t /*id*/) {
    return true; // no-op
}

#endif // ERGO_HAS_VULKAN
