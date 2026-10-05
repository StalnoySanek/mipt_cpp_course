#include <charconv>
#include <cstdio>
#include <fstream>
#include <map>
#include <print>
#include <string>

#include "agent_rules.h"
#include "event_list.h"
#include "parse.h"
#include "rules.h"

void print_context(nano_edr::EventNode* ctx, size_t ctx_size)
{
    while (ctx_size > 0)
    {
        std::print("[CTX] -{}: ts={} type={} pid={}\n",
                   ctx_size,
                   ctx->event.ts,
                   ctx->event.type,
                   ctx->event.pid);
        ctx = ctx->next;
        ctx_size--;
    }
}

struct Options
{
    const size_t ctx_size = 2;
    std::string path = "";
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
        else if (out.path.empty())
            out.path = std::string(argv[i]);
        else
            return false;  // path has already existed
    }
    return true;
}

size_t handle_log(std::ifstream& log, const Options& options, std::map<std::string, size_t>& out_types)
{
    nano_edr::Event event;
    std::string line;
    size_t cnt_events = 0;
    nano_edr::EventList event_list;
    event_list.capacity = options.window_size;
    nano_edr::EventNode* ctx = nullptr;
    out_types.clear();

    while (std::getline(log, line))
    {
        if (nano_edr::IsBlankOrComment(&line))
            continue;

        if (!nano_edr::ParseEventLine(&line, &event))
            continue;

        out_types[event.type]++;
        cnt_events++;
        if (nano_edr::CheckRules(event, nano_edr::AgentRules(), nano_edr::AgentRuleCount()) && !options.quiet)
            print_context(ctx, std::min(event_list.size, options.ctx_size));

        nano_edr::ListPushBack(&event_list, &event);
        ctx = event_list.size <= options.ctx_size ? event_list.head : ctx->next;
    }
    return cnt_events;
}

int main(int argc, char** argv)
{
    try
    {
        Options options;
        std::map<std::string, size_t> types;

        // ------------------------------------
        // ---- parse options and open log ----
        // ------------------------------------

        if (!parse_options(argc, argv, options))
        {
            std::print(stderr, "Ошибка парсинга аргументов");
            return 2;
        }
        std::ifstream log(options.path);
        if (!log)
        {
            std::print(stderr, "не удалось открыть журнал: {}\n", options.path);
            return 2;
        }

        // ----------------------
        // ---- main handler ----
        // ----------------------

        size_t cnt_events = handle_log(log, options, types);

        if (!options.quiet)
        {
            std::print("Всего обнаружено {} событий:\n", cnt_events);
            for (auto& [type, count] : types)
            {
                std::print("\t{} -> {}\n", type, count);
            }
        }
        return 0;
    }
    catch (const std::exception& e)
    {
        std::print(stderr, "Произошла ошибка:\n {}", e.what());
        return 1;
    }
}
