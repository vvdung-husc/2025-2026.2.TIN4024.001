#ifndef ULTILS_H
#define ULTILS_H

#include "main.h"

// Tác vụ xử lý cảm biến
void readDHTTask(unsigned long now);

// Tác vụ xử lý hiệu ứng LED
void ledBlinkTask(unsigned long now);

// Tác vụ cập nhật màn hình
void oledTask(unsigned long now);

#endif