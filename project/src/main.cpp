#include <charconv>
#include <cstdio>
#include <print>
#include <string>
#include <string_view>

#include "agent.h"
#include "file_source.h"
#include "os_source.h"

struct Options
{
    const size_t ctx_size = 2;
    bool is_file_mode = false;
    std::string path;
    bool quiet = false;
    size_t window_size = 64;
};

bool parse_options(int argc, char** argv, Options& out)
{
    for (int i = 1; i < argc; ++i)
    {
        std::string_view arg(argv[i]);
        if (arg == "--quiet")
            out.quiet = true;
        else if (arg == "--file")
            out.is_file_mode = true;
        else if (arg == "--window-size")
        {
            if (i + 1 >= argc)
                return false;
            i++;
            const std::string_view window_size_str(argv[i]);
            const char* begin = window_size_str.data();
            const char* end = begin + window_size_str.size();
            const auto res = std::from_chars(begin, end, out.window_size);

            if (res.ec != std::errc() || res.ptr != end)
                return false;
        }
        else if (!arg.empty() && arg.front() == '-')
            return false;
        else if (out.path.empty())
            out.path = std::string(argv[i]);
        else
            return false;  // path has already existed
    }
    return !out.path.empty();
}

int main(int argc, char** argv)
{
    try
    {
        Options options;
        if (!parse_options(argc, argv, options))
        {
            std::print(stderr, "Ошибка парсинга аргументов\n");
            return 2;
        }

        nano_edr::Agent agent(options.window_size, options.quiet, options.ctx_size);

        if (options.is_file_mode)
        {
            nano_edr::FileSource source(options.path);
            source.Run(&agent);
        }
        else
        {
            nano_edr::OsSource source(options.path);
            source.Run(&agent);
        }
        agent.PrintSummary();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::print(stderr, "Произошла ошибка:\n {}\n", e.what());
        return 1;
    }
}
