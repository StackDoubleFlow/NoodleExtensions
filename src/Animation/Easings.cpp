#include "Animation/Easings.h"
#include <cmath>

using namespace NoodleExtensions::Animation;

float EaseLinear(float p) {
    return p;
}

float EaseStep(float p) {
    return floorf(p);
}

float EaseOutQuad(float p) {
    return -(p * (p - 2));
}

float EaseInQuad(float p) {
    return p * p;
}

float EaseInOutQuad(float p) {
    if (p < 0.5f) {
        return 2 * p * p;
    } else {
        return (-2 * p * p) + (4 * p) - 1;
    }
}

float EaseInCubic(float p) {
    return p * p * p;
}

float EaseOutCubic(float p) {
    float f = p - 1;
    return (f * f * f) + 1;
}

float EaseInOutCubic(float p) {
    if (p < 0.5f) {
        return 4 * p * p * p;
    } else {
        float f = (2 * p) - 2;
        return (0.5f * f * f * f) + 1;
    }
}

float EaseInQuart(float p) {
    return p * p * p * p;
}

float EaseOutQuart(float p) {
    float f = p - 1;
    return (f * f * f * (1 - p)) + 1;
}

float EaseInOutQuart(float p) {
    if (p < 0.5f) {
        return 8 * p * p * p * p;
    } else {
        float f = p - 1;
        return (-8 * f * f * f * f) + 1;
    }
}

float EaseInQuint(float p) {
    return p * p * p * p * p;
}

float EaseOutQuint(float p) {
    float f = p - 1;
    return (f * f * f * f * f) + 1;
}

float EaseInOutQuint(float p) {
    if (p < 0.5f) {
        return 16 * p * p * p * p * p;
    } else {
        float f = (2 * p) - 2;
        return (0.5f * f * f * f * f * f) + 1;
    }
}

float EaseInSine(float p) {
    return sinf((p - 1) * M_PI_2) + 1;
}

float EaseOutSine(float p) {
    return sinf(p * M_PI_2);
}

float EaseInOutSine(float p) {
    return 0.5f * (1 - cosf(p * M_PI));
}

float EaseInCirc(float p) {
    return 1 - sqrtf(1 - (p * p));
}

float EaseOutCirc(float p) {
    return sqrtf((2 - p) * p);
}

float EaseInOutCirc(float p) {
    if (p < 0.5f) {
        return 0.5f * (1 - sqrtf(1 - (4 * (p * p))));
    } else {
        return 0.5f * (sqrtf(-((2 * p) - 3) * ((2 * p) - 1)) + 1);
    }
}

float EaseInExpo(float p) {
    return (p == 0.0f) ? p : powf(2, 10 * (p - 1));
}

float EaseOutExpo(float p) {
    return (p == 1.0f) ? p : 1 - powf(2, -10 * p);
}

float EaseInOutExpo(float p) {
    if (p == 0.0 || p == 1.0) {
        return p;
    }

    if (p < 0.5f) {
        return 0.5f * powf(2, (20 * p) - 10);
    } else {
        return (-0.5f * powf(2, (-20 * p) + 10)) + 1;
    }
}

float EaseInElastic(float p) {
    return sinf(13 * M_PI_2 * p) * powf(2, 10 * (p - 1));
}

float EaseOutElastic(float p) {
    return (sinf(-13 * M_PI_2 * (p + 1)) * powf(2, -10 * p)) + 1;
}

float EaseInOutElastic(float p) {
    if (p < 0.5f) {
        return 0.5f * sinf(13 * M_PI_2 * (2 * p)) * powf(2, 10 * ((2 * p) - 1));
    } else {
        return 0.5f * ((sinf(-13 * M_PI_2 * (2 * p)) * powf(2, -10 * ((2 * p) - 1))) + 2);
    }
}

float EaseInBack(float p) {
    return (p * p * p) - (p * sinf(p * M_PI));
}

float EaseOutBack(float p) {
    float f = 1 - p;
    return 1 - ((f * f * f) - (f * sinf(f * M_PI)));
}

float EaseInOutBack(float p) {
    if (p < 0.5f)
    {
        float f = 2 * p;
        return 0.5f * ((f * f * f) - (f * sinf(f * M_PI)));
    }
    else
    {
        float f = 1 - ((2 * p) - 1);
        return (0.5f * (1 - ((f * f * f) - (f * sinf(f * M_PI))))) + 0.5f;
    }
}

float EaseOutBounce(float p) {
    if (p < 4 / 11.0f) {
        return 121 * p * p / 16.0f;
    } else if (p < 8 / 11.0f) {
        return (363 / 40.0f * p * p) - (99 / 10.0f * p) + (17 / 5.0f);
    } else if (p < 9 / 10.0f) {
        return (4356 / 361.0f * p * p) - (35442 / 1805.0f * p) + (16061 / 1805.0f);
    } else {
        return (54 / 5.0f * p * p) - (513 / 25.0f * p) + (268 / 25.0f);
    }
}

float EaseInBounce(float p) {
    return 1 - EaseOutBounce(1 - p);
}

float EaseInOutBounce(float p) {
    if (p < 0.5f) {
        return 0.5f * EaseInBounce(p * 2);
    } else {
        return (0.5f * EaseOutBounce((p * 2) - 1)) + 0.5f;
    }
}

float NoodleExtensions::Animation::Interpolate(float p, Functions function) {
    switch (function) {
        default:
        case Functions::easeLinear: return EaseLinear(p);
        case Functions::easeStep: return EaseStep(p);
        case Functions::easeOutQuad: return EaseOutQuad(p);
        case Functions::easeInQuad: return EaseInQuad(p);
        case Functions::easeInOutQuad: return EaseInOutQuad(p);
        case Functions::easeInCubic: return EaseInCubic(p);
        case Functions::easeOutCubic: return EaseOutCubic(p);
        case Functions::easeInOutCubic: return EaseInOutCubic(p);
        case Functions::easeInQuart: return EaseInQuart(p);
        case Functions::easeOutQuart: return EaseOutQuart(p);
        case Functions::easeInOutQuart: return EaseInOutQuart(p);
        case Functions::easeInQuint: return EaseInQuint(p);
        case Functions::easeOutQuint: return EaseOutQuint(p);
        case Functions::easeInOutQuint: return EaseInOutQuint(p);
        case Functions::easeInSine: return EaseInSine(p);
        case Functions::easeOutSine: return EaseOutSine(p);
        case Functions::easeInOutSine: return EaseInOutSine(p);
        case Functions::easeInCirc: return EaseInCirc(p);
        case Functions::easeOutCirc: return EaseOutCirc(p);
        case Functions::easeInOutCirc: return EaseInOutCirc(p);
        case Functions::easeInExpo: return EaseInExpo(p);
        case Functions::easeOutExpo: return EaseOutExpo(p);
        case Functions::easeInOutExpo: return EaseInOutExpo(p);
        case Functions::easeInElastic: return EaseInElastic(p);
        case Functions::easeOutElastic: return EaseOutElastic(p);
        case Functions::easeInOutElastic: return EaseInOutElastic(p);
        case Functions::easeInBack: return EaseInBack(p);
        case Functions::easeOutBack: return EaseOutBack(p);
        case Functions::easeInOutBack: return EaseInOutBack(p);
        case Functions::easeInBounce: return EaseInBounce(p);
        case Functions::easeOutBounce: return EaseOutBounce(p);
        case Functions::easeInOutBounce: return EaseInOutBounce(p);
    }
}