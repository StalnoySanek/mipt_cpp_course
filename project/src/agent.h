#pragma once

#include <fstream>
#include <map>
#include <print>
#include <string>

#include "event_list.h"
#include "event_source.h"
#include "rule.h"
#include "rule_engine.h"

namespace nano_edr
{
class Agent : public IEventHandler
{
 public:
    Agent(std::size_t window_size, bool quiet, size_t ctx_size);
    void PrintSummary() const;
    void HandleEvent(const Event&) override;
    void OnSkipped() override;
    void OnBroken() override;
    void OnTick() override;

 private:
    size_t cnt_;
    bool quiet_;
    EventList window_;
    std::map<std::string, size_t> types_;
    RuleEngine engine_;
    size_t ctx_size_;
    const EventNode* ctx_head_;
    size_t skipped_ = 0;
    size_t broken_ = 0;

    void print_ctx_();
};
}  // namespace nano_edr
