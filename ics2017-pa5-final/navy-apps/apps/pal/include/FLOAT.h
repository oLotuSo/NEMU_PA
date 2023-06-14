#ifndef __FLOAT_H__
#define __FLOAT_H__

#include "assert.h"

typedef int FLOAT;

static inline int F2int(FLOAT a) {
  int temp = a & 0xffff0000;
  return temp >> 16;
}

static inline FLOAT int2F(int a) {
  return a << 16;
}

static inline FLOAT F_mul_int(FLOAT a, int b) {
  FLOAT c = a * b;
  return c;
}

static inline FLOAT F_div_int(FLOAT a, int b) {
  FLOAT c = a / b;
  return c;
}

FLOAT f2F(float);
FLOAT F_mul_F(FLOAT, FLOAT);
FLOAT F_div_F(FLOAT, FLOAT);
FLOAT Fabs(FLOAT);
FLOAT Fsqrt(FLOAT);
FLOAT Fpow(FLOAT, FLOAT);

#endif
