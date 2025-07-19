#include "resolver/quadratic_resolver.h"
#include "queue/blocking_queue.h"

#include <gtest/gtest.h>
#include <random>
#include <thread>

using namespace testing;
using namespace tektask::queue;
using namespace tektask::resolver;
using namespace tektask::utils::types;

class DummyQueue
{
public:
    using value_type = Triplet;
};

struct ResolverTestCase
{
    Triplet triplet{};
    std::string expected{};
};

DummyQueue dummyQueue{};
std::vector<EquationSolveResult> dummyResolverResults{};


TEST(QuadraticResolverTest, Resolve_ZeroACoefficientVariations)
{
    std::vector<ResolverTestCase> cases{
        // linear equation
        {{0, 10, -10}, "(0, 10, -10) => (1), no extremum"},

        // infinite roots
        {{0, 0, 0}, "(0, 0, 0) => infinite roots, no extremum"},

        // no solution
        {{0, 0, 10}, "(0, 0, 10) => no solution, no extremum"},
    };

    QuadraticEquationResolver<DummyQueue> resolver(dummyQueue, dummyResolverResults);

    std::string actual{};
    for (const auto& testCase : cases)
    {
        ASSERT_NO_THROW(actual = resolver.resolve(testCase.triplet));
        ASSERT_EQ(testCase.expected, actual);
    }
}

TEST(QuadraticResolverTest, Resolve_NoRealRoots)
{
    ResolverTestCase testCase{{1, 0, 1}, "(1, 0, 1) => no real roots, Xmin=0"};

    QuadraticEquationResolver<DummyQueue> resolver(dummyQueue, dummyResolverResults);
    auto actual{resolver.resolve(testCase.triplet)};
    ASSERT_EQ(actual, testCase.expected);
}

TEST(QuadraticResolverTest, Resolve_SingleRoot)
{
    ResolverTestCase testCase{{1, 2, 1}, "(1, 2, 1) => (-1), Xmin=-1"};

    QuadraticEquationResolver<DummyQueue> resolver(dummyQueue, dummyResolverResults);
    auto actual{resolver.resolve(testCase.triplet)};
    ASSERT_EQ(actual, testCase.expected);
}

TEST(QuadraticResolverTest, Resolve_TwoRoots)
{
    ResolverTestCase testCase{{1, -2, -3}, "(1, -2, -3) => (3, -1), Xmin=1"};

    QuadraticEquationResolver<DummyQueue> resolver(dummyQueue, dummyResolverResults);
    auto actual{resolver.resolve(testCase.triplet)};
    ASSERT_EQ(actual, testCase.expected);
}

TEST(QuadraticResolverTest, Resolve_1MSetOfPredefinedCases)
{
    using Queue = BlockingQueue<Triplet>;
    static constexpr int TRIPLETS_COUNT{1'000'000};
    static constexpr int CONSUMERS_COUNT{4};

    const std::vector<ResolverTestCase> referenceTriplets = {
        {{0, 0, 0}, "(0, 0, 0) => infinite roots, no extremum"},
        {{0, 0, 5}, "(0, 0, 5) => no solution, no extremum"},
        {{0, 5, -10}, "(0, 5, -10) => (2), no extremum"},
        {{1, -4, 3}, "(1, -4, 3) => (3, 1), Xmin=2"},
        {{1, -2, -3}, "(1, -2, -3) => (3, -1), Xmin=1"},
        {{1, 0, 1}, "(1, 0, 1) => no real roots, Xmin=0"},
        {{1, 2, 1}, "(1, 2, 1) => (-1), Xmin=-1"},
        {{2, -6, -8}, "(2, -6, -8) => (4, -1), Xmin=1.5"},
        {{2, 8, 8}, "(2, 8, 8) => (-2), Xmin=-2"},
    };

    std::vector<ResolverTestCase> producerTripletsData{};
    producerTripletsData.reserve(TRIPLETS_COUNT);

    {
        // generate randomised producer dataset
        std::random_device seeder{};
        const auto seed{seeder.entropy() ? seeder() : time(nullptr)};
        auto engine{std::mt19937(static_cast<std::mt19937::result_type>(seed))};
        std::uniform_int_distribution<int> dist(0, static_cast<int>(referenceTriplets.size() - 1));
        for (int i = 0; i < TRIPLETS_COUNT; ++i)
        {
            auto& src{referenceTriplets[dist(engine)]};
            producerTripletsData.emplace_back(ResolverTestCase{
                {src.triplet.a, src.triplet.b, src.triplet.c, i}, {src.expected}
            });
        }
    }

    std::vector<std::thread> consumers{};
    consumers.reserve(CONSUMERS_COUNT);

    std::vector<EquationSolveResult> resolverResults{TRIPLETS_COUNT};
    Queue queue{};

    for (int i = 0; i < CONSUMERS_COUNT; ++i)
    {
        consumers.emplace_back(QuadraticEquationResolver<Queue>(queue, resolverResults));
    }

    for (const auto& data : producerTripletsData)
    {
        queue.waitPush(data.triplet);
    }

    queue.shutdown();

    for (auto& consumer : consumers)
    {
        consumer.join();
    }

    for (int i = 0; i < TRIPLETS_COUNT; ++i)
    {
        ASSERT_EQ(producerTripletsData[i].expected, resolverResults[i].result);
    }
}
