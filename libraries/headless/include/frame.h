#pragma once

#include "videostream.h"
#include <memory>

using VSLFramePtr = std::unique_ptr<VSLFrame, decltype(&vsl_frame_release)>;