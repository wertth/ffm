//
// Created by 缪浩楚 on 2024/6/16.
//
#include "gtest/gtest.h"
#include "player.h"
extern "C" {
#include <libavcodec/avcodec.h>
}

// 使用之前的 PacketQueue 定义

TEST(PacketQueueTest, PushPopTest) {
    PacketQueue queue;

    AVPacket *packet1 = av_packet_alloc();
    AVPacket *packet2 = av_packet_alloc();

    // 推入两个包
    queue.push(packet1);
    queue.push(packet2);

    // 弹出并检查
    AVPacket *poppedPacket1 = queue.pop();
    EXPECT_EQ(poppedPacket1, packet1);

    AVPacket *poppedPacket2 = queue.pop();
    EXPECT_EQ(poppedPacket2, packet2);

    // 弹出空队列
    AVPacket *poppedPacket3 = queue.pop();
    EXPECT_EQ(poppedPacket3, nullptr);
}

TEST(PacketQueueTest, AbortTest) {
    PacketQueue queue;
    queue.abort = true;

    AVPacket *packet1 = av_packet_alloc();
    queue.push(packet1);

    AVPacket *poppedPacket = queue.pop();
    EXPECT_EQ(poppedPacket, nullptr);

    av_packet_free(&packet1);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
