#pragma once
#include "config-utils/shared/config-utils.hpp"

#include "beatsaber-hook/shared/utils/typedefs-string.hpp"

enum struct MaterialBehaviour {
    SMART_COLOR,
    SEMI_BASIC,
    BASIC
};

inline std::vector<StringW> getMaterialBehaviourValues() {
    return {{"Smart Color", "SemiBasic", "Basic"}};
}

DECLARE_CONFIG(NEConfig, 
    CONFIG_VALUE(enableNoteDissolve, bool, "Enable note dissolve", true);
    CONFIG_VALUE(enableObstacleDissolve, bool, "Enable obstacle dissolve", true);
    CONFIG_VALUE(qosmeticsModelDisable, bool, "Disable Qosmetics models on NE maps", true, "If enabled, NE will disable qosmetics walls and notes to improve performance");
    CONFIG_VALUE(materialBehaviour, int, "Obstacle material behaviour", 0);


    CONFIG_INIT_FUNCTION(
        CONFIG_INIT_VALUE(enableNoteDissolve);
        CONFIG_INIT_VALUE(enableObstacleDissolve);
        CONFIG_INIT_VALUE(qosmeticsModelDisable);
        CONFIG_INIT_VALUE(materialBehaviour);
    )
)
