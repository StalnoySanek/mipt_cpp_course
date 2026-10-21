#pragma once
#include <string>

#include "headers/agent.h"
#include "os_handle.h"

namespace nano_edr
{
class OsSource
{
 public:
    explicit OsSource(const std::string& path);
    OsSource(const OsSource&) = delete;
    OsSource& operator=(const OsSource&) = delete;

    void Run(Agent* agent);

 private:
    static void Trampoline(const os_event* event, void* context) noexcept;

    OsHandle os_;
};
}  // namespace nano_edr
