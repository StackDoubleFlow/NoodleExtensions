
#pragma once
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

namespace NoodleExtensions {
    static inline const std::u16string REQUIREMENTNAME = u"Noodle Extensions";
    static inline const std::string U8_REQUIREMENTNAME = "Noodle Extensions";

    static inline const std::string U8_ME_REQUIREMENTNAME = "Mapping Extensions";
    static inline const std::u16string ME_REQUIREMENTNAME = u"Mapping Extensions";
    // TODO: Move all JSON property references to constants?
}

class NELogger {
public:
    static inline ModInfo modInfo = ModInfo();
    static Logger& GetLogger() {
        static auto logger = new Logger(modInfo, LoggerOptions(false, true));
        return *logger;
    }
};

void PrintJSONValue(const rapidjson::Value &json);
void PrintBacktrace(size_t maxLines);