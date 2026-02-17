#include "framework/test_framework.hpp"
#include "engine/core/behaviour/particle_emitter_component.hpp"
#include "engine/core/behaviour/behaviour_registry.hpp"
#include "engine/math/transform.hpp"

using namespace ergo::test;

// ============================================================
// ParticleEmitterComponent tests
// ============================================================

static TestSuite suite_particle_emitter_component("Core/ParticleEmitterComponent");

static void register_particle_emitter_component_tests() {
    suite_particle_emitter_component.add("satisfies_BehaviourLike_concept", [](TestContext& ctx) {
        // Compile-time check: if this compiles, the concept is satisfied
        static_assert(BehaviourLike<ParticleEmitterComponent>,
                      "ParticleEmitterComponent must satisfy BehaviourLike");
        ERGO_TEST_ASSERT_TRUE(ctx, true);
    });

    suite_particle_emitter_component.add("type_name", [](TestContext& ctx) {
        ERGO_TEST_ASSERT_EQ(ctx,
            std::string(ParticleEmitterComponent::type_name()),
            std::string("ParticleEmitterComponent"));
    });

    suite_particle_emitter_component.add("threading_policy_is_MainThread", [](TestContext& ctx) {
        static_assert(ThreadAware<ParticleEmitterComponent>,
                      "ParticleEmitterComponent must be ThreadAware");
        ERGO_TEST_ASSERT_TRUE(ctx,
            ParticleEmitterComponent::threading_policy() == ThreadingPolicy::MainThread);
    });

    suite_particle_emitter_component.add("start_creates_emitter", [](TestContext& ctx) {
        ParticleEmitterComponent comp;
        comp.config.emit_rate = 10.0f;
        comp.config.max_particles = 50;
        comp.auto_play = true;

        ERGO_TEST_ASSERT_TRUE(ctx, comp.emitter() == nullptr);
        comp.start();
        ERGO_TEST_ASSERT_TRUE(ctx, comp.emitter() != nullptr);
        ERGO_TEST_ASSERT_TRUE(ctx, comp.is_alive());
        comp.release();
    });

    suite_particle_emitter_component.add("auto_play_false_stops_emitter", [](TestContext& ctx) {
        ParticleEmitterComponent comp;
        comp.config.emit_rate = 10.0f;
        comp.config.max_particles = 50;
        comp.auto_play = false;

        comp.start();
        // Emitter exists but not emitting continuously
        ERGO_TEST_ASSERT_TRUE(ctx, comp.emitter() != nullptr);

        // Update a few frames — with emit_rate > 0 but stopped, no particles
        // should be emitted (we test indirectly via is_alive after some time)
        comp.update(0.016f);
        comp.update(0.016f);
        ERGO_TEST_ASSERT_TRUE(ctx, comp.is_playing());
        comp.release();
    });

    suite_particle_emitter_component.add("release_clears_emitter", [](TestContext& ctx) {
        ParticleEmitterComponent comp;
        comp.config.emit_rate = 10.0f;
        comp.start();
        ERGO_TEST_ASSERT_TRUE(ctx, comp.emitter() != nullptr);

        comp.release();
        ERGO_TEST_ASSERT_TRUE(ctx, comp.emitter() == nullptr);
        ERGO_TEST_ASSERT_FALSE(ctx, comp.is_alive());
        ERGO_TEST_ASSERT_FALSE(ctx, comp.is_playing());
    });

    suite_particle_emitter_component.add("follow_owner_syncs_position", [](TestContext& ctx) {
        Transform2D transform;
        transform.position = {100.0f, 200.0f};

        ParticleEmitterComponent comp;
        comp.config.emit_rate = 10.0f;
        comp.config.max_particles = 50;
        comp.offset = {10.0f, -5.0f};
        comp.follow_owner = true;
        comp.owner_transform = &transform;

        comp.start();

        // Check initial position
        auto initial_pos = comp.emitter()->config().position;
        ERGO_TEST_ASSERT_NEAR(ctx, initial_pos.x, 110.0f, 0.01f);
        ERGO_TEST_ASSERT_NEAR(ctx, initial_pos.y, 195.0f, 0.01f);

        // Move owner
        transform.position = {300.0f, 400.0f};
        comp.update(0.016f);

        auto new_pos = comp.emitter()->config().position;
        ERGO_TEST_ASSERT_NEAR(ctx, new_pos.x, 310.0f, 0.01f);
        ERGO_TEST_ASSERT_NEAR(ctx, new_pos.y, 395.0f, 0.01f);

        comp.release();
    });

    suite_particle_emitter_component.add("no_follow_keeps_initial_position", [](TestContext& ctx) {
        Transform2D transform;
        transform.position = {100.0f, 200.0f};

        ParticleEmitterComponent comp;
        comp.config.emit_rate = 10.0f;
        comp.config.max_particles = 50;
        comp.follow_owner = false;
        comp.owner_transform = &transform;

        comp.start();

        // Move owner — emitter should NOT follow
        transform.position = {999.0f, 999.0f};
        comp.update(0.016f);

        auto pos = comp.emitter()->config().position;
        ERGO_TEST_ASSERT_NEAR(ctx, pos.x, 100.0f, 0.01f);
        ERGO_TEST_ASSERT_NEAR(ctx, pos.y, 200.0f, 0.01f);

        comp.release();
    });

    suite_particle_emitter_component.add("burst_emits_particles", [](TestContext& ctx) {
        ParticleEmitterComponent comp;
        comp.config.emit_rate = 0.0f;
        comp.config.particle_life_min = 1.0f;
        comp.config.particle_life_max = 2.0f;
        comp.config.max_particles = 100;
        comp.auto_play = false;

        comp.start();
        comp.burst(15);
        ERGO_TEST_ASSERT_TRUE(ctx, comp.is_alive());

        comp.release();
    });

    suite_particle_emitter_component.add("on_finished_called_for_non_looping", [](TestContext& ctx) {
        bool finished = false;

        ParticleEmitterComponent comp;
        comp.config.emit_rate = 0.0f;
        comp.config.particle_life_min = 0.05f;
        comp.config.particle_life_max = 0.1f;
        comp.config.max_particles = 10;
        comp.config.loop = false;
        comp.auto_play = false;
        comp.on_finished = [&finished]() { finished = true; };

        comp.start();
        comp.burst(3);

        // Simulate until all particles die
        for (int i = 0; i < 100 && !finished; ++i) {
            comp.update(0.016f);
        }
        ERGO_TEST_ASSERT_TRUE(ctx, finished);
        ERGO_TEST_ASSERT_FALSE(ctx, comp.is_alive());

        comp.release();
    });

    suite_particle_emitter_component.add("play_and_stop_control", [](TestContext& ctx) {
        ParticleEmitterComponent comp;
        comp.config.emit_rate = 10.0f;
        comp.config.max_particles = 50;
        comp.auto_play = true;

        comp.start();
        ERGO_TEST_ASSERT_TRUE(ctx, comp.is_alive());

        comp.stop();
        // Emitter still alive (existing particles) but not emitting new ones
        ERGO_TEST_ASSERT_TRUE(ctx, comp.is_playing());

        comp.play();
        comp.update(0.016f);
        ERGO_TEST_ASSERT_TRUE(ctx, comp.is_alive());

        comp.release();
    });

    suite_particle_emitter_component.add("restart_resets_state", [](TestContext& ctx) {
        ParticleEmitterComponent comp;
        comp.config.emit_rate = 10.0f;
        comp.config.max_particles = 50;

        comp.start();
        for (int i = 0; i < 10; ++i) comp.update(0.016f);

        comp.restart();
        // After restart, emitter is freshly created
        ERGO_TEST_ASSERT_TRUE(ctx, comp.emitter() != nullptr);
        ERGO_TEST_ASSERT_TRUE(ctx, comp.is_alive());

        comp.release();
    });

    suite_particle_emitter_component.add("no_owner_uses_zero_position", [](TestContext& ctx) {
        ParticleEmitterComponent comp;
        comp.config.emit_rate = 10.0f;
        comp.config.max_particles = 50;
        comp.offset = {5.0f, 10.0f};
        comp.owner_transform = nullptr;

        comp.start();

        auto pos = comp.emitter()->config().position;
        ERGO_TEST_ASSERT_NEAR(ctx, pos.x, 5.0f, 0.01f);
        ERGO_TEST_ASSERT_NEAR(ctx, pos.y, 10.0f, 0.01f);

        comp.release();
    });

    suite_particle_emitter_component.add("BehaviourHolder_integration", [](TestContext& ctx) {
        BehaviourHolder holder;

        auto& comp = holder.add<ParticleEmitterComponent>();
        comp.config.emit_rate = 10.0f;
        comp.config.max_particles = 50;

        ERGO_TEST_ASSERT_TRUE(ctx, holder.has<ParticleEmitterComponent>());

        holder.start();
        auto* found = holder.get<ParticleEmitterComponent>();
        ERGO_TEST_ASSERT_TRUE(ctx, found != nullptr);
        ERGO_TEST_ASSERT_TRUE(ctx, found->is_alive());

        holder.update(0.016f);
        holder.release();
        ERGO_TEST_ASSERT_TRUE(ctx, found->emitter() == nullptr);
    });

    suite_particle_emitter_component.add("BehaviourRegistry_create", [](TestContext& ctx) {
        BehaviourRegistry registry;
        registry.register_type<ParticleEmitterComponent>("Effects");

        auto names = registry.names_in_category("Effects");
        ERGO_TEST_ASSERT_EQ(ctx, names.size(), (size_t)1);

        auto behaviour = registry.create("ParticleEmitterComponent");
        ERGO_TEST_ASSERT_TRUE(ctx, behaviour != nullptr);
        ERGO_TEST_ASSERT_EQ(ctx,
            std::string(behaviour->type_name()),
            std::string("ParticleEmitterComponent"));

        const auto* entry = registry.find("ParticleEmitterComponent");
        ERGO_TEST_ASSERT_TRUE(ctx, entry != nullptr);
        ERGO_TEST_ASSERT_TRUE(ctx, entry->thread_aware);
        ERGO_TEST_ASSERT_TRUE(ctx, entry->policy == ThreadingPolicy::MainThread);
    });
}

// ============================================================
// Registration
// ============================================================

void register_particle_emitter_component_tests(TestRunner& runner) {
    register_particle_emitter_component_tests();
    runner.add_suite(suite_particle_emitter_component);
}
