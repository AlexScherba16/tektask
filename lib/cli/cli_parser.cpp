#include "cli_parser.h"

#include <array>
#include <sstream>
#include <cstring>
#include <iostream>
#include <charconv>
#include <string_view>


namespace tektask::cli_parser
{
    using namespace tektask::utils::types;

    [[nodiscard]] CliArgs CliParser::parse(int argc, const char* argv[])
    {
        const auto argsLen{argc - 1};
        if (argsLen == 0)
        {
            throw std::invalid_argument("Invalid input: missing command line arguments");
        }

        std::vector<Triplet> triplets;
        triplets.reserve(1 + argsLen / 3);

        for (int i = 1; i < argc; i += 3)
        {
            // validate proper length to create Triplet
            if (i + 2 >= argc)
            {
                _processInvalidTriplet(argv, i, argc, "Invalid input: parameter count must be a multiple of 3!");
                break;
            }

            auto triplet{_parseTriplet(argv, i)};
            if (triplet.has_value())
            {
                triplets.emplace_back(triplet.value());
                continue;
            }

            _processInvalidTriplet(argv, i, i + 3, "Invalid input: failed to parse triplet");
        }

        if (triplets.empty())
        {
            throw std::invalid_argument("Invalid input: no valid parameters");
        }
        return {
            std::move(triplets),
        };
    }

    void CliParser::_processInvalidTriplet(const char* argv[], int32_t start, int32_t end,
                                           std::string_view message) noexcept
    {
        std::array<std::string_view, 3> visited;
        for (int i = start; i < end; ++i)
        {
            visited[i - start] = argv[i];
        }

        std::stringstream stream;
        stream << "(";
        std::string separator;
        for (const auto& str : visited)
        {
            stream << separator << str;
            separator = ",";
        }

        stream << ") => " << message;
        std::cout << stream.str() << std::endl;
    }

    std::optional<Triplet> CliParser::_parseTriplet(const char* argv[], int32_t start) noexcept
    {
        Triplet tmp{};
        auto stoi = [](const char* str, auto& out)
        {
            if (!str || std::strlen(str) == 0)
            {
                return false;
            }

            const char* end = str + std::strlen(str);
            std::from_chars_result result = std::from_chars(str, end, out);
            return result.ec == std::errc{} && result.ptr == end;
        };

        bool parsed{true};
        parsed &= stoi(argv[start], tmp.a);
        parsed &= stoi(argv[start + 1], tmp.b);
        parsed &= stoi(argv[start + 2], tmp.c);

        if (parsed)
        {
            return tmp;
        }
        return std::nullopt;
    }
}
