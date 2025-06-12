# ★ 고성능 행렬 연산 모듈 (matrix)

GNSS 정밀 측위를 위한 고성능 수치 연산 엔진입니다.

## ■ 목차

1. [기본 개념](#-기본-개념)
2. [데이터 타입 구조](#-데이터-타입-구조)
3. [데이터 타입 목록](#-데이터-타입-목록)
4. [함수 구조](#-함수-구조)
5. [함수 목록](#-함수-목록)
6. [사용 예시](#-사용-예시)
7. [성능 특성](#-성능-특성)

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
- **초기화/해제 쌍**: `Mat()`/`FreeMat()`, `Vec()`/`FreeVec()`
- **NULL 안전성**: 모든 해제 함수에서 NULL 포인터 검증
- **빈 행렬 지원**: 0×0 차원 행렬 처리 가능

### ◆ 타입 시스템
두 가지 데이터 타입을 지원하여 메모리 효율성과 정확도를 동시에 제공합니다:
- **DOUBLE**: 64비트 부동소수점 (GNSS 계산용)
- **INT**: 32비트 정수 (인덱스, 플래그용)

---

## ▲ 데이터 타입 구조

```
matrix 모듈 타입 계층
├── type_t (enum)
│   ├── INT ────────────── 32비트 정수 타입
│   └── DOUBLE ─────────── 64비트 부동소수점 타입
├── idx_t (struct)
│   ├── n ──────────────── 인덱스 개수
│   └── idx ────────────── 인덱스 배열 (uint32_t*)
└── mat_t (struct)
    ├── rows ───────────── 행 개수
    ├── cols ───────────── 열 개수
    ├── type ───────────── 데이터 타입 (INT/DOUBLE)
    └── data ───────────── 32바이트 정렬 데이터 포인터 (column-major)
```

---

## ▲ 데이터 타입 목록

#### ◆ type_t (enum)
<details>
<summary>상세 설명</summary>

**목적**: 행렬/벡터 데이터 타입 구분

**정의**:
```c
typedef enum {INT, DOUBLE} type_t;
```

**값**:
- `INT`: 32비트 정수 타입
- `DOUBLE`: 64비트 부동소수점 타입

**사용**: 모든 행렬/벡터 생성 시 타입 지정에 활용

</details>

#### ◆ idx_t (struct)
<details>
<summary>상세 설명</summary>

**목적**: 행렬 인덱싱용 인덱스 벡터 구조

**정의**:
```c
typedef struct idx {
    uint32_t n;      // 인덱스 개수
    uint32_t *idx;   // 인덱스 배열
} idx_t;
```

**특징**:
- 행렬/벡터 인덱싱에 사용
- 32비트 정수 배열, 32바이트 정렬
- 동적 크기, 메모리 안전 해제 지원

**접근 함수**:
- `IdxGetI()/IdxSetI()`: 인덱스 접근/설정

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
│   └── Ones() ────────────── 일행렬 생성
├── 행렬 변환
│   ├── MatTr() ───────────── 행렬 전치
│   └── MatTrIn() ─────────── 제자리 전치
├── 행렬 연산
│   ├── MatCopy() ─────────── 행렬 복사
│   ├── MatCopyIn() ───────── 제자리 복사
│   ├── MatAdd() ──────────── 일반화 행렬 덧셈 [C = a*A^T + b*B^T]
│   ├── MatAddIn() ────────── 제자리 덧셈
│   ├── MatMul() ──────────── 일반화 행렬 곱셈 [C = a*A^T * b*B^T]
│   ├── MatMulIn() ────────── 제자리 곱셈
│   ├── MatInv() ──────────── 행렬 역행렬 [Ai = inv(a*A^T)]
│   └── MatInvIn() ────────── 제자리 역행렬
├── 벡터 연산 (mat_t, cols=1)
│   ├── Dot() ──────────────── 벡터 내적 (DOUBLE 전용)
│   ├── Cross3() ──────────── 3차원 외적
│   └── Norm() ────────────── 유클리드 노름 (DOUBLE 전용)
├── 분석 함수
│   └── MatDet() ──────────── 행렬식 계산 (LU 분해)
└── 고급 알고리즘
    ├── Lsq() ─────────────── 최소제곱법 [L = Q*H^T*W, x = L*y, P = L*R*L^T]
    └── Ekf() ─────────────── 확장칼만필터 [K = P*H^T*S^-1, x = x + K*v, P = (I-K*H)*P]
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
- `type_t type`: 데이터 타입 (DOUBLE/INT)

**출력**:
- `mat_t *`: 초기화된 행렬 포인터 (실패 시 NULL)

**함수 로직**:
1. 입력 매개변수 유효성 검사 (행/열 개수 ≥ 0)
2. 행렬 구조체 메모리 할당
3. 32바이트 정렬 데이터 메모리 할당 (플랫폼별 분기)
4. 구조체 필드 초기화 및 반환

</details>

##### ● FreeMat() - 행렬 메모리 해제
<details>
<summary>상세 설명</summary>

**목적**: 행렬 메모리 안전 해제

**입력**:
- `mat_t *mat`: 해제할 행렬 포인터

**출력**:
- `void`: 반환값 없음

**함수 로직**:
1. NULL 포인터 검사로 안전 보장
2. 데이터 메모리 해제 (플랫폼별 분기)
3. 구조체 메모리 해제

</details>

##### ● Idx() - 인덱스 벡터 생성
<details>
<summary>상세 설명</summary>

**목적**: 인덱스 벡터 생성 및 메모리 할당

**입력**:
- `int n`: 인덱스 개수 (≥ 0)

**출력**:
- `idx_t *`: 초기화된 인덱스 벡터 포인터 (실패 시 NULL)

**함수 로직**:
1. 입력 매개변수 유효성 검사 (n ≥ 0)
2. 인덱스 벡터 구조체 메모리 할당
3. 32바이트 정렬 데이터 메모리 할당 (플랫폼별 분기)
4. 구조체 필드 초기화 및 반환

</details>

##### ● FreeIdx() - 인덱스 벡터 해제
<details>
<summary>상세 설명</summary>

**목적**: 인덱스 벡터 메모리 안전 해제

**입력**:
- `idx_t *idx`: 해제할 인덱스 벡터 포인터

**출력**:
- `void`: 반환값 없음

**함수 로직**:
1. NULL 포인터 검사로 안전 보장
2. 데이터 메모리 해제 (플랫폼별 분기)
3. 구조체 메모리 해제

</details>

#### ◆ 특수 행렬 생성 함수

##### ● Eye() - 단위행렬 생성
<details>
<summary>상세 설명</summary>

**목적**: 단위행렬(대각선 원소가 1인 정방행렬) 생성

**입력**:
- `int size`: 행렬 크기 (size × size)
- `type_t type`: 데이터 타입 (DOUBLE/INT)

**출력**:
- `mat_t *`: 단위행렬 포인터 (실패 시 NULL)

**함수 로직**:
1. 정방행렬 생성 (`Mat(size, size, type)`)
2. 모든 원소를 0으로 초기화
3. 대각선 원소를 1로 설정 (i == j인 위치)
4. 생성된 단위행렬 반환

</details>

##### ● Zeros() - 영행렬 생성
<details>
<summary>상세 설명</summary>

**목적**: 모든 원소가 0인 행렬 생성

**입력**:
- `int rows`: 행 개수
- `int cols`: 열 개수
- `type_t type`: 데이터 타입 (DOUBLE/INT)

**출력**:
- `mat_t *`: 영행렬 포인터 (실패 시 NULL)

**함수 로직**:
1. 지정된 크기의 행렬 생성
2. 메모리를 0으로 초기화 (`memset`)
3. 초기화된 영행렬 반환

</details>

##### ● Ones() - 일행렬 생성
<details>
<summary>상세 설명</summary>

**목적**: 모든 원소가 1인 행렬 생성

**입력**:
- `int rows`: 행 개수
- `int cols`: 열 개수
- `type_t type`: 데이터 타입 (DOUBLE/INT)

**출력**:
- `mat_t *`: 일행렬 포인터 (실패 시 NULL)

**함수 로직**:
1. 지정된 크기의 행렬 생성
2. 모든 원소를 1로 설정 (타입별 분기)
3. 초기화된 일행렬 반환

</details>

#### ◆ 행렬 변환 함수

##### ● MatTr() - 행렬 전치
<details>
<summary>상세 설명</summary>

**목적**: 행렬의 전치행렬 생성 (행과 열을 바꿈)

**입력**:
- `const mat_t *A`: 입력 행렬 (m×n)

**출력**:
- `mat_t *`: 전치된 행렬 (n×m), 실패 시 NULL

**함수 로직**:
1. 입력 행렬 유효성 검사
2. 전치된 크기의 새 행렬 생성 (cols × rows)
3. 원소별 전치 복사: `result[j][i] = input[i][j]`
4. 전치된 행렬 반환

</details>

##### ● MatTrIn() - 제자리 행렬 전치
<details>
<summary>상세 설명</summary>

**목적**: 정방행렬의 제자리 전치 (메모리 효율적)

**입력**:
- `mat_t *A`: 전치할 정방행렬 (수정됨)

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:
1. 정방행렬 여부 확인 (rows == cols)
2. 상삼각 영역에서 하삼각 영역으로 원소 교환
3. 대각선 원소는 그대로 유지
4. 성공/실패 상태 반환

</details>

#### ◆ 벡터 연산 함수 (mat_t, cols=1)

##### ● Dot() - 벡터 내적
<details>
<summary>상세 설명</summary>

**목적**: 두 벡터의 내적 계산 (mat_t, cols=1만 지원)

**입력**:
- `const mat_t *a`: 첫 번째 벡터 (DOUBLE 타입, cols=1)
- `const mat_t *b`: 두 번째 벡터 (DOUBLE 타입, cols=1)
- `double *c`: 내적 결과 저장 포인터

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:
1. 벡터 차원 및 타입 호환성 검증 (cols=1, DOUBLE)
2. 원소별 곱셈 및 누적 합산
3. 성공/실패 반환

</details>

##### ● Cross3() - 3차원 외적
<details>
<summary>상세 설명</summary>

**목적**: 3차원 벡터의 외적 계산 (mat_t, cols=1만 지원)

**입력**:
- `const mat_t *a`: 첫 번째 3차원 벡터 (DOUBLE, 3x1)
- `const mat_t *b`: 두 번째 3차원 벡터 (DOUBLE, 3x1)
- `mat_t *c`: 외적 결과 벡터 (DOUBLE, 3x1)

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:
1. 3차원 벡터 여부 및 타입 검증 (rows=3, cols=1, DOUBLE)
2. 외적 공식 적용
3. 성공/실패 반환

</details>

##### ● Norm() - 유클리드 노름
<details>
<summary>상세 설명</summary>

**목적**: 벡터의 유클리드 노름(크기) 계산 (mat_t, cols=1만 지원)

**입력**:
- `const mat_t *a`: 입력 벡터 (DOUBLE 타입, cols=1)

**출력**:
- `double`: 벡터 노름 $\|\boldsymbol{a}\| = \sqrt{\sum_{i=1}^n a_i^2}$ (실패 시 0.0)

**함수 로직**:
1. 벡터 타입 및 차원 검증 (DOUBLE, cols=1)
2. 원소별 제곱합 계산
3. 제곱근 연산으로 노름 계산

</details>

#### ◆ 행렬 연산 함수

##### ● MatCopy() - 행렬 복사
<details>
<summary>상세 설명</summary>

**목적**: 행렬의 전체 복사본 생성

**입력**:
- `const mat_t *A`: 복사할 원본 행렬

**출력**:
- `mat_t *`: 복사된 행렬 포인터 (실패 시 NULL)

**함수 로직**:
1. 동일한 크기와 타입의 새 행렬 생성
2. 메모리 블록 단위 복사 (`memcpy`)
3. 복사된 행렬 반환

</details>

##### ● MatCopyIn() - 제자리 행렬 복사
<details>
<summary>상세 설명</summary>

**목적**: 한 행렬에서 다른 행렬로 직접 복사

**입력**:
- `mat_t *des`: 대상 행렬 (수정됨)
- `const mat_t *src`: 원본 행렬

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:
1. 차원 및 타입 호환성 검증
2. 메모리 블록 단위 직접 복사
3. 추가 메모리 할당 없는 효율적 처리

</details>

##### ● MatAdd() - 일반화 행렬 덧셈
<details>
<summary>상세 설명</summary>

**목적**: 일반화된 행렬 덧셈 연산 수행

**입력**:
- `double a`: 첫 번째 행렬의 스칼라 계수
- `const mat_t *A`: 첫 번째 행렬
- `bool trA`: A의 전치 여부
- `double b`: 두 번째 행렬의 스칼라 계수
- `const mat_t *B`: 두 번째 행렬
- `bool trB`: B의 전치 여부

**출력**:
- `mat_t *`: 결과 행렬 `C = a*A^T + b*B^T` (실패 시 NULL)

**함수 로직**:
1. 입력 행렬 차원 호환성 검증
2. 전치 옵션에 따른 실제 차원 계산
3. 결과 행렬 생성 및 원소별 연산 수행
4. 타입별 분기 처리 (DOUBLE/INT)

</details>

##### ● MatAddIn() - 제자리 행렬 덧셈
<details>
<summary>상세 설명</summary>

**목적**: 첫 번째 행렬에 덧셈 결과 저장

**입력**:
- `mat_t *A`: 결과가 저장될 행렬 (수정됨)
- `double a`: A의 스칼라 계수
- `bool trA`: A의 전치 여부
- `double b`: B의 스칼라 계수
- `const mat_t *B`: 두 번째 행렬
- `bool trB`: B의 전치 여부

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:
1. 차원 호환성 검증
2. 제자리 연산 수행: `A = a*A^T + b*B^T`
3. 메모리 효율적 처리

</details>

##### ● MatMul() - 일반화 행렬 곱셈
<details>
<summary>상세 설명</summary>

**목적**: 일반화된 행렬 곱셈 연산 수행

**입력**:
- `double a`: 첫 번째 행렬의 스칼라 계수
- `const mat_t *A`: 첫 번째 행렬
- `bool trA`: A의 전치 여부
- `double b`: 두 번째 행렬의 스칼라 계수
- `const mat_t *B`: 두 번째 행렬
- `bool trB`: B의 전치 여부

**출력**:
- `mat_t *`: 결과 행렬 `C = a*A^T * b*B^T` (실패 시 NULL)

**함수 로직**:
1. 행렬 곱셈 차원 호환성 검증 (A의 열 == B의 행)
2. 전치 옵션에 따른 실제 차원 계산
3. 결과 행렬 생성 및 곱셈 연산 수행
4. 최적화된 루프 순서 (j-i-k 순서)

</details>

##### ● MatMulIn() - 제자리 행렬 곱셈
<details>
<summary>상세 설명</summary>

**목적**: 첫 번째 행렬에 곱셈 결과 저장

**입력**:
- `mat_t *A`: 결과가 저장될 행렬 (수정됨)
- `double a`: A의 스칼라 계수
- `bool trA`: A의 전치 여부
- `double b`: B의 스칼라 계수
- `const mat_t *B`: 두 번째 행렬
- `bool trB`: B의 전치 여부

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:
1. 차원 호환성 검증
2. 임시 행렬을 통한 안전한 제자리 연산
3. 결과를 원본 행렬에 복사

</details>

##### ● MatInv() - 행렬 역행렬
<details>
<summary>상세 설명</summary>

**목적**: 행렬의 역행렬 계산

**입력**:
- `double a`: 스칼라 계수
- `const mat_t *A`: 입력 정방행렬
- `bool trA`: A의 전치 여부

**출력**:
- `mat_t *`: 역행렬 `Ai = inv(a*A^T)` (실패 시 NULL)

**함수 로직**:
1. 정방행렬 여부 및 특이행렬 검사
2. LU 분해를 통한 역행렬 계산
3. 부분 피벗팅으로 수치 안정성 확보
4. DOUBLE 타입에서만 지원

</details>

##### ● MatInvIn() - 제자리 역행렬
<details>
<summary>상세 설명</summary>

**목적**: 원본 행렬을 역행렬로 교체

**입력**:
- `mat_t *A`: 역행렬로 교체될 행렬 (수정됨)
- `double a`: 스칼라 계수
- `bool trA`: A의 전치 여부

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:
1. 임시 역행렬 계산
2. 원본 행렬에 결과 복사
3. 임시 메모리 해제

</details>

#### ◆ 분석 함수

##### ● MatDet() - 행렬식 계산
<details>
<summary>상세 설명</summary>

**목적**: 정방행렬의 행렬식 계산

**입력**:
- `const mat_t *A`: 입력 정방행렬 (DOUBLE 타입)

**출력**:
- `double`: 행렬식 값 (실패 시 0.0)

**함수 로직**:
1. 정방행렬 및 DOUBLE 타입 검증
2. LU 분해를 통한 행렬식 계산
3. 부분 피벗팅 시 부호 조정
4. 대각선 원소의 곱으로 최종 결과 계산

</details>

#### ◆ 고급 알고리즘

##### ● Lsq() - 최소제곱법
<details>
<summary>상세 설명</summary>

**목적**: 가중 최소제곱법으로 선형 시스템 해결

**입력**:
- `const mat_t *H`: 설계행렬 (m×n)
- `const mat_t *y`: 관측벡터 (m×1)
- `const mat_t *R`: 관측 공분산행렬 (m×m, 필수)
- `mat_t *x`: 해벡터 (n×1, 출력)
- `mat_t *P`: 상태 공분산행렬 (n×n, 출력)
- `mat_t *Hl`: 최소제곱 역행렬 (n×m, 출력, NULL 가능)

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:
1. 가중행렬 계산: $\mathbf{W} = \mathbf{R}^{-1}$
2. 정보행렬 구성: $\mathbf{Q} = (\mathbf{H}^T \mathbf{W} \mathbf{H})^{-1}$
3. 최소제곱 역행렬: $\mathbf{L} = \mathbf{Q} \mathbf{H}^T \mathbf{W}$
4. 상태 추정: $\boldsymbol{x} = \mathbf{L} \boldsymbol{y}$
5. 공분산 전파: $\mathbf{P} = \mathbf{L} \mathbf{R} \mathbf{L}^T$

</details>

##### ● Ekf() - 확장칼만필터
<details>
<summary>상세 설명</summary>

**목적**: 확장칼만필터 갱신 단계 수행

**입력**:
- `const mat_t *H`: 관측행렬 (m×n)
- `const mat_t *v`: 혁신벡터 (m×1)
- `const mat_t *R`: 관측잡음 공분산 (m×m)
- `mat_t *x`: 상태벡터 (n×1, 갱신됨)
- `mat_t *P`: 오차 공분산 (n×n, 갱신됨)
- `mat_t *K`: 칼만이득 (n×m, 출력, NULL 가능)

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:
1. 혁신 공분산: $\mathbf{S} = \mathbf{H} \mathbf{P} \mathbf{H}^T + \mathbf{R}$
2. 칼만이득 계산: $\mathbf{K} = \mathbf{P} \mathbf{H}^T \mathbf{S}^{-1}$
3. 상태벡터 갱신: $\boldsymbol{x} = \boldsymbol{x} + \mathbf{K} \boldsymbol{v}$
4. 공분산 갱신: $\mathbf{P} = (\mathbf{I} - \mathbf{K} \mathbf{H}) \mathbf{P}$ (표준 형태)

</details>

---

## ▲ 사용 예시

### ◆ 기본 행렬/벡터 연산

```c
#include "matrix.h"

int main() {
    // 3×3 DOUBLE 행렬 생성
    mat_t *A = Mat(3, 3, DOUBLE);
    mat_t *B = Mat(3, 3, DOUBLE);
    mat_t *v1 = Mat(3, 1, DOUBLE); // 벡터는 cols=1
    mat_t *v2 = Mat(3, 1, DOUBLE);

    // 값 설정
    for (int i = 0; i < 3; i++) {
        MatSetD(v1, i, 0, (double)(i+1));
        MatSetD(v2, i, 0, (double)(i+4));
    }

    // 벡터 내적
    double dot;
    if (Dot(v1, v2, &dot)) {
        printf("dot = %f\n", dot);
    }

    // 벡터 외적
    mat_t *cross = Mat(3, 1, DOUBLE);
    if (Cross3(v1, v2, cross)) {
        // cross 사용
    }
    FreeMat(cross);

    // 벡터 노름
    double norm = Norm(v1);

    // 메모리 해제
    FreeMat(A); FreeMat(B); FreeMat(v1); FreeMat(v2);
    return 0;
}
```

### ◆ 최소제곱법 적용

```c
// 선형 시스템: H*x = y 해결
mat_t *H = Mat(4, 2, DOUBLE);  // 4개 관측, 2개 미지수
mat_t *y = Mat(4, 1, DOUBLE);  // 관측값
mat_t *x = Mat(2, 1, DOUBLE);  // 해
mat_t *P = Mat(2, 2, DOUBLE);  // 공분산

// H, y 값 설정 (생략)

// 관측 공분산 행렬 (단위행렬로 가정)
mat_t *R = Eye(4, DOUBLE);

// 최소제곱법 적용
int result = Lsq(H, y, R, x, P, NULL);
if (result) {
    printf("해: x1 = %f, x2 = %f\n", MatGetD(x, 0, 0), MatGetD(x, 1, 0));
}

FreeMat(H); FreeMat(y); FreeMat(x); FreeMat(P); FreeMat(R);
```

---

## ▲ 성능 특성

### ◆ 메모리 정렬 최적화
- **SIMD 가속**: 32바이트 정렬로 AVX/AVX2 명령어 활용
- **캐시 효율성**: Column-major 순서로 캐시 미스 최소화
- **성능 향상**: 일반 malloc 대비 2-4배 빠른 벡터 연산

### ◆ 수치 안정성
- **부분 피벗팅**: LU 분해에서 수치 오차 최소화
- **조건수 검사**: 특이행렬 탐지로 안전성 보장
- **정밀도 유지**: DOUBLE 타입으로 15자리 유효숫자 지원

### ◆ 복잡도 분석
- **행렬 곱셈**: $O(n^3)$ - 표준 알고리즘
- **역행렬**: $O(n^3)$ - LU 분해 기반
- **행렬식**: $O(n^3)$ - LU 분해 기반
- **최소제곱**: $O(mn^2 + n^3)$ - m개 관측, n개 미지수

### ◆ 메모리 사용량
- **행렬**: `sizeof(mat_t) + align32(rows*cols*sizeof(type))`
- **인덱스 벡터**: `sizeof(idx_t) + align32(n*sizeof(uint32_t))`
- **정렬 오버헤드**: 평균 16바이트 추가 (최대 31바이트)

### ◆ 플랫폼 호환성
- **Windows**: `_aligned_malloc()` / `_aligned_free()`
- **POSIX**: `posix_memalign()` / `free()`
- **자동 감지**: 컴파일 시점에 플랫폼별 최적화 선택

---

**■ 이 모듈은 GNSS 정밀 측위 연산에 필수적인 고성능 수치 엔진입니다.**
