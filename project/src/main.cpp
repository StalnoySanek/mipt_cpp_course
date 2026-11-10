#include <charconv>
#include <cstdio>
#include <print>
#include <string>
#include <string_view>

#include "agent.h"
#include "event_source.h"
#include "file_source.h"
#include "os_source.h"

struct Options
{
    bool is_file_mode = false;
    std::string path;
    bool quiet = false;
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

        nano_edr::Agent agent(options.quiet);
        std::unique_ptr<nano_edr::IEventSource> source;
        if (options.is_file_mode)
        {
            source = std::make_unique<nano_edr::FileSource>(options.path);
        }
        else
        {
            source = std::make_unique<nano_edr::OsSource>(options.path);
        }
        source->Run(&agent);
        agent.PrintSummary();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::print(stderr, "Произошла ошибка:\n {}\n", e.what());
        return 1;
    }
}
