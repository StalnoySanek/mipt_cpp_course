#include "agent_rules.h"

#include <memory>
#include <string>
#include <vector>

namespace nano_edr
{
namespace
{
std::unique_ptr<ICondition> ScriptHosts()
{
    auto hosts = std::make_unique<AnyOf>();
    hosts->Add(std::make_unique<FieldEndsWith>("image", "wscript.exe"));
    hosts->Add(std::make_unique<FieldEndsWith>("image", "cscript.exe"));
    return hosts;
}

std::unique_ptr<ICondition> TemporaryPaths()
{
    auto paths = std::make_unique<AnyOf>();
    paths->Add(std::make_unique<FieldContains>("cmdline",
                                               "\\appdata\\local\\temp\\"));
    paths->Add(std::make_unique<FieldContains>("cmdline",
                                               "/appdata/local/temp/"));
    paths->Add(std::make_unique<FieldContains>("cmdline",
                                               "\\windows\\temp\\"));
    paths->Add(std::make_unique<FieldContains>("cmdline", "/windows/temp/"));
    paths->Add(std::make_unique<FieldContains>("cmdline", "%temp%"));
    paths->Add(std::make_unique<FieldContains>("cmdline", "%tmp%"));
    return paths;
}

std::unique_ptr<ICondition> StartupPaths(const std::string& field)
{
    auto paths = std::make_unique<AnyOf>();
    paths->Add(std::make_unique<FieldContains>(
        field, R"(\start menu\programs\startup\)"));
    paths->Add(std::make_unique<FieldContains>(
        field, R"(/start menu/programs/startup/)"));
    return paths;
}
}  // namespace

RuleEngine BuildAgentRules()
{
    RuleEngine engine;

    // ---------------------------------
    // ----- script_host_from_temp -----
    // ---------------------------------
    auto script_host_from_temp =
        std::make_unique<MatchRule>("script_host_from_temp", Severity::kHigh);
    script_host_from_temp->AddAction("kill_process");
    script_host_from_temp->AddCondition(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kProcessStart}));
    script_host_from_temp->AddCondition(ScriptHosts());
    script_host_from_temp->AddCondition(TemporaryPaths());
    engine.AddRule(std::move(script_host_from_temp));

    // ---------------------------
    // ----- lolbin_download -----
    // ---------------------------
    auto lolbin_download =
        std::make_unique<MatchRule>("lolbin_download", Severity::kHigh);
    lolbin_download->AddAction("kill_process");
    lolbin_download->AddCondition(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kProcessStart}));

    auto lolbins = std::make_unique<AnyOf>();
    lolbins->Add(std::make_unique<FieldEndsWith>("image", "certutil.exe"));
    lolbins->Add(std::make_unique<FieldEndsWith>("image", "bitsadmin.exe"));
    lolbin_download->AddCondition(std::move(lolbins));

    auto download_arguments = std::make_unique<AnyOf>();
    download_arguments->Add(
        std::make_unique<FieldContains>("cmdline", "urlcache"));
    download_arguments->Add(
        std::make_unique<FieldContains>("cmdline", "transfer"));
    download_arguments->Add(
        std::make_unique<FieldContains>("cmdline", "http:"));
    download_arguments->Add(
        std::make_unique<FieldContains>("cmdline", "https:"));
    lolbin_download->AddCondition(std::move(download_arguments));
    engine.AddRule(std::move(lolbin_download));

    // -----------------------------
    // ----- hidden_powershell -----
    // -----------------------------
    auto hidden_powershell =
        std::make_unique<MatchRule>("hidden_powershell", Severity::kMedium);
    hidden_powershell->AddAction("kill_process");
    hidden_powershell->AddCondition(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kProcessStart}));

    auto powershells = std::make_unique<AnyOf>();
    powershells->Add(
        std::make_unique<FieldEndsWith>("image", "powershell.exe"));
    powershells->Add(std::make_unique<FieldEndsWith>("image", "pwsh.exe"));
    hidden_powershell->AddCondition(std::move(powershells));

    auto hidden_arguments = std::make_unique<AnyOf>();
    hidden_arguments->Add(
        std::make_unique<FieldContains>("cmdline", "-w hidden"));
    hidden_arguments->Add(
        std::make_unique<FieldContains>("cmdline", "-windowstyle hidden"));
    hidden_arguments->Add(
        std::make_unique<FieldContains>("cmdline", "-enc"));
    hidden_arguments->Add(
        std::make_unique<FieldContains>("cmdline", "-encodedcommand"));
    hidden_powershell->AddCondition(std::move(hidden_arguments));
    engine.AddRule(std::move(hidden_powershell));

    // ---------------------------
    // ----- autostart_write -----
    // ---------------------------
    auto autostart_write =
        std::make_unique<MatchRule>("autostart_write", Severity::kHigh);
    autostart_write->AddAction("quarantine_file");
    autostart_write->AddCondition(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kFileCreate,
                               EventType::kFileWrite,
                               EventType::kFileMove}));

    auto autostart_paths = std::make_unique<AnyOf>();
    autostart_paths->Add(StartupPaths("path"));
    autostart_paths->Add(StartupPaths("to"));
    autostart_write->AddCondition(std::move(autostart_paths));
    engine.AddRule(std::move(autostart_write));

    // ----------------------------
    // ----- ransom_extension -----
    // ----------------------------
    auto ransom_extension =
        std::make_unique<MatchRule>("ransom_extension", Severity::kCritical);
    ransom_extension->AddAction("quarantine_file");
    ransom_extension->AddCondition(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kFileCreate,
                               EventType::kFileWrite,
                               EventType::kFileMove}));

    auto locked_paths = std::make_unique<AnyOf>();
    locked_paths->Add(std::make_unique<FieldEndsWith>("path", ".locked"));
    locked_paths->Add(std::make_unique<FieldEndsWith>("to", ".locked"));
    ransom_extension->AddCondition(std::move(locked_paths));
    engine.AddRule(std::move(ransom_extension));

    // ------------------------------
    // ----- script_host_beacon -----
    // ------------------------------
    auto script_host_beacon = std::make_unique<SequenceRule>(
        "script_host_beacon", Severity::kCritical, 10000);
    script_host_beacon->AddAction("kill_process");

    auto beacon_start = std::make_unique<AllOf>();
    beacon_start->Add(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kProcessStart}));
    beacon_start->Add(ScriptHosts());
    beacon_start->Add(TemporaryPaths());
    script_host_beacon->SetFirst(std::move(beacon_start));
    script_host_beacon->SetSecond(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kNetConnect}));
    engine.AddRule(std::move(script_host_beacon));

    return engine;
}
}  // namespace nano_edr
