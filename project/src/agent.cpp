#include "agent.h"

#include <print>
#include <stdexcept>

#include "agent_rules.h"
#include "parse.h"
#include "rules.h"
namespace nano_edr
{
Agent::Agent(std::size_t window_size, bool quiet)
    : cnt_(0), quiet_(quiet), window_(window_size), types_() {}

void Agent::HandleEvent(const Event& event)
{
    ++cnt_;
    ++types_[event.type()];
    const size_t detections = CheckRules(event, AgentRules(), AgentRuleCount());
    if (!quiet_ && detections != 0)
        print_ctx_();
    window_.PushBack(event);
}

void Agent::print_ctx_()
{
    auto ctx = window_.head();
    for (size_t i = window_.size(); i > 0; --i)
    {
        std::print("[CTX] -{}: ts={} type={} pid={}\n",
                   i,
                   ctx->event.raw_ts(),
                   ctx->event.type(),
                   ctx->event.pid());
        ctx = ctx->next;
    }
}

void Agent::PrintSummary() const
{
    if (quiet_) return;
    std::print("Всего обнаружено {} событий:\n", cnt_);
    for (auto& [type, count] : types_)
    {
        std::print("\t{} -> {}\n", type, count);
    }
}

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
        if (IsBlankOrComment(line))
            continue;

        if (!ParseEventParts(line, &parts))
            continue;

        try
        {
            Event event(parts);
            agent->HandleEvent(event);
        }
        catch (...)
        {
        }
    }
    agent->PrintSummary();
}

OsSource::OsSource(const std::string& path) : os_(path) {}

void OsSource::Trampoline(const os_event* event, void* context) noexcept
{
    try
    {
        if (event != nullptr && context != nullptr)
            static_cast<OsSource*>(context)->HandleEvent(*event);
    }
    catch (...)
    {
    }
}

void OsSource::HandleEvent(const os_event& event) noexcept
{
    try
    {
        EventParts parts;
        parts.ts = std::to_string(event.ts);
        parts.type = event.type == nullptr ? "" : event.type;
        if (event.pid != 0)
            parts.pid = std::to_string(event.pid);

        parts.fields.reserve(event.field_count);
        for (size_t i = 0; i < event.field_count; ++i)
        {
            if (event.fields == nullptr ||
                event.fields[i].key == nullptr ||
                event.fields[i].value == nullptr)
                throw std::invalid_argument("invalid os event field");
            parts.fields.push_back({event.fields[i].key, event.fields[i].value});
        }

        agent_->HandleEvent(Event(parts));
    }
    catch (...)
    {
    }
}

void OsSource::Run(Agent* agent)
{
    agent_ = agent;
    os_.Subscribe(&OsSource::Trampoline, this);
    os_.Start();

    while (true)
    {
        const os_status status = os_.Wait(100);
        if (status == OS_TIMEOUT)
            continue;
        if (status == OS_OK)
            break;
        throw std::runtime_error(std::string("Ошибка источника os: ") +
                                 os_status_str(status));
    }

    agent_->PrintSummary();
}
}  // namespace nano_edr