// =============================================================================
// Matrix functions
//
// -----------------------------------------------------------------------------
// Yongrae Jo, 0727ggame@sju.ac.kr
// =============================================================================

#include "matrix.h"
#include <stdlib.h>     // for posix_memalign, malloc, free
#include <assert.h>     // for assert
#include <math.h>       // for fabs, sqrt, floor
#include <stdio.h>      // for printf, snprintf
#include <string.h>     // for strlen, memset, memcpy

// =============================================================================
// Macros
// =============================================================================

#define MAT_ALIGNMENT 32    // Memory alignment for SIMD optimization

// =============================================================================
// Static functions (internal use only)
// =============================================================================

// -----------------------------------------------------------------------------
// Basic matrix multiplication: C = A * B (static function)
//
// args:
//  const mat_t *A   (I) : matrix A
//  const mat_t *B   (I) : matrix B
//
// return:
//        mat_t *C   (O) : result matrix C = A * B
// -----------------------------------------------------------------------------
static mat_t *MatrixMultiplication(const mat_t *A, const mat_t *B)
{
    if (A == NULL || B == NULL) return NULL;
    if (A->cols != B->rows) return NULL;
    if (A->type != B->type) return NULL;

    int m = A->rows, n = A->cols, p = B->cols;

    // Check column
    if (n <= 0) return NULL;

    // Result matrix
    mat_t *C = Mat(m, p, A->type);
    if (C == NULL) return NULL;

    // Empty matrix
    if (m == 0 || p == 0) return C;

    if (A->type == DOUBLE) {
        for (int j = 0; j < p; ++j) {
            for (int i = 0; i < m; ++i) {
                double sum = 0.0;
                for (int k = 0; k < n; ++k) {
                    sum += MatGetD(A, i, k) * MatGetD(B, k, j);
                }
                MatSetD(C, i, j, sum);
            }
        }
    } else { // INT
        for (int j = 0; j < p; ++j) {
            for (int i = 0; i < m; ++i) {
                int sum = 0;
                for (int k = 0; k < n; ++k) {
                    sum += MatGetI(A, i, k) * MatGetI(B, k, j);
                }
                MatSetI(C, i, j, sum);
            }
        }
    }

    return C;
}

// -----------------------------------------------------------------------------
// Basic matrix inverse: Ai = inv(A) (static function, DOUBLE only)
//
// args:
//  const mat_t *A   (I) : matrix A
//
// return:
//        mat_t *Ai  (O) : result matrix Ai = inv(A)
// -----------------------------------------------------------------------------
static mat_t *MatrixInverse(const mat_t *A)
{
    // Check input matrix
    if (A == NULL) return NULL;
    if (A->rows != A->cols) return NULL;  // Must be square matrix
    if (A->type != DOUBLE) return NULL;   // Only double type supported for inverse

    int n = A->rows;

    // Empty matrix inverse is empty matrix
    if (n == 0) return Mat(0, 0, DOUBLE);

    // Check if matrix is singular by computing determinant
    double det = MatDet(A);
    if (fabs(det) < 1e-15) {
        // Matrix is singular, cannot compute inverse
        return NULL;
    }

    // Create working copy for LU decomposition
    mat_t *LU = Mat(n, n, DOUBLE);
    if (LU == NULL) return NULL;

    // Copy original matrix
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            MatSetD(LU, i, j, MatGetD(A, i, j));
        }
    }

    // Create identity matrix for right-hand side
    mat_t *I = Eye(n, DOUBLE);
    if (I == NULL) {
        FreeMat(LU);
        return NULL;
    }

    // Create result matrix
    mat_t *inv = Mat(n, n, DOUBLE);
    if (inv == NULL) {
        FreeMat(LU);
        FreeMat(I);
        return NULL;
    }

    int *pivot = (int *)malloc(n * sizeof(int));
    if (pivot == NULL) {
        FreeMat(LU);
        FreeMat(I);
        FreeMat(inv);
        return NULL;
    }

    // Initialize pivot array
    for (int i = 0; i < n; ++i) {
        pivot[i] = i;
    }

    // Perform LU decomposition with partial pivoting
    for (int k = 0; k < n - 1; ++k) {
        // Find pivot element
        int max_row = k;
        double max_val = fabs(MatGetD(LU, k, k));

        for (int i = k + 1; i < n; ++i) {
            double val = fabs(MatGetD(LU, i, k));
            if (val > max_val) {
                max_val = val;
                max_row = i;
            }
        }

        // Swap rows if necessary
        if (max_row != k) {
            // Swap pivot indices
            int temp = pivot[k];
            pivot[k] = pivot[max_row];
            pivot[max_row] = temp;

            // Swap matrix rows
            for (int j = 0; j < n; ++j) {
                double temp_val = MatGetD(LU, k, j);
                MatSetD(LU, k, j, MatGetD(LU, max_row, j));
                MatSetD(LU, max_row, j, temp_val);

                // Also swap identity matrix rows
                temp_val = MatGetD(I, k, j);
                MatSetD(I, k, j, MatGetD(I, max_row, j));
                MatSetD(I, max_row, j, temp_val);
            }
        }

        // Perform elimination
        for (int i = k + 1; i < n; ++i) {
            double factor = MatGetD(LU, i, k) / MatGetD(LU, k, k);
            MatSetD(LU, i, k, factor);

            for (int j = k + 1; j < n; ++j) {
                double new_val = MatGetD(LU, i, j) - factor * MatGetD(LU, k, j);
                MatSetD(LU, i, j, new_val);
            }

            // Apply elimination to identity matrix
            for (int j = 0; j < n; ++j) {
                double new_val = MatGetD(I, i, j) - factor * MatGetD(I, k, j);
                MatSetD(I, i, j, new_val);
            }
        }
    }

    // Solve L*Y = I (forward substitution)
    for (int j = 0; j < n; ++j) {
        for (int i = 0; i < n; ++i) {
            double sum = MatGetD(I, i, j);
            for (int k = 0; k < i; ++k) {
                sum -= MatGetD(LU, i, k) * MatGetD(inv, k, j);
            }
            MatSetD(inv, i, j, sum);
        }
    }

    // Solve U*X = Y (backward substitution)
    for (int j = 0; j < n; ++j) {
        for (int i = n - 1; i >= 0; --i) {
            double sum = MatGetD(inv, i, j);
            for (int k = i + 1; k < n; ++k) {
                sum -= MatGetD(LU, i, k) * MatGetD(inv, k, j);
            }
            MatSetD(inv, i, j, sum / MatGetD(LU, i, i));
        }
    }

    // Clean up
    free(pivot);
    FreeMat(LU);
    FreeMat(I);

    return inv;
}

// -----------------------------------------------------------------------------
// Basic matrix addition: C = A + B (static function, DOUBLE only)
//
// args:
//  const mat_t *A   (I) : matrix A
//  const mat_t *B   (I) : matrix B
//
// return:
//        mat_t *C   (O) : result matrix C = A + B
// -----------------------------------------------------------------------------
static mat_t *MatrixAddition(const mat_t *A, const mat_t *B)
{
    if (A == NULL || B == NULL) return NULL;
    if (A->type != DOUBLE || B->type != DOUBLE) return NULL;  // Only double type supported
    if (A->rows != B->rows || A->cols != B->cols) return NULL;  // Must have same dimensions

    int rows = A->rows, cols = A->cols;

    // Result matrix
    mat_t *C = Mat(rows, cols, DOUBLE);
    if (C == NULL) return NULL;

    // Empty matrix
    if (rows == 0 || cols == 0) return C;

    // Perform addition
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            double val = MatGetD(A, i, j) + MatGetD(B, i, j);
            MatSetD(C, i, j, val);
        }
    }

    return C;
}

// =============================================================================
// Basic matrix/index vector operations functions
// =============================================================================

// Matrix generation
mat_t *Mat(int rows, int cols, type_t type)
{
    // Check input validity
    if (rows < 0 || cols < 0) return NULL;
    if (type != DOUBLE && type != INT) return NULL;  // Only DOUBLE and INT supported

    mat_t *mat = (mat_t *)malloc(sizeof(mat_t));
    if (mat == NULL) return NULL;

    mat->rows = rows;
    mat->cols = cols;
    mat->type = type;
    mat->data = NULL;

    // If empty matrix, return valid struct with NULL data
    if (rows == 0 || cols == 0) return mat;

    size_t esize = (type == DOUBLE) ? sizeof(double) : sizeof(int);
    size_t tsize = (size_t)rows * cols * esize;
    size_t alignment = MAT_ALIGNMENT;

#ifdef _WIN32
    mat->data = _aligned_malloc(tsize, alignment);
    if (mat->data == NULL) {
        free(mat);
        return NULL;
    }
#else
    int err = posix_memalign(&mat->data, alignment, tsize);
    if (err != 0) {
        free(mat);
        return NULL;
    }
#endif
    return mat;
}

// Matrix memory deallocation
void FreeMat(mat_t *mat)
{
    if (!mat) return;

#ifdef _WIN32
    if (mat->data) _aligned_free(mat->data);
#else
    if (mat->data) free(mat->data);
#endif

    free(mat);
}

// Index vector generation
idx_t *Idx(int n, type_t type)
{
    // Check number of indices
    if (n < 0) return NULL;
    if (type != BOOL && type != INT) return NULL;  // Only BOOL and INT supported

    idx_t *idx = (idx_t *)malloc(sizeof(idx_t));
    if (idx == NULL) return NULL;

    idx->n = n;
    idx->type = type;
    idx->idx = NULL;

    // If empty index vector, return valid struct with NULL data
    if (idx->n == 0) return idx;

    size_t alignment = MAT_ALIGNMENT;
    size_t esize = (type == BOOL) ? sizeof(bool) : sizeof(int);
    size_t tsize = (size_t)idx->n * esize;

#ifdef _WIN32
    idx->idx = _aligned_malloc(tsize, alignment);
    if (idx->idx == NULL) {
        free(idx);
        return NULL;  // Memory allocation failed
    }
#else
    int err = posix_memalign((void**)&idx->idx, alignment, tsize);
    if (err != 0) {
        free(idx);
        return NULL;  // Memory allocation failed
    }
#endif

    return idx;
}

// Index vector memory deallocation
void FreeIdx(idx_t *idx)
{
    if (!idx) return;

#ifdef _WIN32
    if (idx->idx) _aligned_free(idx->idx);
#else
    if (idx->idx) free(idx->idx);
#endif
    free(idx);
}

// Identity matrix generation
mat_t *Eye(int size, type_t type)
{
    // Check size
    if (size < 0) return NULL;
    if (type != DOUBLE && type != INT) return NULL;  // Only DOUBLE and INT supported

    // Generate identity matrix
    mat_t *I = Mat(size, size, type);
    if (I == NULL) return NULL;

    if (size == 0) return I;

    // Initialize all elements to 0
    size_t esize = (type == DOUBLE) ? sizeof(double) : sizeof(int);
    size_t total = (size_t)size * size * esize;
    memset(I->data, 0, total);

    // Set diagonal elements to 1
    if (type == DOUBLE) {
        for (int i = 0; i < size; ++i) {
            MatSetD(I, i, i, 1.0);
        }
    } else { // INT
        for (int i = 0; i < size; ++i) {
            MatSetI(I, i, i, 1);
        }
    }

    return I;
}

// Zeros matrix generation
mat_t *Zeros(int rows, int cols, type_t type)
{
    // Check rows and columns
    if (rows < 0 || cols < 0) return NULL;
    if (type != DOUBLE && type != INT) return NULL;  // Only DOUBLE and INT supported

    // Generate zero matrix
    mat_t *Z = Mat(rows, cols, type);
    if (Z == NULL) return NULL;

    // If empty matrix, return valid struct with NULL data
    if (rows == 0 || cols == 0) return Z;

    size_t esize = (type == DOUBLE) ? sizeof(double) : sizeof(int);
    size_t total = (size_t)rows * cols * esize;
    memset(Z->data, 0, total);

    return Z;
}

// Ones matrix generation
mat_t *Ones(int rows, int cols, type_t type)
{
    // Check rows and columns
    if (rows < 0 || cols < 0) return NULL;
    if (type != DOUBLE && type != INT) return NULL;  // Only DOUBLE and INT supported

    // Generate ones matrix
    mat_t *O = Mat(rows, cols, type);
    if (O == NULL) return NULL;

    if (rows == 0 || cols == 0) return O;

    if (type == DOUBLE) {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                MatSetD(O, i, j, 1.0);
            }
        }
    } else { // INT
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                MatSetI(O, i, j, 1);
            }
        }
    }

    return O;
}

// True index vector generation
idx_t *TrueIdx(int n)
{
    idx_t *idx = Idx(n, BOOL);
    if (idx == NULL) return NULL;

    for (int i = 0; i < n; ++i) {
        IdxSetB(idx, i, true);
    }
    return idx;
}

// False index vector generation
idx_t *FalseIdx(int n)
{
    idx_t *idx = Idx(n, BOOL);
    if (idx == NULL) return NULL;

    for (int i = 0; i < n; ++i) {
        IdxSetB(idx, i, false);
    }
    return idx;
}

// =============================================================================
// Matrix operations functions
// =============================================================================

// Matrix copy - returns new matrix with copied data
mat_t *MatCopy(const mat_t *A)
{
    // Check input matrix
    if (A == NULL) return NULL;

    // Create result matrix
    mat_t *B = Mat(A->rows, A->cols, A->type);
    if (B == NULL) return NULL;

    // Copy matrix data with memcpy (much faster!)
    if (A->rows > 0 && A->cols > 0) {
        size_t esize = (A->type == DOUBLE) ? sizeof(double) : sizeof(int);
        size_t total = (size_t)A->rows * A->cols * esize;
        memcpy(B->data, A->data, total);
    }

    return B;
}

// Matrix in-place copy - copies source matrix data to existing destination matrix
int MatCopyIn(mat_t *des, const mat_t *src)
{
    if (src == NULL || des == NULL) return 0;
    if (src->rows != des->rows || src->cols != des->cols) return 0;
    if (src->type != des->type) return 0;

    size_t esize = (src->type == DOUBLE) ? sizeof(double) : sizeof(int);
    size_t total = (size_t)src->rows * src->cols * esize;
    memcpy(des->data, src->data, total);

    return 1;
}

// Matrix vector indexing
mat_t *MatVecIdx(const mat_t *mat, const idx_t *ridx, const idx_t *cidx)
{
    // Check input parameters
    if (mat == NULL || ridx == NULL || cidx == NULL) return NULL;
    if (ridx->type != INT || cidx->type != INT) return NULL;

    // Extract row and column indices
    int *rows = (int *)ridx->idx;
    int *cols = (int *)cidx->idx;

    // Check index validity
    for (int i = 0; i < ridx->n; i++) {
        if (rows[i] < 0 || rows[i] >= mat->rows) return NULL;
    }
    for (int j = 0; j < cidx->n; j++) {
        if (cols[j] < 0 || cols[j] >= mat->cols) return NULL;
    }

    // Create result matrix
    mat_t *result = Mat(ridx->n, cidx->n, mat->type);
    if (!result) return NULL;

    // Copy indexed elements
    if (mat->type == DOUBLE) {
        for (int j = 0; j < cidx->n; j++) {
            for (int i = 0; i < ridx->n; i++) {
                double val = MatGetD(mat, rows[i], cols[j]);
                MatSetD(result, i, j, val);
            }
        }
    } else { // INT
        for (int j = 0; j < cidx->n; j++) {
            for (int i = 0; i < ridx->n; i++) {
                int val = MatGetI(mat, rows[i], cols[j]);
                MatSetI(result, i, j, val);
            }
        }
    }

    return result;
}

// Matrix in-place vector indexing
int MatVecIdxIn(mat_t *mat, const idx_t *ridx, const idx_t *cidx)
{
    if (mat == NULL || ridx == NULL || cidx == NULL) return 0;

    // Create indexed matrix
    mat_t *indexed = MatVecIdx(mat, ridx, cidx);
    if (!indexed) return 0;

    // Check if in-place operation is valid (same dimensions)
    if (mat->rows != indexed->rows || mat->cols != indexed->cols) {
        FreeMat(indexed);
        return 0;
    }

    // Copy indexed data back to original matrix
    int result = MatCopyIn(mat, indexed);
    FreeMat(indexed);

    return result;
}

// Matrix logical indexing
mat_t *MatLogIdx(const mat_t *mat, const idx_t *ridx, const idx_t *cidx)
{
    // Check input parameters
    if (mat == NULL || ridx == NULL || cidx == NULL) return NULL;
    if (ridx->type != BOOL || cidx->type != BOOL) return NULL;
    if (ridx->n != mat->rows || cidx->n != mat->cols) return NULL;

    bool *row_mask = (bool *)ridx->idx;
    bool *col_mask = (bool *)cidx->idx;

    // Count selected rows and columns
    int selected_rows = 0, selected_cols = 0;
    for (int i = 0; i < ridx->n; i++) {
        if (row_mask[i]) selected_rows++;
    }
    for (int j = 0; j < cidx->n; j++) {
        if (col_mask[j]) selected_cols++;
    }

    // Create result matrix
    mat_t *result = Mat(selected_rows, selected_cols, mat->type);
    if (!result) return NULL;

    // Copy selected elements
    int result_i = 0;
    for (int i = 0; i < mat->rows; i++) {
        if (!row_mask[i]) continue;

        int result_j = 0;
        for (int j = 0; j < mat->cols; j++) {
            if (!col_mask[j]) continue;

            if (mat->type == DOUBLE) {
                double val = MatGetD(mat, i, j);
                MatSetD(result, result_i, result_j, val);
            } else { // INT
                int val = MatGetI(mat, i, j);
                MatSetI(result, result_i, result_j, val);
            }
            result_j++;
        }
        result_i++;
    }

    return result;
}

// Matrix in-place logical indexing
int MatLogIdxIn(mat_t *mat, const idx_t *ridx, const idx_t *cidx)
{
    if (mat == NULL || ridx == NULL || cidx == NULL) return 0;

    // Create logically indexed matrix
    mat_t *indexed = MatLogIdx(mat, ridx, cidx);
    if (!indexed) return 0;

    // Check if in-place operation is valid (same dimensions)
    if (mat->rows != indexed->rows || mat->cols != indexed->cols) {
        FreeMat(indexed);
        return 0;
    }

    // Copy indexed data back to original matrix
    int result = MatCopyIn(mat, indexed);
    FreeMat(indexed);

    return result;
}

// Matrix transpose
mat_t *MatTr(const mat_t *A)
{
    // Check input matrix
    if (A == NULL) return NULL;

    // Generate transposed matrix
    mat_t *T = Mat(A->cols, A->rows, A->type);
    if (T == NULL) return NULL;

    if (A->rows == 0 || A->cols == 0) {
        // Empty matrix
        return T;
    }

    if (A->type == DOUBLE) {
        for (int i = 0; i < A->rows; ++i) {
            for (int j = 0; j < A->cols; ++j) {
                MatSetD(T, j, i, MatGetD(A, i, j));
            }
        }
    } else { // INT
        for (int i = 0; i < A->rows; ++i) {
            for (int j = 0; j < A->cols; ++j) {
                MatSetI(T, j, i, MatGetI(A, i, j));
            }
        }
    }

    return T;
}

// In-place matrix transpose (m×n → n×m)
int MatTrIn(mat_t *A)
{
    if (A == NULL) return 0;

    // Create transposed matrix
    mat_t *At = MatTr(A);
    if (At == NULL) return 0;

    // Copy transposed data to original matrix
    // (same total number of elements: m×n = n×m)
    size_t esize = (A->type == DOUBLE) ? sizeof(double) : sizeof(int);
    size_t total = (size_t)A->rows * A->cols * esize;
    memcpy(A->data, At->data, total);

    // Update dimensions
    int temp = A->rows;
    A->rows = A->cols;
    A->cols = temp;

    // Free transposed matrix
    FreeMat(At);

    return 1;
}

// Matrix addition [C = a * A(^T) + b * B(^T)]
mat_t *MatAdd(double a, const mat_t *A, bool trA, double b, const mat_t *B, bool trB)
{
    if (A == NULL || B == NULL) return NULL;
    if (A->type != DOUBLE || B->type != DOUBLE) return NULL;  // Only double type supported

    // Create temporary matrices for scaled and/or transposed operands
    mat_t *A_temp = NULL;
    mat_t *B_temp = NULL;
    mat_t *C = NULL;

    // Process matrix A (scale and/or transpose)
    if (trA) {
        A_temp = MatTr(A);
        if (A_temp == NULL) return NULL;
    } else {
        A_temp = MatCopy(A);
        if (A_temp == NULL) return NULL;
    }

    // Apply scalar to A
    if (a != 1.0) {
        for (int i = 0; i < A_temp->rows; ++i) {
            for (int j = 0; j < A_temp->cols; ++j) {
                double val = MatGetD(A_temp, i, j) * a;
                MatSetD(A_temp, i, j, val);
            }
        }
    }

    // Process matrix B (scale and/or transpose)
    if (trB) {
        B_temp = MatTr(B);
        if (B_temp == NULL) {
            FreeMat(A_temp);
            return NULL;
        }
    } else {
        B_temp = MatCopy(B);
        if (B_temp == NULL) {
            FreeMat(A_temp);
            return NULL;
        }
    }

    // Apply scalar to B
    if (b != 1.0) {
        for (int i = 0; i < B_temp->rows; ++i) {
            for (int j = 0; j < B_temp->cols; ++j) {
                double val = MatGetD(B_temp, i, j) * b;
                MatSetD(B_temp, i, j, val);
            }
        }
    }

    // Check dimension compatibility
    if (A_temp->rows != B_temp->rows || A_temp->cols != B_temp->cols) {
        FreeMat(A_temp);
        FreeMat(B_temp);
        return NULL;
    }

    // Perform matrix addition using static function
    C = MatrixAddition(A_temp, B_temp);

    // Clean up temporary matrices
    FreeMat(A_temp);
    FreeMat(B_temp);

    return C;
}

// In-place matrix addition [A = a * A(^T) + b * B(^T)]
int MatAddIn(mat_t *A, double a, bool trA, double b, const mat_t *B, bool trB)
{
    if (A == NULL || B == NULL) return 0;
    if (A->type != DOUBLE || B->type != DOUBLE) return 0;  // Only double type supported

    // Compute extended addition
    mat_t *C = MatAdd(a, A, trA, b, B, trB);
    if (C == NULL) return 0;

    // Check dimension compatibility for in-place operation
    if (A->rows != C->rows || A->cols != C->cols) {
        FreeMat(C);
        return 0;  // Dimension mismatch
    }

    // Copy result to A
    int result = MatCopyIn(A, C);

    // Clean up temporary matrix
    FreeMat(C);

    return result;
}

// Matrix multiplication [C = a * A(^T) * b * B(^T)]
mat_t *MatMul(double a, const mat_t *A, bool trA, double b, const mat_t *B, bool trB)
{
    // Check input matrices
    if (A == NULL || B == NULL) return NULL;
    if (A->type != DOUBLE || B->type != DOUBLE) return NULL;
    if (A->cols != B->rows) return NULL;

    // Create temporary matrices for scaled and/or transposed operands
    mat_t *A_temp = NULL;
    mat_t *B_temp = NULL;
    mat_t *C = NULL;

    // Process matrix A (scale and/or transpose)
    if (trA) {
        A_temp = MatTr(A);
        if (A_temp == NULL) return NULL;
    } else {
        A_temp = MatCopy(A);
        if (A_temp == NULL) return NULL;
    }

    // Apply scalar to A
    if (a != 1.0) {
        for (int i = 0; i < A_temp->rows; ++i) {
            for (int j = 0; j < A_temp->cols; ++j) {
                double val = MatGetD(A_temp, i, j) * a;
                MatSetD(A_temp, i, j, val);
            }
        }
    }

    // Process matrix B (scale and/or transpose)
    if (trB) {
        B_temp = MatTr(B);
        if (B_temp == NULL) {
            FreeMat(A_temp);
            return NULL;
        }
    } else {
        B_temp = MatCopy(B);
        if (B_temp == NULL) {
            FreeMat(A_temp);
            return NULL;
        }
    }

    // Apply scalar to B
    if (b != 1.0) {
        for (int i = 0; i < B_temp->rows; ++i) {
            for (int j = 0; j < B_temp->cols; ++j) {
                double val = MatGetD(B_temp, i, j) * b;
                MatSetD(B_temp, i, j, val);
            }
        }
    }

    // Check dimension compatibility
    if (A_temp->cols != B_temp->rows) {
        FreeMat(A_temp);
        FreeMat(B_temp);
        return NULL;
    }

    // Perform matrix multiplication using static function
    C = MatrixMultiplication(A_temp, B_temp);

    // Clean up temporary matrices
    FreeMat(A_temp);
    FreeMat(B_temp);

    return C;
}

// Matrix inverse [Ai = inv(a * A(^T))]
mat_t *MatInv(double a, const mat_t *A, bool trA)
{
    if (A == NULL) return NULL;
    if (A->type != DOUBLE) return NULL;  // Only double type supported
    if (A->rows != A->cols) return NULL; // Must be square matrix

    // Create temporary matrix for scaled and/or transposed operand
    mat_t *A_temp = NULL;
    mat_t *Ai = NULL;

    // Process matrix A (scale and/or transpose)
    if (trA) {
        A_temp = MatTr(A);
        if (A_temp == NULL) return NULL;
    } else {
        A_temp = MatCopy(A);
        if (A_temp == NULL) return NULL;
    }

    // Apply scalar to A
    if (a != 1.0) {
        for (int i = 0; i < A_temp->rows; ++i) {
            for (int j = 0; j < A_temp->cols; ++j) {
                double val = MatGetD(A_temp, i, j) * a;
                MatSetD(A_temp, i, j, val);
            }
        }
    }

    // Compute inverse using static function
    Ai = MatrixInverse(A_temp);

    // Clean up temporary matrix
    FreeMat(A_temp);

    return Ai;
}

// In-place matrix multiplication [A = a * A(^T) * b * B(^T)]
int MatMulIn(mat_t *A, double a, bool trA, double b, const mat_t *B, bool trB)
{
    if (A == NULL || B == NULL) return 0;
    if (A->type != DOUBLE || B->type != DOUBLE) return 0;  // Only double type supported

    // Compute matrix multiplication
    mat_t *C = MatMul(a, A, trA, b, B, trB);
    if (C == NULL) return 0;

    // Free original A's data
#ifdef _WIN32
    if (A->data) _aligned_free(A->data);
#else
    if (A->data) free(A->data);
#endif

    // Replace A's contents with C's contents
    A->rows = C->rows;
    A->cols = C->cols;
    A->type = C->type;
    A->data = C->data;

    // Free C structure (but not its data, which is now owned by A)
    free(C);

    return 1;
}

// In-place matrix inverse [A = inv(a * A(^T))]
int MatInvIn(mat_t *A, double a, bool trA)
{
    if (A == NULL) return 0;
    if (A->type != DOUBLE) return 0;  // Only double type supported
    if (A->rows != A->cols) return 0; // Must be square matrix

    // Compute matrix inverse
    mat_t *Ai = MatInv(a, A, trA);
    if (Ai == NULL) return 0;

    // Copy result to A
    int result = MatCopyIn(A, Ai);

    // Clean up temporary matrix
    FreeMat(Ai);

    return result;
}

// =============================================================================
// Vector (Matrix of m x 1) operations functions
// =============================================================================

// Vector dot product (inner product, DOUBLE only) [c = a^T * b]
int Dot(const mat_t *a, const mat_t *b, double *c)
{
    // Check input matrix
    if (a == NULL || b == NULL || c == NULL) return 0;
    if (a->cols != 1 || b->cols != 1) return 0;
    if (a->rows != b->rows) return 0;
    if (a->type != DOUBLE || b->type != DOUBLE) return 0;

    // For size 0 vectors, dot product is 0.0
    if (a->rows == 0) {
        *c = 0.0;
        return 1;
    }

    *c = 0.0;

    // Correct implementation for column vectors (Nx1)
    for (int i = 0; i < a->rows; ++i) {
        *c += MatGetD(a, i, 0) * MatGetD(b, i, 0);
    }

    return 1;
}

// 3D vector cross product (DOUBLE only) [c = a × b]
int Cross3(const mat_t *a, const mat_t *b, mat_t *c)
{
    // Check input matrix
    if (a == NULL || b == NULL || c == NULL) return 0;
    if (a->rows != 3 || b->rows != 3 || c->rows != 3) return 0;
    if (a->cols != 1 || b->cols != 1 || c->cols != 1) return 0;
    if (a->type != DOUBLE || b->type != DOUBLE || c->type != DOUBLE) return 0;

    // Cross product
    MatSetD(c, 0, 0, MatGetD(a, 1, 0) * MatGetD(b, 2, 0) - MatGetD(a, 2, 0) * MatGetD(b, 1, 0));
    MatSetD(c, 1, 0, MatGetD(a, 2, 0) * MatGetD(b, 0, 0) - MatGetD(a, 0, 0) * MatGetD(b, 2, 0));
    MatSetD(c, 2, 0, MatGetD(a, 0, 0) * MatGetD(b, 1, 0) - MatGetD(a, 1, 0) * MatGetD(b, 0, 0));

    return 1;
}

// Vector 2-norm (Euclidean norm, DOUBLE only, m x 1 or 1 x n)
double Norm(const mat_t *a)
{
    // Check input matrix
    if (a == NULL) return 0.0;
    if (a->cols != 1 && a->rows != 1) return 0.0;
    if (a->type != DOUBLE) return 0.0;

    double norm = 0.0;
    if (a->cols == 1) {
        for (int i = 0; i < a->rows; ++i) {
            norm += MatGetD(a, i, 0) * MatGetD(a, i, 0);
        }
    } else {
        for (int i = 0; i < a->cols; ++i) {
            norm += MatGetD(a, 0, i) * MatGetD(a, 0, i);
        }
    }

    return sqrt(norm);
}

// =============================================================================
// Matrix analysis functions functions
// =============================================================================

// Matrix determinant
double MatDet(const mat_t *A)
{
    // Check input matrix
    if (A == NULL) return 0.0;
    if (A->rows != A->cols) return 0.0;  // Must be square matrix
    if (A->type != DOUBLE) return 0.0;   // Only double type supported for determinant

    int n = A->rows;

    // Empty matrix determinant is 1.0 (convention)
    if (n == 0) return 1.0;

    // For 1x1 matrix, determinant is the element itself
    if (n == 1) return MatGetD(A, 0, 0);

    // Create working copy of matrix for LU decomposition
    mat_t *LU = Mat(n, n, DOUBLE);
    if (LU == NULL) return 0.0;

    // Copy original matrix to LU
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            MatSetD(LU, i, j, MatGetD(A, i, j));
        }
    }

    double det = 1.0;
    int *pivot = (int *)malloc(n * sizeof(int));
    if (pivot == NULL) {
        FreeMat(LU);
        return 0.0;
    }

    // Initialize pivot array
    for (int i = 0; i < n; ++i) {
        pivot[i] = i;
    }

    // Perform LU decomposition with partial pivoting
    for (int k = 0; k < n - 1; ++k) {
        // Find pivot element
        int max_row = k;
        double max_val = fabs(MatGetD(LU, k, k));

        for (int i = k + 1; i < n; ++i) {
            double val = fabs(MatGetD(LU, i, k));
            if (val > max_val) {
                max_val = val;
                max_row = i;
            }
        }

        // Check if matrix is singular
        if (max_val < 1e-15) {
            free(pivot);
            FreeMat(LU);
            return 0.0;  // Singular matrix, determinant is 0
        }

        // Swap rows if necessary
        if (max_row != k) {
            // Swap pivot indices
            int temp = pivot[k];
            pivot[k] = pivot[max_row];
            pivot[max_row] = temp;

            // Swap matrix rows
            for (int j = 0; j < n; ++j) {
                double temp_val = MatGetD(LU, k, j);
                MatSetD(LU, k, j, MatGetD(LU, max_row, j));
                MatSetD(LU, max_row, j, temp_val);
            }

            // Determinant sign changes with row swap
            det = -det;
        }

        // Perform elimination
        for (int i = k + 1; i < n; ++i) {
            double factor = MatGetD(LU, i, k) / MatGetD(LU, k, k);
            MatSetD(LU, i, k, factor);

            for (int j = k + 1; j < n; ++j) {
                double new_val = MatGetD(LU, i, j) - factor * MatGetD(LU, k, j);
                MatSetD(LU, i, j, new_val);
            }
        }
    }

    // Calculate determinant from diagonal elements of U
    for (int i = 0; i < n; ++i) {
        det *= MatGetD(LU, i, i);
    }

    // Clean up
    free(pivot);
    FreeMat(LU);

    return det;
}

// =============================================================================
// Advanced algorithms functions
// =============================================================================

// Linear interpolation (column vectors only, no extrapolation)
int Interp(const mat_t *x0, const mat_t *y0, double x, double *y)
{
    // Input validation
    if (x0 == NULL || y0 == NULL || y == NULL) return 0;
    if (x0->type != DOUBLE || y0->type != DOUBLE) return 0;

    // Only accept column vectors
    if (x0->cols != 1 || y0->cols != 1) return 0;
    if (x0->rows != y0->rows) return 0;

    int n = x0->rows;
    if (n < 2) return 0;  // Need at least 2 points

    // Check if x0 is sorted in ascending order (no duplicates)
    for (int i = 0; i < n - 1; i++) {
        if (MatGetD(x0, i, 0) >= MatGetD(x0, i + 1, 0)) return 0;
    }

    // Handle boundary cases - return boundary values (no extrapolation)
    if (x <= MatGetD(x0, 0, 0)) {
        *y = MatGetD(y0, 0, 0);
        return 1;
    }

    if (x >= MatGetD(x0, n - 1, 0)) {
        *y = MatGetD(y0, n - 1, 0);
        return 1;
    }

    // Binary search for interpolation interval
    int left = 0, right = n - 1;
    while (right - left > 1) {
        int mid = (left + right) / 2;
        if (MatGetD(x0, mid, 0) <= x) {
            left = mid;
        } else {
            right = mid;
        }
    }

    // Linear interpolation: y = y0 + (x - x0) * (y1 - y0) / (x1 - x0)
    double x0_val = MatGetD(x0, left, 0);
    double x1_val = MatGetD(x0, right, 0);
    double y0_val = MatGetD(y0, left, 0);
    double y1_val = MatGetD(y0, right, 0);

    *y = y0_val + (x - x0_val) * (y1_val - y0_val) / (x1_val - x0_val);

    return 1;
}

// Least square estimation
int Lsq(const mat_t *H, const mat_t *y, const mat_t *R, mat_t *x, mat_t *P, mat_t *Hl)
{
    if (H == NULL) return 0;

    // Check if y is required (only when x is not NULL)
    if (x != NULL && y == NULL) return 0;

    // Check measurement dimension
    if (y != NULL && H->rows != y->rows) return 0;
    if (R != NULL && H->rows != R->rows) return 0;

    // Check state dimension for x if provided
    if (x != NULL) {
        if (H->cols != x->rows) return 0;
        if (x->type != DOUBLE) return 0;
    }

    // Check state dimension for P if provided
    if (P != NULL) {
        if (H->cols != P->rows || H->cols != P->cols) return 0;
        if (P->type != DOUBLE) return 0;
    }

    // Check matrix type (Only double type is supported)
    if (H->type != DOUBLE) return 0;
    if (y != NULL && y->type != DOUBLE) return 0;
    if (R != NULL && R->type != DOUBLE) return 0;

    // Check Least square inverse matrix
    if (Hl != NULL) {
        if (Hl->rows != H->cols || Hl->cols != H->rows) return 0;
        if (Hl->type != DOUBLE) return 0;
    }

    // Status variable: 1 = success, 0 = failure
    int info = 1;

    // Declare all temporary matrices (initialized to NULL)
    mat_t *R_local  = NULL;  // For R = I when R is NULL
    mat_t *Ht       = NULL;
    mat_t *W        = NULL;
    mat_t *HtW      = NULL;
    mat_t *HtWH     = NULL;
    mat_t *Q        = NULL;
    mat_t *x_temp   = NULL;
    mat_t *P_temp   = NULL;
    mat_t *L        = NULL;
    mat_t *Lt       = NULL;

    // Step 0: Create identity matrix for R if R is NULL
    if (R == NULL) {
        R_local = Eye(H->rows, DOUBLE);
        if (R_local == NULL) info = 0;
        R = R_local;  // Use local identity matrix
    }

    // Step 1: H' (transpose of H)
    if (info) {
        Ht = MatTr(H);
        if (Ht == NULL) info = 0;
    }

    // Step 2: W = inv(R) (weight matrix)
    if (info) {
        W = MatInv(1.0, R, false);
        if (W == NULL) info = 0;
    }

    // Step 3: H' * W
    if (info) {
        HtW = MatMul(1.0, Ht, false, 1.0, W, false);
        if (HtW == NULL) info = 0;
    }

    // Step 4: H' * W * H (information matrix)
    if (info) {
        HtWH = MatMul(1.0, HtW, false, 1.0, H, false);
        if (HtWH == NULL) info = 0;
    }

    // Step 5: Check if H has full column rank
    if (info) {
        double det = MatDet(HtWH);
        if (fabs(det) < 1e-15) info = 0;  // Singular matrix
    }

    // Step 6: Q = (H' * W * H)^-1 (state covariance matrix)
    if (info) {
        Q = MatInv(1.0, HtWH, false);
        if (Q == NULL) info = 0;
    }

    // Step 7: L = Q * H' * W (least square inverse matrix of H)
    if (info) {
        L = MatMul(1.0, Q, false, 1.0, HtW, false);
        if (L == NULL) info = 0;
    }

    // Step 8: x = L * y (least square estimation)
    if (info && x != NULL) {
        x_temp = MatMul(1.0, L, false, 1.0, y, false);
        if (x_temp == NULL) info = 0;
    }

    // Step 9: P = L * R * L' (state covariance matrix)
    if (info) {
        Lt = MatTr(L);
        if (Lt == NULL) info = 0;
    }

    if (info) {
        P_temp = MatMul(1.0, L, false, 1.0, R, false);
        if (P_temp == NULL) info = 0;
    }

    if (info) {
        if (!MatMulIn(P_temp, 1.0, false, 1.0, Lt, false)) info = 0;
    }

    // Copy results to output matrices only if successful
    if (info) {
        if (        x  != NULL && x_temp != NULL && !MatCopyIn(x , x_temp)) info = 0;
        if (info && P  != NULL && !MatCopyIn(P , P_temp)) info = 0;
        if (info && Hl != NULL && !MatCopyIn(Hl, L     )) info = 0;
    }

    // Clean up all temporary matrices (FreeMat handles NULL pointers safely)
    FreeMat(R_local);
    FreeMat(Ht);
    FreeMat(W);
    FreeMat(HtW);
    FreeMat(HtWH);
    FreeMat(Q);
    FreeMat(x_temp);
    FreeMat(P_temp);
    FreeMat(L);
    FreeMat(Lt);

    return info;
}

// Extended Kalman filter
int Ekf(const mat_t *H, const mat_t *v, const mat_t *R, mat_t *x, mat_t *P, mat_t *K)
{
    if (H == NULL || v == NULL || R == NULL || P == NULL) return 0;

    // Check measurement dimension
    if (H->rows != v->rows || H->rows != R->rows) return 0;

    // Check state dimension for x if provided
    if (x != NULL) {
        if (H->cols != x->rows) return 0;
        if (x->type != DOUBLE) return 0;
    }

    // Check state dimension for P
    if (H->cols != P->rows || H->cols != P->cols) return 0;

    // Check matrix type (Only double type is supported)
    if (H->type != DOUBLE || v->type != DOUBLE || R->type != DOUBLE || P->type != DOUBLE) return 0;

    // Check Kalman gain matrix
    if (K != NULL) {
        if (K->rows != H->cols || K->cols != H->rows) return 0;
        if (K->type != DOUBLE) return 0;
    }

    // Status variable: 1 = success, 0 = failure
    int info = 1;

    // Declare all temporary matrices (initialized to NULL)
    mat_t *Ht         = NULL;
    mat_t *HP         = NULL;
    mat_t *HPHt       = NULL;
    mat_t *S          = NULL;
    mat_t *Sinv       = NULL;
    mat_t *PHt        = NULL;
    mat_t *K_temp     = NULL;
    mat_t *Kv         = NULL;
    mat_t *x_temp     = NULL;
    mat_t *KH         = NULL;
    mat_t *I          = NULL;
    mat_t *IminusKH   = NULL;
    mat_t *IminusKHt  = NULL;
    mat_t *KRKt       = NULL;
    mat_t *Kt         = NULL;
    mat_t *KR         = NULL;
    mat_t *P_temp1    = NULL;
    mat_t *P_temp2    = NULL;

    // Step 1: H' (transpose of H)
    if (info) {
        Ht = MatTr(H);
        if (Ht == NULL) info = 0;
    }

    // Step 2: HP = H * P
    if (info) {
        HP = MatMul(1.0, H, false, 1.0, P, false);
        if (HP == NULL) info = 0;
    }

    // Step 3: HPHt = H * P * H' (predicted measurement covariance)
    if (info) {
        HPHt = MatMul(1.0, HP, false, 1.0, Ht, false);
        if (HPHt == NULL) info = 0;
    }

    // Step 4: S = H * P * H' + R (innovation covariance)
    if (info) {
        S = MatCopy(HPHt);
        if (S == NULL) info = 0;
    }

    if (info) {
        // S = HPHt + R
        for (int i = 0; i < S->rows && info; ++i) {
            for (int j = 0; j < S->cols && info; ++j) {
                double val = MatGetD(S, i, j) + MatGetD(R, i, j);
                MatSetD(S, i, j, val);
            }
        }
    }

    // Step 5: Sinv = inv(S) (innovation covariance inverse)
    if (info) {
        Sinv = MatInv(1.0, S, false);
        if (Sinv == NULL) info = 0;
    }

    // Step 6: PHt = P * H'
    if (info) {
        PHt = MatMul(1.0, P, false, 1.0, Ht, false);
        if (PHt == NULL) info = 0;
    }

    // Step 7: K = P * H' * inv(S) (Kalman gain)
    if (info) {
        K_temp = MatMul(1.0, PHt, false, 1.0, Sinv, false);
        if (K_temp == NULL) info = 0;
    }

    // Step 8: x = x + K * v (state update, only if x is provided)
    if (info && x != NULL) {
        Kv = MatMul(1.0, K_temp, false, 1.0, v, false);
        if (Kv == NULL) info = 0;
    }

    if (info && x != NULL) {
        x_temp = MatCopy(x);
        if (x_temp == NULL) info = 0;
    }

    if (info && x != NULL) {
        // x_temp = x + Kv
        for (int i = 0; i < x_temp->rows && info; ++i) {
            for (int j = 0; j < x_temp->cols && info; ++j) {
                double val = MatGetD(x_temp, i, j) + MatGetD(Kv, i, j);
                MatSetD(x_temp, i, j, val);
            }
        }
    }

    // Step 9: KH = K * H
    if (info) {
        KH = MatMul(1.0, K_temp, false, 1.0, H, false);
        if (KH == NULL) info = 0;
    }

    // Step 10: I = eye(n) (identity matrix)
    if (info) {
        I = Eye(P->rows, DOUBLE);
        if (I == NULL) info = 0;
    }

    // Step 11: IminusKH = I - K * H
    if (info) {
        IminusKH = MatCopy(I);
        if (IminusKH == NULL) info = 0;
    }

    if (info) {
        // IminusKH = I - KH
        for (int i = 0; i < IminusKH->rows && info; ++i) {
            for (int j = 0; j < IminusKH->cols && info; ++j) {
                double val = MatGetD(IminusKH, i, j) - MatGetD(KH, i, j);
                MatSetD(IminusKH, i, j, val);
            }
        }
    }

    // Step 12: Joseph form covariance update: P = (I - K*H) * P * (I - K*H)' + K * R * K'

    // Step 12a: (I - K*H)' = transpose of (I - K*H)
    if (info) {
        IminusKHt = MatTr(IminusKH);
        if (IminusKHt == NULL) info = 0;
    }

    // Step 12b: P_temp1 = (I - K*H) * P
    if (info) {
        P_temp1 = MatMul(1.0, IminusKH, false, 1.0, P, false);
        if (P_temp1 == NULL) info = 0;
    }

    // Step 12c: P_temp2 = P_temp1 * (I - K*H)' = (I - K*H) * P * (I - K*H)'
    if (info) {
        P_temp2 = MatMul(1.0, P_temp1, false, 1.0, IminusKHt, false);
        if (P_temp2 == NULL) info = 0;
    }

    // Step 12d: K' = transpose of K
    if (info) {
        Kt = MatTr(K_temp);
        if (Kt == NULL) info = 0;
    }

    // Step 12e: KR = K * R
    if (info) {
        KR = MatMul(1.0, K_temp, false, 1.0, R, false);
        if (KR == NULL) info = 0;
    }

    // Step 12f: KRKt = K * R * K'
    if (info) {
        KRKt = MatMul(1.0, KR, false, 1.0, Kt, false);
        if (KRKt == NULL) info = 0;
    }

    // Step 12g: P = (I - K*H) * P * (I - K*H)' + K * R * K' (Joseph form)
    if (info) {
        // P_temp2 = P_temp2 + KRKt
        for (int i = 0; i < P_temp2->rows && info; ++i) {
            for (int j = 0; j < P_temp2->cols && info; ++j) {
                double val = MatGetD(P_temp2, i, j) + MatGetD(KRKt, i, j);
                MatSetD(P_temp2, i, j, val);
            }
        }
    }

    // Copy results to output matrices only if successful
    if (info) {
        if (x != NULL && !MatCopyIn(x, x_temp)) info = 0;
        if (info && !MatCopyIn(P, P_temp2)) info = 0;
        if (info && K != NULL && !MatCopyIn(K, K_temp)) info = 0;
    }

    // Clean up all temporary matrices (FreeMat handles NULL pointers safely)
    FreeMat(Ht);
    FreeMat(HP);
    FreeMat(HPHt);
    FreeMat(S);
    FreeMat(Sinv);
    FreeMat(PHt);
    FreeMat(K_temp);
    FreeMat(Kv);
    FreeMat(x_temp);
    FreeMat(KH);
    FreeMat(I);
    FreeMat(IminusKH);
    FreeMat(IminusKHt);
    FreeMat(KRKt);
    FreeMat(Kt);
    FreeMat(KR);
    FreeMat(P_temp1);
    FreeMat(P_temp2);

    return info;
}


// =============================================================================
// Matrix indexing functions
// =============================================================================



// =============================================================================
// End of file
// =============================================================================
