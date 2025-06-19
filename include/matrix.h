// =============================================================================
// Matrix header
//
// -----------------------------------------------------------------------------
// Yongrae Jo, 0727ggame@sju.ac.kr
// =============================================================================

#ifndef MATRIX_H
#define MATRIX_H

#ifdef __cplusplus
extern "C" {
#endif

// Standard library
#include <stdbool.h>    // for bool, true or false

// GNSS library
#include "types.h"      // for type definitions

// =============================================================================
// Inline functions
// =============================================================================

// Matrix get/set functions (Int) (Caution: it can not check the index validity)
static inline int MatGetI(const mat_t *mat, int row, int col) {
    return ((int *)(mat->data))[row + col * mat->rows];
}

static inline void MatSetI(mat_t *mat, int row, int col, int val) {
    ((int *)(mat->data))[row + col * mat->rows] = val;
}

// Matrix get/set functions (Double) (Caution: it can not check the index validity)
static inline double MatGetD(const mat_t *mat, int row, int col) {
    return ((double *)(mat->data))[row + col * mat->rows];
}

static inline void MatSetD(mat_t *mat, int row, int col, double val) {
    ((double *)(mat->data))[row + col * mat->rows] = val;
}

// Index vector get/set functions (Int) (Caution: it can not check the index validity)
static inline int IdxGetI(const idx_t *idx, int i) {
    return ((int *)(idx->idx))[i];
}

static inline void IdxSetI(idx_t *idx, int i, int idxi) {
    ((int *)(idx->idx))[i] = idxi;
}

// Index vector get/set functions (Bool) (Caution: it can not check the index validity)
static inline bool IdxGetB(const idx_t *idx, int i) {
    return ((bool *)(idx->idx))[i];
}

static inline void IdxSetB(idx_t *idx, int i, bool val) {
    ((bool *)(idx->idx))[i] = val;
}

// =============================================================================
// Basic matrix/vector operations
// =============================================================================

// -----------------------------------------------------------------------------
// Matrix generation (with SIMD alignment 32 bytes, column major)
//
// args:
//        int   rows    (I) : number of rows (>= 0)
//        int   cols    (I) : number of columns (>= 0)
//        type_t type   (I) : matrix data type (DOUBLE or INT only)
//
// return:
//        mat_t *mat    (O) : allocated matrix structure (NULL if error)
// -----------------------------------------------------------------------------
mat_t *Mat(int rows, int cols, type_t type);

// -----------------------------------------------------------------------------
// Matrix memory deallocation
//
// args:
//        mat_t *mat    (I) : matrix structure to deallocate
//
// return:
//        void          (-) : no return value
// -----------------------------------------------------------------------------
void FreeMat(mat_t *mat);

// -----------------------------------------------------------------------------
// Index vector generation (with SIMD alignment 32 bytes)
//
// args:
//        int   n       (I) : number of indices (>= 0)
//        type_t type   (I) : index data type (BOOL or INT)
//
// return:
//        idx_t *idx    (O) : allocated index vector structure (NULL if error)
// -----------------------------------------------------------------------------
idx_t *Idx(int n, type_t type);

// -----------------------------------------------------------------------------
// Index vector memory deallocation
//
// args:
//        idx_t *idx    (I) : index vector structure to deallocate
//
// return:
//        void          (-) : no return value
// -----------------------------------------------------------------------------
void FreeIdx(idx_t *idx);

// -----------------------------------------------------------------------------
// Identity matrix generation
//
// args:
//        int   size    (I) : matrix size (size x size, >= 0)
//        type_t type   (I) : matrix data type (DOUBLE or INT only)
//
// return:
//        mat_t *I      (O) : identity matrix (NULL if error)
// -----------------------------------------------------------------------------
mat_t *Eye(int size, type_t type);

// -----------------------------------------------------------------------------
// Zeros matrix generation
//
// args:
//        int   rows    (I) : number of rows (>= 0)
//        int   cols    (I) : number of columns (>= 0)
//        type_t type   (I) : matrix data type (DOUBLE or INT only)
//
// return:
//        mat_t *Z      (O) : zero matrix (NULL if error)
// -----------------------------------------------------------------------------
mat_t *Zeros(int rows, int cols, type_t type);

// -----------------------------------------------------------------------------
// Ones matrix generation
//
// args:
//        int   rows    (I) : number of rows (>= 0)
//        int   cols    (I) : number of columns (>= 0)
//        type_t type   (I) : matrix data type (DOUBLE or INT only)
//
// return:
//        mat_t *O      (O) : ones matrix (NULL if error)
// -----------------------------------------------------------------------------
mat_t *Ones(int rows, int cols, type_t type);

// -----------------------------------------------------------------------------
// True index vector generation
//
// args:
//        int   n       (I) : number of indices (>= 0)
//
// return:
//        idx_t *idx    (O) : true index vector (NULL if error)
// -----------------------------------------------------------------------------
idx_t *TrueIdx(int n);

// -----------------------------------------------------------------------------
// False index vector generation
//
// args:
//        int   n       (I) : number of indices (>= 0)
//
// return:
//        idx_t *idx    (O) : false index vector (NULL if error)
// -----------------------------------------------------------------------------
idx_t *FalseIdx(int n);

// =============================================================================
// Matrix operations
// =============================================================================

// -----------------------------------------------------------------------------
// Matrix copy - returns new matrix with copied data
//
// args:
//  const mat_t *A      (I) : source matrix
//
// return:
//        mat_t *B      (O) : copied matrix (NULL if error)
// -----------------------------------------------------------------------------
mat_t *MatCopy(const mat_t *A);

// -----------------------------------------------------------------------------
// Matrix in-place copy - copies source matrix data to existing destination matrix
//
// args:
//        mat_t *des    (O) : destination matrix
//  const mat_t *src    (I) : source matrix
//
// return:
//        int   info    (O) : 1 if successful, 0 if failed
// -----------------------------------------------------------------------------
int MatCopyIn(mat_t *des, const mat_t *src);

// -----------------------------------------------------------------------------
// Matrix vector indexing
//
// args:
//        mat_t *mat    (I) : matrix
//        idx_t *ridx   (I) : row index vector
//        idx_t *cidx   (I) : column index vector
//
// return:
//        mat_t *mat    (O) : matrix (NULL if error)
// -----------------------------------------------------------------------------
mat_t *MatVecIdx(const mat_t *mat, const idx_t *ridx, const idx_t *cidx);

// -----------------------------------------------------------------------------
// Matrix in-place vector indexing
//
// args:
//        mat_t *mat    (I,O) : matrix
//        idx_t *ridx   (I)   : row index vector
//        idx_t *cidx   (I)   : column index vector
//
// return:
//        int   info    (O)   : 1 if successful, 0 if failed
// -----------------------------------------------------------------------------
int MatVecIdxIn(mat_t *mat, const idx_t *ridx, const idx_t *cidx);

// -----------------------------------------------------------------------------
// Matrix logical indexing
//
// args:
//        mat_t *mat    (I) : matrix
//        idx_t *ridx   (I) : row index vector
//        idx_t *cidx   (I) : column index vector
//
// return:
//        mat_t *mat    (O) : matrix (NULL if error)
// -----------------------------------------------------------------------------
mat_t *MatLogIdx(const mat_t *mat, const idx_t *ridx, const idx_t *cidx);

// -----------------------------------------------------------------------------
// Matrix in-place logical indexing
//
// args:
//        mat_t *mat    (I,O) : matrix
//        idx_t *ridx   (I)   : row index vector
//        idx_t *cidx   (I)   : column index vector
//
// return:
//        int   info    (O)   : 1 if successful, 0 if failed
// -----------------------------------------------------------------------------
int MatLogIdxIn(mat_t *mat, const idx_t *ridx, const idx_t *cidx);

// -----------------------------------------------------------------------------
// Matrix transpose (At = transpose(A))
//
// args:
//  const mat_t *A      (I) : input matrix
//
// return:
//        mat_t *At     (O) : transposed matrix (NULL if error)
// -----------------------------------------------------------------------------
mat_t *MatTr(const mat_t *mat);

// -----------------------------------------------------------------------------
// In-place matrix transpose (A = transpose(A))
//
// args:
//        mat_t *A      (I,O): matrix to transpose (m×n → n×m)
//
// return:
//        int   info    (O)  : 1 if successful, 0 if failed
// -----------------------------------------------------------------------------
int MatTrIn(mat_t *mat);

// -----------------------------------------------------------------------------
// Matrix addition [C = a * A(^T) + b * B(^T)]
//
// args:
//        double       a   (I) : scalar for matrix A
//  const mat_t       *A   (I) : matrix A
//        bool        trA  (I) : transpose flag for A (true if A^T)
//        double       b   (I) : scalar for matrix B
//  const mat_t       *B   (I) : matrix B
//        bool        trB  (I) : transpose flag for B (true if B^T)
//
// return:
//        mat_t       *C   (O) : result matrix C = a * A(^T) + b * B(^T) (NULL if error)
// -----------------------------------------------------------------------------
mat_t *MatAdd(double a, const mat_t *A, bool trA, double b, const mat_t *B, bool trB);

// -----------------------------------------------------------------------------
// In-place matrix addition [A = a * A(^T) + b * B(^T)]
//
// args:
//        mat_t       *A   (I,O) : matrix A (input/output)
//        double       a   (I)   : scalar for matrix A
//        bool        trA  (I)   : transpose flag for A (true if A^T)
//        double       b   (I)   : scalar for matrix B
//  const mat_t       *B   (I)   : matrix B
//        bool        trB  (I)   : transpose flag for B (true if B^T)
//
// return:
//        int         info (O)   : 1 if successful, 0 if failed
// -----------------------------------------------------------------------------
int MatAddIn(mat_t *A, double a, bool trA, double b, const mat_t *B, bool trB);

// -----------------------------------------------------------------------------
// Matrix multiplication [C = a * A(^T) * b * B(^T)]
//
// args:
//        double       a   (I) : scalar for matrix A
//  const mat_t       *A   (I) : matrix A
//        bool        trA  (I) : transpose flag for A (true if A^T)
//        double       b   (I) : scalar for matrix B
//  const mat_t       *B   (I) : matrix B
//        bool        trB  (I) : transpose flag for B (true if B^T)
//
// return:
//        mat_t       *C   (O) : result matrix C = a * A(^T) * b * B(^T) (NULL if error)
// -----------------------------------------------------------------------------
mat_t *MatMul(double a, const mat_t *A, bool trA, double b, const mat_t *B, bool trB);

// -----------------------------------------------------------------------------
// In-place matrix multiplication [A = a * A(^T) * b * B(^T)]
//
// args:
//        mat_t       *A   (I,O) : matrix A (input/output)
//        double       a   (I)   : scalar for matrix A
//        bool        trA  (I)   : transpose flag for A (true if A^T)
//        double       b   (I)   : scalar for matrix B
//  const mat_t       *B   (I)   : matrix B
//        bool        trB  (I)   : transpose flag for B (true if B^T)
//
// return:
//        int         info (O)   : 1 if successful, 0 if failed
// -----------------------------------------------------------------------------
int MatMulIn(mat_t *A, double a, bool trA, double b, const mat_t *B, bool trB);

// -----------------------------------------------------------------------------
// Matrix inverse [Ai = inv(a * A(^T))]
//
// args:
//        double       a   (I) : scalar for matrix A
//  const mat_t       *A   (I) : matrix A (must be square)
//        bool        trA  (I) : transpose flag for A (true if A^T)
//
// return:
//        mat_t       *Ai  (O) : result matrix Ai = inv(a * A(^T)) (NULL if error/singular)
// -----------------------------------------------------------------------------
mat_t *MatInv(double a, const mat_t *A, bool trA);

// -----------------------------------------------------------------------------
// In-place matrix inverse [A = inv(a * A(^T))]
//
// args:
//        mat_t       *A   (I,O) : matrix A (input/output)
//        double       a   (I)   : scalar for matrix A
//        bool        trA  (I)   : transpose flag for A (true if A^T)
//
// return:
//        int         info (O)   : 1 if successful, 0 if failed
// -----------------------------------------------------------------------------
int MatInvIn(mat_t *A, double a, bool trA);

// =============================================================================
// Vector operations (Matrix operations for vector (1 column))
// =============================================================================

// -----------------------------------------------------------------------------
// Vector dot product (inner product, DOUBLE only) [c = a^T * b]
//
// args:
//  const mat_t  *a      (I) : vector a (Nx1, DOUBLE type)
//  const mat_t  *b      (I) : vector b (Nx1, DOUBLE type)
//        double *c      (O) : dot product c = a^T * b
//
// return:
//        int    info    (O) : 1 if successful, 0 if failed
// -----------------------------------------------------------------------------
int Dot(const mat_t *a, const mat_t *b, double *c);

// -----------------------------------------------------------------------------
// 3D vector cross product (DOUBLE only) [c = a × b]
//
// args:
//  const mat_t *a      (I) : vector a (3x1, DOUBLE type)
//  const mat_t *b      (I) : vector b (3x1, DOUBLE type)
//        mat_t *c      (O) : cross product vector c = a × b (3x1, DOUBLE type)
//
// return:
//        int    info    (O) : 1 if successful, 0 if failed
// -----------------------------------------------------------------------------
int Cross3(const mat_t *a, const mat_t *b, mat_t *c);

// -----------------------------------------------------------------------------
// Vector 2-norm (Euclidean norm, DOUBLE only)
//
// args:
//  const mat_t *a      (I) : input vector (Nx1, DOUBLE type)
//
// return:
//        double norm   (O) : 2-norm of vector (m x 1 or 1 x n) (0.0 if error)
// -----------------------------------------------------------------------------
double Norm(const mat_t *a);

// =============================================================================
// Matrix analysis functions
// =============================================================================

// -----------------------------------------------------------------------------
// Matrix determinant using LU decomposition (DOUBLE only)
//
// args:
//  const mat_t *A      (I) : square matrix (DOUBLE type)
//
// return:
//        double det    (O) : determinant of matrix A (0.0 if singular/error)
// -----------------------------------------------------------------------------
double MatDet(const mat_t *A);

// =============================================================================
// Advanced algorithms
// =============================================================================

// -----------------------------------------------------------------------------
// Linear interpolation (column vectors only, no extrapolation)
//
// args:
//  const mat_t  *x0  (I) : x values column vector (n x 1), DOUBLE type, sorted ascending
//  const mat_t  *y0  (I) : y values column vector (n x 1), DOUBLE type
//        double x    (I) : interpolation point
//        double *y   (O) : interpolated value (boundary values if out of range)
//
// return:
//        int   info  (O) : 1 if successful, 0 if failed
// -----------------------------------------------------------------------------
int Interp(const mat_t *x0, const mat_t *y0, double x, double *y);

// -----------------------------------------------------------------------------
// Least square estimation
//
// args:
//  const mat_t *H   (I) : Design matrix (DOUBLE type)
//  const mat_t *y   (I) : (optional) Measurement vector (DOUBLE type, required if x != NULL)
//  const mat_t *R   (I) : (optional) Measurement noise covariance matrix (DOUBLE type, use I if NULL)
//        mat_t *x   (O) : (optional) State vector (DOUBLE type)
//        mat_t *P   (O) : (optional) State covariance matrix (DOUBLE type)
//        mat_t *Hl  (O) : (optional) Least square inverse matrix of H (DOUBLE type)
//
// return:
//        int   info (O) : 1 if successful, 0 if failed
// -----------------------------------------------------------------------------
int Lsq(const mat_t *H, const mat_t *y, const mat_t *R, mat_t *x, mat_t *P, mat_t *Hl);

// -----------------------------------------------------------------------------
// Extended Kalman filter
//
// args:
//  const mat_t *H   (I)   : Design matrix (DOUBLE type)
//  const mat_t *v   (I)   : Measurement residuals (DOUBLE type)
//  const mat_t *R   (I)   : Measurement noise covariance matrix (DOUBLE type)
//        mat_t *x   (I,O) : (optional) State vector (DOUBLE type)
//        mat_t *P   (I,O) : State covariance matrix (DOUBLE type)
//        mat_t *K   (O)   : (optional) Kalman gain matrix (DOUBLE type)
//
// return:
//        int   info (O)   : 1 if successful, 0 if failed
// -----------------------------------------------------------------------------
int Ekf(const mat_t *H, const mat_t *v, const mat_t *R, mat_t *x, mat_t *P, mat_t *K);

// =============================================================================
// End of header
// =============================================================================
#ifdef __cplusplus
}
#endif

#endif // MATRIX_H
