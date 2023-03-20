#include <stdlib.h>

#include <chrono>
#include <iostream>
#include <random>

#define A(i, j) a[(i)*lda + (j)]
#define B(i, j) b[(i)*ldb + (j)]
#define abs(x) ((x) < 0.0 ? -(x) : (x))

void random_matrix(int m, int n, float* a, int lda) {
  double drand48();

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> distrib(0, 10);

  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < n; ++j) {
      A(i, j) = (float)distrib(gen);
    }
  }
}

void copy_matrix(int m, int n, float* a, int lda, float* b, int ldb) {
  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < n; ++j) {
      B(i, j) = A(i, j);
    }
  }
}

float compare_matrices(int m, int n, float* a, int lda, float* b, int ldb) {
  float max_diff = 0.0, diff;
  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < n; ++j) {
      diff = abs(A(i, j) - B(i, j));
      max_diff = std::max(diff, max_diff);

      if (max_diff > 0.5f || max_diff < -0.5f) {
        std::cout << "\n error: i " << i << " j " << j << " diff" << diff;
      }
    }
  }
  return max_diff;
}

static inline void debug_print_2d_nums(void* nums, int rows, int cols) {
  float* arr = (float*)nums;

  for (int row = 0; row < rows; row++) {
    float* arr = (float*)nums;

    for (int col = 0; col < cols; col++) {
      std::cout << " " << arr[row * rows + col] << " ";
    }
    std::cout << "\n";
  }
}

static double time_diff(const std::chrono::system_clock::time_point& before,
                        const std::chrono::system_clock::time_point& after) {
  auto diff = after - before;
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
  return duration.count();
}