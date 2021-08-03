#include "Animation/Track.h"

enum class PropertyName {
    position,
    rotation,
    scale,
    localRotation,
    definitePosition,
    dissolve,
    dissolveArrow,
    time,
    cuttable,
    color
};

Property *Properties::FindProperty(std::string name) {
    static std::unordered_map<std::string, PropertyName> const functions = {
        { "_position", PropertyName::position },
        { "_rotation", PropertyName::rotation },
        { "_scale", PropertyName::scale },
        { "_localRotation", PropertyName::localRotation },
        { "_dissolve", PropertyName::dissolve },
        { "_dissolveArrow", PropertyName::dissolveArrow },
        { "_time", PropertyName::time },
        { "_cuttable", PropertyName::cuttable },
        { "_color", PropertyName::color }
    };

    auto itr = functions.find(name);
    if (itr != functions.end()) {
        switch (itr->second) {
            case PropertyName::position: return &position;
            case PropertyName::rotation: return &rotation;
            case PropertyName::scale: return &scale;
            case PropertyName::localRotation: return &localRotation;
            case PropertyName::dissolve: return &dissolve;
            case PropertyName::dissolveArrow: return &dissolveArrow;
            case PropertyName::time: return &time;
            case PropertyName::cuttable: return &cuttable;
            case PropertyName::color: return &color;
            default: return nullptr;
        }
    } else {
        return nullptr;
    }
}

PathProperty *PathProperties::FindProperty(std::string_view name) {
    static std::unordered_map<std::string_view, PropertyName> const functions = {
        { "_position", PropertyName::position },
        { "_rotation", PropertyName::rotation },
        { "_scale", PropertyName::scale },
        { "_localRotation", PropertyName::localRotation },
        { "_definitePosition", PropertyName::definitePosition },
        { "_dissolve", PropertyName::dissolve },
        { "_dissolveArrow", PropertyName::dissolveArrow },
        { "_cuttable", PropertyName::cuttable },
        { "_color", PropertyName::color }
    };

    auto itr = functions.find(name);
    if (itr != functions.end()) {
        switch (itr->second) {
            case PropertyName::position: return &position;
            case PropertyName::rotation: return &rotation;
            case PropertyName::scale: return &scale;
            case PropertyName::localRotation: return &localRotation;
            case PropertyName::definitePosition: return &definitePosition;
            case PropertyName::dissolve: return &dissolve;
            case PropertyName::dissolveArrow: return &dissolveArrow;
            case PropertyName::cuttable: return &cuttable;
            case PropertyName::color: return &color;
            default: return nullptr;
        }
    } else {
        return nullptr;
    }
}

void Track::ResetVariables() {
    *this = Track();
}