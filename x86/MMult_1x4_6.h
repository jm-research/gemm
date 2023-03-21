#define A(i, j) a[(i)*lda + (j)]
#define B(i, j) b[(i)*ldb + (j)]
#define C(i, j) c[(i)*ldc + (j)]

void AddDot1x4(int k, float* a, int lda, float* b, int ldb, float* c, int ldc) {
  float
    /* hold contributions to
       C( 0, 0 ), C( 0, 1 ), C( 0, 2 ), C( 0, 3 ) */
       c_00_reg,   c_01_reg,   c_02_reg,   c_03_reg,  
    /* holds A( 0, p ) */
       a_0p_reg;
  c_00_reg = 0.0;
  c_01_reg = 0.0;
  c_02_reg = 0.0;
  c_03_reg = 0.0;
  for (int p = 0; p < k; ++p) {
    a_0p_reg = A(0, p);
    c_00_reg += a_0p_reg * B(p, 0);
    c_01_reg += a_0p_reg * B(p, 1);
    c_02_reg += a_0p_reg * B(p, 2);
    c_03_reg += a_0p_reg * B(p, 3);
  }
  C(0, 0) = c_00_reg;
  C(0, 1) = c_01_reg;
  C(0, 2) = c_02_reg;
  C(0, 3) = c_03_reg;
}

void MatrixMul(int m, int n, int k, float* a, int lda, float* b, int ldb,
               float* c, int ldc) {
  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < n; j += 4) {
      AddDot1x4(k, &A(i, 0), lda, &B(0, j), ldb, &C(i, j), ldc);
    }
  }
}