#pragma once
#include "config-utils/shared/config-utils.hpp"
// #include <unordered_map>
// #include <string>

DECLARE_CONFIG(NEConfig, 
    CONFIG_VALUE(enableNoteDissolve, bool, "Enable note dissolve", true);
    CONFIG_VALUE(enableObstacleDissolve, bool, "Enable obstacle dissolve", false);

    CONFIG_INIT_FUNCTION(
        CONFIG_INIT_VALUE(enableNoteDissolve);
        CONFIG_INIT_VALUE(enableObstacleDissolve);
    )
)
