// Tổng hợp 772 frames - 128x64 pixels
// @khoi2mai
#ifndef ALL_FRAMES_H
#define ALL_FRAMES_H

#include "frames/frame_00.h"


// Mảng con trỏ đến tất cả các frame
const unsigned char* frames[24] = {
  frame_00
};

#define TOTAL_FRAMES 24
#define FRAME_WIDTH 128
#define FRAME_HEIGHT 64

#endif // ALL_FRAMES_H
