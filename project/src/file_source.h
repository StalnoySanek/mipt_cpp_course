#pragma once
#include <fstream>
#include <string>

#include "event_source.h"

namespace nano_edr
{
class FileSource : public IEventSource
{
 public:
    explicit FileSource(const std::string& path);
    FileSource(const FileSource&) = delete;
    FileSource& operator=(const FileSource&) = delete;

    void Run(IEventHandler* handler) override;

 private:
    std::ifstream log_;
};
}  // namespace nano_edr
