
#pragma once
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"

class NELogger {
public:
    static inline ModInfo modInfo = ModInfo();
    static const Logger& GetLogger()
    {
        static const Logger& log(modInfo);
        return log;
    }
};

void PrintJSONValue(const rapidjson::Value &json);