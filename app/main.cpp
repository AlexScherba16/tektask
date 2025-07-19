#include "cli/cli_parser.h"
#include "resolver/quadratic_resolver.h"
#include "queue/blocking_queue.h"

#include <iostream>
#include <thread>


using namespace tektask::queue;
using namespace tektask::resolver;
using namespace tektask::cli_parser;
using namespace tektask::utils::types;

int main(int argc, const char* argv[])
{
    try
    {
        using Queue = BlockingQueue<Triplet>;
        using Resolver = QuadraticEquationResolver<Queue>;
        using ResolveBuffer = std::vector<EquationSolveResult>;

        // parse cmd input and prepare proper data for computation
        auto params{CliParser{}.parse(argc, argv)};

        // determine optimal thread count
        auto threadCount{static_cast<uint32_t>(std::thread::hardware_concurrency())};
        threadCount = (threadCount == 0 ? 4 : threadCount);

        // prepare resolver input queue and result storage
        ResolveBuffer output(params.triplets.size());
        Queue input{};

        // create and run resolver threads
        std::vector<std::thread> resolveConsumers;
        resolveConsumers.reserve(threadCount - 1);
        for (int i = 1; i < threadCount; ++i)
        {
            resolveConsumers.emplace_back(Resolver(input, output));
        }

        // push data into the queue for resolvers
        for (int i = 0; i < params.triplets.size(); ++i)
        {
            auto& triplet{params.triplets[i]};
            triplet.id = i;
            input.waitPush(std::move(triplet));
        }

        // no more data to produce, shutdown queue, let consumers drain remaining data
        input.shutdown();

        // join resolver threads
        for (auto& thread : resolveConsumers)
        {
            thread.join();
        }

        // print resolved results
        std::cout << "\n";
        for (const auto& solution : output)
        {
            std::cout << solution.result << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
