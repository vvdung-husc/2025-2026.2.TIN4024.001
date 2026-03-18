/*
THÔNG TIN NHÓM 5
1. Nguyễn Công Hiếu
2. Phạm Đức Thành Đạt
3.Trần Văn Tiến
4.
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
