---
layout: default
title: 위성 궤도력 처리 모듈 (ephemeris)
---

# 위성 궤도력 처리 모듈 (ephemeris)

GNSS 위성의 방송궤도력을 이용한 위성 위치 및 시계 계산을 위한 종합 모듈입니다.

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

### 1.1 방송궤도력 계산 시스템
본 모듈은 **모든 GNSS 시스템의 방송궤도력을 이용한 위성 위치 및 시계 계산**을 제공합니다. 각 시스템의 공식 ICD(Interface Control Document) 표준을 완벽히 준수하여 정밀한 위성 궤도 계산을 수행합니다.

**핵심 기능**:
- **궤도 계산**: Keplerian (GPS/GAL/BDS/QZS/IRN), 수치적분 (GLONASS), 다항식 (SBAS) 모델
- **시계 보정**: 시스템별 위성 시계 바이어스 계산
- **정확도 관리**: URA/SISA 지수 변환 및 분산 계산
- **궤도력 선택**: 최적 궤도력 데이터 자동 선택
- **타입 관리**: 시스템별 궤도력 타입 설정

### 1.2 방송궤도력 개념
방송궤도력(Broadcast Ephemeris)은 위성이 주기적으로 송신하는 궤도 정보로, 실시간 위성 위치 계산에 사용됩니다:
- **실시간성**: 위성에서 실시간으로 방송
- **정확도**: 수 미터 수준의 위성 위치 정확도
- **유효기간**: 2-4시간 (시스템별 상이)

방송궤도력을 이용한 위성 위치 계산의 기본 수식:

$$\boldsymbol{r}^s(t) = f(\text{eph}, t - t_{\text{oe}})$$

여기서:
- $\boldsymbol{r}^s(t)$: 시각 $t$에서의 위성 위치 벡터
- $\text{eph}$: 방송궤도력 파라미터 집합
- $t_{\text{oe}}$: 궤도력 기준 시각 (Time of Ephemeris)

### 1.3 시스템별 궤도력 특성
각 GNSS 시스템은 고유한 방송궤도력 형식을 사용하며, 일부 시스템은 다중 타입을 지원합니다:
- **단일 타입**: GPS, GLONASS, BeiDou, QZSS, IRNSS, SBAS
- **다중 타입**: Galileo (I/NAV, F/NAV)

---

## 2. 데이터 타입 구조

```
ephemeris 모듈 데이터 계층
├── 궤도력 데이터 구조체
│   ├── eph_t ──────────────── 방송궤도력 구조체
│   └── ephs_t ─────────────── 궤도력 데이터셋 (동적 배열)
├── 궤도 계산 상수
│   ├── URA_ERR[15] ────────── GPS/QZS URA 오차 테이블
│   ├── NURA ───────────────── URA 테이블 크기
│   ├── J2_GLO ─────────────── GLONASS J2 섭동 계수
│   ├── MAX_ITER_KEPLER ────── 케플러 방정식 최대 반복 횟수
│   ├── STD_EPH_GLO ────────── GLONASS 궤도력 표준 편차
│   └── MAX_ERR_EPH ────────── 궤도력 최대 허용 오차
├── 시스템별 타입 관리
│   ├── EPHTYPE[NSYS] ──────── 시스템별 궤도력 타입 배열
│   └── 타입 범위 ────────────── 시스템별 유효 타입 범위
└── 계산 매개변수
    ├── TSTEP ──────────────── GLONASS 적분 시간 스텝
    ├── TOL_KEPLER ─────────── 케플러 방정식 허용오차
    ├── SIN_5/COS_5 ────────── BeiDou GEO 회전 상수
    └── MAX_DTOE_* ─────────── 시스템별 궤도력 유효시간
```

---

## 3. 데이터 타입 목록

### 3.1 ephs_t 구조체
<details>
<summary>상세 설명</summary>

**목적**: 다중 궤도력 데이터를 효율적으로 관리하는 동적 배열 구조체

**정의**: `ephemeris.h`에 정의된 궤도력 데이터셋 구조체

**주요 멤버**:
- **현재 개수**: `n` (현재 저장된 궤도력 개수)
- **할당 용량**: `nmax` (현재 할당된 배열 크기)
- **데이터 배열**: `eph` (eph_t 포인터, 동적 할당)

**동적 확장 정책**:
- **초기 할당**: 2개 (`nmax = 2`)
- **확장 조건**: `n >= nmax`일 때
- **확장 크기**: 현재 크기의 2배 (`nnew = nmax * 2`)
- **메모리 관리**: `realloc()` 사용으로 안전한 확장

**사용 패턴**:
```c
ephs_t ephs;
InitEphs(&ephs);    // 초기화
AddEph(&ephs, eph); // 자동 확장
SortEphs(&ephs);    // 정렬/중복제거
FreeEphs(&ephs);    // 해제
```

</details>

### 3.2 eph_t 구조체
<details>
<summary>상세 설명</summary>

**목적**: 모든 GNSS 시스템의 방송궤도력 데이터 저장

**정의**: `ephemeris.h`에 정의된 궤도력 구조체

**주요 멤버**:
- **위성 식별**: `sat` (위성 인덱스), `IODE`/`IODC` (궤도력 식별자)
- **시간 정보**: `toe` (궤도력 시각), `toc` (시계 시각), `ttr` (전송 시각)
- **Keplerian 요소**: `A` (장반축), `e` (이심률), `i0` (경사각), `OMG0` (승교점 경도), `omg` (근지점 인수), `M0` (평균 근점각)
- **섭동 계수**: `deln`, `OMGd`, `iodt`, `cuc`, `cus`, `crc`, `crs`, `cic`, `cis`
- **시계 계수**: `af0`, `af1`, `af2` (GPS/GAL/BDS/QZS/IRN), `taun`, `gamn` (GLONASS)
- **위치/속도**: `pos[3]`, `vel[3]`, `acc[3]` (GLONASS/SBAS)
- **정확도/건강**: `sva` (URA/SISA 인덱스), `svh` (위성 건강성)

**사용**: 모든 GNSS 시스템 공통 구조체로 시스템별 필드 선택적 사용

</details>

### 3.3 URA_ERR 테이블
<details>
<summary>상세 설명</summary>

**목적**: GPS/QZS 시스템의 URA(User Range Accuracy) 오차값 테이블

**정의**:
```c
static const double URA_ERR[] = {
    2.40,   3.40,   4.85,   6.85,   9.65,
   13.65,  24.00,  48.00,  96.00, 192.00,
  384.00, 768.00,1536.00,3072.00,6144.00
};
```

**값**: 15개 URA 등급별 오차값 [m] (GPS ICD 표준)

**사용**: `Ura2Idx()`, `Idx2Ura()` 함수에서 인덱스↔오차값 변환

</details>

### 3.4 궤도력 타입 시스템
<details>
<summary>상세 설명</summary>

**목적**: 시스템별 방송궤도력 타입 구분

**정의**: `int` 타입 (0~2, 시스템별 가변 범위)

**값**:

#### 3.4.1 GPS 시스템 궤도력 타입
| 타입 | 명칭 | 설명 |
|------|------|------|
| **0** | LNAV | Legacy Navigation (기본) |

#### 3.4.2 GLONASS 시스템 궤도력 타입
| 타입 | 명칭 | 설명 |
|------|------|------|
| **0** | LNAV | Navigation Message (기본) |

#### 3.4.3 Galileo 시스템 궤도력 타입
| 타입 | 명칭 | 설명 |
|------|------|------|
| **0** | I/NAV | Integrity Navigation (기본) |
| **1** | F/NAV | Freely accessible Navigation |

#### 3.4.4 BeiDou 시스템 궤도력 타입
| 타입 | 명칭 | 설명 |
|------|------|------|
| **0** | D1 | Navigation Message D1 (기본) |

#### 3.4.5 QZSS 시스템 궤도력 타입
| 타입 | 명칭 | 설명 |
|------|------|------|
| **0** | LNAV | Legacy Navigation (기본) |

#### 3.4.6 IRNSS 시스템 궤도력 타입
| 타입 | 명칭 | 설명 |
|------|------|------|
| **0** | LNAV | Navigation Message (기본) |

#### 3.4.7 SBAS 시스템 궤도력 타입
| 타입 | 명칭 | 설명 |
|------|------|------|
| **0** | RINEX | RINEX SBAS Ephemeris (기본) |

**사용**: 모든 시스템의 기본 타입은 0으로 통일

</details>

### 3.5 궤도 계산 상수들
<details>
<summary>상세 설명</summary>

**목적**: 위성 궤도 계산에 사용되는 다양한 물리/수치 상수들

**정의**: `const.h`에 정의된 상수들

**주요 상수**:

#### 3.5.1 MAX_ITER_KEPLER
- **값**: 30
- **용도**: 케플러 방정식 Newton-Raphson 해법의 최대 반복 횟수
- **의미**: 수치적 안정성을 위한 무한루프 방지

#### 3.5.2 STD_EPH_GLO
- **값**: 5.0 [m]
- **용도**: GLONASS 방송궤도력의 표준 편차
- **의미**: GLONASS 위성 위치 정확도 평가에 사용

#### 3.5.3 MAX_ERR_EPH
- **값**: 300.0 [m]
- **용도**: 방송궤도력 최대 허용 오차
- **의미**: 궤도력 건강성 판단 기준값

**사용**: 궤도 계산 알고리즘의 수치적 안정성과 정확도 보장

</details>

### 3.6 EPHTYPE 배열
<details>
<summary>상세 설명</summary>

**목적**: 시스템별 현재 설정된 궤도력 타입 저장

**정의**:
```c
static int EPHTYPE[NSYS] = {0}; // 모든 시스템 기본값 0
```

**값**:
- **크기**: NSYS (활성화된 시스템 개수)
- **인덱스**: 시스템 ID - 1 (0-based)
- **값**: 궤도력 타입 (0~2)

**사용**:
- 읽기: `GetEphType(sys)` 함수 사용
- 쓰기: `SetEphType(sys, type)` 함수 사용
- 초기값: 모든 시스템이 타입 0으로 초기화

</details>

### 3.7 타입 범위 검증
<details>
<summary>상세 설명</summary>

**목적**: 시스템별 유효한 궤도력 타입 범위 정의

**정의**: 시스템별 하드코딩된 범위 검증 로직

**값**:
- **일반 시스템**: 0~1 (GPS, GLONASS, BeiDou, QZSS, IRNSS, SBAS)
- **확장 시스템**: 0~2 (Galileo)

**사용**:
```c
if (sys == 3) {  // Galileo
    valid_range = (type >= 0 && type <= 2);
} else {         // 기타 시스템
    valid_range = (type >= 0 && type <= 1);
}
```

</details>

---

## 4. 함수 구조

```
ephemeris 모듈 함수 계층
├── 궤도력 데이터 구조체 관리
│   ├── InitEphs() ───────────── 궤도력 데이터셋 초기화
│   ├── FreeEphs() ───────────── 궤도력 데이터셋 해제
│   ├── AddEph() ─────────────── 궤도력 데이터 추가
│   ├── SortEphs() ───────────── 궤도력 데이터 정렬/중복제거
│   ├── ResizeEphs() ─────────── 궤도력 배열 크기 조정 (static)
│   └── CompareEph() ─────────── 궤도력 데이터 비교 (static)
├── 위성 위치/시계 계산
│   ├── 통합 인터페이스
│   │   └── SatPosClkBrdc() ──── 모든 시스템 위성 계산
│   ├── 시스템별 궤도 계산
│   │   ├── Eph2Pos() ────────── Keplerian 궤도 (GPS/GAL/BDS/QZS/IRN) (static)
│   │   ├── GloEph2Pos() ─────── GLONASS 수치 적분 궤도 (static)
│   │   └── SbsEph2Pos() ─────── SBAS 2차 다항식 궤도 (static)
│   └── 수치 계산 보조
│       ├── GloDeq() ─────────── GLONASS 미분방정식 (static)
│       └── GloRK4() ─────────── Runge-Kutta 4차 적분 (static)
├── 궤도력 데이터 관리
│   ├── TestEph() ────────────── 궤도력 유효성 검사
│   └── SelectEph() ──────────── 최적 궤도력 선택
├── 정확도 지수 변환
│   ├── URA 처리
│   │   ├── Ura2Idx() ────────── URA 값→인덱스
│   │   └── Idx2Ura() ────────── URA 인덱스→값
│   └── SISA 처리
│       ├── Sisa2Idx() ───────── SISA 값→인덱스
│       └── Idx2Sisa() ───────── SISA 인덱스→값
└── 궤도력 타입 관리
    ├── GetEphType() ─────────── 궤도력 타입 조회
    └── SetEphType() ─────────── 궤도력 타입 설정
```

---

## 5. 함수 목록

### 5.1 궤도력 데이터 구조체 관리 함수

### 5.1.1 InitEphs() - 궤도력 데이터셋 초기화
<details>
<summary>상세 설명</summary>

**목적**: 궤도력 데이터셋 구조체를 안전하게 초기화

**입력**:
- `ephs_t *ephs`: 궤도력 데이터셋 구조체

**출력**: 없음 (void)

**함수 로직**:
```c
if (!ephs) return;                  // NULL 포인터 검증

ephs->n = 0;                        // 현재 궤도력 개수 초기화
ephs->nmax = 0;                     // 할당된 메모리 크기 초기화
ephs->eph = NULL;                   // 궤도력 배열 포인터 초기화
```

**사용 예시**:
```c
ephs_t ephs;
InitEphs(&ephs);  // 안전한 초기화
```

</details>

### 5.1.2 FreeEphs() - 궤도력 데이터셋 해제
<details>
<summary>상세 설명</summary>

**목적**: 궤도력 데이터셋의 동적 할당 메모리를 안전하게 해제

**입력**:
- `ephs_t *ephs`: 궤도력 데이터셋 구조체

**출력**: 없음 (void)

**함수 로직**:
```c
if (!ephs) return;                  // NULL 포인터 검증

if (ephs->eph) {                    // 궤도력 배열이 할당되어 있으면
    free(ephs->eph);                // 동적 할당된 메모리 해제
    ephs->eph = NULL;               // 포인터 NULL로 설정
}
ephs->n = ephs->nmax = 0;           // 카운터 변수들 초기화
```

**사용 예시**:
```c
ephs_t ephs;
InitEphs(&ephs);
// ... 궤도력 데이터 사용 ...
FreeEphs(&ephs);  // 안전한 메모리 해제
```

</details>

### 5.1.3 AddEph() - 궤도력 데이터 추가
<details>
<summary>상세 설명</summary>

**목적**: 궤도력 데이터셋에 새로운 궤도력 데이터를 추가 (자동 크기 확장)

**입력**:
- `ephs_t *ephs`: 궤도력 데이터셋 구조체
- `const eph_t *eph`: 추가할 궤도력 데이터

**출력**: 성공 시 1, 실패 시 0

**함수 로직**:
```c
if (!ephs || !eph) return 0;        // 입력 매개변수 NULL 검증

// 용량 확인 및 자동 확장
if (ephs->n >= ephs->nmax) {
    int nnew = (ephs->nmax == 0) ? 2 : ephs->nmax * 2;  // 초기 2개, 이후 2배 확장
    if (!ResizeEphs(ephs, nnew)) {
        return 0;                    // 메모리 확장 실패
    }
}

ephs->eph[ephs->n] = *eph;          // 궤도력 데이터 복사
ephs->n++;                          // 카운터 증가
return 1;                           // 성공
```

**메모리 확장 패턴**:
```
용량: 0 → 2 → 4 → 8 → 16 → 32 → ...
```

**사용 예시**:
```c
eph_t new_eph = {...};  // 궤도력 데이터
if (AddEph(&ephs, &new_eph)) {
    printf("궤도력 추가 성공\n");
} else {
    printf("궤도력 추가 실패\n");
}
```

</details>

### 5.1.4 SortEphs() - 궤도력 데이터 정렬/중복제거
<details>
<summary>상세 설명</summary>

**목적**: 궤도력 데이터셋을 위성 인덱스 및 전송 시각 순으로 정렬하고 중복 데이터 제거

**입력**:
- `ephs_t *ephs`: 궤도력 데이터셋 구조체

**출력**: 없음 (void)

**함수 로직**:

**1단계: 정렬 (qsort 사용)**
- **1차 키**: 위성 인덱스 (`sat`)
- **2차 키**: 전송 시각 (`ttr`)

**정렬 기준 공식**:
```c
if (eph1->sat != eph2->sat)
    return eph1->sat - eph2->sat;
else
    return (eph1->ttr > eph2->ttr) ? 1 : -1;
```

**2단계: 중복 제거**
중복 판단 기준:
- `sat`: 위성 인덱스 일치
- `IODE`: 궤도력 식별자 일치
- `data`: 데이터 소스 일치
- `toe`: 궤도력 시각 일치

**중복 제거 알고리즘**:
```c
int n = 0;
for (int i = 0; i < ephs->n; i++) {
    // 이전 궤도력과 중복인지 확인 (sat, IODE, data, toe)
    if (i > 0 &&
        ephs->eph[i].sat  == ephs->eph[i-1].sat  &&
        ephs->eph[i].IODE == ephs->eph[i-1].IODE &&
        ephs->eph[i].data == ephs->eph[i-1].data &&
        ephs->eph[i].toe  == ephs->eph[i-1].toe) {
        continue;  // 중복이면 건너뛰기
    }

    // 중복이 아니면 앞으로 복사
    if (n != i) {
        ephs->eph[n] = ephs->eph[i];
    }
    n++;
}
ephs->n = n;  // 압축된 크기로 업데이트
```

**사용 예시**:
```c
// 여러 궤도력 추가 후 정리
AddEph(&ephs, &eph1);
AddEph(&ephs, &eph2);
AddEph(&ephs, &eph1);  // 중복
SortEphs(&ephs);       // 정렬 및 중복 제거
```

</details>

### 5.1.5 ResizeEphs() - 궤도력 배열 크기 조정 (Static)
<details>
<summary>상세 설명</summary>

**목적**: 궤도력 배열의 메모리 크기를 안전하게 조정

**입력**:
- `ephs_t *ephs`: 궤도력 데이터셋 구조체
- `int nnew`: 새로운 배열 크기

**출력**: 성공 시 1, 실패 시 0

**함수 로직**:
1. 입력 검증: `ephs != NULL && nnew > 0`
2. `realloc()`을 사용한 안전한 메모리 재할당
3. 실패 시 기존 데이터 보존
4. 성공 시 `nmax` 업데이트

**안전한 메모리 패턴**:
```c
eph_t *newEph = (eph_t *)realloc(ephs->eph, nnew * sizeof(eph_t));
if (!newEph) return 0;  // 실패 시 기존 데이터 유지
ephs->eph = newEph;     // 성공 시에만 포인터 업데이트
```

</details>

### 5.1.6 CompareEph() - 궤도력 데이터 비교 (Static)
<details>
<summary>상세 설명</summary>

**목적**: qsort()에서 사용할 궤도력 데이터 비교 함수

**입력**:
- `const void *a`: 첫 번째 궤도력 데이터
- `const void *b`: 두 번째 궤도력 데이터

**출력**: 비교 결과 (-1, 0, 1)

**함수 로직**:
1. void 포인터를 eph_t 포인터로 캐스팅
2. 위성 인덱스 비교 (1차 키)
3. 전송 시각 비교 (2차 키)

**비교 공식**:
- `a.sat < b.sat`: return -1
- `a.sat > b.sat`: return +1
- `a.sat == b.sat`:
  - `a.ttr < b.ttr`: return -1
  - `a.ttr > b.ttr`: return +1
  - `a.ttr == b.ttr`: return 0

</details>

### 5.2 위성 위치/시계 계산 함수

### 5.2.1 SatPosClkBrdc() - 통합 위성 위치/시계 계산
<details>
<summary>상세 설명</summary>

**목적**: 모든 GNSS 시스템의 방송궤도력을 이용한 위성 위치, 속도, 시계 바이어스/드리프트 계산

**입력**:
- `double ephtime`: 궤도력 시각 [s]
- `double time`: 신호 송신 시각 [s]
- `int sat`: 위성 인덱스
- `const nav_t *nav`: 항법 데이터
- `int iode`: IODE 값 (-1: 무시)

**출력**:
- `mat_t *rs`: 위성 위치/속도 (1×6) [m, m/s]
- `mat_t *dts`: 시계 바이어스/드리프트 (1×2) [s, s/s]
- `double *var`: 위치/시계 분산 [m²]
- `eph_t *eph`: 사용된 궤도력 데이터

**함수 로직**:
```c
// 1. 행렬 차원 검증 및 초기화
if (rs)  {if (rs->rows != 1 || rs->cols != 6) return 0;}   // 위성 위치/속도 행렬
if (dts) {if (dts->rows != 1 || dts->cols != 2) return 0;} // 시계 바이어스/드리프트 행렬

// 출력 초기화
if (rs)  {for (int i = 0; i < 6; i++) MatSetD(rs, 0, i, 0.0);}
if (dts) {for (int i = 0; i < 2; i++) MatSetD(dts, 0, i, 0.0);}
if (var) {*var = 0.0;}
if (eph) {*eph = (eph_t){0, 0, 0, 0, 0, 0, -1};}  // svh = -1

// 2. 입력 검증
if (!nav || sat <= 0 || ephtime < 0.0 || time < 0.0) return 0;
int sys = Sat2Prn(sat, NULL);
if (sys <= 0 || sys > NSYS) return 0;

// 3. 최적 궤도력 선택
eph_t *ephSelected = SelectEph(ephtime, sat, nav, iode);
if (!ephSelected) return 0;

// 4. 수치 미분을 위한 3점 계산 (tt = 1ms)
double pos0[3], posf[3], posb[3], clk0, clkf, clkb, var0, tt = 1E-3;

switch (Sys2Str(sys)) {
    case STR_GPS: case STR_GAL: case STR_BDS: case STR_QZS: case STR_IRN:
        if (!Eph2Pos(time, ephSelected, pos0, &clk0, &var0)) return 0;
        if (!Eph2Pos(time + tt, ephSelected, posf, &clkf, NULL)) return 0;
        if (!Eph2Pos(time - tt, ephSelected, posb, &clkb, NULL)) return 0;
        break;
    case STR_GLO:
        if (!GloEph2Pos(time, ephSelected, pos0, &clk0, &var0)) return 0;
        if (!GloEph2Pos(time + tt, ephSelected, posf, &clkf, NULL)) return 0;
        if (!GloEph2Pos(time - tt, ephSelected, posb, &clkb, NULL)) return 0;
        break;
    case STR_SBS:
        if (!SbsEph2Pos(time, ephSelected, pos0, &clk0, &var0)) return 0;
        if (!SbsEph2Pos(time + tt, ephSelected, posf, &clkf, NULL)) return 0;
        if (!SbsEph2Pos(time - tt, ephSelected, posb, &clkb, NULL)) return 0;
        break;
    default: return 0;
}

// 5. 결과 저장 (위치 + 수치미분 속도)
if (rs) {
    for (int i = 0; i < 3; i++) MatSetD(rs, 0, i, pos0[i]);        // 위치
    for (int i = 3; i < 6; i++) MatSetD(rs, 0, i, (posf[i-3] - posb[i-3]) / (2.0 * tt)); // 속도
}
if (dts) {
    MatSetD(dts, 0, 0, clk0);                                      // 시계 바이어스
    MatSetD(dts, 0, 1, (clkf - clkb) / (2.0 * tt));               // 시계 드리프트
}
if (var) *var = var0;                                              // 분산
if (eph) *eph = *ephSelected;                                      // 사용된 궤도력
```

**수치 미분 공식**: $\boldsymbol{v} = \frac{d\boldsymbol{r}}{dt} \approx \frac{\boldsymbol{r}(t+\Delta t) - \boldsymbol{r}(t-\Delta t)}{2\Delta t}$

</details>

### 5.2.2 Eph2Pos() - Keplerian 궤도 계산 (Static)
<details>
<summary>상세 설명</summary>

**목적**: GPS/Galileo/BeiDou/QZSS/IRNSS 시스템의 Keplerian 궤도 모델 위성 위치/시계 계산

**입력**:
- `double time`: 계산 시각 [s]
- `const eph_t *eph`: 방송궤도력 데이터
- `double *pos`: 위성 위치 [m] (optional)
- `double *clk`: 시계 바이어스 [s] (optional)
- `double *var`: 위치 분산 [m²] (optional)

**출력**: 성공 시 1, 실패 시 0

**함수 로직**:

**1단계: 평균 운동 및 평균 근점각 계산**
$$n = \sqrt{\frac{\mu}{a^3}} + \Delta n$$
$$M_k = M_0 + n \cdot t_k$$

여기서 $\mu$는 중력 상수, $a$는 장반축, $\Delta n$은 평균 운동 보정, $t_k = t - t_{oe}$

**2단계: 케플러 방정식 해결 (Newton-Raphson)**
$$E_k - e \sin E_k = M_k$$
$$E_{k+1} = E_k - \frac{E_k - e \sin E_k - M_k}{1 - e \cos E_k}$$

**3단계: 진근점각 및 위도 인수**
$$\nu_k = \arctan2(\sqrt{1-e^2}\sin E_k, \cos E_k - e)$$
$$u_k = \nu_k + \omega + c_{us}\sin(2u_k) + c_{uc}\cos(2u_k)$$

**4단계: 궤도 반지름 및 경사각 보정**
$$r_k = a(1 - e \cos E_k) + c_{rs}\sin(2u_k) + c_{rc}\cos(2u_k)$$
$$i_k = i_0 + \dot{i} \cdot t_k + c_{is}\sin(2u_k) + c_{ic}\cos(2u_k)$$

**5단계: 궤도면 좌표 계산**
$$x_k' = r_k \cos u_k, \quad y_k' = r_k \sin u_k$$

**6단계: ECEF 좌표 변환**
$$\Omega_k = \Omega_0 + (\dot{\Omega} - \omega_e) t_k - \omega_e t_{oe}$$

일반 위성:
$$\begin{bmatrix} x \\ y \\ z \end{bmatrix} = \begin{bmatrix} x_k' \cos\Omega_k - y_k' \cos i_k \sin\Omega_k \\ x_k' \sin\Omega_k + y_k' \cos i_k \cos\Omega_k \\ y_k' \sin i_k \end{bmatrix}$$

BeiDou GEO 위성 (5° 회전 보정):
$$\begin{bmatrix} x \\ y \\ z \end{bmatrix} = \mathbf{R}_z(\omega_e t_k) \mathbf{R}_x(-5°) \begin{bmatrix} x_{gk} \\ y_{gk} \\ z_{gk} \end{bmatrix}$$

**7단계: 위성 시계 보정**
$$\Delta t^s = a_{f0} + a_{f1} \cdot t_k + a_{f2} \cdot t_k^2 - \frac{2\sqrt{\mu a} e \sin E_k}{c^2}$$

마지막 항은 상대론적 보정항입니다.

</details>

### 5.2.3 GloEph2Pos() - GLONASS 궤도 계산 (Static)
<details>
<summary>상세 설명</summary>

**목적**: GLONASS 시스템의 수치 적분 기반 위성 위치/시계 계산

**입력/출력**: `Eph2Pos()`와 동일

**함수 로직**:

GLONASS는 케플러 요소 대신 위치/속도/가속도가 직접 방송궤도력에 포함되어 있어 수치적분 방법을 사용합니다.

**1단계: 초기 상태벡터 설정**
$$\boldsymbol{x}_0 = \begin{bmatrix} \boldsymbol{r}_0 \\ \boldsymbol{v}_0 \end{bmatrix} = \begin{bmatrix} \begin{bmatrix} x_0 & y_0 & z_0 \end{bmatrix}^T \\ \begin{bmatrix} \dot{x}_0 & \dot{y}_0 & \dot{z}_0 \end{bmatrix}^T \end{bmatrix}$$

**2단계: GLONASS 미분방정식**
$$\frac{d\boldsymbol{r}}{dt} = \boldsymbol{v}$$

$$\frac{d\boldsymbol{v}}{dt} = \boldsymbol{a}_{central} + \boldsymbol{a}_{J2} + \boldsymbol{a}_{rotation} + \boldsymbol{a}_{eph}$$

여기서:

**중심력항**:
$$\boldsymbol{a}_{central} = -\frac{\mu}{r^3}\boldsymbol{r}$$

**J2 섭동항**:
$$\boldsymbol{a}_{J2} = \frac{3}{2} J_2 \frac{\mu R_e^2}{r^5} \begin{bmatrix} x(1-5z^2/r^2) \\ y(1-5z^2/r^2) \\ z(3-5z^2/r^2) \end{bmatrix}$$

**지구 자전 효과**:
$$\boldsymbol{a}_{rotation} = \boldsymbol{\Omega}_e \times (\boldsymbol{\Omega}_e \times \boldsymbol{r}) + 2\boldsymbol{\Omega}_e \times \boldsymbol{v}$$

여기서 $\boldsymbol{\Omega}_e = [0, 0, \omega_e]^T$

**방송궤도력 가속도**:
$$\boldsymbol{a}_{eph} = [a_x, a_y, a_z]^T$$

방송궤도력에 포함된 달-태양 섭동 가속도를 외부에서 전달받아 사용

**3단계: Runge-Kutta 4차 적분**
$$\boldsymbol{k}_1 = f(\boldsymbol{x}_n, t_n)$$
$$\boldsymbol{k}_2 = f(\boldsymbol{x}_n + \frac{\Delta t}{2}\boldsymbol{k}_1, t_n + \frac{\Delta t}{2})$$
$$\boldsymbol{k}_3 = f(\boldsymbol{x}_n + \frac{\Delta t}{2}\boldsymbol{k}_2, t_n + \frac{\Delta t}{2})$$
$$\boldsymbol{k}_4 = f(\boldsymbol{x}_n + \Delta t \boldsymbol{k}_3, t_n + \Delta t)$$
$$\boldsymbol{x}_{n+1} = \boldsymbol{x}_n + \frac{\Delta t}{6}(\boldsymbol{k}_1 + 2\boldsymbol{k}_2 + 2\boldsymbol{k}_3 + \boldsymbol{k}_4)$$

**4단계: GLONASS 시계 모델**
$$\Delta t^s = -\tau_n + \gamma_n \cdot (t - t_{oc})$$

여기서 $\tau_n$은 시계 바이어스, $\gamma_n$은 상대 주파수 오프셋

</details>

### 5.2.4 SbsEph2Pos() - SBAS 궤도 계산 (Static)
<details>
<summary>상세 설명</summary>

**목적**: SBAS 시스템의 2차 다항식 궤도 모델 위성 위치/시계 계산

**입력/출력**: `Eph2Pos()`와 동일

**함수 로직**:

**1단계: SBAS 2차 다항식 위치 모델**
$$\boldsymbol{r}(t) = \boldsymbol{r}_0 + \boldsymbol{v}_0 \cdot t_k + \frac{1}{2}\boldsymbol{a}_0 \cdot t_k^2$$

여기서:
- $\boldsymbol{r}_0$: 기준시각 $t_0$에서의 위성 위치 벡터
- $\boldsymbol{v}_0$: 기준시각 $t_0$에서의 위성 속도 벡터
- $\boldsymbol{a}_0$: 기준시각 $t_0$에서의 위성 가속도 벡터
- $t_k = t - t_0$: 기준시각으로부터의 경과시간

**성분별 계산**:
$$x(t) = x_0 + \dot{x}_0 \cdot t_k + \frac{1}{2}\ddot{x}_0 \cdot t_k^2$$
$$y(t) = y_0 + \dot{y}_0 \cdot t_k + \frac{1}{2}\ddot{y}_0 \cdot t_k^2$$
$$z(t) = z_0 + \dot{z}_0 \cdot t_k + \frac{1}{2}\ddot{z}_0 \cdot t_k^2$$

**2단계: SBAS 1차 시계 모델**
$$\Delta t^s = a_{f0} + a_{f1} \cdot t_k$$

여기서:
- $a_{f0}$: 시계 바이어스 [s]
- $a_{f1}$: 시계 드리프트 [s/s]

**특징**: SBAS는 정지궤도 위성이므로 장기간 궤도 예측에 적합한 간단한 다항식 모델 사용

</details>

### 5.3 궤도력 데이터 관리 함수

### 5.3.1 SelectEph() - 최적 궤도력 선택
<details>
<summary>상세 설명</summary>

**목적**: 주어진 조건에 맞는 최적의 방송궤도력 데이터 선택

**입력**:
- `double ephtime`: 궤도력 시각 [s]
- `int sat`: 위성 인덱스
- `const nav_t *nav`: 항법 데이터
- `int iode`: IODE 값 (-1: 무시)

**출력**: 선택된 궤도력 포인터 (실패 시 NULL)

**함수 로직**:
```c
if (ephtime < 0.0 || !nav || sat <= 0) return NULL; // 입력 검증

// 1. 시스템별 TOE 허용 시간 설정
int sys = Sat2Prn(sat, NULL);
if (sys <= 0 || sys > NSYS) return NULL;

double maxdtoe;
switch (Sys2Str(sys)) {
    case STR_GPS: maxdtoe = MAX_DTOE_GPS; break;
    case STR_GLO: maxdtoe = MAX_DTOE_GLO; break;
    case STR_GAL: maxdtoe = MAX_DTOE_GAL; break;
    case STR_BDS: maxdtoe = MAX_DTOE_BDS; break;
    case STR_QZS: maxdtoe = MAX_DTOE_QZS; break;
    case STR_IRN: maxdtoe = MAX_DTOE_IRN; break;
    case STR_SBS: maxdtoe = MAX_DTOE_SBS; break;
    default: return NULL;
}

// 2. 위성별 궤도력 배열에서 최적 궤도력 탐색
int idx = -1;
double dttr = 0.0;

for (int i = 0; i < nav->ephs[sat-1].n; i++) {
    const eph_t *eph = nav->ephs[sat-1].eph + i;

    if (eph->sat != sat) continue;              // 위성 일치 확인
    if (iode >= 0 && eph->IODE != iode) continue; // IODE 일치 (옵션)
    if (eph->ttr > ephtime) continue;           // 미래 메시지 배제
    if (fabs(ephtime - eph->toe) > maxdtoe) continue; // TOE 범위 확인

    // Galileo 타입 특별 처리 (I/NAV vs F/NAV)
    if (Sys2Str(sys) == STR_GAL) {
        if (GetEphType(sys) == 1) {
            if (!(eph->data & (1<<8))) continue;  // F/NAV 확인
        } else {
            if (!(eph->data & (1<<9))) continue;  // I/NAV 확인
        }
    }

    // TTR 최근접 선택
    if (idx < 0 || fabs(eph->ttr - ephtime) < dttr) {
        idx = i;
        dttr = fabs(eph->ttr - ephtime);
    }
}

return (idx < 0) ? NULL : nav->ephs[sat-1].eph + idx; // 선택된 궤도력
```

</details>

### 5.3.2 TestEph() - 궤도력 유효성 검사
<details>
<summary>상세 설명</summary>

**목적**: 방송궤도력 데이터의 건강성 및 정확도 유효성 검사

**입력**: `const eph_t *eph` - 궤도력 데이터

**출력**: 유효 시 1, 무효 시 0

**함수 로직**:
```c
if (!eph) return 0;                             // NULL 포인터 검사

// 1. 위성 시스템 유효성 검사
int sys = Sat2Prn(eph->sat, NULL);
if (sys <= 0 || sys > NSYS) return 0;

// 2. 건강성 플래그 초기 검사
if (eph->svh == -1) return 0;

// 3. 시스템별 세부 검사
switch (Sys2Str(sys)) {
    case STR_GAL: {
        // SISA 정확도 검사
        double err = Idx2Sisa(eph->sva);
        if (err < 0.0 || err > MAX_ERR_EPH) return 0;

        // E1-B/E5a/E5b DVS/HS 비트 검사
        if (eph->svh & 0b111110111) return 0;
    } break;

    case STR_QZS: {
        // URA 정확도 검사
        double err = Idx2Ura(eph->sva);
        if (err < 0.0 || err > MAX_ERR_EPH) return 0;

        // QZSS 건강성 플래그 검사
        if (eph->svh & 0b101110) return 0;
    } break;

    default: {
        // 기타 시스템 URA 정확도 검사
        double err = Idx2Ura(eph->sva);
        if (err < 0.0 || err > MAX_ERR_EPH) return 0;

        // 일반 건강성 플래그 검사
        if (eph->svh) return 0;
    } break;
}

return 1;                                       // 유효한 궤도력
```

</details>

### 5.4 정확도 지수 변환 함수

### 5.4.1 Ura2Idx()/Idx2Ura() - URA 변환
<details>
<summary>상세 설명</summary>

**목적**: GPS/QZS 시스템의 URA(User Range Accuracy) 오차값↔인덱스 변환

**URA 변환 공식**: GPS ICD Table 20-I 표준
- **값→인덱스**: $\text{idx} = \min\{i : \text{err} \leq \text{URA\_ERR}[i]\}$
- **인덱스→값**: $\text{err} = \text{URA\_ERR}[\text{idx}]$

**함수 로직**:
```c
// Ura2Idx() - URA 값을 인덱스로 변환
int Ura2Idx(double ura) {
    for (int i = 0; i < NURA; i++) {
        if (ura <= URA_ERR[i]) return i;        // 최소 상한 인덱스 반환
    }
    return NURA - 1;                            // 최대값 초과 시 최대 인덱스
}

// Idx2Ura() - 인덱스를 URA 값으로 변환
double Idx2Ura(int idx) {
    if (idx < 0 || idx >= NURA) return 0.0;    // 범위 검증
    return URA_ERR[idx];                        // 직접 테이블 인덱싱
}
```

</details>

### 5.4.2 Sisa2Idx()/Idx2Sisa() - SISA 변환
<details>
<summary>상세 설명</summary>

**목적**: Galileo 시스템의 SISA(Signal-In-Space Accuracy) 오차값↔인덱스 변환

**SISA 변환 공식**: Galileo ICD 4구간 모델
- **0~49**: $\text{err} = \text{idx} \times 0.01$ (0.00~0.49 m)
- **50~74**: $\text{err} = 0.5 + (\text{idx}-50) \times 0.02$ (0.50~0.98 m)
- **75~99**: $\text{err} = 1.0 + (\text{idx}-75) \times 0.04$ (1.00~1.96 m)
- **100~125**: $\text{err} = 2.0 + (\text{idx}-100) \times 0.16$ (2.00~6.00 m)

**함수 로직**:
```c
// Sisa2Idx() - SISA 값을 인덱스로 변환
int Sisa2Idx(double err) {
    if (err < 0.0) return -1;                                   // 오류값 반환

    // 구간별 변환 (실제 구현에 따른 정확한 공식)
    if (err >= 0.0 && err <= 0.5) return (int)((err - 0.0) / 0.01) +  0;   // 0~50
    if (err >  0.5 && err <= 1.0) return (int)((err - 0.5) / 0.02) + 50;   // 51~75
    if (err >  1.0 && err <= 2.0) return (int)((err - 1.0) / 0.04) + 75;   // 76~100
    if (err >  2.0 && err <= 6.0) return (int)((err - 2.0) / 0.16) + 100;  // 101~125

    return -1;                                                  // 범위 초과 시 오류
}

// Idx2Sisa() - 인덱스를 SISA 값으로 변환
double Idx2Sisa(int sisa) {
    if (sisa < 0) return -1.0;                                  // 오류값 반환

    // 구간별 역변환 (실제 구현과 일치)
    if (sisa <= 49)  return (sisa - 0  ) * 0.01 + 0.0;         // 0~49
    if (sisa <= 74)  return (sisa - 50 ) * 0.02 + 0.5;         // 50~74
    if (sisa <= 99)  return (sisa - 75 ) * 0.04 + 1.0;         // 75~99
    if (sisa <= 125) return (sisa - 100) * 0.16 + 2.0;         // 100~125

    return -1.0;                                                // 범위 초과 시 오류
}
```

</details>

### 5.5 궤도력 타입 관리 함수

### 5.5.1 GetEphType() - 궤도력 타입 조회
<details>
<summary>상세 설명</summary>

**목적**: 지정된 GNSS 시스템의 방송궤도력 타입 조회

**입력**:
- `int sys`: GNSS 시스템 ID (1~7)

**출력**:
- `int`: 방송궤도력 타입 (시스템별 0~2), 오류 시 -1

**함수 로직**:
```c
if (sys < 1 || sys > NSYS) return -1;          // 시스템 ID 유효성 검사

return EPHTYPE[sys - 1];                       // EPHTYPE 배열에서 타입 조회 (0-based)
```

</details>

### 5.5.2 SetEphType() - 궤도력 타입 설정
<details>
<summary>상세 설명</summary>

**목적**: 지정된 GNSS 시스템의 방송궤도력 타입 설정

**입력**:
- `int sys`: GNSS 시스템 ID (1~7)
- `int type`: 방송궤도력 타입

**출력**:
- `void`: 반환값 없음

**함수 로직**:
```c
if (sys < 1 || sys > NSYS) return;             // 시스템 ID 유효성 검사

// 시스템별 타입 범위 검증
int max_type = 1;                               // 기본 최대 타입
if (sys == SYS_GAL) max_type = 2;               // Galileo는 타입 0~2

if (type < 0 || type > max_type) return;       // 타입 범위 검증

EPHTYPE[sys - 1] = type;                       // 유효한 타입만 저장 (0-based)
```

</details>

---

## 6. 사용 예시

### 6.1 궤도력 데이터 구조체 관리
```c
#include "ephemeris.h"

// 궤도력 데이터셋 초기화 및 사용
ephs_t ephs;
InitEphs(&ephs);  // 안전한 초기화

// 궤도력 데이터 추가
eph_t eph1 = {
    .sat = 1,      // GPS PRN 1
    .IODE = 100,
    .toe = 604800.0,
    .ttr = 604800.0,
    // ... 기타 궤도력 매개변수
};

eph_t eph2 = {
    .sat = 1,      // 같은 위성의 다른 시간 궤도력
    .IODE = 101,
    .toe = 608400.0,
    .ttr = 608400.0,
    // ... 기타 궤도력 매개변수
};

eph_t eph3 = {
    .sat = 2,      // GPS PRN 2
    .IODE = 50,
    .toe = 604800.0,
    .ttr = 604800.0,
    // ... 기타 궤도력 매개변수
};

// 궤도력 데이터 추가 (자동 메모리 확장)
if (AddEph(&ephs, &eph1)) printf("eph1 추가 성공\n");
if (AddEph(&ephs, &eph2)) printf("eph2 추가 성공\n");
if (AddEph(&ephs, &eph3)) printf("eph3 추가 성공\n");
if (AddEph(&ephs, &eph1)) printf("eph1 중복 추가\n");  // 중복

printf("추가된 궤도력 개수: %d\n", ephs.n);  // 4개

// 정렬 및 중복 제거
SortEphs(&ephs);
printf("정렬/중복제거 후 개수: %d\n", ephs.n);  // 3개

// 정렬 결과 확인
for (int i = 0; i < ephs.n; i++) {
    printf("궤도력 %d: 위성=%d, IODE=%d, TTR=%.0f\n",
           i, ephs.eph[i].sat, ephs.eph[i].IODE, ephs.eph[i].ttr);
}

// 메모리 해제
FreeEphs(&ephs);
```

### 6.2 통합 위성 위치 계산
```c
#include "ephemeris.h"
#include "matrix.h"

// 위성 위치/속도 및 시계 계산 예시
nav_t nav;        // 항법 데이터
mat_t *rs = Mat(1, 6, DOUBLE);   // 위성 위치/속도 [m, m/s]
mat_t *dts = Mat(1, 2, DOUBLE);  // 시계 바이어스/드리프트 [s, s/s]
double var;       // 위치/시계 분산 [m²]
eph_t eph;        // 사용된 궤도력

double ephtime = 604800.0;  // 궤도력 시각 [GPST]
double time = 604801.0;     // 신호 송신 시각 [GPST]
int sat = 1;                // GPS PRN 1

// 위성 위치/시계 계산
if (SatPosClkBrdc(ephtime, time, sat, &nav, -1, rs, dts, &var, &eph)) {
    printf("위성 위치: [%.3f, %.3f, %.3f] m\n",
           MatGetD(rs, 0, 0), MatGetD(rs, 0, 1), MatGetD(rs, 0, 2));
    printf("위성 속도: [%.3f, %.3f, %.3f] m/s\n",
           MatGetD(rs, 0, 3), MatGetD(rs, 0, 4), MatGetD(rs, 0, 5));
    printf("시계 바이어스: %.9f s\n", MatGetD(dts, 0, 0));
    printf("시계 드리프트: %.12f s/s\n", MatGetD(dts, 0, 1));
    printf("위치 분산: %.3f m²\n", var);
}

FreeMat(rs);
FreeMat(dts);
```

### 6.3 개별 함수 사용 예제

#### 6.3.1 SelectEph() 함수 사용 예제
```c
// 최적 궤도력 선택
nav_t nav;              // 항법 데이터 (궤도력 배열 포함)
double ephtime = 604800.0;
int sat = 1;            // GPS PRN 1
int iode = -1;          // IODE 무시

// 최적 궤도력 선택
eph_t *selected = SelectEph(ephtime, sat, &nav, iode);
if (selected) {
    printf("선택된 궤도력:\n");
    printf("  위성: %d\n", selected->sat);
    printf("  TOE: %.0f\n", selected->toe);
    printf("  TTR: %.0f\n", selected->ttr);
    printf("  IODE: %d\n", selected->IODE);
} else {
    printf("사용 가능한 궤도력이 없습니다\n");
}
```

#### 6.3.2 정확도 지수 변환 예제
```c
// URA 변환 (GPS/QZS)
double ura_values[] = {2.4, 4.85, 24.0, 96.0};
for (int i = 0; i < 4; i++) {
    int idx = Ura2Idx(ura_values[i]);
    double back = Idx2Ura(idx);
    printf("URA %.1f m → 인덱스 %d → %.1f m\n",
           ura_values[i], idx, back);
}

// SISA 변환 (Galileo)
double sisa_values[] = {0.25, 0.75, 1.5, 4.0};
for (int i = 0; i < 4; i++) {
    int idx = Sisa2Idx(sisa_values[i]);
    double back = Idx2Sisa(idx);
    printf("SISA %.2f m → 인덱스 %d → %.2f m\n",
           sisa_values[i], idx, back);
}
```

#### 6.3.3 궤도력 타입 관리 예제
```c
// 시스템별 타입 설정
printf("현재 타입 설정:\n");
printf("GPS: %d\n", GetEphType(1));
printf("Galileo: %d\n", GetEphType(3));

// Galileo를 F/NAV로 변경
SetEphType(3, 1);
printf("Galileo F/NAV 설정 후: %d\n", GetEphType(3));

// 잘못된 타입 설정 시도 (무시됨)
SetEphType(3, 5);  // 유효 범위 벗어남
printf("잘못된 타입 설정 후: %d\n", GetEphType(3)); // 여전히 1
```

### 6.4 궤도력 유효성 검사 및 선택
```c
// 궤도력 유효성 검사
eph_t ephemeris;  // 방송궤도력 데이터
if (TestEph(&ephemeris)) {
    printf("궤도력 데이터 유효\n");

    // 정확도 지수 확인
    int sys = Sat2Prn(ephemeris.sat, NULL);
    if (Sys2Str(sys) == STR_GAL) {
        double sisa = Idx2Sisa(ephemeris.sva);
        printf("Galileo SISA: %.3f m\n", sisa);
    } else {
        double ura = Idx2Ura(ephemeris.sva);
        printf("URA: %.3f m\n", ura);
    }
} else {
    printf("궤도력 데이터 무효\n");
}

// 최적 궤도력 선택
eph_t *selected = SelectEph(ephtime, sat, &nav, -1);
if (selected) {
    printf("최적 궤도력 선택됨: TOE=%.0f, TTR=%.0f\n",
           selected->toe, selected->ttr);
} else {
    printf("사용 가능한 궤도력 없음\n");
}
```

### 6.5 정확도 지수 변환
```c
// URA 변환 (GPS/QZS)
double ura_value = 2.4;  // URA 값 [m]
int ura_idx = Ura2Idx(ura_value);
printf("URA %.1f m → 인덱스 %d\n", ura_value, ura_idx);

double ura_back = Idx2Ura(ura_idx);
printf("인덱스 %d → URA %.1f m\n", ura_idx, ura_back);

// SISA 변환 (Galileo)
double sisa_value = 1.5;  // SISA 값 [m]
int sisa_idx = Sisa2Idx(sisa_value);
printf("SISA %.1f m → 인덱스 %d\n", sisa_value, sisa_idx);

double sisa_back = Idx2Sisa(sisa_idx);
printf("인덱스 %d → SISA %.2f m\n", sisa_idx, sisa_back);
```

### 6.6 시스템별 궤도력 타입 설정
```c
// 시스템별 방송궤도력 타입 설정
SetEphType(1, 0);  // GPS LNAV 설정
SetEphType(3, 1);  // Galileo F/NAV 설정
SetEphType(4, 0);  // BeiDou D1 설정

// 현재 설정된 타입 조회
int gpsEphType = GetEphType(1);     // 0 (LNAV)
int galEphType = GetEphType(3);     // 1 (F/NAV)
int bdsEphType = GetEphType(4);     // 0 (D1)

printf("GPS 궤도력 타입: %d\n", gpsEphType);
printf("Galileo 궤도력 타입: %d\n", galEphType);
printf("BeiDou 궤도력 타입: %d\n", bdsEphType);
```

### 6.7 전체 시스템 초기화
```c
// 모든 시스템을 기본 타입으로 초기화
for (int sys = 1; sys <= NSYS; sys++) {
    SetEphType(sys, 0);  // 모든 시스템을 기본 타입으로 설정
}

// 시스템별 타입 확인
printf("현재 궤도력 타입 설정:\n");
if (SYS_GPS) printf("GPS: %d (LNAV)\n", GetEphType(1));
if (SYS_GLO) printf("GLONASS: %d (LNAV)\n", GetEphType(2));
if (SYS_GAL) printf("Galileo: %d (%s)\n", GetEphType(3),
                    GetEphType(3) == 0 ? "I/NAV" : "F/NAV");
if (SYS_BDS) printf("BeiDou: %d (D1)\n", GetEphType(4));
if (SYS_QZS) printf("QZSS: %d (LNAV)\n", GetEphType(5));
if (SYS_IRN) printf("IRNSS: %d (LNAV)\n", GetEphType(6));
if (SYS_SBS) printf("SBAS: %d (RINEX)\n", GetEphType(7));
```

### 6.8 에러 처리 및 검증
```c
// 잘못된 입력 처리
SetEphType(0, 0);   // 무효한 시스템 - 무시됨
SetEphType(1, 5);   // 무효한 타입 - 무시됨
SetEphType(10, 0);  // 범위 초과 - 무시됨

// 안전한 타입 설정 함수 예시
void SetSafeEphType(int sys, int type) {
    int oldType = GetEphType(sys);
    SetEphType(sys, type);
    int newType = GetEphType(sys);

    if (oldType == newType && type != oldType) {
        printf("경고: 시스템 %d에 타입 %d 설정 실패\n", sys, type);
    } else {
        printf("시스템 %d 타입을 %d로 변경\n", sys, newType);
    }
}

// 사용 예시
SetSafeEphType(1, 0);  // GPS LNAV 설정
SetSafeEphType(3, 1);  // Galileo F/NAV 설정
SetSafeEphType(3, 3);  // 잘못된 타입 - 경고 출력
```

### 6.9 Galileo 다중 타입 처리
```c
// Galileo 시스템의 I/NAV와 F/NAV 전환
int galileo_sys = 3;

// I/NAV 모드 설정
SetEphType(galileo_sys, 0);
int current_type = GetEphType(galileo_sys);
printf("Galileo 모드: %s\n", current_type == 0 ? "I/NAV" : "F/NAV");

// F/NAV 모드로 전환
SetEphType(galileo_sys, 1);
current_type = GetEphType(galileo_sys);
printf("Galileo 모드: %s\n", current_type == 0 ? "I/NAV" : "F/NAV");

// 타입별 처리 분기
switch (GetEphType(galileo_sys)) {
    case 0:
        printf("I/NAV 궤도력 처리 로직\n");
        break;
    case 1:
        printf("F/NAV 궤도력 처리 로직\n");
        break;
    default:
        printf("알 수 없는 Galileo 궤도력 타입\n");
}
```

### 6.10 설정 백업 및 복원
```c
// 현재 설정 백업
int ephtype_backup[NSYS];
for (int sys = 1; sys <= NSYS; sys++) {
    ephtype_backup[sys-1] = GetEphType(sys);
}

// 임시 설정 변경
SetEphType(1, 0);  // GPS
SetEphType(3, 1);  // Galileo F/NAV

// 일부 처리 수행...

// 원래 설정 복원
for (int sys = 1; sys <= NSYS; sys++) {
    SetEphType(sys, ephtype_backup[sys-1]);
}

printf("궤도력 타입 설정이 복원되었습니다.\n");
```

---

## 7. 성능 특성

### 7.1 궤도 계산 정확도
- **Keplerian 모델**: GPS/GAL/BDS/QZS/IRN에서 수 미터 정확도
- **GLONASS 수치적분**: RK4 방법으로 1-2m 정확도 달성
- **SBAS 다항식**: 정지궤도 위성에 최적화된 간단 모델
- **상대론적 보정**: 모든 시스템에서 ns 수준 시계 정확도

### 7.2 연산 성능

| 함수 | 시간복잡도 | 주요 연산 | 메모리 사용 |
|------|------------|-----------|-------------|
| **InitEphs()** | O(1) | 포인터 초기화 | 없음 |
| **FreeEphs()** | O(1) | 메모리 해제 | 없음 |
| **AddEph()** | O(1) 평균 | 데이터 복사 + 간헐적 확장 | 2배 확장 |
| **SortEphs()** | O(n log n) | qsort + 중복제거 | 없음 |
| **ResizeEphs()** | O(n) | realloc + 데이터 복사 | 새 배열 |
| **SatPosClkBrdc()** | O(n) | 궤도력 선택 + 위성 계산 | 임시 배열 |
| **Eph2Pos()** | O(1) | 케플러 방정식 (최대 10회 반복) | 로컬 변수 |
| **GloEph2Pos()** | O(k) | RK4 적분 (k=시간간격/60초) | 상태벡터 |
| **SelectEph()** | O(n) | 궤도력 선형 검색 | 없음 |
| **URA/SISA 변환** | O(1)/O(15) | 직접 변환/선형 검색 | 정적 테이블 |

### 7.3 메모리 효율성
- **정적 테이블**: URA_ERR[15] = 120바이트 고정
- **최소 할당**: 대부분 스택 변수 사용
- **재사용 구조**: 궤도력 구조체 공유로 메모리 절약
- **타입 관리**: EPHTYPE[NSYS] = 28바이트 (최대)

### 7.4 수치적 안정성
- **케플러 방정식**: Newton-Raphson 수렴성 보장 (허용오차 1E-13)
- **GLONASS 적분**: 적응적 스텝으로 안정성 확보
- **특이점 처리**: 영벡터, 무한대 등 예외 상황 대응
- **정밀도 유지**: double 정밀도로 나노초 수준 시계 계산

### 7.5 시스템 호환성
- **표준 준수**: 모든 GNSS ICD 공식 표준 완벽 구현
- **크로스 플랫폼**: C 표준 라이브러리만 사용
- **확장 가능**: 새로운 GNSS 시스템 추가 용이
- **역호환성**: 기존 코드와 완벽 호환

### 7.6 실시간 성능
- **빠른 계산**: 일반적으로 1ms 이내 위성 위치 계산
- **예측 가능**: 최대 반복 횟수 제한으로 실행시간 보장
- **배치 처리**: 다중 위성 동시 계산 지원
- **메모리 안전**: 동적 할당 최소화로 실시간 적합

---

**이 모듈은 모든 GNSS 시스템의 방송궤도력을 이용한 정밀한 위성 위치 및 시계 계산을 제공합니다.**
