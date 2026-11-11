#pragma once

#include <fstream>
#include <map>
#include <print>
#include <string>

#include "event_list.h"
#include "event_source.h"
#include "ring_buffer.h"
#include "rule.h"
#include "rule_engine.h"

namespace nano_edr
{

struct WindowEntry
{
    Timestamp ts;
    std::string type;
    std::string pid;
};
class Agent : public IEventHandler
{
 public:
    static constexpr std::size_t kWindowCapacity = 256;

    explicit Agent(bool quiet);
    void PrintSummary() const;
    void HandleEvent(const Event&) override;
    void OnSkipped() override;
    void OnBroken() override;
    void OnTick() override;

 private:
    bool quiet_;
    RingBuffer<WindowEntry, kWindowCapacity> window_;
    std::map<std::string, size_t> types_;
    RuleEngine engine_;
    size_t skipped_ = 0;
    size_t broken_ = 0;
    size_t cnt_ = 0;

    void PrintCtx();
};
}  // namespace nano_edr
