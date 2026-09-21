#include <cstdio>
#include <fstream>
#include <map>
#include <print>
#include <string>

#include "event_list.h"
#include "parse.h"

std::string to_string(const nano_edr::Event* event)
{
    return std::format("ts={} type={} pid={}", event->ts, event->type, event->pid);
}

bool get_before_last(const nano_edr::EventList* list, nano_edr::Event* out)
{
    if (list->size < 2)
        return false;
    nano_edr::EventNode* cur = list->head;
    while (cur->next != list->tail)
        cur = cur->next;
    *out = cur->event;
    return true;
}

int main(int argc, char** argv)
{
    const std::vector<const char*> patterns = {"wscript.exe",
                                               ".locked",
                                               "certutil.exe",
                                               "\\Startup\\"};
    bool quiet = false;
    size_t window_size = 64;

    // -------------------------------
    // ----- Открытие файла лога -----
    // -------------------------------

    if (argc < 2 || std::string_view(argv[1]).starts_with("-"))
    {
        std::print(stderr, "использование: nano-edr <журнал.log> <флаги>\n");
        return 2;
    }
    std::ifstream log(argv[1]);
    if (!log)
    {
        std::print(stderr, "не удалось открыть журнал: {}\n", argv[1]);
        return 2;
    }

    // ------------------------------
    // ----- Парсинг параметров -----
    // ------------------------------

    for (int i = 2; i < argc; ++i)
    {
        std::string_view arg(argv[i]);
        if (arg == "--quiet")
            quiet = true;
        else if (arg == "--window-size")
        {
            if (i + 1 >= argc)
                std::print(stderr, "неверное исользование --window-size: --window-size `Число`");
            i++;
            window_size = std::stoull(argv[i]);
        }
        else
        {
            std::print(stderr, "неизвестный аргумент: {}\n", arg);
            return 2;
        }
    }

    nano_edr::Event event;
    nano_edr::EventList event_list;
    event_list.capacity = window_size;
    std::string line;
    std::map<std::string, int> types;
    long long lines = 0;
    long long events = 0;
    while (std::getline(log, line))
    {
        ++lines;
        if (nano_edr::IsBlankOrComment(&line))
            continue;

        if (!nano_edr::ParseEventLine(&line, &event))
            continue;

        types[event.type]++;
        for (auto& pattern : patterns)
            if (line.find(pattern) != std::string::npos)
            {
                std::print("[DETECT] строка {}, признак {}: {}\n", lines, pattern, line);
                if (!quiet)
                {
                    nano_edr::Event tmp;
                    if (get_before_last(&event_list, &tmp))
                        std::print("[CTX] -2: {}\n", to_string(&tmp));
                    if (event_list.tail)
                        std::print("[CTX] -1: {}\n", to_string(&event_list.tail->event));
                }
            }
        nano_edr::ListPushBack(&event_list, &event);
    }

    if (!quiet)
    {
        std::print("Всего обнаружено {} событий:\n", events);
        for (auto& [type, count] : types)
        {
            std::print("\t{} -> {}\n", type, count);
        }
    }
    return 0;
}
