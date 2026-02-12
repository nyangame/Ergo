#pragma once
#include "double_buffer.hpp"
#include "command_buffer.hpp"
#include "mesh.hpp"
#include "../math/mat4.hpp"
#include <vector>
#include <unordered_map>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>

// Render job for parallel command generation
struct RenderJob {
    uint32_t job_id = 0;
    uint32_t begin = 0;  // start index in entity list
    uint32_t end = 0;    // end index in entity list
    std::function<void(CommandBuffer& out, uint32_t begin, uint32_t end)> execute;
};

// Multi-threaded rendering pipeline
// - Game threads record commands into thread-local CommandBuffers
// - Commands are merged and double-buffered
// - Render thread consumes the front buffer
class RenderPipeline {
public:
    // Pipeline stage identifiers
    enum class Stage : uint32_t {
        Shadow,
        Opaque,
        Transparent,
        PostProcess,
        UI,
        Max
    };

private:
    // Double-buffered command streams per stage
    struct StageData {
        DoubleBufferedCommands commands;
        SharedCommandCollector collector;
    };
    std::array<StageData, static_cast<size_t>(Stage::Max)> stages_;

    // Mesh & material registry
    std::unordered_map<uint64_t, MeshData> meshes_;
    std::unordered_map<uint64_t, MaterialData> materials_;
    uint64_t next_mesh_id_ = 1;
    uint64_t next_material_id_ = 1;
    std::mutex resource_mutex_;

    // Worker thread pool for parallel command generation
    std::vector<std::thread> workers_;
    std::vector<RenderJob> job_queue_;
    std::mutex job_mutex_;
    std::condition_variable job_cv_;
    std::atomic<uint32_t> jobs_remaining_{0};
    std::condition_variable jobs_done_cv_;
    std::atomic<bool> shutdown_{false};

    // Camera data
    Mat4 view_matrix_;
    Mat4 projection_matrix_;

    // Frame tracking
    std::atomic<uint64_t> frame_number_{0};

    void worker_thread_func();

public:
    RenderPipeline();
    ~RenderPipeline();

    // Initialize with worker thread count (0 = auto-detect based on CPU cores)
    void initialize(uint32_t worker_count = 0);
    void shutdown();

    // Frame lifecycle
    void begin_frame();        // Swap buffers, prepare for new frame
    void end_frame();          // Finalize command collection

    // Submit commands to a specific stage
    void submit(Stage stage, const CommandBuffer& buffer);

    // Get a stage's read buffer (for the render backend to consume)
    const CommandBuffer& stage_commands(Stage stage) const;

    // Parallel job dispatch: splits work across worker threads
    void dispatch_jobs(const std::vector<RenderJob>& jobs);
    void wait_for_jobs();

    // Resource management
    uint64_t register_mesh(MeshData mesh);
    uint64_t register_material(MaterialData material);
    void unregister_mesh(uint64_t id);
    void unregister_material(uint64_t id);
    MeshData* get_mesh(uint64_t id);
    MaterialData* get_material(uint64_t id);

    // Camera
    void set_view_matrix(const Mat4& view) { view_matrix_ = view; }
    void set_projection_matrix(const Mat4& proj) { projection_matrix_ = proj; }
    const Mat4& view_matrix() const { return view_matrix_; }
    const Mat4& projection_matrix() const { return projection_matrix_; }

    uint64_t frame_number() const { return frame_number_.load(); }
};
