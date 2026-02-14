#pragma once
#include "skinned_mesh.hpp"
#include "command_buffer.hpp"
#include "render_pipeline.hpp"
#include "../animation/skeleton.hpp"
#include "../animation/animation_clip.hpp"
#include "../animation/animation_player.hpp"
#include "../math/transform3d.hpp"
#include "../shader/skinned_mesh_shader.hpp"
#include <string>
#include <string_view>
#include <vector>

// ============================================================
// SkinnedMeshRenderer
//
// Rendering component for skeletal-animated meshes.
// Owns a SkinnedMeshData, a Skeleton, and an AnimationPlayer.
//
// Satisfies:
//   Updatable   — update(dt) advances the animation
//   CommandSubmittable — record_commands(CommandBuffer&) emits draw commands
//
// Typical usage:
//   SkinnedMeshRenderer renderer;
//   renderer.set_mesh(mesh_data);
//   renderer.set_skeleton(skeleton);
//   renderer.add_clip(walk_clip);
//   renderer.add_clip(run_clip);
//   renderer.play("walk");
//
//   // per frame
//   renderer.update(dt);
//   renderer.record_commands(cmd_buf);
// ============================================================

class SkinnedMeshRenderer {
public:
    SkinnedMeshRenderer() = default;

    // ---- Mesh data ----

    void set_mesh(SkinnedMeshData mesh);
    const SkinnedMeshData& mesh() const { return mesh_; }
    SkinnedMeshData& mesh() { return mesh_; }

    // ---- Skeleton ----

    void set_skeleton(Skeleton skeleton);
    const Skeleton& skeleton() const { return skeleton_; }

    // ---- Material ----

    void set_material_id(uint64_t id) { material_id_ = id; }
    uint64_t material_id() const { return material_id_; }

    // ---- Transform ----

    void set_transform(const Transform3D& t) { transform_ = t; }
    Transform3D& transform() { return transform_; }
    const Transform3D& transform() const { return transform_; }

    // ---- Animation control (delegates to AnimationPlayer) ----

    void add_clip(const AnimationClip& clip);
    void play(std::string_view clip_name, bool loop = true);
    void stop();
    void pause();
    void resume();

    float playback_speed() const { return player_.playback_speed; }
    void set_playback_speed(float speed) { player_.playback_speed = speed; }

    float blend_factor() const { return player_.blend_factor; }
    void set_blend_factor(float factor) { player_.blend_factor = factor; }

    bool is_playing() const { return player_.is_playing(); }
    float current_time() const { return player_.current_time(); }
    std::string_view current_clip_name() const { return player_.current_clip_name(); }

    // ---- Per-frame update (Updatable) ----

    void update(float dt);

    // ---- Render command recording (CommandSubmittable) ----

    void record_commands(CommandBuffer& out) const;

    // ---- Shader source access ----

    const SkinnedMeshShader& shader() const { return shader_; }
    std::string vertex_shader_source() const { return shader_.generate_vertex(); }
    std::string fragment_shader_source() const { return shader_.generate_fragment(); }

    // ---- Read-only access to current bone matrices ----

    const std::vector<Mat4>& bone_matrices() const { return player_.bone_matrices(); }

private:
    SkinnedMeshData mesh_;
    Skeleton skeleton_;
    AnimationPlayer player_;
    Transform3D transform_;
    uint64_t material_id_ = 0;
    SkinnedMeshShader shader_;
};
