#include "utils/constants/constants.h"
#include "queue/blocking_queue.h"

#include <gtest/gtest.h>
#include <thread>

using namespace testing;
using namespace tektask::queue;
using namespace tektask::utils;

TEST(BlockingQueueTest, PushPop_SingleThread)
{
    BlockingQueue<int> q{};
    q.waitPush(123);

    int actual{0};
    ASSERT_TRUE(q.waitPop(actual));
    ASSERT_EQ(actual, 123);
}

TEST(BlockingQueueTest, WaitPop_BlocksUntilPush)
{
    BlockingQueue<int> q{};
    std::atomic<bool> pushed{false};

    int result{0};
    std::thread reader([&]
    {
        ASSERT_TRUE(q.waitPop(result));
        pushed = true;
    });

    // verify no spurious readings in reader thread
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_FALSE(pushed);

    q.waitPush(123);
    reader.join();

    ASSERT_EQ(result, 123);
}

TEST(BlockingQueueTest, BlockPop_UntilShutdown)
{
    BlockingQueue<int> q{};
    std::atomic<bool> popResult{true};

    std::thread consumer([&]
    {
        int out;
        popResult = q.waitPop(out);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    q.shutdown();

    consumer.join();
    ASSERT_FALSE(popResult);
}

TEST(BlockingQueueTest, PushPopSequence_SingleThread)
{
    BlockingQueue<std::string> q;

    std::vector<std::string> input = {"1", "2", "3", "Hello"};
    for (const auto& s : input)
    {
        q.waitPush(s);
    }

    std::vector<std::string> output;
    output.reserve(input.size());

    for (size_t i = 0; i < input.size(); ++i)
    {
        std::string out;
        ASSERT_TRUE(q.waitPop(out));
        output.emplace_back(std::move(out));
    }

    ASSERT_EQ(input, output);
}

TEST(BlockingQueueTest, SingleProducer_MutiConsumerThreads)
{
    static constexpr int COUNT{50000};
    static constexpr int THREADS{4};

    struct alignas(constants::CACHE_SIZE) Slot
    {
        int actual{0};
    };

    std::array<Slot, COUNT> resultStorage{};
    BlockingQueue<int> q{};

    std::vector<std::thread> consumers;
    consumers.reserve(THREADS);

    for (int i = 0; i < THREADS; ++i)
    {
        consumers.emplace_back([&]()
        {
            int value;
            while (q.waitPop(value))
            {
                resultStorage[value].actual = value;
            }
        });
    }

    for (int i = 0; i < COUNT; ++i)
    {
        q.waitPush(i);
    }

    q.shutdown();
    for (auto& consumer : consumers)
    {
        consumer.join();
    }

    for (int i = 0; i < COUNT; ++i)
    {
        ASSERT_EQ(i, resultStorage[i].actual);
    }
}

TEST(BlockingQueueTest, Shutdown_ReadAllStoredItems)
{
    BlockingQueue<int> q{};

    std::vector<int> expectedItems{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<int> actualItems{};
    actualItems.reserve(expectedItems.size());


    // store the values, that should be drained
    for (int i = 0; i < expectedItems.size(); ++i)
    {
        q.waitPush(expectedItems[i]);
    }


    std::thread reader([&]
    {
        int result{0};
        while (q.waitPop(result))
        {
            actualItems.push_back(result);
        };
    });

    // read all enqueued items after shutdown
    q.shutdown();
    reader.join();

    ASSERT_EQ(expectedItems, actualItems);
}
