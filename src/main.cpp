// It doesn't like it if i include this after
#include "NEConfig.h"

#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/stringbuffer.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/prettywriter.h"
#include "modloader/shared/modloader.hpp"
#include "custom-types/shared/logging.hpp"

#include "NEHooks.h"
#include "NELogger.h"

#include <string>
#include <iostream>

#include "System/Exception.hpp"
#include "GlobalNamespace/BeatmapData.hpp"

void PrintJSONValue(const rapidjson::Value &json) {
    using namespace rapidjson;

    StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);
    json.Accept(writer);
    auto str = sb.GetString();
    NELogger::GetLogger().info("%s", str);
}

extern "C" void setup(ModInfo &info) {
    info.id = "NoodleExtensions";
    info.version = VERSION;
    NELogger::modInfo = info;
    getNEConfig().Init(info);
}

extern "C" void load() {
    NELogger::GetLogger().info("Installing NoodleExtensions Hooks!");
    // This prevents any and all Utils logging
    // Logger::get().options.silent = false;

    // Install hooks

    InstallAndRegisterAll();

    NELogger::GetLogger().info("Installed NoodleExtensions Hooks!");
}
