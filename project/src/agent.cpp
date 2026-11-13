#include "agent.h"

#include <algorithm>
#include <print>
#include <stdexcept>

#include "agent_rules.h"
#include "dedup_sink.h"
#include "parse.h"

namespace nano_edr
{
Agent::Agent(bool quiet, uint64_t dedup_period_ms)
    : quiet_(quiet), dedup_period_ms_(dedup_period_ms), engine_(BuildAgentRules(&model_)) { engine_.SetModel(&model_); }

void Agent::HandleEvent(const Event& event)
{
    model_.Observe(event);
    ++cnt_;
    ++types_[event.type()];

    context_printed_ = false;
    engine_.ProcessEvent(event, nullptr);

    window_.PushBack(WindowEntry{.ts = event.ts(),
                                 .type = event.type(),
                                 .pid = event.pid()});
}

void Agent::PrintContextOnce()
{
    if (window_.empty() || context_printed_ || quiet_) return;
    context_printed_ = true;

    if (window_.size() > 1)
    {
        const auto& entry = window_[window_.size() - 2];
        std::print("[CTX] -2: {} {} pid={}\n", entry.ts.ms, entry.type, entry.pid);
    }

    const auto& entry = window_.back();
    std::print("[CTX] -1: {} {} pid={}\n", entry.ts.ms, entry.type, entry.pid);
}

void Agent::PrintSummary() const
{
    if (quiet_) return;
    std::print("[STAT] Обработано {} строк. Из них:\n\tСломано -> {}\n\tПропущено -> {}\n\tОбработано -> {}\n",
               broken_ + skipped_ + cnt_, broken_, skipped_, cnt_);
    std::print("[STAT] Всего обнаружено {} событий:\n", cnt_);
    for (auto& [type, count] : types_)
    {
        std::print("\t{} -> {}\n", type, count);
    }
    std::print("\n\n[STAT] Детекты по правилам:");
    for (size_t i = 0; i < engine_.size(); ++i)
    {
        std::print("\t{} -> {}\n", engine_.rule(i).id(), engine_.rule(i).hits());
    }
    std::print("\n\n[STAT] В модели процессов {}, файлов {}",
               model_.process_count(), model_.file_count());

    if (suppressed_ > 0)
    {
        std::print("\n[STAT] подавлено дубликатов {} (срок {} мс)\n",
                   suppressed_, dedup_period_ms_);
    }
}

void Agent::OnTick() {}
void Agent::OnSkipped() { ++skipped_; }
void Agent::OnBroken() { ++broken_; }

void Agent::Subscribe(AlertSink sink)
{
    FanOutSink all;
    all.Add([this](const Detection&)
            { PrintContextOnce(); });
    all.Add(std::move(sink));
    engine_.Subscribe(
        DedupSink(std::move(all), dedup_period_ms_, &suppressed_));
}
}  // namespace nano_edr