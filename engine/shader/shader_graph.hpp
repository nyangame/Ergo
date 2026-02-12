#pragma once
#include "shader_node.hpp"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <string>
#include <cassert>

// ============================================================
// ShaderGraph: a directed acyclic graph of shader nodes
// ============================================================

class ShaderGraph {
    std::unordered_map<uint32_t, ShaderNode> nodes_;
    std::vector<ShaderConnection> connections_;
    uint32_t next_node_id_ = 1;
    uint64_t next_conn_id_ = 1;
    std::string name_ = "Untitled";

public:
    ShaderGraph() = default;
    explicit ShaderGraph(const std::string& name) : name_(name) {}

    const std::string& name() const { return name_; }
    void set_name(const std::string& name) { name_ = name; }

    // --- Satisfies ShaderGraphLike concept ---
    size_t node_count() const { return nodes_.size(); }
    size_t connection_count() const { return connections_.size(); }

    // --- Node management ---

    uint32_t add_node(ShaderNode node) {
        uint32_t id = next_node_id_++;
        node.id = id;
        nodes_.emplace(id, std::move(node));
        return id;
    }

    void remove_node(uint32_t id) {
        nodes_.erase(id);
        // Remove all connections referencing this node
        connections_.erase(
            std::remove_if(connections_.begin(), connections_.end(),
                [id](const ShaderConnection& c) {
                    return c.source_node == id || c.target_node == id;
                }),
            connections_.end());
    }

    ShaderNode* get_node(uint32_t id) {
        auto it = nodes_.find(id);
        return (it != nodes_.end()) ? &it->second : nullptr;
    }

    const ShaderNode* get_node(uint32_t id) const {
        auto it = nodes_.find(id);
        return (it != nodes_.end()) ? &it->second : nullptr;
    }

    const std::unordered_map<uint32_t, ShaderNode>& nodes() const { return nodes_; }

    // --- Connection management ---

    uint64_t connect(uint32_t src_node, uint32_t src_port,
                     uint32_t dst_node, uint32_t dst_port) {
        // Validate nodes exist
        if (!get_node(src_node) || !get_node(dst_node)) return 0;

        // Prevent duplicate connections to the same input port
        for (auto& c : connections_) {
            if (c.target_node == dst_node && c.target_port == dst_port) {
                // Replace existing connection
                c.source_node = src_node;
                c.source_port = src_port;
                return c.id;
            }
        }

        uint64_t id = next_conn_id_++;
        connections_.push_back({id, src_node, src_port, dst_node, dst_port});
        return id;
    }

    void disconnect(uint64_t conn_id) {
        connections_.erase(
            std::remove_if(connections_.begin(), connections_.end(),
                [conn_id](const ShaderConnection& c) { return c.id == conn_id; }),
            connections_.end());
    }

    const std::vector<ShaderConnection>& connections() const { return connections_; }

    // --- Query helpers ---

    // Find the output node (NodeOutput type)
    uint32_t find_output_node() const {
        for (auto& [id, node] : nodes_) {
            if (std::holds_alternative<NodeOutput>(node.data)) {
                return id;
            }
        }
        return 0;
    }

    // Find all connections feeding into a given node's input port
    const ShaderConnection* find_input_connection(uint32_t node_id,
                                                  uint32_t port_index) const {
        for (auto& c : connections_) {
            if (c.target_node == node_id && c.target_port == port_index) {
                return &c;
            }
        }
        return nullptr;
    }

    // Find all connections from a given node's output port
    std::vector<const ShaderConnection*> find_output_connections(uint32_t node_id,
                                                                 uint32_t port_index) const {
        std::vector<const ShaderConnection*> result;
        for (auto& c : connections_) {
            if (c.source_node == node_id && c.source_port == port_index) {
                result.push_back(&c);
            }
        }
        return result;
    }

    // Check if a node's output is used by any connection
    bool is_node_connected(uint32_t node_id) const {
        for (auto& c : connections_) {
            if (c.source_node == node_id || c.target_node == node_id) return true;
        }
        return false;
    }

    // --- Topological sort ---
    // Returns node IDs in evaluation order (dependencies first)

    std::vector<uint32_t> topological_sort() const {
        std::unordered_map<uint32_t, int> in_degree;
        std::unordered_map<uint32_t, std::vector<uint32_t>> adjacency;

        for (auto& [id, _] : nodes_) {
            in_degree[id] = 0;
        }

        for (auto& conn : connections_) {
            adjacency[conn.source_node].push_back(conn.target_node);
            in_degree[conn.target_node]++;
        }

        std::vector<uint32_t> queue;
        for (auto& [id, deg] : in_degree) {
            if (deg == 0) queue.push_back(id);
        }

        std::vector<uint32_t> sorted;
        sorted.reserve(nodes_.size());

        while (!queue.empty()) {
            uint32_t current = queue.back();
            queue.pop_back();
            sorted.push_back(current);

            if (adjacency.count(current)) {
                for (uint32_t next : adjacency[current]) {
                    if (--in_degree[next] == 0) {
                        queue.push_back(next);
                    }
                }
            }
        }

        return sorted;
    }

    // --- Validation ---

    bool validate() const {
        // Must have exactly one output node
        uint32_t output_count = 0;
        for (auto& [id, node] : nodes_) {
            if (std::holds_alternative<NodeOutput>(node.data)) output_count++;
        }
        if (output_count != 1) return false;

        // Cycle detection: topological sort must include all nodes
        auto sorted = topological_sort();
        if (sorted.size() != nodes_.size()) return false;

        // Validate all connections reference valid nodes and ports
        for (auto& conn : connections_) {
            auto* src = get_node(conn.source_node);
            auto* dst = get_node(conn.target_node);
            if (!src || !dst) return false;
            if (conn.source_port >= src->outputs.size()) return false;
            if (conn.target_port >= dst->inputs.size()) return false;
        }

        return true;
    }

    // --- Type compatibility check ---

    static bool types_compatible(ShaderDataType from, ShaderDataType to) {
        if (from == to) return true;
        // Float can promote to Vec2/Vec3/Vec4 (broadcast)
        if (from == ShaderDataType::Float &&
            (to == ShaderDataType::Vec2 || to == ShaderDataType::Vec3 ||
             to == ShaderDataType::Vec4)) return true;
        // Vec3 -> Vec4 (with w=1) and Vec4 -> Vec3 (truncate)
        if (from == ShaderDataType::Vec3 && to == ShaderDataType::Vec4) return true;
        if (from == ShaderDataType::Vec4 && to == ShaderDataType::Vec3) return true;
        return false;
    }

    // Collect all unique uniform names required by this graph
    std::vector<std::pair<std::string, ShaderDataType>> collect_uniforms() const {
        std::vector<std::pair<std::string, ShaderDataType>> uniforms;
        for (auto& [id, node] : nodes_) {
            std::visit([&](auto& data) {
                using T = std::decay_t<decltype(data)>;
                if constexpr (std::is_same_v<T, NodePropertyFloat>) {
                    uniforms.push_back({data.uniform_name, ShaderDataType::Float});
                } else if constexpr (std::is_same_v<T, NodePropertyVec4>) {
                    uniforms.push_back({data.uniform_name, ShaderDataType::Vec4});
                } else if constexpr (std::is_same_v<T, NodeTextureSample>) {
                    uniforms.push_back({data.texture_uniform, ShaderDataType::Texture2D});
                }
            }, node.data);
        }
        return uniforms;
    }
};
