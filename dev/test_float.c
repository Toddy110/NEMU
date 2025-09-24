#include <stdio.h>
#include <stdint.h>
#include "lib-common/FLOAT.h"
#include "lib-common/FLOAT/FLOAT.c"

static inline FLOAT f(FLOAT x) {
  return F_div_F(int2F(1), int2F(1) + F_mul_int(F_mul_F(x, x), 25));
}

static FLOAT computeT(int n, FLOAT a, FLOAT b, FLOAT (*fun)(FLOAT)) {
  int k;
  FLOAT s,h;
  h = F_div_int((b - a), n);
  s = F_div_int(fun(a) + fun(b), 2 );
  for(k = 1; k < n; k ++) {
    s += fun(a + F_mul_int(h, k));
  }
  s = F_mul_F(s, h);
  return s;
}

static double F_to_double(FLOAT x){
  return (double)x / 65536.0;
}

int main(){
  FLOAT a = computeT(10, f2F(-1.0f), f2F(1.0f), f);
  double ad = F_to_double(a);
  double ans = 0.551222;
  printf("a(FIX)=%f  err=%e\n", ad, ad-ans);
  return 0;
}
