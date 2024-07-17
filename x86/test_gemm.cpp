#include <string.h>

#include <chrono>
#include <cstdlib>
#include <iostream>

#include "MMult0.h"
#if 0
#include "MMult1.h"
#include "MMult2.h"
#include "MMult_1x4_3.h"
#include "MMult_1x4_4.h"
#include "MMult_1x4_5.h"
#else
#include "MMult_1x4_10.h"
#endif
#include "util.h"

int main() {
  struct timespec start, end;
  double time_used = 0.0;

  int m, n, k, lda, ldb, ldc;
  double gflops, time_tmp, time_best, diff;
  float *a, *b, *c, *prec, *nowc;

  for (int i = 40; i <= 1000; i += 40) {
    m = i;
    n = i;
    k = i;
    gflops = 2.0 * m * n * k * 1.0e-09;
    lda = m;
    ldb = k;
    ldc = m;
    a = (float *)malloc(lda * k * sizeof(float));
    b = (float *)malloc(ldb * n * sizeof(float));
    c = (float *)malloc(ldc * n * sizeof(float));
    prec = (float *)malloc(ldc * n * sizeof(float));
    nowc = (float *)malloc(ldc * n * sizeof(float));

    random_matrix(m, k, a, lda);
    random_matrix(k, n, b, ldb);
    random_matrix(m, n, prec, ldc);
    memset(prec, 0, ldc * n * sizeof(float));

    copy_matrix(m, n, prec, ldc, nowc, ldc);

    // std::cout << "a matrix: \n";
    // debug_print_2d_nums(a, 4, 4);
    // std::cout << "b matrix: \n";
    // debug_print_2d_nums(b, 4, 4);
    // std::cout << "c matrix: \n";
    // debug_print_2d_nums(c, 4, 4);

    MatrixMultiply(m, n, k, a, lda, b, ldb, nowc, ldc);

    // std::cout << "nowc matrix: \n";
    // debug_print_2d_nums(nowc, 4, 4);

    for (int j = 0; j < 20; ++j) {
      copy_matrix(m, n, prec, ldc, c, ldc);

      auto before = std::chrono::system_clock::now();
      MatrixMultiply(m, n, k, a, lda, b, ldb, c, ldc);
      auto after = std::chrono::system_clock::now();

      time_tmp = time_diff(before, after);
      //std::cout << "time_tmp: " << time_tmp << "\n";
      if (j == 0)
        time_best = time_tmp;
      else
        time_best = std::min(time_best, time_tmp);
    }

    diff = compare_matrices(m, n, c, ldc, nowc, ldc);

    if (diff > 0.5f || diff < -0.5f) {
      exit(0);
    }
    std::cout << i << " " << gflops / time_best  << " " << diff << "\n";
  }
}