// Tổng hợp 772 frames - 128x64 pixels
// @khoi2mai
#ifndef ALL_FRAMES_H
#define ALL_FRAMES_H

#define FRAME_SIZE 1024 // Para tela 128x64 (monocromática)
#include "frames/wifi_00.h"

// Mảng con trỏ đến tất cả các frame => PTBR = Array de ponteiros para todos os frames
const unsigned char* frames[1] = {
  wifi_00
};

// Mảng con trỏ đến tất cả các frame => PTBR = Array de ponteiros para todos os frames
const unsigned char* not_connet[1] = {
  wifi_00
};


#define TOTAL_FRAMES 1
#define FRAME_WIDTH 128
#define FRAME_HEIGHT 64

#endif // ALL_FRAMES_H
