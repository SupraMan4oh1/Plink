#pragma once

/**
@file KyaniteConstants.h
@brief Constants used within the Kyanite library.

@note Kyanite constants should only be values that need to be or should be set at the time the library 
is compiled. Usually these are values that affect cross-platform or cross-hardware compatability.
*/

#include <cstddef>
#include <string>

#include "AL/alure.h"

/** @brief The maximum number of concurrent audio sources allowed.

Even if more concurrent sources are reported by the audio lib as supported, we limit it to this value. 
This is enforced because certain systems may report an erroneously high number of sources, but fail to function properly 
long before the reported number is reached. */
static const ALCint MAX_AUDIO_SOURCES = 256;
