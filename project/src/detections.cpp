#include "detection.h"

namespace nano_edr
{
std::string ToString(const Detection& detection)
{
    auto text = std::format("[DETECT] {}  {}  ts={} pid={}",
                            SeverityName(detection.severity),
                            detection.rule,
                            detection.ts.ms,
                            detection.pid);

    if (!detection.chain.empty())
        text += " chain=" + detection.chain;

    return text;
}
}  // namespace nano_edr
