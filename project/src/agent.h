#include <fstream>
#include <map>
#include <print>
#include <string>

#include "event_list.h"
#include "os_handle.h"
#include "rule.h"

#ifndef NANO_EDR_AGENT_H
#define NANO_EDR_AGENT_H

namespace nano_edr
{
class Agent
{
 public:
    Agent(std::size_t window_size, bool quiet);
    void HandleEvent(const Event& event);
    void PrintSummary() const;

 private:
    size_t cnt_;
    bool quiet_;
    EventList window_;
    std::map<std::string, size_t> types_;
    std::vector<MatchRule> rules_;

    void print_ctx_();
};

class FileSource
{
 public:
    explicit FileSource(const std::string& path);
    FileSource(const FileSource&) = delete;
    FileSource& operator=(const FileSource&) = delete;

    void Run(Agent* agent);

 private:
    std::ifstream log_;
};

class OsSource
{
 public:
    explicit OsSource(const std::string& path);
    OsSource(const OsSource&) = delete;
    OsSource& operator=(const OsSource&) = delete;

    void Run(Agent* agent);

 private:
    static void Trampoline(const os_event* event, void* context) noexcept;
    void HandleEvent(const os_event& event) noexcept;

    OsHandle os_;
    Agent* agent_ = nullptr;
};
}  // namespace nano_edr

#endif