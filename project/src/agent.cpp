#include "agent.h"

#include <print>
#include <stdexcept>

#include "agent_rules.h"
#include "parse.h"
#include "rules.h"
namespace nano_edr
{
Agent::Agent(std::size_t window_size, bool quiet, size_t ctx_size)
    : cnt_(0), quiet_(quiet), window_(window_size), types_(), rules_(BuildAgentRules()), ctx_size_(ctx_size), ctx_head_(nullptr) {}

void Agent::HandleEvent(const Event& event)
{
    ++cnt_;
    ++types_[event.type()];
    bool detected = false;
    for (auto& rule : rules_)
    {
        if (rule.Check(event))
        {
            detected = true;
            std::print("[DETECT] {}  {}  ts={} pid={}\n",
                       SeverityName(rule.severity()),
                       rule.id(),
                       event.raw_ts(),
                       event.pid());
        }
    }
    if (!quiet_ && detected)
        print_ctx_();
    window_.PushBack(event);
    ctx_head_ = window_.size() <= ctx_size_ ? window_.head() : ctx_head_->next;
}

void Agent::print_ctx_()
{
    auto ctx = ctx_head_;
    for (size_t i = std::min(window_.size(), ctx_size_); i > 0; --i)
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
    std::print("\n\nДетекты по правилам:");
    for (auto& rule : rules_)
    {
        std::print("\t{} -> {}\n", rule.id(), rule.hits());
    }
}
}  // namespace nano_edr