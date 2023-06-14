#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>
#include "common.h"
#include <inttypes.h>

#define MAX_NUM 0x7FFFFFFF

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  long long c = (long long)a * b;
  FLOAT d = (FLOAT)(c >> 32);
  d = d << 16;
  return d;
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
  FLOAT p, q;
  asm volatile("idiv %2" : "=a"(p), "=d"(q) : "r"(b), "a"(a << 16), "d"(a >> 16));
  return p;
}

FLOAT f2F(float a) {
  /* You should figure out how to convert `a' into FLOAT without
   * introducing x87 floating point instructions. Else you can
   * not run this code in NEMU before implementing x87 floating
   * point instructions, which is contrary to our expectation.
   *
   * Hint: The bit representation of `a' is already on the
   * stack. How do you retrieve it to another variable without
   * performing arithmetic operations on it directly?
   */
  int b = *(int *)&a; // IEEE 754
  int sign = b >> 31; //取最高符号位
  int exp = (b & 0x7F800000) >> 23; //取指数位
  int frac = b & 0x7FFFFF; //取尾数位
  switch(exp){
    case 0:
      return 0;
    case 0xFF:
      if(sign)
        return -MAX_NUM;
      else
        return MAX_NUM;
    default:
      frac |= 0x800000; //高位补1，转化为规范浮点数
      exp -= 134; //IEEE 754减去127，转化为规范浮点数需要左移23位，转化为FOLAT要右移16位，故127+23-16=134
      if (exp < 0) frac >>= -exp;
	    else frac <<= exp;
      if (sign) return -frac;
      else return frac;
    }
}

FLOAT Fabs(FLOAT a) {
  return a >= 0 ? a : -a;
}

/* Functions below are already implemented */

FLOAT Fsqrt(FLOAT x) {
  FLOAT dt, t = int2F(2);
  do
  {
      dt = F_div_int((F_div_F(x, t) - t), 2);
      t += dt;
  } while (Fabs(dt) > f2F(1e-4));

  return t;
}

FLOAT Fpow(FLOAT x, FLOAT y) {
  /* we only compute x^0.333 */
  FLOAT t2, dt, t = int2F(2);
  do
  {
      t2 = F_mul_F(t, t);
      dt = (F_div_F(x, t2) - t) / 3;
      t += dt;
  } while (Fabs(dt) > f2F(1e-4));
  return t;
}
