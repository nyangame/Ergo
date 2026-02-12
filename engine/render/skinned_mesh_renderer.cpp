#include "skinned_mesh_renderer.hpp"

// ---- Mesh ----

void SkinnedMeshRenderer::set_mesh(SkinnedMeshData mesh) {
    mesh_ = std::move(mesh);
}

// ---- Skeleton ----

void SkinnedMeshRenderer::set_skeleton(Skeleton skeleton) {
    skeleton_ = std::move(skeleton);
    player_.set_skeleton(&skeleton_);
}

// ---- Animation control ----

void SkinnedMeshRenderer::add_clip(const AnimationClip& clip) {
    player_.add_clip(clip);
}

void SkinnedMeshRenderer::play(std::string_view clip_name, bool loop) {
    player_.play(clip_name, loop);
}

void SkinnedMeshRenderer::stop() {
    player_.stop();
}

void SkinnedMeshRenderer::pause() {
    player_.pause();
}

void SkinnedMeshRenderer::resume() {
    player_.resume();
}

// ---- Per-frame update ----

void SkinnedMeshRenderer::update(float dt) {
    player_.update(dt);
}

// ---- Render command recording ----

void SkinnedMeshRenderer::record_commands(CommandBuffer& out) const {
    if (mesh_.vertices.empty()) return;

    RenderCmd_DrawSkinnedMesh cmd;
    cmd.mesh_id = mesh_.id;
    cmd.world_transform = transform_.to_mat4();
    cmd.material_id = material_id_;

    const auto& matrices = player_.bone_matrices();
    cmd.bone_count = static_cast<uint32_t>(matrices.size());
    cmd.bone_matrices = matrices;

    out.push(std::move(cmd));
}
