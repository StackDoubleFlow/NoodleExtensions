
#pragma once
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

namespace NoodleExtensions {
static inline const std::u16string REQUIREMENTNAME = u"Noodle Extensions";
static inline const std::string U8_REQUIREMENTNAME = "Noodle Extensions";

static inline const std::string U8_ME_REQUIREMENTNAME = "Mapping Extensions";
static inline const std::u16string ME_REQUIREMENTNAME = u"Mapping Extensions";
// TODO: Move all JSON property references to constants?
} // namespace NoodleExtensions

class NELogger {
public:
  static Logger& GetLogger() {
    static auto logger = new Logger(modloader::ModInfo{"NoodleExtensions", VERSION, 0}, LoggerOptions(false, true));
    return *logger;
  }
};

void PrintJSONValue(rapidjson::Value const& json);
void PrintBacktrace(size_t maxLines);