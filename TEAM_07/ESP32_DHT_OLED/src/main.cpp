/*
THÔNG TIN NHÓM 07
1. Nguyễn Văn Phong
2. Hồ Văn Diễn
3. Lương Thanh Ngọc Như
4. Nguyễn Văn Tiến Dạt
5. Bùi Khắc Hiếu
*/
#include <Arduino.h>

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}