#include <fstream>
#include <map>
#include <print>
#include <string>

#include "event_list.h"
#include "rule.h"

#ifndef NANO_EDR_AGENT_H
#define NANO_EDR_AGENT_H

namespace nano_edr
{
class Agent
{
 public:
    Agent(std::size_t window_size, bool quiet, size_t ctx_size);
    void HandleEvent(const Event& event);
    void PrintSummary() const;

 private:
    size_t cnt_;
    bool quiet_;
    EventList window_;
    std::map<std::string, size_t> types_;
    std::vector<MatchRule> rules_;
    size_t ctx_size_;
    const EventNode* ctx_head_;

    void print_ctx_();
};
}  // namespace nano_edr

#endif