# Test Coverage Analysis

## Current State

The Ergo engine has **9 test modules** with approximately **150+ test cases** using a custom lightweight C++ test framework (`tests/framework/test_framework.hpp`) integrated with CTest.

### Modules with existing tests

| Module | Test File | Suites | Cases | Coverage Quality |
|--------|-----------|--------|-------|-----------------|
| Math (Vec2f, Vec3f, Mat4, Quat) | `test_math.cpp` | 5 | ~46 | Thorough |
| Physics 2D (AABB, Circle, hit_test) | `test_physics.cpp` | 4 | ~20 | Good |
| Physics 3D (Collision3D, RigidBody, SpatialGrid) | `test_physics_extended.cpp` | 3 | ~19 | Good |
| Core (StateMachine, GameObject, IdGenerator) | `test_core.cpp` | 3 | ~17 | Moderate |
| Gameplay (Camera, Easing, Animation, InputMap, Time) | `test_gameplay.cpp` | 5 | ~35 | Thorough |
| Render (CommandBuffer, PostProcess, Light) | `test_render.cpp` | 3 | ~19 | Moderate |
| UI / Shader (UIElement, ShaderGraph) | `test_ui_shader.cpp` | 2 | ~15 | Good |
| Animation / Debug (AnimationClip, Profiler, Serialization) | `test_animation_debug.cpp` | 3 | ~20 | Good |
| ECS / Task (World, TaskSystem) | `test_ecs_task.cpp` | 2 | ~10 | Basic |

---

## Modules with NO test coverage

The following subsystems have zero tests. They are listed in priority order based on complexity, bug risk, and how testable they are without hardware dependencies.

### Priority 1 -- High-value, immediately testable

#### 1. Behaviour Tree (`engine/core/behaviour/behaviour_tree.hpp`)

- **Risk:** The `BTNode::tick()` dispatch via `std::visit` over 7 variant types is complex logic that can silently break. Sequence/Selector index tracking and Repeater count logic are stateful and prone to off-by-one errors.
- **What to test:**
  - `BTAction` returning each `BTStatus` value
  - `BTCondition` check returning true/false mapped to Success/Failure
  - `BTWait` elapsed timer, mid-duration Running status, completion
  - `BTSequence` short-circuiting on Failure, advancing on Success, Running propagation
  - `BTSelector` short-circuiting on Success, advancing on Failure
  - `BTRepeater` with finite count, infinite (max_count=0) repeat
  - `BTInverter` flipping Success/Failure, passing through Running
  - Nested trees (Sequence containing Selector containing Actions)
  - `BehaviourTree::update()` calling root tick and storing last_status

#### 2. Tween System (`engine/core/tween.hpp`)

- **Risk:** Pointer-based target mutation (`float* target`), easing integration, and cleanup of finished tweens in `TweenManager::update()` are easy to get wrong.
- **What to test:**
  - `Tween::update()` interpolating between start and end values
  - Completion callback (`on_complete`) fires exactly once
  - Target value snaps to `end_value` on completion (no floating-point drift)
  - `TweenManager::add()` initializes target to start value
  - `TweenManager::update()` removes finished tweens
  - `active_count()` reflects additions and removals
  - Zero-duration tween completes immediately
  - Null target pointer doesn't crash

#### 3. Scene Manager (`engine/core/scene_manager.hpp`)

- **Risk:** Scene stack management (push/pop/change), lifecycle callbacks (`on_enter`, `on_exit`, `on_pause`, `on_resume`), and fade transitions have ordering constraints that are hard to verify without tests.
- **What to test:**
  - `change()` calls `on_exit` on old scene, `on_enter` on new scene
  - `push()` calls `on_pause` on current, `on_enter` on pushed scene
  - `pop()` calls `on_exit` on top, `on_resume` on underlying scene
  - `stack_size()` and `empty()` reflect operations correctly
  - `current()` returns top of stack, or nullptr when empty
  - Fade transition timer progression and alpha interpolation
  - Double-pop on empty stack doesn't crash

#### 4. UTF-8 Utilities (`engine/resource/utf8.hpp`)

- **Risk:** Byte-level UTF-8 decoding is a classic source of buffer overflows and incorrect codepoint handling. The replacement character (0xFFFD) fallback needs validation.
- **What to test:**
  - ASCII decoding (1-byte sequences)
  - 2-byte sequences (e.g., `\xC3\xA9` -> U+00E9, e with accent)
  - 3-byte sequences (e.g., CJK characters like U+3042 hiragana 'a')
  - 4-byte sequences (e.g., emoji U+1F600)
  - Invalid continuation bytes return 0xFFFD
  - Truncated sequences at end of string return 0xFFFD
  - `count_codepoints()` on mixed ASCII/multibyte strings
  - Empty string returns 0 count

#### 5. 2D Raycasting (`engine/physics/raycast2d.hpp`)

- **Risk:** Geometric intersection math with AABB and circle colliders, distance sorting, and tag-based filtering.
- **What to test:**
  - Ray hitting an AABB collider returns correct point, normal, distance
  - Ray missing a collider returns `std::nullopt`
  - Ray hitting a circle collider at tangent vs through center
  - `raycast2d_all()` returns hits sorted by distance
  - ColliderTag mask filtering excludes non-matching colliders
  - Ray with zero-length direction
  - Ray originating inside a collider

### Priority 2 -- Important but more complex to set up

#### 6. Job System (`engine/core/job_system.hpp`)

- **Risk:** Multi-threaded worker pool with atomic counters and condition variables. Race conditions and deadlocks are possible.
- **What to test:**
  - `initialize()` creates expected number of workers (0 = auto-detect)
  - `parallel_for()` processes all elements in range
  - `parallel_for()` with range smaller than chunk_size uses single chunk
  - `submit()` + `wait()` executes and completes
  - Multiple concurrent `parallel_for()` calls produce correct results
  - `shutdown()` joins all workers cleanly
  - `worker_count()` returns correct count after init

#### 7. Particle System (`engine/render/particle_system.hpp`)

- **Risk:** Particle lifecycle (spawn, update, death), emitter rate accumulation, and burst emission.
- **What to test:**
  - `ParticleEmitter::update()` spawns particles at configured rate
  - Particles die when `life >= max_life`
  - `burst()` creates exact count of particles
  - `stop()` prevents new emission, existing particles still update
  - `is_alive()` returns false when stopped and all particles dead
  - `ParticleManager::add()`, `clear()`, `emitter_count()`
  - Color interpolation between `color_start` and `color_end`

#### 8. UI Hierarchy (`engine/ui/ui_node.hpp`, `ui_canvas.hpp`, `ui_manager.hpp`)

- **Risk:** Tree-based parent-child hierarchy, world rect computation from local rects, and hit testing through nested nodes. The existing `test_ui_shader.cpp` only tests `UIElement` contains-point and basic widget properties -- it doesn't test the hierarchy, canvas scaling, or manager.
- **What to test:**
  - `UINode::add_child()` / `remove_child()` parent pointer maintenance
  - `compute_world_rect()` with nested parents
  - `find_by_name()` / `find_by_id()` traversal
  - `set_sibling_index()` reordering
  - `UICanvas` scale modes and `scale_factor()` computation
  - `UIManager::hit_test()` through overlapping nodes
  - Inactive/invisible nodes excluded from updates and hit tests

#### 9. Shader Library (`engine/shader/shader_library.hpp`)

- **Risk:** 30+ factory methods creating shader nodes. The existing shader graph test only covers basic graph connectivity and GLSL generation -- it doesn't test the node library's factory methods or the variety of node types.
- **What to test:**
  - Each factory method produces a node with correct type, inputs, and outputs
  - Nodes connect correctly (type compatibility of inputs/outputs)
  - Generated GLSL from standard node combinations (e.g., texture sample -> color multiply -> output)
  - WGSL generation path produces valid output

### Priority 3 -- Valuable but requires mocking or external dependencies

#### 10. Network Manager (`engine/net/network_manager.hpp`)

- **Risk:** TCP/UDP socket management, server/client state, message dispatch.
- **What to test (with loopback/mocking):**
  - State transitions: inactive -> server, inactive -> client
  - `send()` / `poll()` roundtrip with loopback connection
  - `set_handler()` callback invocation on message receipt
  - `shutdown()` cleanup
  - `is_active()`, `is_server()`, `is_client()` state queries

#### 11. Resource Manager (`engine/resource/resource_manager.hpp`)

- **Risk:** Reference-counted resource loading, garbage collection, and shutdown cleanup. Requires filesystem fixtures.
- **What to test:**
  - `texture_count()` / `font_count()` tracking
  - Double load returns same handle (ref counting)
  - `release_texture()` decrements ref count
  - `collect_garbage()` removes zero-ref resources
  - `shutdown()` releases all resources

#### 12. Text Layout Engine (`engine/text/text_layout.hpp`)

- **Risk:** Complex layout logic including word wrap, CJK line breaking, alignment, and overflow handling. Requires a FontAsset fixture.
- **What to test (with a stub FontAsset):**
  - Single-line layout produces correct glyph count and total_width
  - Word wrap at max_width boundary
  - CJK character-level line breaking
  - TextAlign (Left, Center, Right) glyph offset calculation
  - Ellipsis overflow mode truncates and adds "..."
  - RTL direction reversal
  - `measure()` returns consistent size with full `layout()`

---

## Gaps within existing test files

Even modules that have tests have notable gaps:

| Module | Existing Coverage | Missing |
|--------|------------------|---------|
| ECS World | create, destroy, add/get/has component, basic query | `parallel_each()`, `entity_count()` edge cases, archetype transitions when components are added/removed |
| TaskSystem | register, run, destroy, layer count | Concept-based threading dispatch, `threading_report()`, interaction between layers |
| StateMachine | transition, update, monostate | `draw()` dispatch, `is_state<T>()` queries, re-entry into same state |
| GameObject | properties, components | Component removal/replacement semantics, component iteration |
| Render CommandBuffer | push, clear, merge, double buffer | Command ordering guarantees, capacity limits |
| ShaderGraph | add node, connect, validate, sort, generate, optimize | Disconnected subgraphs, cycles, multiple outputs, type mismatch errors |
| Physics RigidBody | mass, force, impulse, gravity | Angular velocity integration, multiple body collision resolution, sleep/wake thresholds |
| Camera2D/3D | world-to-screen, zoom, forward vector | Viewport boundary mapping, aspect ratio changes, near/far plane clipping |

---

## Recommended implementation order

1. **Behaviour Tree** -- Pure logic, high complexity, zero dependencies. Best ROI.
2. **Tween System** -- Small API, easy to test, pointer mutation is a common bug source.
3. **Scene Manager** -- Critical orchestration logic with lifecycle callbacks.
4. **UTF-8** -- Pure function, easy to write exhaustive tests, prevents encoding bugs.
5. **Raycasting** -- Geometric math that's error-prone and used by gameplay code.
6. **ECS gaps** -- Extend `test_ecs_task.cpp` to cover `parallel_each()` and edge cases.
7. **Job System** -- Important concurrency correctness, but harder to write deterministic tests.
8. **Particle System** -- Moderate complexity, can test without rendering context.
9. **UI Hierarchy** -- Fill the gap between the existing UIElement tests and the full UI stack.
10. **Shader Library factories** -- Extend `test_ui_shader.cpp` to cover node creation.
11. **Network / Resources / Text Layout** -- Require mock/stub infrastructure.
