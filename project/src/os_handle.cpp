#include "os_handle.h"

#include <format>
#include <stdexcept>

#include "os.h"

namespace nano_edr
{
OsHandle::OsHandle(const std::string& config_path)
{
    if (auto res = os_init(config_path.data(), &handle_);
        res != os_status::OS_OK)
    {
        throw std::runtime_error(std::format("Error in creating OsHandler: `path = {}` `os_status = {}`",
                                             config_path, os_status_str(res)));
    }
}

OsHandle::~OsHandle()
{
    os_free(handle_);
}

void OsHandle::Subscribe(os_event_cb callback, void* context)
{
    if (auto res = os_event_subscribe(handle_, callback, context);
        res != os_status::OS_OK)
    {
        throw std::runtime_error("Error in `OsHandle::Subscribe`. Status -> " + std::string(os_status_str(res)));
    }
}

void OsHandle::Start()
{
    if (auto res = os_start(handle_);
        res != os_status::OS_OK)
    {
        throw std::runtime_error("Error in `OsHandle::Start`. Status -> " + std::string(os_status_str(res)));
    }
}

os_status OsHandle::Wait(uint32_t timeout_ms)
{
    return os_wait(handle_, timeout_ms);
}

void OsHandle::Stop()
{
    os_stop(handle_);
}
}  // namespace nano_edr