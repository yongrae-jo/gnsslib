// =============================================================================
// Matrix functions
//
// -----------------------------------------------------------------------------
// Yongrae Jo, 0727ggame@sju.ac.kr
// =============================================================================

#include "matrix.h"
#include <stdlib.h>     // for posix_memalign, malloc, free
#include <stddef.h>     // for size_t
#include <assert.h>     // for assert
#include <math.h>       // for fabs, sqrt, floor
#include <stdio.h>      // for printf, snprintf
#include <string.h>     // for strlen, memset, memcpy

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
    assert(A != NULL && B != NULL);
    assert(A->cols == B->rows);
    assert(A->type == B->type);

    int m = A->rows, n = A->cols, p = B->cols;

    // Check column
    assert(n > 0);

    // Result matrix
    mat_t *C = Mat(m, p, A->type);
    assert(C != NULL);

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
    assert(A != NULL);
    assert(A->rows == A->cols);  // Must be square matrix
    assert(A->type == DOUBLE);   // Only double type supported for inverse

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
    assert(LU != NULL);

    // Copy original matrix
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            MatSetD(LU, i, j, MatGetD(A, i, j));
        }
    }

    // Create identity matrix for right-hand side
    mat_t *I = Eye(n, DOUBLE);
    assert(I != NULL);

    // Create result matrix
    mat_t *inv = Mat(n, n, DOUBLE);
    assert(inv != NULL);

    int *pivot = (int *)malloc(n * sizeof(int));
    assert(pivot != NULL);

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
    assert(A != NULL && B != NULL);
    assert(A->type == DOUBLE && B->type == DOUBLE);  // Only double type supported
    assert(A->rows == B->rows && A->cols == B->cols);  // Must have same dimensions

    int rows = A->rows, cols = A->cols;

    // Result matrix
    mat_t *C = Mat(rows, cols, DOUBLE);
    assert(C != NULL);

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
// Basic matrix/vector operations functions
// =============================================================================

// Matrix generation
mat_t *Mat(int rows, int cols, type_t type)
{
    assert(rows >= 0 && cols >= 0);

    mat_t *mat = (mat_t *)malloc(sizeof(mat_t));
    assert(mat != NULL);

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
    assert(mat->data != NULL);
#else
    int err = posix_memalign(&mat->data, alignment, tsize);
    assert(err == 0);
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
idx_t *Idx(int n)
{
    // Check number of indices
    if (n < 0) return NULL;

    idx_t *idx = (idx_t *)malloc(sizeof(idx_t));
    if (idx == NULL) return NULL;

    idx->n = (uint32_t)n;
    idx->idx = NULL;

    // If empty index vector, return valid struct with NULL data
    if (idx->n == 0) return idx;

    // Allocate memory for index array
    idx->idx = (uint32_t *)malloc(idx->n * sizeof(uint32_t));
    if (idx->idx == NULL) {
        free(idx);
        return NULL;  // Memory allocation failed
    }

    size_t alignment = MAT_ALIGNMENT;
    size_t tsize = (size_t)idx->n * sizeof(uint32_t);

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
    assert(size >= 0);

    // Generate identity matrix
    mat_t *I = Mat(size, size, type);
    assert(I != NULL);

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
    assert(rows >= 0 && cols >= 0);

    // Generate zero matrix
    mat_t *Z = Mat(rows, cols, type);
    assert(Z != NULL);

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
    assert(rows >= 0 && cols >= 0);

    // Generate ones matrix
    mat_t *O = Mat(rows, cols, type);
    assert(O != NULL);

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

// =============================================================================
// Matrix operations functions
// =============================================================================

// Matrix transpose
mat_t *MatTr(const mat_t *A)
{
    // Check input matrix
    assert(A != NULL);

    // Generate transposed matrix
    mat_t *T = Mat(A->cols, A->rows, A->type);
    assert(T != NULL);

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
    assert(A != NULL);

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
    assert(A != NULL && B != NULL);
    assert(A->type == DOUBLE && B->type == DOUBLE);  // Only double type supported

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
    assert(A != NULL && B != NULL);
    assert(A->type == DOUBLE && B->type == DOUBLE);  // Only double type supported

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
    assert(A != NULL && B != NULL);
    assert(A->type == DOUBLE && B->type == DOUBLE);  // Only double type supported

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
    assert(A != NULL);
    assert(A->type == DOUBLE);  // Only double type supported
    assert(A->rows == A->cols); // Must be square matrix

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
    assert(A != NULL && B != NULL);
    assert(A->type == DOUBLE && B->type == DOUBLE);  // Only double type supported

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
    assert(A != NULL);
    assert(A->type == DOUBLE);  // Only double type supported
    assert(A->rows == A->cols); // Must be square matrix

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
// Vector (Matrix of 1 row) operations functions
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
    if (a->cols == 0) return 0;

    *c = 0.0;

    for (int i = 0; i < a->cols; ++i) {
        *c += MatGetD(a, 0, i) * MatGetD(b, 0, i);
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

// Vector 2-norm (Euclidean norm, DOUBLE only)
double Norm(const mat_t *a)
{
    // Check input matrix
    if (a == NULL) return 0.0;
    if (a->cols != 1) return 0.0;
    if (a->type != DOUBLE) return 0.0;

    double norm = 0.0;
    for (int i = 0; i < a->rows; ++i) {
        norm += MatGetD(a, i, 0) * MatGetD(a, i, 0);
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
    assert(A != NULL);
    assert(A->rows == A->cols);  // Must be square matrix
    assert(A->type == DOUBLE);   // Only double type supported for determinant

    int n = A->rows;

    // Empty matrix determinant is 1.0 (convention)
    if (n == 0) return 1.0;

    // For 1x1 matrix, determinant is the element itself
    if (n == 1) return MatGetD(A, 0, 0);

    // Create working copy of matrix for LU decomposition
    mat_t *LU = Mat(n, n, DOUBLE);
    assert(LU != NULL);

    // Copy original matrix to LU
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            MatSetD(LU, i, j, MatGetD(A, i, j));
        }
    }

    double det = 1.0;
    int *pivot = (int *)malloc(n * sizeof(int));
    assert(pivot != NULL);

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

// Matrix copy - returns new matrix with copied data
mat_t *MatCopy(const mat_t *A)
{
    // Check input matrix
    assert(A != NULL);

    // Create result matrix
    mat_t *B = Mat(A->rows, A->cols, A->type);
    assert(B != NULL);

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
    assert(src != NULL && des != NULL);
    assert(src->rows == des->rows && src->cols == des->cols);
    assert(src->type == des->type);

    size_t esize = (src->type == DOUBLE) ? sizeof(double) : sizeof(int);
    size_t total = (size_t)src->rows * src->cols * esize;
    memcpy(des->data, src->data, total);

    return 1;
}

// =============================================================================
// Advanced algorithms functions
// =============================================================================

// Least square estimation
int Lsq(const mat_t *H, const mat_t *y, const mat_t *R, mat_t *x, mat_t *P, mat_t *Hl)
{
    assert(H != NULL && y != NULL && R != NULL && x != NULL && P != NULL);

    // Check measurement dimension
    assert(H->rows == y->rows && H->rows == R->rows);

    // Check state dimension
    assert(H->cols == x->rows && H->cols == P->rows && H->cols == P->cols);

    // Check matrix type (Only double type is supported)
    assert(H->type == DOUBLE && y->type == DOUBLE && R->type == DOUBLE && P->type == DOUBLE);

    // Check Least square inverse matrix
    if (Hl != NULL) {
        assert(Hl->rows == H->cols && Hl->cols == H->rows);
        assert(Hl->type == DOUBLE);
    }

    // Status variable: 1 = success, 0 = failure
    int info = 1;

    // Declare all temporary matrices (initialized to NULL)
    mat_t *Ht       = NULL;
    mat_t *W        = NULL;
    mat_t *HtW      = NULL;
    mat_t *HtWH     = NULL;
    mat_t *Q        = NULL;
    mat_t *x_temp   = NULL;
    mat_t *P_temp   = NULL;
    mat_t *L        = NULL;
    mat_t *Lt       = NULL;

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
    if (info) {
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
        if (!MatCopyIn(x, x_temp)) info = 0;
        if (info && !MatCopyIn(P, P_temp)) info = 0;
        if (info && Hl != NULL && !MatCopyIn(Hl, L)) info = 0;
    }

    // Clean up all temporary matrices (FreeMat handles NULL pointers safely)
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
    assert(H != NULL && v != NULL && R != NULL && x != NULL && P != NULL);

    // Check measurement dimension
    assert(H->rows == v->rows && H->rows == R->rows);

    // Check state dimension
    assert(H->cols == x->rows && H->cols == P->rows && H->cols == P->cols);

    // Check matrix type (Only double type is supported)
    assert(H->type == DOUBLE && v->type == DOUBLE && R->type == DOUBLE && P->type == DOUBLE);

    // Check Kalman gain matrix
    if (K != NULL) {
        assert(K->rows == H->cols && K->cols == H->rows);
        assert(K->type == DOUBLE);
    }

    // Status variable: 1 = success, 0 = failure
    int info = 1;

    // Declare all temporary matrices (initialized to NULL)
    mat_t *Ht       = NULL;
    mat_t *HP       = NULL;
    mat_t *HPHt     = NULL;
    mat_t *S        = NULL;
    mat_t *Sinv     = NULL;
    mat_t *PHt      = NULL;
    mat_t *K_temp   = NULL;
    mat_t *Kv       = NULL;
    mat_t *x_temp   = NULL;
    mat_t *KH       = NULL;
    mat_t *I        = NULL;
    mat_t *IminusKH = NULL;
    mat_t *P_temp   = NULL;

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

    // Step 8: x = x + K * v (state update)
    if (info) {
        Kv = MatMul(1.0, K_temp, false, 1.0, v, false);
        if (Kv == NULL) info = 0;
    }

    if (info) {
        x_temp = MatCopy(x);
        if (x_temp == NULL) info = 0;
    }

    if (info) {
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
        I = Eye(x->rows, DOUBLE);
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

    // Step 12: P = (I - K * H) * P (covariance update)
    if (info) {
        P_temp = MatMul(1.0, IminusKH, false, 1.0, P, false);
        if (P_temp == NULL) info = 0;
    }

    // Copy results to output matrices only if successful
    if (info) {
        if (!MatCopyIn(x, x_temp)) info = 0;
        if (info && !MatCopyIn(P, P_temp)) info = 0;
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
    FreeMat(P_temp);

    return info;
}

// =============================================================================
// End of file
// =============================================================================
