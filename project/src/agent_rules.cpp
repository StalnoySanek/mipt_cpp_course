#include "agent_rules.h"

#include <algorithm>
#include <array>

#include "fields.h"

namespace nano_edr
{
std::vector<MatchRule> BuildAgentRules()
{
    // ---------------------------------
    // ----- script_host_from_temp -----
    // ---------------------------------
    MatchRule script_host_from_temp("script_host_from_temp", Severity::kHigh);
    script_host_from_temp.AddAction("kill_process");
    script_host_from_temp.AddCondition(Condition::Equals("type", {"process_start"}));
    script_host_from_temp.AddCondition(Condition::EndsWith("image", {"wscript.exe",
                                                                     "cscript.exe"}));
    script_host_from_temp.AddCondition(Condition::Contains("cmdline", {"\\appdata\\local\\temp\\",
                                                                       "/appdata/local/temp/",
                                                                       "\\windows\\temp\\",
                                                                       "/windows/temp/",
                                                                       "%temp%",
                                                                       "%tmp%"}));

    // ---------------------------
    // ----- lolbin_download -----
    // ---------------------------
    MatchRule lolbin_download("lolbin_download", Severity::kHigh);
    lolbin_download.AddAction("kill_process");
    lolbin_download.AddCondition(Condition::Equals("type", {"process_start"}));
    lolbin_download.AddCondition(Condition::EndsWith("image", {"certutil.exe",
                                                               "bitsadmin.exe"}));
    lolbin_download.AddCondition(Condition::Contains("cmdline", {"urlcache", "transfer",
                                                                 "http:",
                                                                 "https:"}));

    // -----------------------------
    // ----- hidden_powershell -----
    // -----------------------------
    MatchRule hidden_powershell("hidden_powershell", Severity::kMedium);
    hidden_powershell.AddAction("kill_process");
    hidden_powershell.AddCondition(Condition::Equals("type", {"process_start"}));
    hidden_powershell.AddCondition(Condition::EndsWith("image", {"powershell.exe",
                                                                 "pwsh.exe"}));
    hidden_powershell.AddCondition(Condition::Contains("cmdline", {"-w hidden",
                                                                   "-windowstyle hidden",
                                                                   "-enc",
                                                                   "encodedcommand"}));

    // --------------------------------
    // ----- autostart_write_path -----
    // --------------------------------
    MatchRule autostart_write_path("autostart_write", Severity::kHigh);
    autostart_write_path.AddAction("quarantine_file");
    autostart_write_path.AddCondition(Condition::Equals("type", {"file_create",
                                                                 "file_write"}));
    autostart_write_path.AddCondition(Condition::Contains("path", {R"(\start menu\programs\startup\)",
                                                                   R"(/start menu/programs/startup/)"}));

    // --------------------------------
    // ----- autostart_write_to -----
    // --------------------------------
    MatchRule autostart_write_to("autostart_write", Severity::kHigh);
    autostart_write_to.AddAction("quarantine_file");
    autostart_write_to.AddCondition(Condition::Equals("type", {"file_move"}));
    autostart_write_to.AddCondition(Condition::Contains("to", {R"(\start menu\programs\startup\)",
                                                               R"(/start menu/programs/startup/)"}));

    // ---------------------------------
    // ----- ransom_extension_path -----
    // ---------------------------------
    MatchRule ransom_extension_path("ransom_extension", Severity::kCritical);
    ransom_extension_path.AddAction("quarantine_file");
    ransom_extension_path.AddCondition(Condition::Equals("type", {"file_create",
                                                                  "file_write"}));
    ransom_extension_path.AddCondition(Condition::EndsWith("path", {".locked"}));

    // -------------------------------
    // ----- ransom_extension_to -----
    // -------------------------------
    MatchRule ransom_extension_to("ransom_extension", Severity::kCritical);
    ransom_extension_to.AddAction("quarantine_file");
    ransom_extension_to.AddCondition(Condition::Equals("type", {"file_move"}));
    ransom_extension_to.AddCondition(Condition::EndsWith("to", {".locked"}));

    return {
        script_host_from_temp,
        lolbin_download,
        hidden_powershell,
        autostart_write_path,
        autostart_write_to,
        ransom_extension_path,
        ransom_extension_to,
    };
}
}  // namespace nano_edr