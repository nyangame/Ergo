#include "render_pipeline.hpp"
#include <algorithm>

RenderPipeline::RenderPipeline() = default;

RenderPipeline::~RenderPipeline() {
    shutdown();
}

void RenderPipeline::initialize(uint32_t worker_count) {
    if (worker_count == 0) {
        worker_count = std::max(1u, std::thread::hardware_concurrency() - 1);
    }

    shutdown_.store(false);
    workers_.reserve(worker_count);
    for (uint32_t i = 0; i < worker_count; ++i) {
        workers_.emplace_back(&RenderPipeline::worker_thread_func, this);
    }
}

void RenderPipeline::shutdown() {
    shutdown_.store(true);
    job_cv_.notify_all();
    for (auto& w : workers_) {
        if (w.joinable()) w.join();
    }
    workers_.clear();
}

void RenderPipeline::worker_thread_func() {
    while (!shutdown_.load(std::memory_order_acquire)) {
        RenderJob job;
        {
            std::unique_lock lock(job_mutex_);
            job_cv_.wait(lock, [this] {
                return !job_queue_.empty() || shutdown_.load(std::memory_order_acquire);
            });

            if (shutdown_.load(std::memory_order_acquire) && job_queue_.empty()) return;
            if (job_queue_.empty()) continue;

            job = std::move(job_queue_.back());
            job_queue_.pop_back();
        }

        // Execute job with a thread-local command buffer
        CommandBuffer local_buffer;
        if (job.execute) {
            job.execute(local_buffer, job.begin, job.end);
        }

        // Submit results to the Opaque stage collector by default
        stages_[static_cast<size_t>(Stage::Opaque)].collector.submit(local_buffer);

        if (jobs_remaining_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            jobs_done_cv_.notify_all();
        }
    }
}

void RenderPipeline::begin_frame() {
    frame_number_.fetch_add(1, std::memory_order_relaxed);

    // Swap all stage buffers: previous back -> new front (for render), clear new back
    for (auto& stage : stages_) {
        stage.commands.swap();
    }
}

void RenderPipeline::end_frame() {
    // Collect all submitted commands from workers into stage back buffers
    for (auto& stage : stages_) {
        auto collected = stage.collector.take();
        if (!collected.empty()) {
            stage.commands.write_buffer().merge(collected);
        }
    }
}

void RenderPipeline::submit(Stage stage, const CommandBuffer& buffer) {
    auto idx = static_cast<size_t>(stage);
    stages_[idx].collector.submit(buffer);
}

const CommandBuffer& RenderPipeline::stage_commands(Stage stage) const {
    return stages_[static_cast<size_t>(stage)].commands.read_buffer();
}

void RenderPipeline::dispatch_jobs(const std::vector<RenderJob>& jobs) {
    if (jobs.empty()) return;

    jobs_remaining_.store(static_cast<uint32_t>(jobs.size()), std::memory_order_release);

    {
        std::lock_guard lock(job_mutex_);
        for (auto& job : jobs) {
            job_queue_.push_back(job);
        }
    }
    job_cv_.notify_all();
}

void RenderPipeline::wait_for_jobs() {
    std::mutex wait_mutex;
    std::unique_lock lock(wait_mutex);
    jobs_done_cv_.wait(lock, [this] {
        return jobs_remaining_.load(std::memory_order_acquire) == 0;
    });
}

uint64_t RenderPipeline::register_mesh(MeshData mesh) {
    std::lock_guard lock(resource_mutex_);
    uint64_t id = next_mesh_id_++;
    mesh.id = id;
    meshes_[id] = std::move(mesh);
    return id;
}

uint64_t RenderPipeline::register_material(MaterialData material) {
    std::lock_guard lock(resource_mutex_);
    uint64_t id = next_material_id_++;
    material.id = id;
    materials_[id] = std::move(material);
    return id;
}

void RenderPipeline::unregister_mesh(uint64_t id) {
    std::lock_guard lock(resource_mutex_);
    meshes_.erase(id);
}

void RenderPipeline::unregister_material(uint64_t id) {
    std::lock_guard lock(resource_mutex_);
    materials_.erase(id);
}

MeshData* RenderPipeline::get_mesh(uint64_t id) {
    std::lock_guard lock(resource_mutex_);
    auto it = meshes_.find(id);
    return (it != meshes_.end()) ? &it->second : nullptr;
}

MaterialData* RenderPipeline::get_material(uint64_t id) {
    std::lock_guard lock(resource_mutex_);
    auto it = materials_.find(id);
    return (it != materials_.end()) ? &it->second : nullptr;
}
