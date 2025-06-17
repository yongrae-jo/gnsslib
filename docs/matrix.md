---
layout: default
title: 고성능 행렬 연산 모듈 (matrix)
---

# 고성능 행렬 연산 모듈 (matrix)

GNSS 정밀 측위를 위한 고성능 수치 연산 엔진입니다.

## 목차

1. [기본 개념](#1-기본-개념)
2. [데이터 타입 구조](#2-데이터-타입-구조)
3. [데이터 타입 목록](#3-데이터-타입-목록)
4. [함수 구조](#4-함수-구조)
5. [함수 목록](#5-함수-목록)
6. [사용 예시](#6-사용-예시)
7. [성능 특성](#7-성능-특성)

---

## 1. 기본 개념

### 1.1 Column-major 저장 방식
행렬 데이터는 **열 우선(column-major)** 방식으로 연속 메모리에 저장됩니다.

**예시: 3×3 행렬 저장**
```
행렬 A:
[ a11 a12 a13 ]
[ a21 a22 a23 ]     메모리: [a11, a21, a31, a12, a22, a32, a13, a23, a33]
[ a31 a32 a33 ]     접근:   A[i + j*rows]
```

### 1.2 SIMD 정렬 최적화
- **32바이트 경계 정렬**: AVX 명령어 최적화
- **플랫폼별 할당**: Windows `_aligned_malloc()`, POSIX `posix_memalign()`
- **성능 향상**: 벡터화 연산으로 4-8배 속도 개선

### 1.3 메모리 관리 체계
- **초기화/해제 쌍**: `Mat()`/`FreeMat()`, `Idx()`/`FreeIdx()`
- **NULL 안전성**: 모든 해제 함수에서 NULL 포인터 검증
- **빈 행렬 지원**: 0×0 차원 행렬 처리 가능

### 1.4 확장된 타입 시스템
세 가지 데이터 타입을 지원하여 메모리 효율성과 정확도를 동시에 제공합니다:
- **DOUBLE**: 64비트 부동소수점 (GNSS 계산용) - mat_t 전용
- **INT**: 32비트 정수 (인덱스, 플래그용) - mat_t 전용
- **BOOL**: 불리언 타입 (논리 마스크용) - idx_t 전용

### 1.5 고급 인덱싱 시스템
Matrix 모듈은 MATLAB/NumPy 스타일의 고급 인덱싱을 지원합니다:
- **벡터 인덱싱**: 정수 인덱스 배열을 사용한 부분 행렬 추출
- **논리 인덱싱**: 불리언 마스크를 사용한 조건부 선택
- **제자리 연산**: 메모리 효율적인 인덱싱 처리

---

## 2. 데이터 타입 구조

```
matrix 모듈 타입 계층
├── type_t (enum)
│   ├── BOOL ───────────────── 불리언 타입 (idx_t 전용)
│   ├── INT ────────────────── 32비트 정수 타입 (mat_t 전용)
│   └── DOUBLE ─────────────── 64비트 부동소수점 타입 (mat_t 전용)
├── idx_t (struct)
│   ├── n ──────────────────── 인덱스 개수
│   ├── type ───────────────── 인덱스 타입 (BOOL/INT)
│   └── idx ────────────────── 인덱스 배열 포인터 (32바이트 정렬)
└── mat_t (struct)
    ├── rows ───────────────── 행 개수
    ├── cols ───────────────── 열 개수
    ├── type ───────────────── 데이터 타입 (INT/DOUBLE)
    └── data ───────────────── 32바이트 정렬 데이터 포인터 (column-major)
```

---

## 3. 데이터 타입 목록

### 3.1 type_t (enum)
<details>
<summary>상세 설명</summary>

**목적**: 행렬/벡터 데이터 타입 구분

**정의**:
```c
typedef enum {BOOL, INT, DOUBLE} type_t;
```

**값**:
- `BOOL`: 불리언 타입 (idx_t 전용)
- `INT`: 32비트 정수 타입 (mat_t 전용)
- `DOUBLE`: 64비트 부동소수점 타입 (mat_t 전용)

**사용**: 모든 행렬/벡터 생성 시 타입 지정에 활용, 타입별 메모리 크기 최적화

</details>

### 3.2 idx_t (struct)
<details>
<summary>상세 설명</summary>

**목적**: 행렬 인덱싱용 인덱스 벡터 구조

**정의**:
```c
typedef struct idx {
    int n;         // 인덱스 개수
    type_t type;   // 인덱스 타입 (BOOL/INT)
    void *idx;     // 인덱스 배열
} idx_t;
```

**특징**:
- 행렬/벡터 인덱싱에 사용
- BOOL/INT 타입 지원, 32바이트 정렬
- 동적 크기, 메모리 안전 해제 지원
- 벡터 인덱싱과 논리 인덱싱 지원

**접근 함수**:
- `IdxGetI()/IdxSetI()`: INT 타입 인덱스 접근/설정
- `IdxGetB()/IdxSetB()`: BOOL 타입 인덱스 접근/설정

</details>

### 3.3 mat_t (struct)
<details>
<summary>상세 설명</summary>

**목적**: 2차원 행렬 데이터 구조

**정의**:
```c
typedef struct mat {
    int rows, cols;     // 행×열 차원
    type_t type;        // 데이터 타입 (INT/DOUBLE)
    void *data;         // 32바이트 정렬 데이터 포인터
} mat_t;
```

**특징**:
- Column-major 저장 방식 (FORTRAN/MATLAB 호환)
- SIMD 최적화를 위한 32바이트 정렬
- 빈 행렬(0×0) 지원
- INT/DOUBLE 타입만 지원 (BOOL 타입 차단)

**메모리 레이아웃**:
- 실제 크기: `sizeof(type) * rows * cols`
- 정렬 크기: 32바이트 배수로 할당
- 접근 공식: `data[i + j*rows]` (i:행, j:열)

**접근 함수**:
- `MatGetD()/MatSetD()`: DOUBLE 타입 접근
- `MatGetI()/MatSetI()`: INT 타입 접근

</details>

---

## 4. 함수 구조

```
matrix 모듈 함수 계층
├── 기본 생성/해제
│   ├── Mat() ──────────────── 행렬 생성 (SIMD 정렬)
│   ├── FreeMat() ─────────── 행렬 메모리 해제
│   ├── Idx() ──────────────── 인덱스 벡터 생성 (SIMD 정렬)
│   └── FreeIdx() ─────────── 인덱스 벡터 해제
├── 특수 행렬 생성
│   ├── Eye() ──────────────── 단위행렬 생성
│   ├── Zeros() ───────────── 영행렬 생성
│   ├── Ones() ────────────── 일행렬 생성
│   ├── TrueIdx() ─────────── True 인덱스 벡터 생성
│   └── FalseIdx() ────────── False 인덱스 벡터 생성
├── 행렬 복사/변환
│   ├── MatCopy() ─────────── 행렬 복사
│   ├── MatCopyIn() ───────── 제자리 복사
│   ├── MatTr() ───────────── 행렬 전치
│   └── MatTrIn() ─────────── 제자리 전치
├── 고급 인덱싱
│   ├── MatVecIdx() ───────── 벡터 인덱싱
│   ├── MatVecIdxIn() ─────── 제자리 벡터 인덱싱
│   ├── MatLogIdx() ───────── 논리 인덱싱
│   └── MatLogIdxIn() ─────── 제자리 논리 인덱싱
├── 행렬 연산
│   ├── MatAdd() ──────────── 일반화 행렬 덧셈 $[C = a \cdot A^{T_A} + b \cdot B^{T_B}]$
│   ├── MatAddIn() ────────── 제자리 덧셈
│   ├── MatMul() ──────────── 일반화 행렬 곱셈 $[C = a \cdot A^{T_A} \times b \cdot B^{T_B}]$
│   ├── MatMulIn() ────────── 제자리 곱셈
│   ├── MatInv() ──────────── 행렬 역행렬 $[A_i = \text{inv}(a \cdot A^{T_A})]$
│   └── MatInvIn() ────────── 제자리 역행렬
├── 벡터 연산 (mat_t, N×1 또는 1×N)
│   ├── Dot() ──────────────── 벡터 내적 (출력 매개변수)
│   ├── Cross3() ──────────── 3차원 외적 (출력 매개변수)
│   └── Norm() ────────────── 유클리드 노름 (DOUBLE 전용)
├── 분석 함수
│   └── MatDet() ──────────── 행렬식 계산 (LU 분해)
└── 고급 알고리즘
    ├── Interp() ──────────── 선형 보간 (1차원)
    ├── Lsq() ─────────────── 최소제곱법 (optional 매개변수 지원)
    └── Ekf() ─────────────── 확장칼만필터 (Joseph 형태)
```

---

## 5. 함수 목록

### 5.1 기본 생성/해제 함수

#### 5.1.1 Mat() - 행렬 생성
<details>
<summary>상세 설명</summary>

**목적**: 행렬 생성 및 메모리 할당

**입력**:
- `int rows`: 행 개수 (≥ 0)
- `int cols`: 열 개수 (≥ 0)
- `type_t type`: 데이터 타입 (DOUBLE 또는 INT 전용)

**출력**:
- `mat_t *`: 할당된 행렬 구조체 (오류 시 NULL)

**함수 로직**:
- 32바이트 경계 정렬 메모리 할당
- Column-major 저장 방식 적용
- BOOL 타입 차단으로 타입 안전성 보장
- 0×0 차원 빈 행렬 지원

**사용 예시**:
```c
// 3×3 DOUBLE 행렬 생성
mat_t *A = Mat(3, 3, DOUBLE);
if (A) {
    printf("행렬 A 생성 성공: %d×%d, 타입: %d\n", A->rows, A->cols, A->type);

    // 행렬 원소 설정
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            MatSetD(A, i, j, i * 3 + j + 1);  // 1~9 값 설정
        }
    }

    // 행렬 출력
    printf("행렬 A:\n");
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            printf("%6.1f", MatGetD(A, i, j));
        }
        printf("\n");
    }
}

// 빈 행렬 생성 테스트
mat_t *empty = Mat(0, 0, DOUBLE);
if (empty) {
    printf("빈 행렬 생성 성공: %d×%d\n", empty->rows, empty->cols);
}

// 메모리 해제
FreeMat(A);
FreeMat(empty);
```

</details>

#### 5.1.2 FreeMat() - 행렬 메모리 해제
<details>
<summary>상세 설명</summary>

**목적**: 행렬 메모리 해제

**입력**:
- `mat_t *mat`: 해제할 행렬 구조체

**출력**:
- `void`: 반환값 없음

**함수 로직**:
- NULL 포인터 안전 처리
- SIMD 정렬 메모리 해제
- 구조체 메모리 해제

**사용 예시**:
```c
// 행렬 생성 및 해제
mat_t *A = Mat(5, 5, DOUBLE);
mat_t *B = Mat(3, 4, INT);

// 행렬 사용...
printf("행렬 A: %d×%d\n", A->rows, A->cols);
printf("행렬 B: %d×%d\n", B->rows, B->cols);

// 안전한 메모리 해제
FreeMat(A);  // A는 NULL로 설정되지 않음 (사용자가 관리)
FreeMat(B);

// NULL 포인터 해제 테스트 (안전함)
mat_t *null_mat = NULL;
FreeMat(null_mat);  // 아무 일도 일어나지 않음
printf("NULL 포인터 해제 완료\n");
```

</details>

#### 5.1.3 Idx() - 인덱스 벡터 생성
<details>
<summary>상세 설명</summary>

**목적**: 인덱스 벡터 생성 및 메모리 할당

**입력**:
- `int n`: 인덱스 개수 (≥ 0)
- `type_t type`: 인덱스 타입 (BOOL 또는 INT)

**출력**:
- `idx_t *`: 할당된 인덱스 벡터 구조체 (오류 시 NULL)

**함수 로직**:
- 32바이트 경계 정렬 메모리 할당
- 타입별 메모리 크기 최적화
- BOOL/INT 타입 지원

**사용 예시**:
```c
// INT 타입 인덱스 벡터 생성 (행 인덱스용)
idx_t *row_idx = Idx(3, INT);
if (row_idx) {
    // 인덱스 설정: [0, 2, 4]
    IdxSetI(row_idx, 0, 0);
    IdxSetI(row_idx, 1, 2);
    IdxSetI(row_idx, 2, 4);

    printf("행 인덱스: ");
    for (int i = 0; i < row_idx->n; i++) {
        printf("%d ", IdxGetI(row_idx, i));
    }
    printf("\n");
}

// BOOL 타입 인덱스 벡터 생성 (논리 마스크용)
idx_t *mask = Idx(5, BOOL);
if (mask) {
    // 마스크 설정: [true, false, true, false, true]
    IdxSetB(mask, 0, true);
    IdxSetB(mask, 1, false);
    IdxSetB(mask, 2, true);
    IdxSetB(mask, 3, false);
    IdxSetB(mask, 4, true);

    printf("논리 마스크: ");
    for (int i = 0; i < mask->n; i++) {
        printf("%s ", IdxGetB(mask, i) ? "T" : "F");
    }
    printf("\n");
}

// 메모리 해제
FreeIdx(row_idx);
FreeIdx(mask);
```

</details>

#### 5.1.4 FreeIdx() - 인덱스 벡터 해제
<details>
<summary>상세 설명</summary>

**목적**: 인덱스 벡터 메모리 해제

**입력**:
- `idx_t *idx`: 해제할 인덱스 벡터 구조체

**출력**:
- `void`: 반환값 없음

**함수 로직**:
- NULL 포인터 안전 처리
- SIMD 정렬 메모리 해제

</details>

### 5.2 특수 행렬 생성 함수

#### 5.2.1 Eye() - 단위행렬 생성
<details>
<summary>상세 설명</summary>

**목적**: 단위행렬 생성

**입력**:
- `int size`: 행렬 크기 (size × size, ≥ 0)
- `type_t type`: 데이터 타입 (DOUBLE 또는 INT 전용)

**출력**:
- `mat_t *`: 단위행렬 (오류 시 NULL)

**함수 로직**:
- 대각선 원소를 1로 설정
- 나머지 원소는 0으로 초기화
- BOOL 타입 차단

**사용 예시**:
```c
// 3×3 단위행렬 생성
mat_t *I3 = Eye(3, DOUBLE);
if (I3) {
    printf("3×3 단위행렬:\n");
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            printf("%4.1f", MatGetD(I3, i, j));
        }
        printf("\n");
    }
}

// 5×5 INT 타입 단위행렬
mat_t *I5 = Eye(5, INT);
if (I5) {
    printf("5×5 INT 단위행렬 대각선: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", MatGetI(I5, i, i));
    }
    printf("\n");
}

// 1×1 단위행렬 (스칼라)
mat_t *I1 = Eye(1, DOUBLE);
if (I1) {
    printf("1×1 단위행렬: %.1f\n", MatGetD(I1, 0, 0));
}

FreeMat(I3); FreeMat(I5); FreeMat(I1);
```

</details>

#### 5.2.2 Zeros() - 영행렬 생성
<details>
<summary>상세 설명</summary>

**목적**: 영행렬 생성

**입력**:
- `int rows`: 행 개수 (≥ 0)
- `int cols`: 열 개수 (≥ 0)
- `type_t type`: 데이터 타입 (DOUBLE 또는 INT 전용)

**출력**:
- `mat_t *`: 영행렬 (오류 시 NULL)

**함수 로직**:
- 모든 원소를 0으로 초기화
- BOOL 타입 차단

**사용 예시**:
```c
// 4×3 DOUBLE 영행렬 생성
mat_t *Z1 = Zeros(4, 3, DOUBLE);
if (Z1) {
    printf("4×3 영행렬:\n");
    for (int i = 0; i < Z1->rows; i++) {
        for (int j = 0; j < Z1->cols; j++) {
            printf("%4.1f", MatGetD(Z1, i, j));
        }
        printf("\n");
    }
}

// 2×2 INT 영행렬 생성
mat_t *Z2 = Zeros(2, 2, INT);
if (Z2) {
    printf("2×2 INT 영행렬:\n");
    for (int i = 0; i < Z2->rows; i++) {
        for (int j = 0; j < Z2->cols; j++) {
            printf("%3d", MatGetI(Z2, i, j));
        }
        printf("\n");
    }
}

// 빈 영행렬 (0×0)
mat_t *Z_empty = Zeros(0, 0, DOUBLE);
if (Z_empty) {
    printf("빈 영행렬 생성 성공: %d×%d\n", Z_empty->rows, Z_empty->cols);
}

FreeMat(Z1); FreeMat(Z2); FreeMat(Z_empty);
```

</details>

#### 5.2.3 Ones() - 일행렬 생성
<details>
<summary>상세 설명</summary>

**목적**: 일행렬 생성

**입력**:
- `int rows`: 행 개수 (≥ 0)
- `int cols`: 열 개수 (≥ 0)
- `type_t type`: 데이터 타입 (DOUBLE 또는 INT 전용)

**출력**:
- `mat_t *`: 일행렬 (오류 시 NULL)

**함수 로직**:
- 모든 원소를 1로 초기화
- BOOL 타입 차단

**사용 예시**:
```c
// 3×4 DOUBLE 일행렬 생성
mat_t *O1 = Ones(3, 4, DOUBLE);
if (O1) {
    printf("3×4 일행렬:\n");
    for (int i = 0; i < O1->rows; i++) {
        for (int j = 0; j < O1->cols; j++) {
            printf("%4.1f", MatGetD(O1, i, j));
        }
        printf("\n");
    }
}

// 벡터 형태의 일행렬 (5×1)
mat_t *ones_vec = Ones(5, 1, INT);
if (ones_vec) {
    printf("5×1 일벡터: ");
    for (int i = 0; i < ones_vec->rows; i++) {
        printf("%d ", MatGetI(ones_vec, i, 0));
    }
    printf("\n");
}

// 가중치 벡터 생성 (일행렬에 스칼라 곱셈)
mat_t *weights = Ones(3, 1, DOUBLE);
if (weights) {
    // 모든 원소에 0.5 곱하기
    for (int i = 0; i < weights->rows; i++) {
        MatSetD(weights, i, 0, MatGetD(weights, i, 0) * 0.5);
    }

    printf("가중치 벡터: ");
    for (int i = 0; i < weights->rows; i++) {
        printf("%.1f ", MatGetD(weights, i, 0));
    }
    printf("\n");
}

FreeMat(O1); FreeMat(ones_vec); FreeMat(weights);
```

</details>

#### 5.2.4 TrueIdx() - True 인덱스 벡터 생성
<details>
<summary>상세 설명</summary>

**목적**: 모든 원소가 true인 불리언 인덱스 벡터 생성

**입력**:
- `int n`: 인덱스 개수 (≥ 0)

**출력**:
- `idx_t *`: true 인덱스 벡터 (오류 시 NULL)

**함수 로직**:
- BOOL 타입으로 인덱스 벡터 생성
- 모든 원소를 true로 초기화

**사용 예시**:
```c
// 5개 원소의 true 마스크 생성
idx_t *all_true = TrueIdx(5);
if (all_true) {
    printf("전체 선택 마스크: ");
    for (int i = 0; i < all_true->n; i++) {
        printf("%s ", IdxGetB(all_true, i) ? "T" : "F");
    }
    printf("\n");
}

// 행렬의 모든 행/열 선택용 마스크
mat_t *A = Mat(4, 3, DOUBLE);
// A에 데이터 설정...
for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 3; j++) {
        MatSetD(A, i, j, i * 3 + j + 1);
    }
}

idx_t *row_mask = TrueIdx(4);  // 모든 행 선택
idx_t *col_mask = TrueIdx(3);  // 모든 열 선택

// 논리 인덱싱으로 전체 행렬 복사 (실제로는 복사와 동일)
mat_t *A_copy = MatLogIdx(A, row_mask, col_mask);
if (A_copy) {
    printf("논리 인덱싱으로 복사된 행렬:\n");
    for (int i = 0; i < A_copy->rows; i++) {
        for (int j = 0; j < A_copy->cols; j++) {
            printf("%4.0f", MatGetD(A_copy, i, j));
        }
        printf("\n");
    }
}

FreeMat(A); FreeMat(A_copy);
FreeIdx(all_true); FreeIdx(row_mask); FreeIdx(col_mask);
```

</details>

#### 5.2.5 FalseIdx() - False 인덱스 벡터 생성
<details>
<summary>상세 설명</summary>

**목적**: 모든 원소가 false인 불리언 인덱스 벡터 생성

**입력**:
- `int n`: 인덱스 개수 (≥ 0)

**출력**:
- `idx_t *`: false 인덱스 벡터 (오류 시 NULL)

**함수 로직**:
- BOOL 타입으로 인덱스 벡터 생성
- 모든 원소를 false로 초기화

**사용 예시**:
```c
// 6개 원소의 false 마스크 생성
idx_t *all_false = FalseIdx(6);
if (all_false) {
    printf("전체 제외 마스크: ");
    for (int i = 0; i < all_false->n; i++) {
        printf("%s ", IdxGetB(all_false, i) ? "T" : "F");
    }
    printf("\n");
}

// 조건부 마스크 생성 (false로 시작해서 조건에 맞는 것만 true로 변경)
idx_t *selective_mask = FalseIdx(8);
if (selective_mask) {
    // 짝수 인덱스만 true로 설정
    for (int i = 0; i < selective_mask->n; i++) {
        if (i % 2 == 0) {
            IdxSetB(selective_mask, i, true);
        }
    }

    printf("짝수 인덱스 선택 마스크: ");
    for (int i = 0; i < selective_mask->n; i++) {
        printf("%s ", IdxGetB(selective_mask, i) ? "T" : "F");
    }
    printf("\n");
}

// 빈 선택 마스크 (아무것도 선택하지 않음)
idx_t *empty_mask = FalseIdx(0);
if (empty_mask) {
    printf("빈 마스크 생성 성공: 크기 %d\n", empty_mask->n);
}

FreeIdx(all_false); FreeIdx(selective_mask); FreeIdx(empty_mask);
```

</details>

### 5.3 행렬 복사/변환 함수

#### 5.3.1 MatCopy() - 행렬 복사
<details>
<summary>상세 설명</summary>

**목적**: 행렬을 새로운 메모리에 복사

**입력**:
- `const mat_t *A`: 소스 행렬

**출력**:
- `mat_t *`: 복사된 행렬 (오류 시 NULL)

**함수 로직**:
- 새로운 행렬 메모리 할당
- 데이터 완전 복사
- 동일한 차원과 타입 유지

</details>

#### 5.3.2 MatCopyIn() - 제자리 복사
<details>
<summary>상세 설명</summary>

**목적**: 기존 행렬에 소스 행렬 데이터 복사

**입력**:
- `mat_t *des`: 목적지 행렬
- `const mat_t *src`: 소스 행렬

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:
- 차원과 타입 일치 검증
- 데이터 제자리 복사
- 메모리 효율적 처리

**사용 예시**:
```c
// 소스 행렬 생성 및 데이터 설정
mat_t *src = Mat(3, 3, DOUBLE);
for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
        MatSetD(src, i, j, (i + 1) * 10 + (j + 1));  // 11, 12, 13, 21, 22, 23, ...
    }
}

// 목적지 행렬 생성 (같은 크기)
mat_t *dest = Mat(3, 3, DOUBLE);

// 제자리 복사 수행
int success = MatCopyIn(dest, src);
if (success) {
    printf("제자리 복사 성공!\n");
    printf("복사된 행렬:\n");
    for (int i = 0; i < dest->rows; i++) {
        for (int j = 0; j < dest->cols; j++) {
            printf("%6.0f", MatGetD(dest, i, j));
        }
        printf("\n");
    }
}

// 크기가 다른 행렬로 복원 시도 (실패 예시)
mat_t *wrong_size = Mat(2, 2, DOUBLE);
int fail_result = MatCopyIn(wrong_size, src);
if (!fail_result) {
    printf("크기가 다른 행렬로 복원 실패 (예상된 결과)\n");
}

// 백업 및 복원 시나리오
mat_t *original = Mat(2, 2, DOUBLE);
mat_t *backup = Mat(2, 2, DOUBLE);

// 원본 데이터 설정
MatSetD(original, 0, 0, 1.0); MatSetD(original, 0, 1, 2.0);
MatSetD(original, 1, 0, 3.0); MatSetD(original, 1, 1, 4.0);

// 백업 생성
MatCopyIn(backup, original);

// 원본 수정
MatSetD(original, 0, 0, 999.0);

// 백업에서 복원
MatCopyIn(original, backup);
printf("백업에서 복원된 값: %.1f\n", MatGetD(original, 0, 0));  // 1.0

FreeMat(src); FreeMat(dest); FreeMat(wrong_size);
FreeMat(original); FreeMat(backup);
```

</details>

#### 5.3.3 MatTr() - 행렬 전치
<details>
<summary>상세 설명</summary>

**목적**: 행렬 전치 계산

**입력**:
- `const mat_t *A`: 입력 행렬

**출력**:
- `mat_t *`: 전치된 행렬 $A^T$ (오류 시 NULL)

**함수 로직**:
- 새로운 전치 행렬 메모리 할당
- 행과 열 차원 교환
- Column-major 저장 방식 유지

</details>

#### 5.3.4 MatTrIn() - 제자리 전치
<details>
<summary>상세 설명</summary>

**목적**: 제자리 행렬 전치 ($A = A^T$)

**입력**:
- `mat_t *A`: 전치할 행렬 (m×n → n×m)

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:
- 정사각행렬에 최적화
- 메모리 재할당 최소화
- 차원 정보 업데이트

**사용 예시**:
```c
// 3×2 행렬 생성
mat_t *A = Mat(3, 2, DOUBLE);
MatSetD(A, 0, 0, 1); MatSetD(A, 0, 1, 2);
MatSetD(A, 1, 0, 3); MatSetD(A, 1, 1, 4);
MatSetD(A, 2, 0, 5); MatSetD(A, 2, 1, 6);

printf("원본 행렬 A (3×2):\n");
for (int i = 0; i < A->rows; i++) {
    for (int j = 0; j < A->cols; j++) {
        printf("%4.0f", MatGetD(A, i, j));
    }
    printf("\n");
}

// 제자리 전치 수행 (3×2 → 2×3)
int success = MatTrIn(A);
if (success) {
    printf("\n전치된 행렬 A (2×3):\n");
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < A->cols; j++) {
            printf("%4.0f", MatGetD(A, i, j));
        }
        printf("\n");
    }
    printf("새로운 차원: %d×%d\n", A->rows, A->cols);
}

// 정사각행렬 전치 (메모리 효율적)
mat_t *square = Mat(3, 3, DOUBLE);
for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
        MatSetD(square, i, j, i * 3 + j + 1);
    }
}

printf("\n정사각행렬 전치 전:\n");
for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
        printf("%4.0f", MatGetD(square, i, j));
    }
    printf("\n");
}

MatTrIn(square);
printf("\n정사각행렬 전치 후:\n");
for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
        printf("%4.0f", MatGetD(square, i, j));
    }
    printf("\n");
}

FreeMat(A); FreeMat(square);
```

</details>

### 5.4 고급 인덱싱 함수

#### 5.4.1 MatVecIdx() - 벡터 인덱싱
<details>
<summary>상세 설명</summary>

**목적**: 정수 인덱스 배열을 사용한 부분 행렬 추출

**입력**:
- `const mat_t *mat`: 입력 행렬
- `const idx_t *ridx`: 행 인덱스 벡터 (INT 타입)
- `const idx_t *cidx`: 열 인덱스 벡터 (INT 타입)

**출력**:
- `mat_t *`: 인덱싱된 부분 행렬 (오류 시 NULL)

**함수 로직**:
- MATLAB/NumPy 스타일 벡터 인덱싱
- 새로운 행렬 메모리 할당
- 인덱스 범위 검증

**사용 예시**:
```c
// 5×5 원본 행렬 생성
mat_t *A = Mat(5, 5, DOUBLE);
for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
        MatSetD(A, i, j, i * 5 + j + 1);  // 1~25 값 설정
    }
}

// 행 인덱스: [1, 3] (2번째, 4번째 행)
idx_t *ridx = Idx(2, INT);
IdxSetI(ridx, 0, 1);
IdxSetI(ridx, 1, 3);

// 열 인덱스: [0, 2, 4] (1번째, 3번째, 5번째 열)
idx_t *cidx = Idx(3, INT);
IdxSetI(cidx, 0, 0);
IdxSetI(cidx, 1, 2);
IdxSetI(cidx, 2, 4);

// 벡터 인덱싱으로 2×3 부분 행렬 추출
mat_t *sub = MatVecIdx(A, ridx, cidx);
if (sub) {
    printf("원본 행렬 A (5×5):\n");
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            printf("%4.0f", MatGetD(A, i, j));
        }
        printf("\n");
    }

    printf("\n부분 행렬 (2×3):\n");
    for (int i = 0; i < sub->rows; i++) {
        for (int j = 0; j < sub->cols; j++) {
            printf("%4.0f", MatGetD(sub, i, j));
        }
        printf("\n");
    }
}

FreeMat(A); FreeMat(sub);
FreeIdx(ridx); FreeIdx(cidx);
```

</details>

#### 5.4.2 MatVecIdxIn() - 제자리 벡터 인덱싱
<details>
<summary>상세 설명</summary>

**목적**: 제자리 벡터 인덱싱

**입력**:
- `mat_t *mat`: 입력/출력 행렬
- `const idx_t *ridx`: 행 인덱스 벡터 (INT 타입)
- `const idx_t *cidx`: 열 인덱스 벡터 (INT 타입)

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:
- 메모리 효율적 제자리 처리
- 차원 정보 자동 업데이트
- 인덱스 범위 검증

**사용 예시**:
```c
// 4×4 원본 행렬 생성
mat_t *A = Mat(4, 4, DOUBLE);
for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
        MatSetD(A, i, j, i * 4 + j + 1);  // 1~16 값 설정
    }
}

printf("원본 4×4 행렬:\n");
for (int i = 0; i < A->rows; i++) {
    for (int j = 0; j < A->cols; j++) {
        printf("%4.0f", MatGetD(A, i, j));
    }
    printf("\n");
}

// 대각선 원소만 선택하는 인덱스 (정사각 부분행렬)
idx_t *diag_idx = Idx(2, INT);
IdxSetI(diag_idx, 0, 1);  // 2번째 행
IdxSetI(diag_idx, 1, 2);  // 3번째 행

// 같은 인덱스를 열에도 적용
idx_t *col_idx = Idx(2, INT);
IdxSetI(col_idx, 0, 1);   // 2번째 열
IdxSetI(col_idx, 1, 2);   // 3번째 열

// 제자리 벡터 인덱싱 수행 (4×4 → 2×2)
int success = MatVecIdxIn(A, diag_idx, col_idx);
if (success) {
    printf("\n인덱싱 후 2×2 행렬:\n");
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < A->cols; j++) {
            printf("%4.0f", MatGetD(A, i, j));
        }
        printf("\n");
    }
    printf("새로운 차원: %d×%d\n", A->rows, A->cols);
}

// 단일 행 선택 예시
mat_t *B = Mat(3, 4, DOUBLE);
for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 4; j++) {
        MatSetD(B, i, j, (i + 1) * 10 + (j + 1));
    }
}

idx_t *single_row = Idx(1, INT);
IdxSetI(single_row, 0, 1);  // 2번째 행만 선택

idx_t *all_cols = Idx(4, INT);
for (int j = 0; j < 4; j++) {
    IdxSetI(all_cols, j, j);  // 모든 열 선택
}

printf("\n원본 3×4 행렬:\n");
for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 4; j++) {
        printf("%4.0f", MatGetD(B, i, j));
    }
    printf("\n");
}

MatVecIdxIn(B, single_row, all_cols);
printf("\n단일 행 선택 후 1×4 행렬:\n");
for (int j = 0; j < B->cols; j++) {
    printf("%4.0f", MatGetD(B, 0, j));
}
printf("\n");

FreeMat(A); FreeMat(B);
FreeIdx(diag_idx); FreeIdx(col_idx); FreeIdx(single_row); FreeIdx(all_cols);
```

</details>

#### 5.4.3 MatLogIdx() - 논리 인덱싱
<details>
<summary>상세 설명</summary>

**목적**: 불리언 마스크를 사용한 조건부 부분 행렬 추출

**입력**:
- `const mat_t *mat`: 입력 행렬
- `const idx_t *ridx`: 행 논리 마스크 (BOOL 타입)
- `const idx_t *cidx`: 열 논리 마스크 (BOOL 타입)

**출력**:
- `mat_t *`: 논리 인덱싱된 부분 행렬 (오류 시 NULL)

**함수 로직**:
- MATLAB/NumPy 스타일 논리 인덱싱
- true 위치의 원소만 추출
- 동적 크기 결정

**사용 예시**:
```c
// 4×3 원본 행렬 생성
mat_t *A = Mat(4, 3, DOUBLE);
for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 3; j++) {
        MatSetD(A, i, j, i * 3 + j + 1);  // 1~12 값 설정
    }
}

printf("원본 4×3 행렬:\n");
for (int i = 0; i < A->rows; i++) {
    for (int j = 0; j < A->cols; j++) {
        printf("%4.0f", MatGetD(A, i, j));
    }
    printf("\n");
}

// 행 논리 마스크: [true, false, true, false] (1, 3번째 행 선택)
idx_t *row_mask = Idx(4, BOOL);
IdxSetB(row_mask, 0, true);
IdxSetB(row_mask, 1, false);
IdxSetB(row_mask, 2, true);
IdxSetB(row_mask, 3, false);

// 열 논리 마스크: [false, true, true] (2, 3번째 열 선택)
idx_t *col_mask = Idx(3, BOOL);
IdxSetB(col_mask, 0, false);
IdxSetB(col_mask, 1, true);
IdxSetB(col_mask, 2, true);

// 논리 인덱싱으로 2×2 부분 행렬 추출
mat_t *sub = MatLogIdx(A, row_mask, col_mask);
if (sub) {
    printf("\n논리 인덱싱 결과 (2×2):\n");
    for (int i = 0; i < sub->rows; i++) {
        for (int j = 0; j < sub->cols; j++) {
            printf("%4.0f", MatGetD(sub, i, j));
        }
        printf("\n");
    }
}

// 조건부 선택 예시: 값이 6보다 큰 원소들의 행/열 선택
idx_t *value_row_mask = Idx(4, BOOL);
idx_t *value_col_mask = Idx(3, BOOL);

// 각 행에 6보다 큰 값이 있는지 확인
for (int i = 0; i < 4; i++) {
    bool has_large = false;
    for (int j = 0; j < 3; j++) {
        if (MatGetD(A, i, j) > 6.0) {
            has_large = true;
            break;
        }
    }
    IdxSetB(value_row_mask, i, has_large);
}

// 각 열에 6보다 큰 값이 있는지 확인
for (int j = 0; j < 3; j++) {
    bool has_large = false;
    for (int i = 0; i < 4; i++) {
        if (MatGetD(A, i, j) > 6.0) {
            has_large = true;
            break;
        }
    }
    IdxSetB(value_col_mask, j, has_large);
}

mat_t *filtered = MatLogIdx(A, value_row_mask, value_col_mask);
if (filtered) {
    printf("\n조건부 선택 결과 (값 > 6인 행/열):\n");
    for (int i = 0; i < filtered->rows; i++) {
        for (int j = 0; j < filtered->cols; j++) {
            printf("%4.0f", MatGetD(filtered, i, j));
        }
        printf("\n");
    }
}

FreeMat(A); FreeMat(sub); FreeMat(filtered);
FreeIdx(row_mask); FreeIdx(col_mask); FreeIdx(value_row_mask); FreeIdx(value_col_mask);
```
</details>

### 5.5 행렬 연산 함수

#### 5.5.1 MatAdd() - 일반화 행렬 덧셈
<details>
<summary>상세 설명</summary>

**목적**: 일반화 행렬 덧셈 $C = a \cdot A^{T_A} + b \cdot B^{T_B}$

**입력**:
- `double a`: 행렬 A의 스칼라 계수
- `const mat_t *A`: 첫 번째 행렬
- `bool trA`: A의 전치 플래그 (true면 $A^T$ 사용)
- `double b`: 행렬 B의 스칼라 계수
- `const mat_t *B`: 두 번째 행렬
- `bool trB`: B의 전치 플래그 (true면 $B^T$ 사용)

**출력**:
- `mat_t *`: 덧셈 결과 행렬 (오류 시 NULL)

**함수 로직**:
- 스칼라 곱셈과 전치 연산 적용
- 결과 행렬의 차원 호환성 검증
- DOUBLE 타입만 지원

**사용 예시**:
```c
// 2×3 행렬 A와 3×2 행렬 B 생성
mat_t *A = Mat(2, 3, DOUBLE);
mat_t *B = Mat(3, 2, DOUBLE);

// 행렬 A 설정: [[1, 2, 3], [4, 5, 6]]
MatSetD(A, 0, 0, 1.0); MatSetD(A, 0, 1, 2.0); MatSetD(A, 0, 2, 3.0);
MatSetD(A, 1, 0, 4.0); MatSetD(A, 1, 1, 5.0); MatSetD(A, 1, 2, 6.0);

// 행렬 B 설정: [[1, 2], [3, 4], [5, 6]]
MatSetD(B, 0, 0, 1.0); MatSetD(B, 0, 1, 2.0);
MatSetD(B, 1, 0, 3.0); MatSetD(B, 1, 1, 4.0);
MatSetD(B, 2, 0, 5.0); MatSetD(B, 2, 1, 6.0);

// C = 2*A + 3*B^T (A: 2×3, B^T: 2×3)
mat_t *C = MatAdd(2.0, A, false, 3.0, B, true);
if (C) {
    printf("C = 2*A + 3*B^T (2×3):\n");
    for (int i = 0; i < C->rows; i++) {
        for (int j = 0; j < C->cols; j++) {
            printf("%6.0f", MatGetD(C, i, j));
        }
        printf("\n");
    }
}

// D = A^T - 0.5*B (A^T: 3×2, B: 3×2)
mat_t *D = MatAdd(1.0, A, true, -0.5, B, false);
if (D) {
    printf("\nD = A^T - 0.5*B (3×2):\n");
    for (int i = 0; i < D->rows; i++) {
        for (int j = 0; j < D->cols; j++) {
            printf("%6.1f", MatGetD(D, i, j));
        }
        printf("\n");
    }
}

FreeMat(A); FreeMat(B); FreeMat(C); FreeMat(D);
```

</details>

#### 5.5.2 MatAddIn() - 제자리 덧셈
<details>
<summary>상세 설명</summary>

**목적**: 제자리 일반화 행렬 덧셈 $A = a \cdot A^{T_A} + b \cdot B^{T_B}$

**입력**:
- `mat_t *A`: 입력/출력 행렬
- `double a`: 행렬 A의 스칼라 계수
- `bool trA`: A의 전치 플래그 (true면 $A^T$ 사용)
- `double b`: 행렬 B의 스칼라 계수
- `const mat_t *B`: 두 번째 행렬
- `bool trB`: B의 전치 플래그 (true면 $B^T$ 사용)

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:
- 일반화 덧셈 계산 후 결과를 A에 복사
- 차원 호환성 검증
- 메모리 효율적 처리

**사용 예시**:
```c
// 2×2 행렬 A와 B 생성
mat_t *A = Mat(2, 2, DOUBLE);
mat_t *B = Mat(2, 2, DOUBLE);

// 행렬 A 설정: [[1, 2], [3, 4]]
MatSetD(A, 0, 0, 1.0); MatSetD(A, 0, 1, 2.0);
MatSetD(A, 1, 0, 3.0); MatSetD(A, 1, 1, 4.0);

// 행렬 B 설정: [[5, 6], [7, 8]]
MatSetD(B, 0, 0, 5.0); MatSetD(B, 0, 1, 6.0);
MatSetD(B, 1, 0, 7.0); MatSetD(B, 1, 1, 8.0);

printf("원본 A:\n");
for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
        printf("%4.0f", MatGetD(A, i, j));
    }
    printf("\n");
}

// A = 2*A + 0.5*B^T 제자리 연산
int success = MatAddIn(A, 2.0, false, 0.5, B, true);
if (success) {
    printf("\nA = 2*A + 0.5*B^T 결과:\n");
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < A->cols; j++) {
            printf("%6.1f", MatGetD(A, i, j));
        }
        printf("\n");
    }
}

// 차원 불일치 테스트
mat_t *C = Mat(3, 2, DOUBLE);
int fail_result = MatAddIn(A, 1.0, false, 1.0, C, false);
if (!fail_result) {
    printf("차원 불일치로 인한 실패 (예상된 결과)\n");
}

FreeMat(A); FreeMat(B); FreeMat(C);
```

</details>

#### 5.5.3 MatMul() - 일반화 행렬 곱셈
<details>
<summary>상세 설명</summary>

**목적**: 일반화 행렬 곱셈 $C = a \cdot A^{T_A} \times b \cdot B^{T_B}$

**입력**:
- `double a`: 행렬 A의 스칼라 계수
- `const mat_t *A`: 첫 번째 행렬
- `bool trA`: A의 전치 플래그 (true면 $A^T$ 사용)
- `double b`: 행렬 B의 스칼라 계수
- `const mat_t *B`: 두 번째 행렬
- `bool trB`: B의 전치 플래그 (true면 $B^T$ 사용)

**출력**:
- `mat_t *`: 곱셈 결과 행렬 (오류 시 NULL)

**함수 로직**:
- 스칼라 곱셈과 전치 연산 적용
- 행렬 곱셈 차원 호환성 검증 (A의 열 = B의 행)
- DOUBLE 타입만 지원

**사용 예시**:
```c
// 2×3 행렬 A와 3×2 행렬 B 생성
mat_t *A = Mat(2, 3, DOUBLE);
mat_t *B = Mat(3, 2, DOUBLE);

// 행렬 A 설정: [[1, 2, 3], [4, 5, 6]]
MatSetD(A, 0, 0, 1.0); MatSetD(A, 0, 1, 2.0); MatSetD(A, 0, 2, 3.0);
MatSetD(A, 1, 0, 4.0); MatSetD(A, 1, 1, 5.0); MatSetD(A, 1, 2, 6.0);

// 행렬 B 설정: [[1, 2], [3, 4], [5, 6]]
MatSetD(B, 0, 0, 1.0); MatSetD(B, 0, 1, 2.0);
MatSetD(B, 1, 0, 3.0); MatSetD(B, 1, 1, 4.0);
MatSetD(B, 2, 0, 5.0); MatSetD(B, 2, 1, 6.0);

// C = 2*A * 0.5*B (2×3 × 3×2 = 2×2)
mat_t *C = MatMul(2.0, A, false, 0.5, B, false);
if (C) {
    printf("C = 2*A * 0.5*B (2×2):\n");
    for (int i = 0; i < C->rows; i++) {
        for (int j = 0; j < C->cols; j++) {
            printf("%6.0f", MatGetD(C, i, j));
        }
        printf("\n");
    }
}

// D = A^T * B^T (3×2 × 2×3 = 3×3)
mat_t *D = MatMul(1.0, A, true, 1.0, B, true);
if (D) {
    printf("\nD = A^T * B^T (3×3):\n");
    for (int i = 0; i < D->rows; i++) {
        for (int j = 0; j < D->cols; j++) {
            printf("%6.0f", MatGetD(D, i, j));
        }
        printf("\n");
    }
}

// 벡터 내적 예시: v^T * v (1×3 × 3×1 = 1×1)
mat_t *v = Mat(3, 1, DOUBLE);
MatSetD(v, 0, 0, 1.0); MatSetD(v, 1, 0, 2.0); MatSetD(v, 2, 0, 3.0);

mat_t *dot_result = MatMul(1.0, v, true, 1.0, v, false);
if (dot_result) {
    printf("\nv^T * v = %.0f (벡터 내적)\n", MatGetD(dot_result, 0, 0));
}

FreeMat(A); FreeMat(B); FreeMat(C); FreeMat(D); FreeMat(v); FreeMat(dot_result);
```

</details>

#### 5.5.4 MatMulIn() - 제자리 곱셈
<details>
<summary>상세 설명</summary>

**목적**: 제자리 일반화 행렬 곱셈 $A = a \cdot A^{T_A} \times b \cdot B^{T_B}$

**입력**:
- `mat_t *A`: 입력/출력 행렬
- `double a`: 행렬 A의 스칼라 계수
- `bool trA`: A의 전치 플래그 (true면 $A^T$ 사용)
- `double b`: 행렬 B의 스칼라 계수
- `const mat_t *B`: 두 번째 행렬
- `bool trB`: B의 전치 플래그 (true면 $B^T$ 사용)

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:
- 일반화 곱셈 계산 후 결과로 A를 교체
- 차원 호환성 검증
- 메모리 재할당으로 차원 변경 지원

**사용 예시**:
```c
// 2×3 행렬 A와 3×2 행렬 B 생성
mat_t *A = Mat(2, 3, DOUBLE);
mat_t *B = Mat(3, 2, DOUBLE);

// 행렬 A 설정: [[1, 2, 3], [4, 5, 6]]
MatSetD(A, 0, 0, 1.0); MatSetD(A, 0, 1, 2.0); MatSetD(A, 0, 2, 3.0);
MatSetD(A, 1, 0, 4.0); MatSetD(A, 1, 1, 5.0); MatSetD(A, 1, 2, 6.0);

// 행렬 B 설정: [[1, 2], [3, 4], [5, 6]]
MatSetD(B, 0, 0, 1.0); MatSetD(B, 0, 1, 2.0);
MatSetD(B, 1, 0, 3.0); MatSetD(B, 1, 1, 4.0);
MatSetD(B, 2, 0, 5.0); MatSetD(B, 2, 1, 6.0);

printf("원본 A (2×3):\n");
for (int i = 0; i < A->rows; i++) {
    for (int j = 0; j < A->cols; j++) {
        printf("%4.0f", MatGetD(A, i, j));
    }
    printf("\n");
}

// A = A * B 제자리 연산 (2×3 × 3×2 = 2×2)
int success = MatMulIn(A, 1.0, false, 1.0, B, false);
if (success) {
    printf("\nA = A * B 결과 (2×2):\n");
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < A->cols; j++) {
            printf("%6.0f", MatGetD(A, i, j));
        }
        printf("\n");
    }
    printf("새로운 차원: %d×%d\n", A->rows, A->cols);
}

// 정사각행렬 제자리 곱셈 예시
mat_t *C = Mat(2, 2, DOUBLE);
MatSetD(C, 0, 0, 2.0); MatSetD(C, 0, 1, 1.0);
MatSetD(C, 1, 0, 1.0); MatSetD(C, 1, 1, 2.0);

// C = 0.5 * C^T * C (대칭행렬 생성)
MatMulIn(C, 0.5, true, 1.0, C, false);
printf("\nC = 0.5 * C^T * C (대칭행렬):\n");
for (int i = 0; i < C->rows; i++) {
    for (int j = 0; j < C->cols; j++) {
        printf("%6.1f", MatGetD(C, i, j));
    }
    printf("\n");
}

FreeMat(A); FreeMat(B); FreeMat(C);
```

</details>

#### 5.5.5 MatInv() - 행렬 역행렬
<details>
<summary>상세 설명</summary>

**목적**: 일반화 행렬 역행렬 계산 $A_i = \text{inv}(a \cdot A^{T_A})$

**입력**:
- `double a`: 행렬 A의 스칼라 계수
- `const mat_t *A`: 입력 행렬 (정사각행렬)
- `bool trA`: A의 전치 플래그 (true면 $A^T$ 사용)

**출력**:
- `mat_t *`: 역행렬 (오류 시 NULL)

**함수 로직**:
- LU 분해를 이용한 역행렬 계산
- 특이행렬 검출 (행렬식이 0에 가까운 경우)
- DOUBLE 타입만 지원

**사용 예시**:
```c
// 3×3 가역행렬 생성
mat_t *A = Mat(3, 3, DOUBLE);

// 가역행렬 설정: [[2, 1, 0], [1, 2, 1], [0, 1, 2]]
MatSetD(A, 0, 0, 2.0); MatSetD(A, 0, 1, 1.0); MatSetD(A, 0, 2, 0.0);
MatSetD(A, 1, 0, 1.0); MatSetD(A, 1, 1, 2.0); MatSetD(A, 1, 2, 1.0);
MatSetD(A, 2, 0, 0.0); MatSetD(A, 2, 1, 1.0); MatSetD(A, 2, 2, 2.0);

printf("원본 행렬 A:\n");
for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
        printf("%6.1f", MatGetD(A, i, j));
    }
    printf("\n");
}

// A_inv = inv(A) 계산
mat_t *A_inv = MatInv(1.0, A, false);
if (A_inv) {
    printf("\nA의 역행렬:\n");
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            printf("%8.4f", MatGetD(A_inv, i, j));
        }
        printf("\n");
    }

    // 검증: A * A_inv = I
    mat_t *I_check = MatMul(1.0, A, false, 1.0, A_inv, false);
    if (I_check) {
        printf("\n검증 A * A_inv (단위행렬이어야 함):\n");
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                printf("%8.4f", MatGetD(I_check, i, j));
            }
            printf("\n");
        }
        FreeMat(I_check);
    }
}

// 스칼라 곱셈 역행렬: inv(2*A^T)
mat_t *scaled_inv = MatInv(2.0, A, true);
if (scaled_inv) {
    printf("\ninv(2*A^T):\n");
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            printf("%8.4f", MatGetD(scaled_inv, i, j));
        }
        printf("\n");
    }
}

// 특이행렬 테스트 (역행렬이 존재하지 않음)
mat_t *singular = Mat(2, 2, DOUBLE);
MatSetD(singular, 0, 0, 1.0); MatSetD(singular, 0, 1, 2.0);
MatSetD(singular, 1, 0, 2.0); MatSetD(singular, 1, 1, 4.0);  // 두 번째 행이 첫 번째 행의 2배

mat_t *singular_inv = MatInv(1.0, singular, false);
if (singular_inv == NULL) {
    printf("\n특이행렬의 역행렬 계산 실패 (예상된 결과)\n");
}

FreeMat(A); FreeMat(A_inv); FreeMat(scaled_inv); FreeMat(singular);
```

</details>

#### 5.5.6 MatInvIn() - 제자리 역행렬
<details>
<summary>상세 설명</summary>

**목적**: 제자리 일반화 행렬 역행렬 $A = \text{inv}(a \cdot A^{T_A})$

**입력**:
- `mat_t *A`: 입력/출력 행렬 (정사각행렬)
- `double a`: 행렬 A의 스칼라 계수
- `bool trA`: A의 전치 플래그 (true면 $A^T$ 사용)

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:
- 일반화 역행렬 계산 후 결과를 A에 복사
- 정사각행렬만 지원
- 메모리 효율적 처리

**사용 예시**:
```c
// 3×3 가역행렬 생성
mat_t *A = Mat(3, 3, DOUBLE);

// 가역행렬 설정: [[4, 2, 1], [2, 3, 1], [1, 1, 2]]
MatSetD(A, 0, 0, 4.0); MatSetD(A, 0, 1, 2.0); MatSetD(A, 0, 2, 1.0);
MatSetD(A, 1, 0, 2.0); MatSetD(A, 1, 1, 3.0); MatSetD(A, 1, 2, 1.0);
MatSetD(A, 2, 0, 1.0); MatSetD(A, 2, 1, 1.0); MatSetD(A, 2, 2, 2.0);

printf("원본 행렬 A:\n");
for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
        printf("%6.1f", MatGetD(A, i, j));
    }
    printf("\n");
}

// 원본 행렬 백업 (검증용)
mat_t *A_backup = MatCopy(A);

// A = inv(A) 제자리 역행렬 수행
int success = MatInvIn(A, 1.0, false);
if (success) {
    printf("\n제자리 역행렬 결과:\n");
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < A->cols; j++) {
            printf("%8.4f", MatGetD(A, i, j));
        }
        printf("\n");
    }

    // 검증: A_backup * A = I
    mat_t *I_check = MatMul(1.0, A_backup, false, 1.0, A, false);
    if (I_check) {
        printf("\n검증 원본 * 역행렬 (단위행렬이어야 함):\n");
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                printf("%8.4f", MatGetD(I_check, i, j));
            }
            printf("\n");
        }
        FreeMat(I_check);
    }
}

// 스칼라와 전치를 포함한 제자리 역행렬
mat_t *B = Mat(2, 2, DOUBLE);
MatSetD(B, 0, 0, 1.0); MatSetD(B, 0, 1, 2.0);
MatSetD(B, 1, 0, 3.0); MatSetD(B, 1, 1, 4.0);

printf("\n원본 B:\n");
for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
        printf("%6.1f", MatGetD(B, i, j));
    }
    printf("\n");
}

// B = inv(0.5 * B^T) 제자리 연산
int success2 = MatInvIn(B, 0.5, true);
if (success2) {
    printf("\nB = inv(0.5 * B^T) 결과:\n");
    for (int i = 0; i < B->rows; i++) {
        for (int j = 0; j < B->cols; j++) {
            printf("%8.4f", MatGetD(B, i, j));
        }
        printf("\n");
    }
}

FreeMat(A); FreeMat(A_backup); FreeMat(B);
```

</details>

#### 5.5.7 Dot() - 벡터 내적
<details>
<summary>상세 설명</summary>

**목적**: 벡터 내적 계산 $c = \boldsymbol{a}^T \cdot \boldsymbol{b}$

**입력**:
- `const mat_t *a`: 첫 번째 벡터 (N×1, DOUBLE 타입)
- `const mat_t *b`: 두 번째 벡터 (N×1, DOUBLE 타입)
- `double *c`: 내적 결과 (출력)

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:
- 열벡터(N×1) 형태만 지원
- 같은 차원의 벡터 검증
- DOUBLE 타입만 지원

**사용 예시**:
```c
// 3×1 벡터 생성
mat_t *a = Mat(3, 1, DOUBLE);
mat_t *b = Mat(3, 1, DOUBLE);

// 벡터 a 설정: [1, 2, 3]^T
MatSetD(a, 0, 0, 1.0);
MatSetD(a, 1, 0, 2.0);
MatSetD(a, 2, 0, 3.0);

// 벡터 b 설정: [4, 5, 6]^T
MatSetD(b, 0, 0, 4.0);
MatSetD(b, 1, 0, 5.0);
MatSetD(b, 2, 0, 6.0);

// 벡터 내적 계산: a^T * b = 1*4 + 2*5 + 3*6 = 32
double dot_result;
int success = Dot(a, b, &dot_result);
if (success) {
    printf("벡터 a와 b의 내적: %.1f\n", dot_result);
}

// 벡터 노름 제곱 계산: ||a||^2 = a^T * a
double norm_squared;
Dot(a, a, &norm_squared);
printf("벡터 a의 노름 제곱: %.1f\n", norm_squared);
printf("벡터 a의 노름: %.3f\n", sqrt(norm_squared));

// 영벡터와의 내적
mat_t *zero_vec = Zeros(3, 1, DOUBLE);
double zero_dot;
Dot(a, zero_vec, &zero_dot);
printf("벡터 a와 영벡터의 내적: %.1f\n", zero_dot);

// 직교 벡터 예시
mat_t *orthogonal = Mat(3, 1, DOUBLE);
MatSetD(orthogonal, 0, 0, 1.0);   // [1, -2, 1]^T
MatSetD(orthogonal, 1, 0, -2.0);  // a와 직교하는 벡터
MatSetD(orthogonal, 2, 0, 1.0);

double ortho_dot;
Dot(a, orthogonal, &ortho_dot);
printf("직교 벡터와의 내적: %.1f (0에 가까워야 함)\n", ortho_dot);

// 차원 불일치 테스트
mat_t *wrong_size = Mat(4, 1, DOUBLE);
int fail_result = Dot(a, wrong_size, &dot_result);
if (!fail_result) {
    printf("차원 불일치로 인한 실패 (예상된 결과)\n");
}

FreeMat(a); FreeMat(b); FreeMat(zero_vec); FreeMat(orthogonal); FreeMat(wrong_size);
```

</details>

#### 5.5.8 Cross3() - 3차원 외적
<details>
<summary>상세 설명</summary>

**목적**: 3차원 외적 계산 $\boldsymbol{c} = \boldsymbol{a} \times \boldsymbol{b}$

**입력**:
- `const mat_t *a`: 첫 번째 벡터 (3×1, DOUBLE 타입)
- `const mat_t *b`: 두 번째 벡터 (3×1, DOUBLE 타입)
- `mat_t *c`: 외적 결과 벡터 (3×1, DOUBLE 타입, 출력)

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:
- 3차원 벡터만 지원 (3×1 형태)
- 외적 공식: $\boldsymbol{c} = [a_2b_3 - a_3b_2, a_3b_1 - a_1b_3, a_1b_2 - a_2b_1]^T$
- DOUBLE 타입만 지원

**사용 예시**:
```c
// 3×1 벡터 생성
mat_t *a = Mat(3, 1, DOUBLE);
mat_t *b = Mat(3, 1, DOUBLE);
mat_t *c = Mat(3, 1, DOUBLE);

// 벡터 a 설정: [1, 0, 0]^T (x축 단위벡터)
MatSetD(a, 0, 0, 1.0);
MatSetD(a, 1, 0, 0.0);
MatSetD(a, 2, 0, 0.0);

// 벡터 b 설정: [0, 1, 0]^T (y축 단위벡터)
MatSetD(b, 0, 0, 0.0);
MatSetD(b, 1, 0, 1.0);
MatSetD(b, 2, 0, 0.0);

// 외적 계산: x × y = z
int success = Cross3(a, b, c);
if (success) {
    printf("x축 × y축 = z축:\n");
    printf("[%.1f, %.1f, %.1f]^T\n",
           MatGetD(c, 0, 0), MatGetD(c, 1, 0), MatGetD(c, 2, 0));
}

// 일반적인 벡터 외적 예시
MatSetD(a, 0, 0, 1.0); MatSetD(a, 1, 0, 2.0); MatSetD(a, 2, 0, 3.0);
MatSetD(b, 0, 0, 4.0); MatSetD(b, 1, 0, 5.0); MatSetD(b, 2, 0, 6.0);

Cross3(a, b, c);
printf("\n[1,2,3] × [4,5,6] = [%.1f, %.1f, %.1f]^T\n",
       MatGetD(c, 0, 0), MatGetD(c, 1, 0), MatGetD(c, 2, 0));

// 외적의 성질 검증: a × b = -(b × a)
mat_t *c_reverse = Mat(3, 1, DOUBLE);
Cross3(b, a, c_reverse);
printf("반대 순서: [4,5,6] × [1,2,3] = [%.1f, %.1f, %.1f]^T\n",
       MatGetD(c_reverse, 0, 0), MatGetD(c_reverse, 1, 0), MatGetD(c_reverse, 2, 0));

// 외적 결과가 두 벡터에 직교함을 검증
double dot_ac, dot_bc;
Dot(a, c, &dot_ac);
Dot(b, c, &dot_bc);
printf("\n직교성 검증:\n");
printf("a · (a × b) = %.6f (0이어야 함)\n", dot_ac);
printf("b · (a × b) = %.6f (0이어야 함)\n", dot_bc);

// 평행한 벡터의 외적 (영벡터)
mat_t *parallel = Mat(3, 1, DOUBLE);
MatSetD(parallel, 0, 0, 2.0); MatSetD(parallel, 1, 0, 4.0); MatSetD(parallel, 2, 0, 6.0);

mat_t *zero_cross = Mat(3, 1, DOUBLE);
Cross3(a, parallel, zero_cross);  // [1,2,3] × [2,4,6] = [0,0,0]
printf("\n평행 벡터 외적: [%.1f, %.1f, %.1f]^T (영벡터)\n",
       MatGetD(zero_cross, 0, 0), MatGetD(zero_cross, 1, 0), MatGetD(zero_cross, 2, 0));

// 외적의 크기 = |a||b|sin(θ)
double norm_a = Norm(a);
double norm_b = Norm(b);
double norm_cross = Norm(c);
double dot_ab;
Dot(a, b, &dot_ab);
double cos_theta = dot_ab / (norm_a * norm_b);
double sin_theta = sqrt(1.0 - cos_theta * cos_theta);
double expected_norm = norm_a * norm_b * sin_theta;

printf("\n외적 크기 검증:\n");
printf("||a × b|| = %.6f\n", norm_cross);
printf("||a|| ||b|| sin(θ) = %.6f\n", expected_norm);

FreeMat(a); FreeMat(b); FreeMat(c); FreeMat(c_reverse);
FreeMat(parallel); FreeMat(zero_cross);
```

</details>

#### 5.5.9 Norm() - 유클리드 노름
<details>
<summary>상세 설명</summary>

**목적**: 벡터의 유클리드 노름 계산 $||\boldsymbol{a}||_2 = \sqrt{\sum_{i} a_i^2}$

**입력**:
- `const mat_t *a`: 입력 벡터 (N×1 또는 1×N, DOUBLE 타입)

**출력**:
- `double`: 노름 결과 (오류 시 0.0)

**함수 로직**:
- 열벡터(N×1) 또는 행벡터(1×N) 지원
- 유클리드 노름 공식 적용
- DOUBLE 타입만 지원

**사용 예시**:
```c
// 3×1 열벡터 생성
mat_t *col_vec = Mat(3, 1, DOUBLE);

// 벡터 설정: [3, 4, 0]^T (3-4-5 직각삼각형)
MatSetD(col_vec, 0, 0, 3.0);
MatSetD(col_vec, 1, 0, 4.0);
MatSetD(col_vec, 2, 0, 0.0);

// 노름 계산: ||[3,4,0]|| = sqrt(9+16+0) = 5
double norm1 = Norm(col_vec);
printf("벡터 [3,4,0]의 노름: %.1f\n", norm1);

// 1×4 행벡터 생성
mat_t *row_vec = Mat(1, 4, DOUBLE);
MatSetD(row_vec, 0, 0, 1.0);
MatSetD(row_vec, 0, 1, 2.0);
MatSetD(row_vec, 0, 2, 2.0);
MatSetD(row_vec, 0, 3, 0.0);

// 행벡터 노름 계산: ||[1,2,2,0]|| = sqrt(1+4+4+0) = 3
double norm2 = Norm(row_vec);
printf("벡터 [1,2,2,0]의 노름: %.1f\n", norm2);

// 단위벡터 생성 및 검증
mat_t *unit_vec = Mat(3, 1, DOUBLE);
double original_norm = Norm(col_vec);
for (int i = 0; i < 3; i++) {
    double val = MatGetD(col_vec, i, 0) / original_norm;
    MatSetD(unit_vec, i, 0, val);
}

double unit_norm = Norm(unit_vec);
printf("단위벡터의 노름: %.6f (1.0이어야 함)\n", unit_norm);

// 영벡터의 노름
mat_t *zero_vec = Zeros(5, 1, DOUBLE);
double zero_norm = Norm(zero_vec);
printf("영벡터의 노름: %.1f\n", zero_norm);

// 다양한 차원의 벡터 노름
mat_t *high_dim = Mat(10, 1, DOUBLE);
for (int i = 0; i < 10; i++) {
    MatSetD(high_dim, i, 0, 1.0);  // 모든 원소가 1
}
double high_norm = Norm(high_dim);  // sqrt(10) ≈ 3.162
printf("10차원 일벡터의 노름: %.3f\n", high_norm);

// 노름의 성질 검증: ||ka|| = |k| * ||a||
double scale = -2.5;
mat_t *scaled_vec = Mat(3, 1, DOUBLE);
for (int i = 0; i < 3; i++) {
    double val = MatGetD(col_vec, i, 0) * scale;
    MatSetD(scaled_vec, i, 0, val);
}

double scaled_norm = Norm(scaled_vec);
double expected_norm = fabs(scale) * norm1;
printf("\n스케일링 성질 검증:\n");
printf("||%.1f * a|| = %.3f\n", scale, scaled_norm);
printf("|%.1f| * ||a|| = %.3f\n", scale, expected_norm);

// 삼각부등식 검증: ||a + b|| <= ||a|| + ||b||
mat_t *vec_a = Mat(2, 1, DOUBLE);
mat_t *vec_b = Mat(2, 1, DOUBLE);
mat_t *vec_sum = Mat(2, 1, DOUBLE);

MatSetD(vec_a, 0, 0, 3.0); MatSetD(vec_a, 1, 0, 4.0);  // ||a|| = 5
MatSetD(vec_b, 0, 0, 1.0); MatSetD(vec_b, 1, 0, 1.0);  // ||b|| = sqrt(2)

// a + b 계산
for (int i = 0; i < 2; i++) {
    double sum = MatGetD(vec_a, i, 0) + MatGetD(vec_b, i, 0);
    MatSetD(vec_sum, i, 0, sum);
}

double norm_a = Norm(vec_a);
double norm_b = Norm(vec_b);
double norm_sum = Norm(vec_sum);

printf("\n삼각부등식 검증:\n");
printf("||a|| = %.3f, ||b|| = %.3f\n", norm_a, norm_b);
printf("||a + b|| = %.3f\n", norm_sum);
printf("||a|| + ||b|| = %.3f\n", norm_a + norm_b);
printf("삼각부등식 성립: %s\n", (norm_sum <= norm_a + norm_b + 1e-10) ? "예" : "아니오");

FreeMat(col_vec); FreeMat(row_vec); FreeMat(unit_vec); FreeMat(zero_vec);
FreeMat(high_dim); FreeMat(scaled_vec); FreeMat(vec_a); FreeMat(vec_b); FreeMat(vec_sum);
```

</details>

### 5.6 분석 함수

#### 5.6.1 MatDet() - 행렬식 계산
<details>
<summary>상세 설명</summary>

**목적**: 행렬식 계산

**입력**:
- `const mat_t *A`: 입력 행렬 (정사각행렬, DOUBLE 타입)

**출력**:
- `double`: 행렬식 결과 (특이행렬이거나 오류 시 0.0)

**함수 로직**:
- LU 분해를 이용한 행렬식 계산: $\det(\mathbf{A}) = \det(\mathbf{L}) \times \det(\mathbf{U}) = \prod_{i=1}^{n} u_{ii}$
- 정사각행렬만 지원 (DOUBLE 타입)
- 특이행렬 검출 (행렬식이 0에 가까운 경우)

**사용 예시**:
```c
// 3×3 행렬 생성
mat_t *A = Mat(3, 3, DOUBLE);

// 행렬 원소 설정
for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
        MatSetD(A, i, j, i * 3 + j + 1);  // 1~9 값 설정
    }
}

// 행렬식 계산
double det = MatDet(A);
printf("행렬 A의 행렬식: %f\n", det);

FreeMat(A);
```

</details>

### 5.7 고급 알고리즘 함수

#### 5.7.1 Lsq() - 최소제곱법
<details>
<summary>상세 설명</summary>

**목적**: 가중 최소제곱법 추정 $\hat{\boldsymbol{x}} = (\mathbf{H}^T \mathbf{W} \mathbf{H})^{-1} \mathbf{H}^T \mathbf{W} \boldsymbol{y}$

**입력**:
- `const mat_t *H`: 설계 행렬 (m×n, DOUBLE 타입) - 관측 모델의 야코비안 행렬
- `const mat_t *y`: 관측 벡터 (m×1, DOUBLE 타입, optional) - 실제 관측값 또는 잔차
- `const mat_t *R`: 관측 노이즈 공분산 행렬 (m×m, DOUBLE 타입, optional) - 관측 불확실성
- `mat_t *x`: 상태 벡터 (n×1, DOUBLE 타입, 출력, optional) - 추정된 매개변수
- `mat_t *P`: 상태 공분산 행렬 (n×n, DOUBLE 타입, 출력, optional) - 추정 불확실성
- `mat_t *Hl`: 최소제곱 역행렬 (n×m, DOUBLE 타입, 출력, optional) - 일반화 역행렬

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:

**1. 가중치 행렬 계산**:
- $\mathbf{W} = \mathbf{R}^{-1}$ (R이 NULL이면 $\mathbf{W} = \mathbf{I}$)

**2. 정보 행렬 계산**:
- $\mathbf{H}^T \mathbf{W} \mathbf{H}$ (정보 행렬)
- $\mathbf{Q} = (\mathbf{H}^T \mathbf{W} \mathbf{H})^{-1}$ (상태 공분산)

**3. 최소제곱 역행렬**:
- $\mathbf{L} = \mathbf{Q} \mathbf{H}^T \mathbf{W}$

**4. 상태 추정 및 공분산**:
- $\hat{\boldsymbol{x}} = \mathbf{L} \boldsymbol{y}$ (y가 제공된 경우)
- $\mathbf{P} = \mathbf{L} \mathbf{R} \mathbf{L}^T$ (공분산 전파)

**사용 예시**:
```c
// 과결정 시스템: 4개 관측으로 3개 매개변수 추정
mat_t *H = Mat(4, 3, DOUBLE);
mat_t *y = Mat(4, 1, DOUBLE);

// 설계 행렬 H: 선형 모델 y = a + b*x + c*x^2
// 관측점: x = [0, 1, 2, 3]
MatSetD(H, 0, 0, 1.0); MatSetD(H, 0, 1, 0.0); MatSetD(H, 0, 2, 0.0);  // [1, 0, 0]
MatSetD(H, 1, 0, 1.0); MatSetD(H, 1, 1, 1.0); MatSetD(H, 1, 2, 1.0);  // [1, 1, 1]
MatSetD(H, 2, 0, 1.0); MatSetD(H, 2, 1, 2.0); MatSetD(H, 2, 2, 4.0);  // [1, 2, 4]
MatSetD(H, 3, 0, 1.0); MatSetD(H, 3, 1, 3.0); MatSetD(H, 3, 2, 9.0);  // [1, 3, 9]

// 관측값: y = 1 + 2*x + 0.5*x^2 + 노이즈
MatSetD(y, 0, 0, 1.1);   // y(0) = 1 + 노이즈
MatSetD(y, 1, 0, 3.6);   // y(1) = 3.5 + 노이즈
MatSetD(y, 2, 0, 7.8);   // y(2) = 7 + 노이즈
MatSetD(y, 3, 0, 13.2);  // y(3) = 12.5 + 노이즈

// 출력 행렬 생성
mat_t *x = Mat(3, 1, DOUBLE);
mat_t *P = Mat(3, 3, DOUBLE);
mat_t *Hl = Mat(3, 4, DOUBLE);

// 최소제곱법 추정 (균등 가중치)
int success = Lsq(H, y, NULL, x, P, Hl);
if (success) {
    printf("추정된 매개변수:\n");
    printf("a = %.4f (실제: 1.0)\n", MatGetD(x, 0, 0));
    printf("b = %.4f (실제: 2.0)\n", MatGetD(x, 1, 0));
    printf("c = %.4f (실제: 0.5)\n", MatGetD(x, 2, 0));

    printf("\n상태 공분산 행렬 P:\n");
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            printf("%8.4f", MatGetD(P, i, j));
        }
        printf("\n");
    }

    // 추정 불확실성 (표준편차)
    printf("\n매개변수 표준편차:\n");
    printf("σ_a = %.4f\n", sqrt(MatGetD(P, 0, 0)));
    printf("σ_b = %.4f\n", sqrt(MatGetD(P, 1, 1)));
    printf("σ_c = %.4f\n", sqrt(MatGetD(P, 2, 2)));
}

// 가중 최소제곱법 (관측 정확도가 다른 경우)
mat_t *R = Mat(4, 4, DOUBLE);
// 대각 공분산 행렬 (첫 번째 관측이 가장 정확)
MatSetD(R, 0, 0, 0.01); MatSetD(R, 0, 1, 0.0); MatSetD(R, 0, 2, 0.0); MatSetD(R, 0, 3, 0.0);
MatSetD(R, 1, 0, 0.0); MatSetD(R, 1, 1, 0.04); MatSetD(R, 1, 2, 0.0); MatSetD(R, 1, 3, 0.0);
MatSetD(R, 2, 0, 0.0); MatSetD(R, 2, 1, 0.0); MatSetD(R, 2, 2, 0.09); MatSetD(R, 2, 3, 0.0);
MatSetD(R, 3, 0, 0.0); MatSetD(R, 3, 1, 0.0); MatSetD(R, 3, 2, 0.0); MatSetD(R, 3, 3, 0.16);

mat_t *x_weighted = Mat(3, 1, DOUBLE);
mat_t *P_weighted = Mat(3, 3, DOUBLE);

int success2 = Lsq(H, y, R, x_weighted, P_weighted, NULL);
if (success2) {
    printf("\n가중 최소제곱법 결과:\n");
    printf("a = %.4f\n", MatGetD(x_weighted, 0, 0));
    printf("b = %.4f\n", MatGetD(x_weighted, 1, 0));
    printf("c = %.4f\n", MatGetD(x_weighted, 2, 0));

    printf("\n가중 표준편차:\n");
    printf("σ_a = %.4f\n", sqrt(MatGetD(P_weighted, 0, 0)));
    printf("σ_b = %.4f\n", sqrt(MatGetD(P_weighted, 1, 1)));
    printf("σ_c = %.4f\n", sqrt(MatGetD(P_weighted, 2, 2)));
}

// 최소제곱 역행렬만 계산 (Hl = (H^T W H)^-1 H^T W)
mat_t *Hl_only = Mat(3, 4, DOUBLE);
int success3 = Lsq(H, NULL, NULL, NULL, NULL, Hl_only);
if (success3) {
    printf("\n최소제곱 역행렬 계산 성공\n");
    printf("Hl 크기: %d×%d\n", Hl_only->rows, Hl_only->cols);
}

FreeMat(H); FreeMat(y); FreeMat(x); FreeMat(P); FreeMat(Hl);
FreeMat(R); FreeMat(x_weighted); FreeMat(P_weighted); FreeMat(Hl_only);
```

</details>

#### 5.7.2 Ekf() - 확장칼만필터 (Joseph 형태)
<details>
<summary>상세 설명</summary>

**목적**: Joseph 형태 확장칼만필터 업데이트 $\mathbf{P}^{+} = (\mathbf{I} - \mathbf{K}\mathbf{H})\mathbf{P}^{-}(\mathbf{I} - \mathbf{K}\mathbf{H})^T + \mathbf{K}\mathbf{R}\mathbf{K}^T$

**입력**:
- `const mat_t *H`: 관측 행렬 (m×n, DOUBLE 타입) - 선형화된 관측 모델
- `const mat_t *v`: 측정 잔차 벡터 (m×1, DOUBLE 타입, optional) - 혁신 벡터
- `const mat_t *R`: 관측 노이즈 공분산 (m×m, DOUBLE 타입, optional) - 측정 불확실성
- `mat_t *x`: 상태 벡터 (n×1, DOUBLE 타입, 입출력, optional) - 추정 상태
- `mat_t *P`: 상태 공분산 (n×n, DOUBLE 타입, 입출력, optional) - 상태 불확실성
- `mat_t *K`: 칼만 이득 (n×m, DOUBLE 타입, 출력, optional) - 최적 이득 행렬

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:

**1. 혁신 공분산 계산**:
- $\mathbf{S} = \mathbf{H} \mathbf{P}^{-} \mathbf{H}^T + \mathbf{R}$

**2. 칼만 이득 계산**:
- $\mathbf{K} = \mathbf{P}^{-} \mathbf{H}^T \mathbf{S}^{-1}$

**3. 상태 업데이트** (x가 제공된 경우):
- $\boldsymbol{x}^{+} = \boldsymbol{x}^{-} + \mathbf{K} \boldsymbol{v}$

**4. Joseph 형태 공분산 업데이트**:
- $\mathbf{P}^{+} = (\mathbf{I} - \mathbf{K}\mathbf{H}) \mathbf{P}^{-} (\mathbf{I} - \mathbf{K}\mathbf{H})^T + \mathbf{K} \mathbf{R} \mathbf{K}^T$

**사용 예시**:
```c
// 2차원 위치 추적 시스템 (상태: [x, y, vx, vy])
int n = 4;  // 상태 차원
int m = 2;  // 관측 차원

// 관측 행렬 H (위치만 관측)
mat_t *H = Mat(m, n, DOUBLE);
MatSetD(H, 0, 0, 1.0); MatSetD(H, 0, 1, 0.0); MatSetD(H, 0, 2, 0.0); MatSetD(H, 0, 3, 0.0);
MatSetD(H, 1, 0, 0.0); MatSetD(H, 1, 1, 1.0); MatSetD(H, 1, 2, 0.0); MatSetD(H, 1, 3, 0.0);

// 초기 상태 벡터 [x=10, y=20, vx=1, vy=2] (x^-)
mat_t *x = Mat(n, 1, DOUBLE);
MatSetD(x, 0, 0, 10.0);  // x 위치
MatSetD(x, 1, 0, 20.0);  // y 위치
MatSetD(x, 2, 0, 1.0);   // x 속도
MatSetD(x, 3, 0, 2.0);   // y 속도

// 초기 공분산 행렬 (P^-)
mat_t *P = Mat(n, n, DOUBLE);
for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
        MatSetD(P, i, j, (i == j) ? 1.0 : 0.0);
    }
}

// 관측 노이즈 공분산
mat_t *R = Mat(m, m, DOUBLE);
MatSetD(R, 0, 0, 0.1); MatSetD(R, 0, 1, 0.0);
MatSetD(R, 1, 0, 0.0); MatSetD(R, 1, 1, 0.1);

// 관측값 (실제 위치에서 약간 벗어남)
mat_t *z_obs = Mat(m, 1, DOUBLE);
MatSetD(z_obs, 0, 0, 10.2);  // 관측된 x 위치
MatSetD(z_obs, 1, 0, 19.8);  // 관측된 y 위치

// 예측된 관측값 계산: h(x) = H * x
mat_t *h_pred = MatMul(1.0, H, false, 1.0, x, false);

// 측정 잔차 벡터: v = z - h(x)
mat_t *v = Mat(m, 1, DOUBLE);
for (int i = 0; i < m; i++) {
    double residual = MatGetD(z_obs, i, 0) - MatGetD(h_pred, i, 0);
    MatSetD(v, i, 0, residual);
}

printf("업데이트 전 상태 (x^-):\n");
printf("x^- = [%.2f, %.2f, %.2f, %.2f]^T\n",
       MatGetD(x, 0, 0), MatGetD(x, 1, 0), MatGetD(x, 2, 0), MatGetD(x, 3, 0));

printf("\n측정 잔차 벡터:\n");
printf("v = [%.3f, %.3f]^T\n", MatGetD(v, 0, 0), MatGetD(v, 1, 0));

// 칼만 이득 출력용
mat_t *K = Mat(n, m, DOUBLE);

// EKF 업데이트 수행
int success = Ekf(H, v, R, x, P, K);
if (success) {
    printf("\nEKF 업데이트 성공!\n");

    printf("\n업데이트 후 상태 (x^+):\n");
    printf("x^+ = [%.2f, %.2f, %.2f, %.2f]^T\n",
           MatGetD(x, 0, 0), MatGetD(x, 1, 0), MatGetD(x, 2, 0), MatGetD(x, 3, 0));

    printf("\n칼만 이득 K:\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            printf("%8.4f", MatGetD(K, i, j));
        }
        printf("\n");
    }

    printf("\n업데이트된 공분산 P^+ (대각선 원소):\n");
    for (int i = 0; i < n; i++) {
        printf("P^+[%d,%d] = %.6f\n", i, i, MatGetD(P, i, i));
    }
}

// 칼만 이득만 계산하는 경우
mat_t *K_only = Mat(n, m, DOUBLE);
mat_t *P_copy = MatCopy(P);  // 원본 보존

int success2 = Ekf(H, NULL, R, NULL, P_copy, K_only);
if (success2) {
    printf("\n칼만 이득만 계산 성공\n");
    printf("K 크기: %d×%d\n", K_only->rows, K_only->cols);
}

FreeMat(H); FreeMat(x); FreeMat(P); FreeMat(R); FreeMat(z_obs); FreeMat(h_pred);
FreeMat(v); FreeMat(K); FreeMat(K_only); FreeMat(P_copy);
```

</details>

#### 5.7.3 Interp() - 선형 보간
<details>
<summary>상세 설명</summary>

**목적**: 1차원 선형 보간

**입력**:
- `const mat_t *x0`: x 좌표 벡터 (N×1, DOUBLE 타입, 오름차순 정렬)
- `const mat_t *y0`: y 값 벡터 (N×1, DOUBLE 타입)
- `double x`: 보간할 x 좌표
- `double *y`: 보간된 y 값 (출력)

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:
- 선형 보간: $y = y_1 + \frac{x - x_1}{x_2 - x_1}(y_2 - y_1)$
- 범위 외 값은 최근접 값으로 외삽
- 오름차순 정렬된 x0 벡터 필요

**사용 예시**:
```c
// 보간 데이터 포인트 생성 (5개 점)
mat_t *x_data = Mat(5, 1, DOUBLE);
mat_t *y_data = Mat(5, 1, DOUBLE);

// 데이터 포인트: (0,0), (1,1), (2,4), (3,9), (4,16) - y = x^2
MatSetD(x_data, 0, 0, 0.0); MatSetD(y_data, 0, 0, 0.0);
MatSetD(x_data, 1, 0, 1.0); MatSetD(y_data, 1, 0, 1.0);
MatSetD(x_data, 2, 0, 2.0); MatSetD(y_data, 2, 0, 4.0);
MatSetD(x_data, 3, 0, 3.0); MatSetD(y_data, 3, 0, 9.0);
MatSetD(x_data, 4, 0, 4.0); MatSetD(y_data, 4, 0, 16.0);

printf("원본 데이터 포인트:\n");
for (int i = 0; i < 5; i++) {
    printf("(%.1f, %.1f) ", MatGetD(x_data, i, 0), MatGetD(y_data, i, 0));
}
printf("\n\n");

// 다양한 x 값에서 보간 수행
double x_interp[] = {0.5, 1.5, 2.5, 3.5};
int n_interp = sizeof(x_interp) / sizeof(double);

printf("선형 보간 결과:\n");
for (int i = 0; i < n_interp; i++) {
    double y_result;
    int success = Interp(x_data, y_data, x_interp[i], &y_result);

    if (success) {
        printf("x = %.1f → y = %.3f\n", x_interp[i], y_result);
    }
}

// 범위 외 값 테스트 (외삽)
double y_extrap;
printf("\n범위 외 값 (외삽):\n");

// 왼쪽 외삽 (x < 0)
Interp(x_data, y_data, -1.0, &y_extrap);
printf("x = -1.0 → y = %.3f (왼쪽 외삽)\n", y_extrap);

// 오른쪽 외삽 (x > 4)
Interp(x_data, y_data, 5.0, &y_extrap);
printf("x = 5.0 → y = %.3f (오른쪽 외삽)\n", y_extrap);

// 시간 시리즈 보간 예시
mat_t *time = Mat(4, 1, DOUBLE);
mat_t *temperature = Mat(4, 1, DOUBLE);

// 시간별 온도 데이터 (시간: 0, 6, 12, 18시, 온도: 15, 25, 30, 20도)
MatSetD(time, 0, 0, 0.0);  MatSetD(temperature, 0, 0, 15.0);  // 자정
MatSetD(time, 1, 0, 6.0);  MatSetD(temperature, 1, 0, 25.0);  // 오전 6시
MatSetD(time, 2, 0, 12.0); MatSetD(temperature, 2, 0, 30.0);  // 정오
MatSetD(time, 3, 0, 18.0); MatSetD(temperature, 3, 0, 20.0);  // 오후 6시

printf("\n시간별 온도 보간:\n");
for (double t = 0.0; t <= 18.0; t += 3.0) {
    double temp;
    Interp(time, temperature, t, &temp);
    printf("시간 %4.1f시 → 온도 %5.1f°C\n", t, temp);
}

// 단조 증가 함수 보간 (로그 함수 근사)
mat_t *x_log = Mat(6, 1, DOUBLE);
mat_t *y_log = Mat(6, 1, DOUBLE);

// ln(x) 데이터 포인트
double x_vals[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
for (int i = 0; i < 6; i++) {
    MatSetD(x_log, i, 0, x_vals[i]);
    MatSetD(y_log, i, 0, log(x_vals[i]));  // 자연로그
}

printf("\n로그 함수 보간 (ln(x)):\n");
for (double x = 1.5; x <= 5.5; x += 1.0) {
    double y_interp, y_exact;
    Interp(x_log, y_log, x, &y_interp);
    y_exact = log(x);
    printf("x = %.1f → 보간값: %.4f, 정확값: %.4f, 오차: %.4f\n",
           x, y_interp, y_exact, fabs(y_interp - y_exact));
}

FreeMat(x_data); FreeMat(y_data); FreeMat(time); FreeMat(temperature);
FreeMat(x_log); FreeMat(y_log);
```

</details>

---

## 6. 사용 예시

### 6.1 기본 행렬 연산 워크플로우

```c
#include "matrix.h"
#include <stdio.h>

int main() {
    // 1. 행렬 생성 및 초기화
    mat_t *A = Mat(3, 3, DOUBLE);
    mat_t *B = Mat(3, 3, DOUBLE);

    // 2. 데이터 설정
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            MatSetD(A, i, j, i + j + 1);
            MatSetD(B, i, j, (i + 1) * (j + 1));
        }
    }

    // 3. 행렬 연산 수행
    mat_t *C = MatMul(1.0, A, false, 1.0, B, false);  // C = A * B
    mat_t *D = MatAdd(2.0, A, true, -1.0, B, false);  // D = 2*A^T - B

    // 4. 결과 출력
    printf("행렬 곱셈 결과 C = A * B:\n");
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            printf("%8.2f", MatGetD(C, i, j));
        }
        printf("\n");
    }

    // 5. 메모리 해제
    FreeMat(A); FreeMat(B); FreeMat(C); FreeMat(D);
    return 0;
}
```

### 6.2 GNSS 위치 추정 예시

```c
// GNSS 최소제곱법 위치 추정
void gnss_positioning_example() {
    // 설계 행렬 H: [방향코사인, 시계오차]
    mat_t *H = Mat(4, 4, DOUBLE);  // 4개 위성, 4개 매개변수 (x,y,z,dt)

    // 의사거리 잔차 벡터
    mat_t *delta_rho = Mat(4, 1, DOUBLE);

    // 관측 가중치 (위성별 신호 품질)
    mat_t *R = Eye(4, DOUBLE);
    for (int i = 0; i < 4; i++) {
        MatSetD(R, i, i, 0.5 + 0.1 * i);  // 차등 가중치
    }

    // 위치 보정량 및 공분산
    mat_t *dx = Mat(4, 1, DOUBLE);
    mat_t *P = Mat(4, 4, DOUBLE);

    // 최소제곱법 수행
    int success = Lsq(H, delta_rho, R, dx, P, NULL);

    if (success) {
        printf("위치 보정량: [%.3f, %.3f, %.3f] m\n",
               MatGetD(dx, 0, 0), MatGetD(dx, 1, 0), MatGetD(dx, 2, 0));
        printf("시계 오차: %.6f s\n", MatGetD(dx, 3, 0) / 299792458.0);

        // 위치 정확도 (PDOP 계산)
        double pdop = sqrt(MatGetD(P, 0, 0) + MatGetD(P, 1, 1) + MatGetD(P, 2, 2));
        printf("위치 정밀도 (PDOP): %.2f\n", pdop);
    }

    FreeMat(H); FreeMat(delta_rho); FreeMat(R); FreeMat(dx); FreeMat(P);
}
```

### 6.3 칼만 필터 추적 예시

```c
// 이동체 추적을 위한 칼만 필터
void tracking_kalman_filter() {
    int n = 4;  // 상태 차원 [x, y, vx, vy]
    int m = 2;  // 관측 차원 [x, y]

    // 상태 벡터 및 공분산 초기화
    mat_t *x = Mat(n, 1, DOUBLE);
    mat_t *P = Eye(n, DOUBLE);

    // 관측 행렬 (위치만 관측)
    mat_t *H = Mat(m, n, DOUBLE);
    MatSetD(H, 0, 0, 1.0); MatSetD(H, 1, 1, 1.0);  // [1 0 0 0; 0 1 0 0]

    // 관측 노이즈
    mat_t *R = Eye(m, DOUBLE);
    MatSetD(R, 0, 0, 0.1); MatSetD(R, 1, 1, 0.1);

    // 시뮬레이션 루프
    for (int k = 0; k < 10; k++) {
        // 예측 단계 (상태 전이는 별도 구현)
        // ...

        // 관측 업데이트
        mat_t *z_obs = Mat(m, 1, DOUBLE);
        MatSetD(z_obs, 0, 0, 10.0 + k + 0.1 * (rand() % 100 - 50));
        MatSetD(z_obs, 1, 0, 20.0 + k + 0.1 * (rand() % 100 - 50));

        // 측정 잔차 계산
        mat_t *h_pred = MatMul(1.0, H, false, 1.0, x, false);
        mat_t *v = Mat(m, 1, DOUBLE);
        for (int i = 0; i < m; i++) {
            double residual = MatGetD(z_obs, i, 0) - MatGetD(h_pred, i, 0);
            MatSetD(v, i, 0, residual);
        }

        // 칼만 필터 업데이트
        mat_t *K = Mat(n, m, DOUBLE);
        Ekf(H, v, R, x, P, K);

        printf("Step %d: 위치 [%.2f, %.2f], 속도 [%.2f, %.2f]\n", k,
               MatGetD(x, 0, 0), MatGetD(x, 1, 0),
               MatGetD(x, 2, 0), MatGetD(x, 3, 0));

        FreeMat(z_obs); FreeMat(h_pred); FreeMat(v); FreeMat(K);
    }

    FreeMat(x); FreeMat(P); FreeMat(H); FreeMat(R);
}
```

---

## 7. 성능 특성

### 7.1 메모리 최적화

#### 7.1.1 SIMD 정렬 최적화
- **32바이트 경계 정렬**: AVX/AVX2 명령어 최적화
- **벡터화 연산**: 4-8배 성능 향상 (데이터 크기에 따라)
- **캐시 효율성**: 연속 메모리 접근으로 캐시 미스 최소화

#### 7.1.2 메모리 사용량
```
데이터 타입별 메모리 사용량:
- DOUBLE: 8 bytes/element + 32-byte alignment overhead
- INT: 4 bytes/element + 32-byte alignment overhead
- BOOL: 1 byte/element + 32-byte alignment overhead

예시: 1000×1000 DOUBLE 행렬
- 실제 데이터: 8,000,000 bytes (≈7.6 MB)
- 정렬 오버헤드: 최대 32 bytes
- 총 메모리: ≈7.6 MB
```

### 7.2 연산 성능

#### 7.2.1 연산 복잡도
| 연산 | 시간 복잡도 | 공간 복잡도 | 특징 |
|------|-------------|-------------|------|
| 행렬 곱셈 (m×n × n×p) | O(mnp) | O(mp) | SIMD 최적화 적용 |
| 행렬 역행렬 (n×n) | O(n³) | O(n²) | LU 분해 기반 |
| 벡터 내적 (n×1) | O(n) | O(1) | 출력 매개변수 사용 |
| 최소제곱법 (m×n) | O(mn² + n³) | O(mn + n²) | 과결정 시스템 지원 |

### 7.3 수치적 안정성

#### 7.3.1 정밀도 특성
- **배정밀도 부동소수점**: IEEE 754 표준 준수
- **상대 오차**: 일반적으로 1e-15 수준
- **조건수 임계값**: 1e12 이상에서 경고

#### 7.3.2 알고리즘 안정성
| 알고리즘 | 수치적 안정성 | 조건수 한계 | 특징 |
|----------|---------------|-------------|------|
| LU 분해 | 보통 | ~1e12 | 행렬식, 역행렬 계산 |
| Joseph 형태 EKF | 우수 | ~1e14 | 공분산 양정치성 보장 |
| 최소제곱법 | 보통 | ~1e12 | 과결정 시스템 해법 |

### 7.4 확장성 및 제한사항

#### 7.4.1 메모리 사용량
| 데이터 타입 | 원소당 크기 | 정렬 오버헤드 | 1000×1000 행렬 |
|-------------|-------------|---------------|----------------|
| DOUBLE | 8 bytes | 최대 32 bytes | ~7.6 MB |
| INT | 4 bytes | 최대 32 bytes | ~3.8 MB |
| BOOL (idx_t) | 1 byte | 최대 32 bytes | ~1.0 MB |

#### 7.4.2 권장 사용 범위
| 응용 분야 | 권장 행렬 크기 | 특징 |
|-----------|----------------|------|
| GNSS 실시간 처리 | 10×10 ~ 50×50 | 밀리초 단위 처리 |
| 일반 수치 계산 | 100×100 ~ 1000×1000 | 범용 과학 계산 |
| 대용량 처리 | 1000×1000 이상 | 충분한 메모리 필요 |

### 7.5 최적화 권장사항

#### 7.5.1 메모리 효율성
- **제자리 연산 활용**: `MatMulIn()`, `MatAddIn()` 등으로 메모리 사용량 최소화
- **재사용 가능한 작업 공간**: 반복 연산에서 임시 행렬 재활용
- **적절한 데이터 타입 선택**: INT/DOUBLE 타입을 용도에 맞게 선택

#### 7.5.2 GNSS 실시간 처리
- **고정 크기 행렬**: 동적 할당 최소화로 실시간 성능 확보
- **조건부 역행렬**: 행렬식 검사 후 안전한 경우만 역행렬 계산
- **배치 처리**: 여러 위성 데이터를 한 번에 처리하여 효율성 향상

---
