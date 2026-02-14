#pragma once
#include "../math/vec2.hpp"
#include "collider.hpp"
#include <unordered_map>
#include <vector>
#include <cstdint>

class SpatialGrid2D {
public:
    explicit SpatialGrid2D(float cell_size = 64.0f);

    void clear();
    void insert(Collider* c);
    std::vector<Collider*> query(Vec2f min, Vec2f max) const;
    std::vector<Collider*> query_radius(Vec2f center, float radius) const;

    float cell_size() const { return cell_size_; }

private:
    float cell_size_;
    float inv_cell_size_;

    struct CellKey {
        int32_t x, y;
        bool operator==(const CellKey& o) const { return x == o.x && y == o.y; }
    };

    struct CellKeyHash {
        size_t operator()(const CellKey& k) const {
            return std::hash<int64_t>()(
                (static_cast<int64_t>(k.x) << 32) | static_cast<uint32_t>(k.y));
        }
    };

    std::unordered_map<CellKey, std::vector<Collider*>, CellKeyHash> cells_;

    CellKey to_cell(float x, float y) const;
};
