#ifndef TYPES_H
#define TYPES_H

#include "utils/constants/constants.h"
#include <vector>


namespace tektask::utils::types
{
    /**
    * @struct Triplet
    * @brief Structure for holding quadratic equation coefficients and id,
    * for computation result association in parallel processing.
    *
    * ax^2 + bx + c = 0
    */
    struct Triplet
    {
        int64_t a{0};
        int64_t b{0};
        int64_t c{0};
        int64_t id{constants::INVALID_TRIPLET_ID};

        bool operator==(const Triplet& other) const noexcept
        {
            return a == other.a && b == other.b && c == other.c;
        }
    };

    /**
     * @struct CliArgs
     * @brief Holds parsed command-line arguments.
     *
     * Stores valid `Triplet` collection extracted from the command line.
     */
    struct CliArgs
    {
        std::vector<Triplet> triplets{};
    };
}
#endif //TYPES_H
