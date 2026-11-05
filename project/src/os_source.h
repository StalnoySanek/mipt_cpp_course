#pragma once
#include <string>

#include "event_source.h"
#include "os_handle.h"

namespace nano_edr
{
class OsSource : public IEventSource
{
 public:
    explicit OsSource(const std::string& path);
    OsSource(const OsSource&) = delete;
    OsSource& operator=(const OsSource&) = delete;

    void Run(IEventHandler* handler) override;

 private:
    static void Trampoline(const os_event* event, void* context) noexcept;

    OsHandle os_;
};
}  // namespace nano_edr
