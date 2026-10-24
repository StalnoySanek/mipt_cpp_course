#include "file_source.h"

#include <print>

#include "parse.h"
namespace nano_edr
{
FileSource::FileSource(const std::string& path)
    : log_(std::ifstream(path))
{
    if (!log_)
        throw std::runtime_error("Error: can not open log `path = `" + path);
}

void FileSource::Run(Agent* agent)
{
    std::string line;
    EventParts parts;
    while (getline(log_, line))
    {
        try
        {
            if (IsBlankOrComment(line))
                continue;

            if (!ParseEventParts(line, &parts))
                continue;

            Event event(parts);
            agent->HandleEvent(event);
        }
        catch (const std::exception& e)
        {
            std::print(stderr, "Ошибка в FileSource::Run:\n{}\n", e.what());
        }
        catch (...)
        {
            std::print(stderr, "Неизвестная ошибка в FileSource::Run\n");
        }
    }
}
}  // namespace nano_edr
