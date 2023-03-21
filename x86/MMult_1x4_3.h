#define A(i, j) a[(i)*lda + (j)]
#define B(i, j) b[(i)*ldb + (j)]
#define C(i, j) c[(i)*ldc + (j)]

#define Y(i) y[(i)*incx]

void AddDot(int k, float* x, int incx, float* y, float* gamma) {
  for (int p = 0; p < k; ++p) {
    *gamma += x[p] * Y(p);
  }
}

void AddDot1x4(int k, float* a, int lda, float* b, int ldb, float* c, int ldc) {
  AddDot(k, &A(0, 0), lda, &B(0, 0), &C(0, 0));
  AddDot(k, &A(0, 0), lda, &B(0, 1), &C(0, 1));
  AddDot(k, &A(0, 0), lda, &B(0, 2), &C(0, 2));
  AddDot(k, &A(0, 0), lda, &B(0, 3), &C(0, 3));
}

void MatrixMul(int m, int n, int k, float* a, int lda, float* b, int ldb,
               float* c, int ldc) {
  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < n; j += 4) {
      AddDot1x4(k, &A(i, 0), lda, &B(0, j), ldb, &C(i, j), ldc);
    }
  }
}