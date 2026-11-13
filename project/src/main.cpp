#include <charconv>
#include <cstdio>
#include <print>
#include <string>
#include <string_view>

#include "agent.h"
#include "edr_handle.h"
#include "event_source.h"
#include "field_traits.h"
#include "file_source.h"
#include "os_source.h"

struct Options
{
    bool is_file_mode = false;
    std::string path;
    bool quiet = false;
    std::string out;
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
        else if (arg == "--out")
        {
            if (i + 1 == argc) return false;
            out.out = argv[++i];
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

        nano_edr::EdrHandle edr(options.path);
        const std::string text = edr.Config("dedup_period_ms", "10000");
        uint64_t dedup_period_ms = 0;
        if (!nano_edr::FieldTraits<uint64_t>::Parse(text, &dedup_period_ms))
        {
            std::print(stderr, "[WARN] dedup_period_ms не разбирается: {}\n", text);
            dedup_period_ms = 10000;
        }
        nano_edr::Agent agent(options.quiet, dedup_period_ms);
        std::unique_ptr<nano_edr::IEventSource> source;
        if (options.is_file_mode)
        {
            source = std::make_unique<nano_edr::FileSource>(options.path);
        }
        else
        {
            source = std::make_unique<nano_edr::OsSource>(options.path);
        }

        nano_edr::FanOutSink targets;
        targets.Add(nano_edr::ConsoleSink{});
        if (!options.out.empty())
            targets.Add(nano_edr::FileSink(options.out));
        targets.Add(nano_edr::EdrSink(&edr));
        agent.Subscribe(std::move(targets));

        source->Run(&agent);
        if (edr.Flush() != EDR_OK)
        {
            std::print(stderr, "[WARN] детекты не отправлены\n");
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
