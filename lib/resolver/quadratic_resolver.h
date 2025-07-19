#ifndef QUADRATIC_RESOLVER_H
#define QUADRATIC_RESOLVER_H

#include "utils/types/types.h"

#include <sstream>
#include <cmath>

namespace tektask::resolver
{
    /**
     * @class QuadraticEquationResolver
     * @brief Solves quadratic equation based on Triplet coefficients.
     *
     * This class implemented as a runner in a worker thread. Continuously retrieves
     * triplets from the input queue, solves the quadratic equation, and stores
     * a formatted result into a shared result buffer at the index given by Triplet::id.
     *
     * @tparam QueueType The queue type used for feeding triplets (BlockingQueue, LockFreeQueue, etc).
     */
    template <typename QueueType>
    class QuadraticEquationResolver
    {
        using InputType = typename QueueType::value_type;

    public:
        /**
         * @brief Constructs a resolver with references at input queue and result buffer.
         *
         * @param queue The shared input queue for receiving Triplets.
         * @param resolveStorage The result buffer to write outputs into, by Triplet::id.
         */
        explicit QuadraticEquationResolver(QueueType& queue,
                                           std::vector<utils::types::EquationSolveResult>& resolveStorage) :
            m_queue(queue),
            m_resolveStorage(resolveStorage)
        {
        }

        ~QuadraticEquationResolver() = default;
        QuadraticEquationResolver(const QuadraticEquationResolver&) = delete;
        QuadraticEquationResolver& operator=(const QuadraticEquationResolver&) = delete;

        QuadraticEquationResolver(QuadraticEquationResolver&& other) noexcept : m_queue(other.m_queue),
            m_resolveStorage(other.m_resolveStorage)
        {
        }

        QuadraticEquationResolver& operator=(QuadraticEquationResolver&& other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }
            m_queue = other.m_queue;
            m_resolveStorage = other.m_resolveStorage;
            return *this;
        }

        /**
         * @brief Solves a single quadratic equation based on Triplet.
         *
         * Produces a human-readable string with the roots and extremum location.
         *
         * @param t The input Triplet containing equation coefficients.
         * @return A formatted string representing the solution, extremum point or no solution.s
         */
        [[nodiscard]] std::string resolve(const utils::types::Triplet& t) const noexcept
        {
            std::stringstream stream;
            stream << "(" << t.a << ", " << t.b << ", " << t.c << ") => ";

            const auto a{static_cast<double>(t.a)};
            const auto b{static_cast<double>(t.b)};
            const auto c{static_cast<double>(t.c)};

            if (a == 0.0)
            {
                // case a == 0, b != 0; linear function
                if (b != 0.0)
                {
                    const double x{-c / b};
                    stream << "(" << x << "), no extremum";
                }

                // case a == b == c == 0
                else if (c == 0.0)
                {
                    stream << "infinite roots, no extremum";
                }

                // case a == b == 0, c != 0; constant line
                else
                {
                    stream << "no solution, no extremum";
                }
                return stream.str();
            }

            const double D{b * b - 4 * a * c};

            // case with complex roots solution
            if (D < 0.0)
            {
                stream << "no real roots";
            }

            // case single root
            else if (D == 0.0)
            {
                const double x = -b / (2 * a);
                stream << "(" << x << ")";
            }

            // case with classical solution
            else
            {
                const double sqrtD{std::sqrt(D)};
                const double x1{(-b + sqrtD) / (2 * a)};
                const double x2{(-b - sqrtD) / (2 * a)};
                stream << "(" << x1 << ", " << x2 << ")";
            }

            // find extremum, f(x) == a * x^2 + b * x + c
            // calculate derivative f'(x) = 2 * a * x + b
            // set derivative to zero, f'(x) = 0
            // find x, x = -b / 2 * a
            const double xMin{-b / (2 * a)};

            // normalization, double can return -0.0
            stream << ", Xmin=" << (xMin == 0.0 ? 0.0 : xMin);
            return stream.str();
        }

        /**
         * @brief Resolver runner loop.
         *
         * Continuously reads Triplets from the queue, solves them, and stores
         * the result into resolve storage at the position given by Triplet::id.
         *
         * Terminates when the queue signals shutdown.
         */
        void operator()()
        {
            while (true)
            {
                InputType item;
                if (!m_queue.waitPop(item))
                    break;

                auto result{resolve(item)};
                m_resolveStorage[item.id].result = std::move(result);
            }
        }

    private :
        QueueType& m_queue;
        std::vector<utils::types::EquationSolveResult>& m_resolveStorage;
    };
}
#endif //QUADRATIC_RESOLVER_H
