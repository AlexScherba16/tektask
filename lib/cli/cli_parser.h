#ifndef CLI_PARSER_H
#define CLI_PARSER_H

#include "utils/types/types.h"

#include <optional>


namespace tektask::cli_parser
{
    /**
     * @class CliParser
     * @brief Command-line arguments parser.
     *
     * This class is responsible for parsing command-line arguments
     * and extracting necessary information for proper application configuration
     */
    class CliParser
    {
    public:
        CliParser() = default;
        CliParser(const CliParser&) = delete;
        CliParser(CliParser&&) = delete;
        CliParser& operator=(const CliParser&) = delete;
        CliParser& operator=(CliParser&&) = delete;
        ~CliParser() = default;

        /**
         * @brief Parses command-line arguments into CliArgs struct.
         *
         * Expects input in form of triplets, like 1, 2, 3, ...
         * Filters garbage triplet sequence or with invalid size.
         *
         * @param argc Number of command-line arguments.
         * @param argv Array of command-line arguments.
         * @return CliArgs structure with valid triplets for further processing.
         *
         * @throws if no valid triplets.
         */
        [[nodiscard]] utils::types::CliArgs parse(int argc, const char* argv[]);

    private:
        /**
         * @brief Prints an invalid triplet with a custom error message.
         *
         * @param argv Argument array.
         * @param start Index of the first triplet element.
         * @param end Index after the last element.
         * @param message Error message to be printed.
         */
        void _processInvalidTriplet(const char* argv[], int32_t start, int32_t end, std::string_view message) noexcept;

        /**
         * @brief Attempts to parse a valid len triplet starting from a given index.
         *
         * Extracts three consecutive arguments from argv and tries to convert them
         * into a numeric triplet. Returns nullopt if conversion fails.
         *
         * @param argv Argument array.
         * @param start Index of the first triplet element.
         * @return Triplet structure on success or std::nullopt if parsing failed.
         */
        std::optional<utils::types::Triplet> _parseTriplet(const char* argv[], int32_t start) noexcept;
    };
}

#endif //CLI_PARSER_H
