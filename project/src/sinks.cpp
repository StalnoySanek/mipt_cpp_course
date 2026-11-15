#include <algorithm>
#include <print>
#include <stdexcept>
#include <string>

#include "alert_sink.h"
#include "dedup_sink.h"
#include "detection.h"

namespace nano_edr
{

// --- ConsoleSink -----------------------------------------------------------

void ConsoleSink::operator()(const Detection& detection) const
{
    std::print("{}\n", ToString(detection));
}

// --- FileSink --------------------------------------------------------------

FileSink::FileSink(const std::string& path) : out_(path)
{
    if (!out_.is_open())
    {
        throw std::runtime_error("не удалось открыть файл: " + path);
    }
}

void FileSink::operator()(const Detection& detection)
{
    out_ << ToString(detection) << '\n';
}

// --- CountingSink ----------------------------------------------------------

void CountingSink::operator()(const Detection& detection) const
{
    counter_->seen.push_back(detection);
    ++counter_->total;
}

// --- FanOutSink ------------------------------------------------------------

void FanOutSink::Add(AlertSink sink) { sinks_.push_back(std::move(sink)); }

void FanOutSink::operator()(const Detection& detection) const
{
    for (const auto& sink : sinks_)
        sink(detection);
}

// --- DedupSink -------------------------------------------------------------

DedupSink::DedupSink(AlertSink inner, uint64_t period_ms,
                     std::size_t* suppressed)
    : inner_(std::move(inner)),
      period_ms_(period_ms),
      suppressed_(suppressed)
{
}

void DedupSink::operator()(const Detection& detection)
{
    if (period_ms_ == 0)
    {
        inner_(detection);
        return;
    }

    const auto entry = std::find_if(
        entries_.begin(), entries_.end(),
        [&detection](const Entry& item)
        {
            return item.rule == detection.rule && item.pid == detection.pid;
        });

    const uint64_t ts_ms = detection.ts.ms;
    if (entry != entries_.end() &&
        ts_ms >= entry->first_ts_ms &&
        ts_ms - entry->first_ts_ms <= period_ms_)
    {
        if (suppressed_ != nullptr)
            ++*suppressed_;
        return;
    }

    if (entry == entries_.end())
        entries_.push_back({detection.rule, detection.pid, ts_ms});
    else
        entry->first_ts_ms = ts_ms;

    inner_(detection);
}

}  // namespace nano_edr
