#include <cstdio>
#include <fstream>
#include <map>
#include <print>
#include <string>

std::string parse_param(std::string line, std::string key)
{
    size_t left = line.find(key) + key.length() + 1;
    size_t right = line.find(" ", left);
    if (right == std::string::npos)
        right = line.size();
    return line.substr(left, right - left);
}

int main(int argc, char** argv)
{
    bool quiet = false;

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
        else
        {
            std::print(stderr, "неизвестный аргумент: {}\n", arg);
            return 2;
        }
    }

    std::string line;
    std::map<std::string, int> types;
    long long lines = 0;
    long long events = 0;
    while (std::getline(log, line))
    {
        ++lines;
        if (line.empty() ||
            line.starts_with("#") ||
            line.starts_with(";") ||
            line.find("ts") == std::string::npos)
            continue;
        ++events;
        ++types[parse_param(line, "type")];

        for (auto& pattern : {"wscript.exe", ".locked", "certutil.exe", "\\Startup\\"})
            if (line.find(pattern) != std::string::npos)
                std::print("[DETECT] строка {}, признак {}: {}\n", lines, pattern, line);
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
