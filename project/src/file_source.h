#pragma once
#include <string>

#include "agent.h"

namespace nano_edr
{
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
}  // namespace nano_edr
