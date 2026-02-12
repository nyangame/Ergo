#include "test_framework.hpp"
#include "engine/render/render_command.hpp"
#include "engine/render/command_buffer.hpp"
#include "engine/render/double_buffer.hpp"

TEST_CASE(CommandBuffer_PushAndSize) {
    CommandBuffer buf;
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(buf.size(), (size_t)0);

    buf.push(RenderCmd_Clear{{0, 0, 0, 255}, 1.0f});
    ASSERT_FALSE(buf.empty());
    ASSERT_EQ(buf.size(), (size_t)1);
}

TEST_CASE(CommandBuffer_Clear) {
    CommandBuffer buf;
    buf.push(RenderCmd_Clear{});
    buf.push(RenderCmd_DrawRect{});
    buf.clear();
    ASSERT_TRUE(buf.empty());
}

TEST_CASE(CommandBuffer_Merge) {
    CommandBuffer a, b;
    a.push(RenderCmd_Clear{});
    b.push(RenderCmd_DrawRect{});
    b.push(RenderCmd_DrawCircle{});

    a.merge(b);
    ASSERT_EQ(a.size(), (size_t)3);
}

TEST_CASE(DoubleBuffer_WriteRead) {
    DoubleBufferedCommands db;

    db.write_buffer().push(RenderCmd_Clear{});
    ASSERT_EQ(db.write_buffer().size(), (size_t)1);
    ASSERT_EQ(db.read_buffer().size(), (size_t)0);
}

TEST_CASE(DoubleBuffer_Swap) {
    DoubleBufferedCommands db;

    db.write_buffer().push(RenderCmd_Clear{});
    db.write_buffer().push(RenderCmd_DrawRect{});
    db.swap();

    // After swap, read buffer should have old data, write buffer is clear
    ASSERT_EQ(db.read_buffer().size(), (size_t)2);
    ASSERT_EQ(db.write_buffer().size(), (size_t)0);
}

TEST_CASE(SharedCommandCollector_SubmitAndTake) {
    SharedCommandCollector collector;

    CommandBuffer buf1;
    buf1.push(RenderCmd_Clear{});
    CommandBuffer buf2;
    buf2.push(RenderCmd_DrawRect{});
    buf2.push(RenderCmd_DrawCircle{});

    collector.submit(buf1);
    collector.submit(buf2);

    auto merged = collector.take();
    ASSERT_EQ(merged.size(), (size_t)3);
}
