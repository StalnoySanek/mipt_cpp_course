#include "os_source.h"

#include <print>

namespace nano_edr
{
OsSource::OsSource(const std::string& path) : os_(path) {}

void OsSource::Trampoline(const os_event* event, void* context) noexcept
{
    auto handler = static_cast<IEventHandler*>(context);
    try
    {
        EventParts parts;
        parts.ts = std::to_string(event->ts);
        parts.type = event->type == nullptr ? "" : event->type;
        if (event->pid != 0)
            parts.pid = std::to_string(event->pid);

        parts.fields.reserve(event->field_count);
        for (size_t i = 0; i < event->field_count; ++i)
        {
            parts.fields.push_back({event->fields[i].key, event->fields[i].value});
        }

        handler->HandleEvent(Event(parts));
    }
    catch (const std::exception& e)
    {
        handler->OnBroken();
        std::print(stderr, "Ошибка в OsSource::Trampoline:\n{}\n", e.what());
    }
    catch (...)
    {
        handler->OnBroken();
        std::print(stderr, "Неизвестная ошибка в OsSource::Trampoline\n");
    }
}

void OsSource::Run(IEventHandler* handler)
{
    os_.Subscribe(&OsSource::Trampoline, handler);
    os_.Start();

    while (true)
    {
        const os_status status = os_.Wait(100);
        if (status == OS_TIMEOUT)
        {
            handler->OnTick();
            continue;
        }
        if (status == OS_OK)
            break;
        throw std::runtime_error(std::string("Ошибка источника os: ") +
                                 os_status_str(status) + "\n");
    }
}
}  // namespace nano_edr
