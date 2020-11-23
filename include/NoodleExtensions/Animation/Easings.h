#pragma once

namespace NoodleExtensions::Animation {

enum struct Functions {
    easeLinear,
    easeStep,
    easeInQuad,
    easeOutQuad,
    easeInOutQuad,
    easeInCubic,
    easeOutCubic,
    easeInOutCubic,
    easeInQuart,
    easeOutQuart,
    easeInOutQuart,
    easeInQuint,
    easeOutQuint,
    easeInOutQuint,
    easeInSine,
    easeOutSine,
    easeInOutSine,
    easeInCirc,
    easeOutCirc,
    easeInOutCirc,
    easeInExpo,
    easeOutExpo,
    easeInOutExpo,
    easeInElastic,
    easeOutElastic,
    easeInOutElastic,
    easeInBack,
    easeOutBack,
    easeInOutBack,
    easeInBounce,
    easeOutBounce,
    easeInOutBounce,
};

float Interpolate(float p, Functions function);

} // end namespace NoodleExtensions::Animation