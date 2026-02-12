#include "test_framework.hpp"
#include "engine/animation/animation_clip.hpp"
#include "engine/animation/skeleton.hpp"

TEST_CASE(Skeleton_FindBone) {
    Skeleton skel;
    Bone root;
    root.name = "root";
    root.parent_index = -1;
    skel.bones.push_back(root);

    Bone spine;
    spine.name = "spine";
    spine.parent_index = 0;
    skel.bones.push_back(spine);

    Bone head;
    head.name = "head";
    head.parent_index = 1;
    skel.bones.push_back(head);

    ASSERT_EQ(skel.find_bone("root"), 0);
    ASSERT_EQ(skel.find_bone("spine"), 1);
    ASSERT_EQ(skel.find_bone("head"), 2);
    ASSERT_EQ(skel.find_bone("missing"), -1);
}

TEST_CASE(Skeleton_BoneCount) {
    Skeleton skel;
    ASSERT_EQ(skel.bone_count(), (size_t)0);

    Bone b; b.name = "root"; b.parent_index = -1;
    skel.bones.push_back(b);
    ASSERT_EQ(skel.bone_count(), (size_t)1);
}

TEST_CASE(AnimationClip_Properties) {
    AnimationClip clip;
    clip.name = "idle";
    clip.duration = 2.0f;
    clip.loop = true;

    ASSERT_TRUE(clip.name == "idle");
    ASSERT_NEAR(clip.duration, 2.0f, 0.001f);
    ASSERT_TRUE(clip.loop);
}

TEST_CASE(BoneChannel_FindKeyframes_Single) {
    BoneChannel ch;
    ch.bone_index = 0;
    ch.keyframes.push_back({0.0f, {0, 0, 0}, Quat::identity(), {1, 1, 1}});

    auto [a, b] = ch.find_keyframes(0.0f);
    ASSERT_EQ(a, (size_t)0);
    ASSERT_EQ(b, (size_t)0);
}

TEST_CASE(BoneChannel_FindKeyframes_Between) {
    BoneChannel ch;
    ch.bone_index = 1;
    ch.keyframes.push_back({0.0f, {0, 0, 0}, Quat::identity(), {1, 1, 1}});
    ch.keyframes.push_back({1.0f, {10, 0, 0}, Quat::identity(), {1, 1, 1}});

    auto [a, b] = ch.find_keyframes(0.5f);
    ASSERT_EQ(a, (size_t)0);
    ASSERT_EQ(b, (size_t)1);
}

TEST_CASE(BoneChannel_FindKeyframes_BeyondEnd) {
    BoneChannel ch;
    ch.bone_index = 2;
    ch.keyframes.push_back({0.0f, {0, 0, 0}, Quat::identity(), {1, 1, 1}});
    ch.keyframes.push_back({1.0f, {10, 0, 0}, Quat::identity(), {2, 2, 2}});

    auto [a, b] = ch.find_keyframes(2.0f);
    // Beyond end should return last keyframe
    ASSERT_EQ(a, (size_t)1);
    ASSERT_EQ(b, (size_t)1);
}

TEST_CASE(BoneChannel_KeyframeData) {
    BoneChannel ch;
    ch.bone_index = 0;
    ch.keyframes.push_back({0.5f, {1, 2, 3}, Quat::identity(), {1, 1, 1}});

    ASSERT_NEAR(ch.keyframes[0].time, 0.5f, 0.001f);
    ASSERT_NEAR(ch.keyframes[0].position.x, 1.0f, 0.001f);
    ASSERT_NEAR(ch.keyframes[0].position.y, 2.0f, 0.001f);
    ASSERT_NEAR(ch.keyframes[0].position.z, 3.0f, 0.001f);
    ASSERT_NEAR(ch.keyframes[0].rotation.w, 1.0f, 0.001f);
}
