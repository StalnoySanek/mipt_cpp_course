#include "file_source.h"

#include <fstream>
#include <print>
#include <string>

#include "parse.h"
namespace nano_edr
{
FileSource::FileSource(const std::string& path)
    : log_(std::ifstream(path))
{
    if (!log_)
        throw std::runtime_error("Error: can not open log `path = `" + path);
}

void FileSource::Run(IEventHandler* handler)
{
    std::string line;
    EventParts parts;
    while (getline(log_, line))
    {
        try
        {
            if (IsBlankOrComment(line))
            {
                handler->OnSkipped();
                continue;
            }

            if (!ParseEventParts(line, &parts))
            {
                handler->OnBroken();
                continue;
            }

            Event event(parts);
            handler->HandleEvent(event);
        }
        catch (const std::exception& e)
        {
            handler->OnBroken();
            std::print(stderr, "Ошибка в FileSource::Run:\n{}\n", e.what());
        }
        catch (...)
        {
            handler->OnBroken();
            std::print(stderr, "Неизвестная ошибка в FileSource::Run\n");
        }
    }
}
}  // namespace nano_edr
