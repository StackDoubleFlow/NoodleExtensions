#pragma once
#include "config-utils/shared/config-utils.hpp"

#include "beatsaber-hook/shared/utils/typedefs-string.hpp"

enum struct MaterialBehaviour {
    SMART_COLOR = 0,
    SEMI_BASIC = 1,
    BASIC = 2
};

inline std::vector<StringW> getMaterialBehaviourValues() {
    return {{"Smart Color", "SemiBasic", "Basic"}};
}

DECLARE_CONFIG(NEConfig, 
    CONFIG_VALUE(enableNoteDissolve, bool, "Enable note dissolve", true);
    CONFIG_VALUE(enableMirrorNoteDissolve, bool, "Enable mirror note dissolve", true, "If enabled, allows note mirrors to dissolve. When disabled, hides the notes if dissolved");
    CONFIG_VALUE(enableObstacleDissolve, bool, "Enable obstacle dissolve", true);
    CONFIG_VALUE(qosmeticsModelDisable, bool, "Disable Qosmetics models on NE maps", true, "If enabled, NE will disable qosmetics walls and notes to improve performance");
    CONFIG_VALUE(materialBehaviour, int, "Obstacle material behaviour", 0);


    CONFIG_INIT_FUNCTION(
        CONFIG_INIT_VALUE(enableNoteDissolve);
        CONFIG_INIT_VALUE(enableMirrorNoteDissolve);
        CONFIG_INIT_VALUE(enableObstacleDissolve);
        CONFIG_INIT_VALUE(qosmeticsModelDisable);
        CONFIG_INIT_VALUE(materialBehaviour);
    )
)
