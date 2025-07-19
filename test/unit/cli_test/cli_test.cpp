#include "cli/cli_parser.h"

#include <gtest/gtest.h>

using namespace testing;
using namespace tektask::cli_parser;
using namespace tektask::utils::types;

struct CliParserTestCase
{
    std::vector<const char*> argv{};
    CliArgs expected{};
};


TEST(CliParserTest, ParseEmptyCliArguments_ThrowsException)
{
    CliParserTestCase emptyInputCase{
        {"app_name"}, {}
    };

    auto argc{static_cast<int>(emptyInputCase.argv.size())};
    const char** argv = const_cast<const char**>(emptyInputCase.argv.data());
    CliParser cli{};

    // verify expected exception type
    ASSERT_THROW(CliArgs arg{cli.parse(argc, argv)}, std::invalid_argument);

    // verify expected exception message
    try
    {
        CliArgs arg{cli.parse(argc, argv)};
    }
    catch (const std::invalid_argument& e)
    {
        std::string_view actualMessage{e.what()};
        std::string_view expectedMessage{"Invalid input: missing command line arguments"};
        ASSERT_EQ(expectedMessage, actualMessage);
    }
}

TEST(CliParserTest, ParseInvalidInput)
{
    std::vector<CliParserTestCase> cases{
        // empty parameters
        {{"app_name", ""}, {}},

        // invalid first triplet size
        {{"app_name", "1", "2"}, {}},

        // invalid triplet
        {{"app_name", "a", "1", "2"}, {}},
        {{"app_name", "1", "b", "2"}, {}},
        {{"app_name", "1", "2", "c"}, {}},
        {{"app_name", "1", "b", "c"}, {}},
        {{"app_name", "a", "1", "c"}, {}},
        {{"app_name", "a", "b", "1"}, {}},
        {{"app_name", "a", "b", "c"}, {}},
        {{"app_name", "a_0", "b_0", "c_0", "a_1", "b_1"}, {}},
    };

    CliParser cli{};
    CliArgs args{};
    for (const auto& testCase : cases)
    {
        auto argc{static_cast<int>(testCase.argv.size())};
        const char** argv = const_cast<const char**>(testCase.argv.data());

        // verify expected exception type
        ASSERT_THROW(args = cli.parse(argc, argv), std::invalid_argument);

        // verify expected exception message
        try
        {
            args = cli.parse(argc, argv);
        }
        catch (const std::invalid_argument& e)
        {
            std::string_view actualMessage{e.what()};
            std::string_view expectedMessage{"Invalid input: no valid parameters"};
            ASSERT_EQ(expectedMessage, actualMessage);
        }
        ASSERT_EQ(args.triplets, testCase.expected.triplets);
    }
}

TEST(CliParserTest, ParseValidInput)
{
    std::vector<CliParserTestCase> cases{
        // single positive triplet
        {{"app_name", "0", "1", "2"}, {{{0, 1, 2}}}},

        // single negative triplet
        {{"app_name", "-1", "-2", "-3"}, {{{-1, -2, -3}}}},

        // triplets pack
        {
            {"app_name", "-1", "-2", "-3", "10", "20", "30", "100", "200", "300"}, {
                {
                    {-1, -2, -3},
                    {10, 20, 30},
                    {100, 200, 300},
                }
            }
        },
    };

    CliParser cli{};
    CliArgs args{};
    for (const auto& testCase : cases)
    {
        auto argc{static_cast<int>(testCase.argv.size())};
        const char** argv = const_cast<const char**>(testCase.argv.data());

        ASSERT_NO_THROW(args = cli.parse(argc, argv));
        ASSERT_EQ(args.triplets, testCase.expected.triplets);
    }
}

TEST(CliParserTest, ParseCombination_Valid_InvalidInput)
{
    std::vector<CliParserTestCase> cases{
        // skip first triplet (empty parameter in sequence)
        {{"app_name", "", "1", "2", "10", "20", "30"}, {{{10, 20, 30}}}},

        // skip first triplet (garbage value in sequence)
        {{"app_name", "1", "a", "2", "10", "20", "30"}, {{{10, 20, 30}}}},
        {
            // scip second triplet (garbage value in sequence), scip last triplet (invalid size)
            {"app_name", "1", "2", "3", "10", "b", "30", "-10", "-20", "-30", "a"}, {
                {
                    {1, 2, 3},
                    {-10, -20, -30},
                }
            }
        },
    };

    CliParser cli{};
    CliArgs args{};
    for (const auto& testCase : cases)
    {
        auto argc{static_cast<int>(testCase.argv.size())};
        const char** argv = const_cast<const char**>(testCase.argv.data());

        ASSERT_NO_THROW(args = cli.parse(argc, argv));
        ASSERT_EQ(args.triplets, testCase.expected.triplets);
    }
}
