#include <fstream>
#include <map>
#include <print>

#include "event_list.h"

#ifndef NANO_EDR_AGENT_H
#define NANO_EDR_AGENT_H

namespace nano_edr
{
class Agent
{
 public:
    Agent(std::size_t window_size, bool quiet);  // из --window-size и --quiet
    void HandleEvent(const Event& event);
    void PrintSummary() const;  // сводка в конце прогона
                                // ...
 private:
    size_t cnt_;
    bool quiet_;
    EventList window_;  // последние события, не больше window_size
    std::map<std::string, size_t> types_;

    void print_ctx_();
};

class FileSource
{
 public:
    explicit FileSource(const std::string& path);
    FileSource(const FileSource&) = delete;
    FileSource& operator=(const FileSource&) = delete;

    void Run(Agent* agent);  // строка → EventParts → Event → HandleEvent
 private:
    std::ifstream log_;
};
}  // namespace nano_edr

#endif