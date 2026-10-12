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
    cnt_ += CheckRules(event, AgentRules(), AgentRuleCount());
    if (!quiet_)
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
}  // namespace nano_edr