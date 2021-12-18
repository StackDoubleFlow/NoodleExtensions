#pragma once
#include "config-utils/shared/config-utils.hpp"

DECLARE_CONFIG(NEConfig, 
    CONFIG_VALUE(enableNoteDissolve, bool, "Enable note dissolve", true);
    CONFIG_VALUE(enableObstacleDissolve, bool, "Enable obstacle dissolve", true);

    CONFIG_INIT_FUNCTION(
        CONFIG_INIT_VALUE(enableNoteDissolve);
        CONFIG_INIT_VALUE(enableObstacleDissolve);
    )
)
