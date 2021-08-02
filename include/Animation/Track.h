#pragma once
#include <map>
#include <string>
#include "Vector.h"
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
    NEVector::Vector3 vector3;
    NEVector::Vector4 vector4;
    NEVector::Quaternion quaternion;
};

struct Property {
    // Property(PropertyType t, UnityEngine::Vector4 v) : type{t}, value{{}} { value.value().vector4 = v; };
    // Property(PropertyType t, UnityEngine::Vector3 v) : type{t}, value{} { value.value().vector3 = v; };
    // Property(PropertyType t, UnityEngine::Quaternion v) : type{t}, value{} { value.value().quaternion = v; };
    // Property(PropertyType t, float v) : type{t}, value{v} {};
    Property(PropertyType t) : type{t}, value{std::nullopt} {};
    PropertyType type;
    std::optional<PropertyValue> value;
};

struct PathProperty {
    PathProperty(PropertyType t) : type{t}, value{std::nullopt} {};
    PropertyType type;
    std::optional<PointDefinitionInterpolation> value;
};

class Properties {
public:
    Properties() : position{Property(PropertyType::vector3)},
                   rotation{Property(PropertyType::quaternion)},
                   scale{Property(PropertyType::vector3)},
                   localRotation{Property(PropertyType::quaternion)},
                   dissolve{Property(PropertyType::linear)},
                   dissolveArrow{Property(PropertyType::linear)},
                   time{Property(PropertyType::linear)},
                   cuttable{Property(PropertyType::linear)} {};
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
    PathProperties() : position{PropertyType::vector3},
                       rotation{PropertyType::quaternion},
                       scale{PropertyType::vector3},
                       localRotation{PropertyType::quaternion},
                       definitePosition{PropertyType::linear},
                       dissolve{PropertyType::linear},
                       dissolveArrow{PropertyType::linear},
                       cuttable{PropertyType::linear} {};
    PathProperty *FindProperty(std::string_view name);

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
    void ResetVariables();
};
