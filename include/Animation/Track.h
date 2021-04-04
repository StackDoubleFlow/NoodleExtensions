#pragma once
#include <map>
#include <string>
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Vector4.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "Animation/PointDefinitionInterpolation.h"

namespace Events {
    struct AnimateTrackContext;
}

enum struct PropertyType {
    vector3,
    vector4,
    quaternion,
    linear
};

union PropertyValue {
    float linear;
    UnityEngine::Vector3 vector3;
    UnityEngine::Vector4 vector4;
    UnityEngine::Quaternion quaternion;
};

struct Property {
    Property(PropertyType t, UnityEngine::Vector4 v) : type{t}, value{} { value.vector4 = v; };
    Property(PropertyType t, UnityEngine::Vector3 v) : type{t}, value{} { value.vector3 = v; };
    Property(PropertyType t, UnityEngine::Quaternion v) : type{t}, value{} { value.quaternion = v; };
    Property(PropertyType t, float v) : type{t}, value{v} {};
    PropertyType type;
    PropertyValue value;
    Events::AnimateTrackContext *coroutine;
};

struct PathProperty {
    PropertyType type;
    PointDefinitionInterpolation value;
};

class Properties {
public:
    Properties() : position{Property(PropertyType::vector3, UnityEngine::Vector3())},
                   rotation{Property(PropertyType::quaternion, UnityEngine::Quaternion())},
                   scale{Property(PropertyType::vector3, UnityEngine::Vector3(1, 1, 1))},
                   localRotation{Property(PropertyType::quaternion, UnityEngine::Quaternion())},
                   dissolve{Property(PropertyType::linear, 1)},
                   dissolveArrow{Property(PropertyType::linear, 1)},
                   time{Property(PropertyType::linear, UnityEngine::Quaternion())},
                   cuttable{Property(PropertyType::linear, 1)} {};
    Property *FindProperty(std::string name);
    
    Property position;
    Property rotation;
    Property scale;
    Property localRotation;
    Property dissolve;
    Property dissolveArrow;
    Property time;
    Property cuttable;
};

class PathProperties {
public:
    PathProperties() : position{PropertyType::vector3, PointDefinitionInterpolation()},
                       rotation{PropertyType::quaternion, PointDefinitionInterpolation()},
                       scale{PropertyType::vector3, PointDefinitionInterpolation()},
                       localRotation{PropertyType::quaternion, PointDefinitionInterpolation()},
                       definitePosition{PropertyType::linear, PointDefinitionInterpolation()},
                       dissolve{PropertyType::linear, PointDefinitionInterpolation()},
                       dissolveArrow{PropertyType::linear, PointDefinitionInterpolation()},
                       cuttable{PropertyType::linear, PointDefinitionInterpolation()} {};
    PathProperty *FindProperty(std::string name);

    PathProperty position;
    PathProperty rotation;
    PathProperty scale;
    PathProperty localRotation;
    PathProperty definitePosition;
    PathProperty dissolve;
    PathProperty dissolveArrow;
    PathProperty cuttable;
};

struct Track {
    Properties properties;
    PathProperties pathProperties;
};
