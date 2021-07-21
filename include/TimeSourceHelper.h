#pragma once

#include "GlobalNamespace/IAudioTimeSource.hpp"

namespace TimeSourceHelper {

float getSongTime(GlobalNamespace::IAudioTimeSource *timeSource);

} // end namespace TimeSourceHelper