---
layout: default
title: 고성능 행렬 연산 모듈 (matrix)
---

# ★ 고성능 행렬 연산 모듈 (matrix)

GNSS 정밀 측위를 위한 고성능 수치 연산 엔진입니다.

## ■ 목차

1. [기본 개념](#▲-기본-개념)
2. [데이터 타입 구조](#▲-데이터-타입-구조)
3. [데이터 타입 목록](#▲-데이터-타입-목록)
4. [함수 구조](#▲-함수-구조)
5. [함수 목록](#▲-함수-목록)
6. [사용 예시](#▲-사용-예시)
7. [성능 특성](#▲-성능-특성)

---

## ▲ 기본 개념

### ◆ Column-major 저장 방식
행렬 데이터는 **열 우선(column-major)** 방식으로 연속 메모리에 저장됩니다.

**예시: 3×3 행렬 저장**
```
행렬 A:
[ a11 a12 a13 ]
[ a21 a22 a23 ]     메모리: [a11, a21, a31, a12, a22, a32, a13, a23, a33]
[ a31 a32 a33 ]     접근:   A[i + j*rows]
```

### ◆ SIMD 정렬 최적화
- **32바이트 경계 정렬**: AVX 명령어 최적화
- **플랫폼별 할당**: Windows `_aligned_malloc()`, POSIX `posix_memalign()`
- **성능 향상**: 벡터화 연산으로 4-8배 속도 개선

### ◆ 메모리 관리 체계
- **초기화/해제 쌍**: `Mat()`/`FreeMat()`, `Idx()`/`FreeIdx()`
- **NULL 안전성**: 모든 해제 함수에서 NULL 포인터 검증
- **빈 행렬 지원**: 0×0 차원 행렬 처리 가능

### ◆ 확장된 타입 시스템
세 가지 데이터 타입을 지원하여 메모리 효율성과 정확도를 동시에 제공합니다:
- **DOUBLE**: 64비트 부동소수점 (GNSS 계산용) - mat_t 전용
- **INT**: 32비트 정수 (인덱스, 플래그용) - mat_t 전용
- **BOOL**: 불리언 타입 (논리 마스크용) - idx_t 전용

### ◆ 고급 인덱싱 시스템
Matrix 모듈은 MATLAB/NumPy 스타일의 고급 인덱싱을 지원합니다:
- **벡터 인덱싱**: 정수 인덱스 배열을 사용한 부분 행렬 추출
- **논리 인덱싱**: 불리언 마스크를 사용한 조건부 선택
- **제자리 연산**: 메모리 효율적인 인덱싱 처리

---

## ▲ 데이터 타입 구조

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

## ▲ 데이터 타입 목록

#### ◆ type_t (enum)
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

#### ◆ idx_t (struct)
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

#### ◆ mat_t (struct)
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

## ▲ 함수 구조

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
│   ├── MatAdd() ──────────── 일반화 행렬 덧셈 $[C = a \cdot A^T + b \cdot B^T]$
│   ├── MatAddIn() ────────── 제자리 덧셈
│   ├── MatMul() ──────────── 일반화 행렬 곱셈 $[C = a \cdot A^T \times b \cdot B^T]$
│   ├── MatMulIn() ────────── 제자리 곱셈
│   ├── MatInv() ──────────── 행렬 역행렬 $[A_i = \text{inv}(a \cdot A^T)]$
│   └── MatInvIn() ────────── 제자리 역행렬
├── 벡터 연산 (mat_t, cols=1)
│   ├── Dot() ──────────────── 벡터 내적 (DOUBLE 전용)
│   ├── Cross3() ──────────── 3차원 외적
│   └── Norm() ────────────── 유클리드 노름 (DOUBLE 전용)
├── 분석 함수
│   └── MatDet() ──────────── 행렬식 계산 (LU 분해)
└── 고급 알고리즘
    ├── Lsq() ─────────────── 최소제곱법 (optional 매개변수 지원)
    └── Ekf() ─────────────── 확장칼만필터 (optional 매개변수 지원)
```

---

## ▲ 함수 목록

#### ◆ 기본 생성/해제 함수

##### ● Mat() - 행렬 생성
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

</details>

##### ● FreeMat() - 행렬 메모리 해제
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

</details>

##### ● Idx() - 인덱스 벡터 생성
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

</details>

##### ● FreeIdx() - 인덱스 벡터 해제
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

#### ◆ 특수 행렬 생성 함수

##### ● Eye() - 단위행렬 생성
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

</details>

##### ● Zeros() - 영행렬 생성
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

</details>

##### ● Ones() - 일행렬 생성
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

</details>

##### ● TrueIdx() - True 인덱스 벡터 생성
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

</details>

##### ● FalseIdx() - False 인덱스 벡터 생성
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

</details>

#### ◆ 행렬 복사/변환 함수

##### ● MatCopy() - 행렬 복사
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

##### ● MatCopyIn() - 제자리 복사
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

</details>

##### ● MatTr() - 행렬 전치
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

##### ● MatTrIn() - 제자리 전치
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

</details>

#### ◆ 고급 인덱싱 함수

##### ● MatVecIdx() - 벡터 인덱싱
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

</details>

##### ● MatVecIdxIn() - 제자리 벡터 인덱싱
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

</details>

##### ● MatLogIdx() - 논리 인덱싱
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

</details>

##### ● MatLogIdxIn() - 제자리 논리 인덱싱
<details>
<summary>상세 설명</summary>

**목적**: 제자리 논리 인덱싱

**입력**:
- `mat_t *mat`: 입력/출력 행렬
- `const idx_t *ridx`: 행 논리 마스크 (BOOL 타입)
- `const idx_t *cidx`: 열 논리 마스크 (BOOL 타입)

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:
- 메모리 효율적 제자리 처리
- 논리 마스크 기반 조건부 선택
- 차원 정보 자동 업데이트

</details>

#### ◆ 행렬 연산 함수

##### ● MatAdd() - 일반화 행렬 덧셈
<details>
<summary>상세 설명</summary>

**목적**: 일반화 행렬 덧셈 수행

**입력**:
- `double a`: 행렬 A의 스칼라
- `const mat_t *A`: 행렬 A
- `bool trA`: A 전치 플래그 (true이면 $A^T$)
- `double b`: 행렬 B의 스칼라
- `const mat_t *B`: 행렬 B
- `bool trB`: B 전치 플래그 (true이면 $B^T$)

**출력**:
- `mat_t *`: 결과 행렬 $C = a \cdot A^T + b \cdot B^T$ (오류 시 NULL)

**함수 로직**:
- 선택적 전치 연산 지원
- 스칼라 곱셈 통합 처리
- 차원 호환성 검증

</details>

##### ● MatAddIn() - 제자리 행렬 덧셈
<details>
<summary>상세 설명</summary>

**목적**: 제자리 일반화 행렬 덧셈

**입력**:
- `mat_t *A`: 입력/출력 행렬 A
- `double a`: 행렬 A의 스칼라
- `bool trA`: A 전치 플래그
- `double b`: 행렬 B의 스칼라
- `const mat_t *B`: 행렬 B
- `bool trB`: B 전치 플래그

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:
- $A = a \cdot A^T + b \cdot B^T$ 계산
- 메모리 효율적 제자리 처리
- 차원 호환성 검증

</details>

##### ● MatMul() - 일반화 행렬 곱셈
<details>
<summary>상세 설명</summary>

**목적**: 일반화 행렬 곱셈 수행

**입력**:
- `double a`: 행렬 A의 스칼라
- `const mat_t *A`: 행렬 A
- `bool trA`: A 전치 플래그
- `double b`: 행렬 B의 스칼라
- `const mat_t *B`: 행렬 B
- `bool trB`: B 전치 플래그

**출력**:
- `mat_t *`: 결과 행렬 $C = a \cdot A^T \times b \cdot B^T$ (오류 시 NULL)

**함수 로직**:
- 선택적 전치 연산 지원
- 스칼라 곱셈 통합 처리
- BLAS 스타일 일반화 곱셈

</details>

##### ● MatMulIn() - 제자리 행렬 곱셈
<details>
<summary>상세 설명</summary>

**목적**: 제자리 일반화 행렬 곱셈

**입력**:
- `mat_t *A`: 입력/출력 행렬 A
- `double a`: 행렬 A의 스칼라
- `bool trA`: A 전치 플래그
- `double b`: 행렬 B의 스칼라
- `const mat_t *B`: 행렬 B
- `bool trB`: B 전치 플래그

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:
- $A = a \cdot A^T \times b \cdot B^T$ 계산
- 메모리 효율적 제자리 처리
- 임시 행렬을 통한 안전한 계산

</details>

##### ● MatInv() - 행렬 역행렬
<details>
<summary>상세 설명</summary>

**목적**: 일반화 행렬 역행렬 계산

**입력**:
- `double a`: 행렬 A의 스칼라
- `const mat_t *A`: 행렬 A (정사각행렬)
- `bool trA`: A 전치 플래그

**출력**:
- `mat_t *`: 역행렬 $A_i = \text{inv}(a \cdot A^T)$ (특이행렬/오류 시 NULL)

**함수 로직**:
- LU 분해를 통한 역행렬 계산
- 특이행렬 검출
- 스칼라 곱셈 통합 처리

</details>

##### ● MatInvIn() - 제자리 행렬 역행렬
<details>
<summary>상세 설명</summary>

**목적**: 제자리 일반화 행렬 역행렬

**입력**:
- `mat_t *A`: 입력/출력 행렬 A
- `double a`: 행렬 A의 스칼라
- `bool trA`: A 전치 플래그

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:
- $A = \text{inv}(a \cdot A^T)$ 계산
- 메모리 효율적 제자리 처리
- 특이행렬 안전 처리

</details>

#### ◆ 벡터 연산 함수

##### ● Dot() - 벡터 내적
<details>
<summary>상세 설명</summary>

**목적**: 벡터 내적 계산 (DOUBLE 전용)

**입력**:
- `const mat_t *a`: 벡터 a (N×1, DOUBLE 타입)
- `const mat_t *b`: 벡터 b (N×1, DOUBLE 타입)
- `double *c`: 내적 결과 $c = \boldsymbol{a}^T \boldsymbol{b}$

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:
- $c = \sum_{i=1}^n a_i \cdot b_i$ 계산
- 벡터 차원 일치 검증
- DOUBLE 타입 전용

</details>

##### ● Cross3() - 3차원 외적
<details>
<summary>상세 설명</summary>

**목적**: 3차원 벡터 외적 계산 (DOUBLE 전용)

**입력**:
- `const mat_t *a`: 벡터 a (3×1, DOUBLE 타입)
- `const mat_t *b`: 벡터 b (3×1, DOUBLE 타입)
- `mat_t *c`: 외적 벡터 $\boldsymbol{c} = \boldsymbol{a} \times \boldsymbol{b}$ (3×1, DOUBLE 타입)

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:
- $\boldsymbol{c} = \begin{bmatrix} a_2b_3 - a_3b_2 \\ a_3b_1 - a_1b_3 \\ a_1b_2 - a_2b_1 \end{bmatrix}$ 계산
- 3차원 벡터 검증
- DOUBLE 타입 전용

</details>

##### ● Norm() - 유클리드 노름
<details>
<summary>상세 설명</summary>

**목적**: 벡터 2-노름 계산 (DOUBLE 전용)

**입력**:
- `const mat_t *a`: 입력 벡터 (m×1 또는 1×n, DOUBLE 타입)

**출력**:
- `double`: 2-노름 $\|\boldsymbol{a}\|_2$ (오류 시 0.0)

**함수 로직**:
- $\|\boldsymbol{a}\|_2 = \sqrt{\sum_{i=1}^n a_i^2}$ 계산
- 행벡터/열벡터 모두 지원
- DOUBLE 타입 전용

</details>

#### ◆ 분석 함수

##### ● MatDet() - 행렬식
<details>
<summary>상세 설명</summary>

**목적**: LU 분해를 통한 행렬식 계산 (DOUBLE 전용)

**입력**:
- `const mat_t *A`: 정사각행렬 (DOUBLE 타입)

**출력**:
- `double`: 행렬식 $\det(A)$ (특이행렬/오류 시 0.0)

**함수 로직**:
- LU 분해: $A = PLU$
- $\det(A) = \det(P) \cdot \prod_{i=1}^n u_{ii}$ 계산
- 피벗팅을 통한 수치 안정성 확보

</details>

#### ◆ 고급 알고리즘 함수

##### ● Lsq() - 최소제곱법
<details>
<summary>상세 설명</summary>

**목적**: 최소제곱법 추정

**입력**:
- `const mat_t *H`: 설계 행렬 (DOUBLE 타입)
- `const mat_t *y`: (선택적) 측정 벡터 (DOUBLE 타입, x가 NULL이 아닐 때만 필요)
- `const mat_t *R`: (선택적) 측정 노이즈 공분산 행렬 (DOUBLE 타입, NULL이면 단위행렬)
- `mat_t *x`: (선택적) 상태 벡터 (DOUBLE 타입)
- `mat_t *P`: (선택적) 상태 공분산 행렬 (DOUBLE 타입)
- `mat_t *Hl`: (선택적) H의 최소제곱 역행렬 (DOUBLE 타입)

**출력**:
- `int`: 성공 시 1, 실패 시 0

**수학 공식 (실제 구현 알고리즘)**:

1. **가중치 행렬**: $\mathbf{W} = \mathbf{R}^{-1}$

2. **일반화 정규 방정식**: $\mathbf{H}^T \mathbf{W} \mathbf{H} \boldsymbol{x} = \mathbf{H}^T \mathbf{W} \boldsymbol{y}$

3. **단계별 계산**:
   - $\mathbf{H}^T$ = H의 전치행렬
   - $\mathbf{W} = \mathbf{R}^{-1}$ (R이 NULL이면 $\mathbf{I}$)
   - $\mathbf{H}^T\mathbf{W} = \mathbf{H}^T \times \mathbf{W}$
   - $\mathbf{H}^T\mathbf{W}\mathbf{H} = \mathbf{H}^T\mathbf{W} \times \mathbf{H}$
   - $\mathbf{Q} = (\mathbf{H}^T\mathbf{W}\mathbf{H})^{-1}$ (상태 공분산)
   - $\mathbf{L} = \mathbf{Q} \times \mathbf{H}^T\mathbf{W}$ (최소제곱 역행렬)

4. **결과 계산**:
   - $\boldsymbol{x} = \mathbf{L} \times \boldsymbol{y}$ (상태 추정, x가 NULL이 아닐 때만)
   - $\mathbf{P} = \mathbf{L} \times \mathbf{R} \times \mathbf{L}^T$ (상태 공분산)
   - $\mathbf{H}_l = \mathbf{L}$ (H의 최소제곱 역행렬)

**특이행렬 검출**: $|\det(\mathbf{H}^T\mathbf{W}\mathbf{H})| < 10^{-15}$이면 실패

**함수 로직**:
- R이 NULL이면 자동으로 단위행렬 사용
- 모든 매개변수가 optional (H만 필수)
- 특이행렬 안전 처리 및 메모리 효율적 계산

</details>

##### ● Ekf() - 확장칼만필터
<details>
<summary>상세 설명</summary>

**목적**: 확장칼만필터 업데이트

**입력**:
- `const mat_t *H`: 설계 행렬 (DOUBLE 타입)
- `const mat_t *v`: 측정 잔차 (DOUBLE 타입)
- `const mat_t *R`: 측정 노이즈 공분산 (DOUBLE 타입)
- `mat_t *x`: (선택적) 상태벡터 (n×1, 갱신됨)
- `mat_t *P`: 오차 공분산 (n×n, 갱신됨, 필수)
- `mat_t *K`: (선택적) 칼만이득 (n×m, 출력)

**출력**:
- `int`: 성공 시 1, 실패 시 0

**수학 공식 (실제 구현 알고리즘)**:

**단계별 계산**:

1. **전치행렬**: $\mathbf{H}^T$ = H의 전치행렬

2. **예측 측정 공분산**:
   - $\mathbf{H}\mathbf{P} = \mathbf{H} \times \mathbf{P}$
   - $\mathbf{H}\mathbf{P}\mathbf{H}^T = \mathbf{H}\mathbf{P} \times \mathbf{H}^T$

3. **혁신 공분산**:
   $$\mathbf{S} = \mathbf{H}\mathbf{P}\mathbf{H}^T + \mathbf{R}$$

4. **혁신 공분산 역행렬**: $\mathbf{S}^{-1} = \text{inv}(\mathbf{S})$

5. **칼만 이득 계산**:
   - $\mathbf{P}\mathbf{H}^T = \mathbf{P} \times \mathbf{H}^T$
   - $\mathbf{K} = \mathbf{P}\mathbf{H}^T \times \mathbf{S}^{-1}$

6. **상태벡터 갱신** (x가 NULL이 아닐 때만):
   - $\mathbf{K}\boldsymbol{v} = \mathbf{K} \times \boldsymbol{v}$
   - $\boldsymbol{x} = \boldsymbol{x} + \mathbf{K}\boldsymbol{v}$

7. **공분산 갱신 (Joseph 형태)**:
   - $\mathbf{K}\mathbf{H} = \mathbf{K} \times \mathbf{H}$
   - $\mathbf{I} = \text{eye}(n)$ (n×n 단위행렬)
   - $(\mathbf{I} - \mathbf{K}\mathbf{H}) = \mathbf{I} - \mathbf{K}\mathbf{H}$
   - $(\mathbf{I} - \mathbf{K}\mathbf{H})^T$ = $(\mathbf{I} - \mathbf{K}\mathbf{H})$의 전치행렬
   - $\mathbf{K}^T$ = $\mathbf{K}$의 전치행렬
   - $\mathbf{K}\mathbf{R}\mathbf{K}^T = \mathbf{K} \times \mathbf{R} \times \mathbf{K}^T$
   - $\mathbf{P} = (\mathbf{I} - \mathbf{K}\mathbf{H}) \times \mathbf{P} \times (\mathbf{I} - \mathbf{K}\mathbf{H})^T + \mathbf{K}\mathbf{R}\mathbf{K}^T$

**Joseph 형태**: $\mathbf{P} = (\mathbf{I} - \mathbf{K}\mathbf{H}) \mathbf{P} (\mathbf{I} - \mathbf{K}\mathbf{H})^T + \mathbf{K}\mathbf{R}\mathbf{K}^T$로 수치적 안정성 보장

**함수 로직**:
- P는 필수 매개변수 (반드시 갱신됨)
- x는 선택적 (NULL이면 상태벡터 갱신 안 함)
- K는 선택적 (NULL이면 칼만이득 출력 안 함)
- 모든 중간 계산에서 메모리 안전성 보장

</details>

---

## ▲ 사용 예시

### ◆ 기본 행렬 연산
```c
// 3×3 DOUBLE 행렬 생성
mat_t *A = Mat(3, 3, DOUBLE);
mat_t *B = Eye(3, DOUBLE);

// 행렬 덧셈: C = 2*A + 3*B^T
mat_t *C = MatAdd(2.0, A, false, 3.0, B, true);

// 행렬 곱셈: D = A * B
mat_t *D = MatMul(1.0, A, false, 1.0, B, false);

// 메모리 해제
FreeMat(A); FreeMat(B); FreeMat(C); FreeMat(D);
```

### ◆ 고급 인덱싱
```c
// 5×5 행렬 생성
mat_t *A = Mat(5, 5, DOUBLE);

// 인덱스 벡터 생성 (2번째, 4번째 행/열 선택)
idx_t *ridx = Idx(2, INT);
IdxSetI(ridx, 0, 1); IdxSetI(ridx, 1, 3);
idx_t *cidx = Idx(2, INT);
IdxSetI(cidx, 0, 0); IdxSetI(cidx, 1, 2);

// 벡터 인덱싱으로 2×2 부분 행렬 추출
mat_t *sub = MatVecIdx(A, ridx, cidx);

// 메모리 해제
FreeMat(A); FreeMat(sub);
FreeIdx(ridx); FreeIdx(cidx);
```

### ◆ 최소제곱법 예시
```c
// 설계 행렬 H (6×3), 측정 벡터 y (6×1)
mat_t *H = Mat(6, 3, DOUBLE);
mat_t *y = Mat(6, 1, DOUBLE);
mat_t *x = Mat(3, 1, DOUBLE);
mat_t *P = Mat(3, 3, DOUBLE);

// 최소제곱법 수행 (R=NULL이면 자동으로 단위행렬 사용)
int info = Lsq(H, y, NULL, x, P, NULL);

if (info) {
    // 추정 성공
    double estimate = MatGetD(x, 0, 0);
}

// 메모리 해제
FreeMat(H); FreeMat(y); FreeMat(x); FreeMat(P);
```

---

## ▲ 성능 특성

### ◆ 메모리 효율성
- **SIMD 최적화**: 32바이트 정렬로 AVX 명령어 활용
- **타입별 최적화**: BOOL(1바이트), INT(4바이트), DOUBLE(8바이트)
- **제자리 연산**: 메모리 사용량 최소화

### ◆ 수치 안정성
- **LU 분해**: 부분 피벗팅으로 수치 안정성 확보
- **특이행렬 검출**: 안전한 역행렬 계산
- **방어적 프로그래밍**: 모든 입력 검증

### ◆ 호환성
- **플랫폼 독립**: Windows, macOS, Linux 지원
- **MATLAB 호환**: Column-major 저장 방식
- **표준 준수**: C99 표준 완전 호환

### ◆ 확장성
- **모듈화 설계**: 독립적인 함수 단위
- **타입 안전성**: 컴파일 타임 타입 검증
- **optional 매개변수**: 유연한 인터페이스 제공

---

**■ 이 모듈은 GNSS 정밀 측위 연산에 필수적인 고성능 수치 엔진입니다.**
