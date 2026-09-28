#include "agent_rules.h"

#include <algorithm>
#include <array>

#include "fields.h"

#define all(x) x.begin(), x.end()

namespace nano_edr
{

bool CommandLineContainsPath(const Event& event, const std::string& path)
{
    const auto cmd = FindField(event, "cmdline");
    if (!cmd)
        return false;
    return NormalizePath(*cmd).contains(NormalizePath(path));
}

bool ImageIs(const Event& event, const std::string& image)
{
    const std::string this_image = NormalizePath(GetRequiredField(event, "image"));
    return this_image == image || this_image.ends_with("\\" + image);
}

bool ScriptHostFromTemp(const Event& event)
{
    if (!IsProcessStart(event))
        return false;
    if (!ImageIs(event, "wscript.exe") &&
        !ImageIs(event, "cscript.exe"))
        return false;
    if (!CommandLineContainsPath(event, R"(\appdata\local\temp\)") &&
        !CommandLineContainsPath(event, R"(\windows\temp\)"))
        return false;
    return true;
}

bool LolbinDownload(const Event& event)
{
    if (!IsProcessStart(event))
        return false;
    if (!ImageIs(event, "certutil.exe") &&
        !ImageIs(event, "bitsadmin.exe"))
        return false;
    if (!CommandLineContains(event, "urlcache") &&
        !CommandLineContains(event, "transfer") &&
        !CommandLineContains(event, "http:") &&
        !CommandLineContains(event, "https:"))
        return false;
    return true;
}

bool HiddenPowershell(const Event& event)
{
    if (!IsProcessStart(event))
        return false;
    if (!ImageIs(event, "powershell.exe") &&
        !ImageIs(event, "pwsh.exe"))
        return false;
    if (!CommandLineContains(event, "-w hidden") &&
        !CommandLineContains(event, "-windowstyle hidden") &&
        !CommandLineContains(event, "-enc") &&
        !CommandLineContains(event, "-encodedcommand"))
        return false;
    return true;
}

bool AutostartWrite(const Event& event)
{
    if (event.type != "file_create" &&
        event.type != "file_move" &&
        event.type != "file_write")
        return false;
    const auto path = FindField(event, event.type == "file_move"
                                           ? "to"
                                           : "path");
    return path && NormalizePath(*path).contains(R"(\start menu\programs\startup\)");
}

bool RansomExtension(const Event& event)
{
    if (event.type != "file_create" &&
        event.type != "file_move" &&
        event.type != "file_write")
        return false;
    const auto path = FindField(event, event.type == "file_move"
                                           ? "to"
                                           : "path");
    return path && NormalizePath(*path).ends_with(".locked");
}

constexpr Rule kRules[] = {
    {"script_host_from_temp", ScriptHostFromTemp, Severity::kHigh},
    {"lolbin_download", LolbinDownload, Severity::kHigh},
    {"hidden_powershell", HiddenPowershell, Severity::kMedium},
    {"autostart_write", AutostartWrite, Severity::kHigh},
    {"ransom_extension", RansomExtension, Severity::kCritical},
};

const Rule* AgentRules() { return kRules; }
size_t AgentRuleCount() { return std::size(kRules); }
}  // namespace nano_edr