#include "task_system.hpp"
#include <algorithm>

void TaskManager::destroy(TaskHandle handle) {
    for (auto& layer : layers_) {
        for (auto& entry : layer) {
            if (entry.id == handle.id) {
                entry.pending_destroy = true;
                return;
            }
        }
    }
}

void TaskManager::run(RunPhase phase, float dt, RenderContext* ctx) {
    for (auto& layer : layers_) {
        switch (phase) {
            case RunPhase::Start:
                for (auto& entry : layer) {
                    if (!entry.initialized && !entry.pending_destroy) {
                        entry.impl->start();
                        entry.initialized = true;
                    }
                }
                break;

            case RunPhase::Update:
                // Initialize any new tasks first
                for (auto& entry : layer) {
                    if (!entry.initialized && !entry.pending_destroy) {
                        entry.impl->start();
                        entry.initialized = true;
                    }
                }
                for (auto& entry : layer) {
                    if (entry.initialized && !entry.pending_destroy) {
                        entry.impl->update(dt);
                    }
                }
                break;

            case RunPhase::Physics:
                for (auto& entry : layer) {
                    if (entry.initialized && !entry.pending_destroy) {
                        entry.impl->physics(dt);
                    }
                }
                break;

            case RunPhase::Draw:
                if (ctx) {
                    for (auto& entry : layer) {
                        if (entry.initialized && !entry.pending_destroy) {
                            entry.impl->draw(*ctx);
                        }
                    }
                }
                break;

            case RunPhase::Destroy:
                // Release and remove destroyed tasks
                for (auto& entry : layer) {
                    if (entry.pending_destroy && entry.initialized) {
                        entry.impl->release();
                    }
                }
                layer.erase(
                    std::remove_if(layer.begin(), layer.end(),
                        [](const TaskEntry& e) { return e.pending_destroy; }),
                    layer.end()
                );
                break;
        }
    }
}
