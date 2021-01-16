
#pragma once
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"

class NELogger {
public:
    static inline ModInfo modInfo = ModInfo();
    static Logger& GetLogger() {
        static auto logger = new Logger(modInfo, LoggerOptions(false, true));
        return *logger;
    }
};

void PrintJSONValue(const rapidjson::Value &json);