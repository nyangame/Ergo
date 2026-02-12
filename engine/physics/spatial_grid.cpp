#include "spatial_grid.hpp"
#include <cmath>
#include <algorithm>

SpatialGrid2D::SpatialGrid2D(float cell_size)
    : cell_size_(cell_size), inv_cell_size_(1.0f / cell_size) {}

void SpatialGrid2D::clear() {
    cells_.clear();
}

SpatialGrid2D::CellKey SpatialGrid2D::to_cell(float x, float y) const {
    return {
        static_cast<int32_t>(std::floor(x * inv_cell_size_)),
        static_cast<int32_t>(std::floor(y * inv_cell_size_))
    };
}

void SpatialGrid2D::insert(Collider* c) {
    if (!c || !c->transform) return;

    Vec2f pos = c->transform->position;
    float extent = 0.0f;

    if (auto* aabb = std::get_if<AABBData>(&c->shape)) {
        extent = std::max(aabb->half_extent.x, aabb->half_extent.y);
    } else if (auto* circle = std::get_if<CircleData>(&c->shape)) {
        extent = circle->radius;
    }

    CellKey min_cell = to_cell(pos.x - extent, pos.y - extent);
    CellKey max_cell = to_cell(pos.x + extent, pos.y + extent);

    for (int32_t cy = min_cell.y; cy <= max_cell.y; ++cy) {
        for (int32_t cx = min_cell.x; cx <= max_cell.x; ++cx) {
            cells_[{cx, cy}].push_back(c);
        }
    }
}

std::vector<Collider*> SpatialGrid2D::query(Vec2f min, Vec2f max) const {
    std::vector<Collider*> result;
    CellKey min_cell = to_cell(min.x, min.y);
    CellKey max_cell = to_cell(max.x, max.y);

    for (int32_t cy = min_cell.y; cy <= max_cell.y; ++cy) {
        for (int32_t cx = min_cell.x; cx <= max_cell.x; ++cx) {
            auto it = cells_.find({cx, cy});
            if (it != cells_.end()) {
                for (auto* c : it->second) {
                    // Avoid duplicates
                    if (std::find(result.begin(), result.end(), c) == result.end()) {
                        result.push_back(c);
                    }
                }
            }
        }
    }

    return result;
}

std::vector<Collider*> SpatialGrid2D::query_radius(Vec2f center, float radius) const {
    Vec2f min = {center.x - radius, center.y - radius};
    Vec2f max = {center.x + radius, center.y + radius};
    auto candidates = query(min, max);

    // Filter by actual distance
    float r2 = radius * radius;
    candidates.erase(
        std::remove_if(candidates.begin(), candidates.end(),
            [&](const Collider* c) {
                if (!c->transform) return true;
                Vec2f d = c->transform->position - center;
                return d.length_sq() > r2;
            }),
        candidates.end());

    return candidates;
}
