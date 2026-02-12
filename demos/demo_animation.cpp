#include "demo_framework.hpp"
#include "engine/animation/animation_clip.hpp"
#include "engine/animation/skeleton.hpp"
#include <cstdio>

DEMO(Animation_Skeleton) {
    Skeleton skel;

    Bone root_bone;
    root_bone.name = "root";
    root_bone.parent_index = -1;
    skel.bones.push_back(root_bone);

    Bone spine;
    spine.name = "spine";
    spine.parent_index = 0;
    skel.bones.push_back(spine);

    Bone head;
    head.name = "head";
    head.parent_index = 1;
    skel.bones.push_back(head);

    Bone arm_l;
    arm_l.name = "arm_left";
    arm_l.parent_index = 1;
    skel.bones.push_back(arm_l);

    std::printf("  Skeleton bones: %zu\n", skel.bone_count());
    for (size_t i = 0; i < skel.bones.size(); ++i) {
        auto& b = skel.bones[i];
        std::printf("    [%zu] '%s' parent=%d\n",
                    i, b.name.c_str(), b.parent_index);
    }

    int idx = skel.find_bone("head");
    std::printf("  find_bone('head') = %d\n", idx);
    idx = skel.find_bone("missing");
    std::printf("  find_bone('missing') = %d\n", idx);
}

DEMO(Animation_Clip) {
    AnimationClip clip;
    clip.name = "walk";
    clip.duration = 1.0f;
    clip.loop = true;

    BoneChannel ch;
    ch.bone_index = 1;  // spine
    ch.keyframes.push_back({0.0f, {0, 0, 0}, Quat::identity(), {1, 1, 1}});
    ch.keyframes.push_back({0.5f, {0, 0.1f, 0}, Quat::from_axis_angle(Vec3f{0, 0, 1}, 0.1f), {1, 1, 1}});
    ch.keyframes.push_back({1.0f, {0, 0, 0}, Quat::identity(), {1, 1, 1}});
    clip.channels.push_back(ch);

    std::printf("  Clip: '%s' duration=%.1f loop=%s channels=%zu\n",
                clip.name.c_str(), clip.duration,
                clip.loop ? "yes" : "no", clip.channels.size());

    // Find keyframes at different times
    for (float t : {0.0f, 0.25f, 0.5f, 0.75f, 1.0f}) {
        auto [a, b] = ch.find_keyframes(t);
        std::printf("    t=%.2f -> keyframes[%zu, %zu]\n", t, a, b);
    }
}
