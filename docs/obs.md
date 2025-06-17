---
layout: default
title: GNSS 관측 데이터 처리 모듈 (obs)
---

# GNSS 관측 데이터 처리 모듈 (obs)

GNSS 관측 데이터를 종합적으로 처리하는 핵심 모듈로, 관측 코드 변환부터 주파수 정보 추출까지 GNSS 측위에 필요한 모든 관측 데이터 처리 기능을 제공합니다.

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

### 1.1 GNSS 관측 데이터 구조
obs 모듈은 다음과 같은 관측 데이터를 종합적으로 처리합니다:

**핵심 관측 타입**:
- **의사거리 (Pseudorange)**: 위성-수신기 간 거리 측정
- **반송파 위상 (Carrier Phase)**: 고정밀 거리 측정
- **도플러 주파수 (Doppler)**: 위성-수신기 상대속도
- **신호강도 (SNR)**: 신호 품질 지표

### 1.2 GNSS 관측 코드 구조
GNSS 관측 코드는 **LXX** 형식으로 구성됩니다:
- **L**: 반송파 신호 식별자
- **첫 번째 X**: 주파수 밴드 (1~9)
- **두 번째 X**: 채널 타입 (I, Q, C, S, L, X, D, P, A, B, C, E, Z 등)

### 1.3 RINEX 밴드 번호 체계
**중요**: RINEX에서 밴드 번호는 **시스템별로 독립적**으로 할당됩니다. 같은 밴드 번호라도 시스템에 따라 다른 주파수를 의미할 수 있습니다.

### 1.4 주파수 인덱스 매핑 철학
시스템별로 주파수 밴드를 논리적 인덱스(Fidx)로 매핑하여 배열 기반 효율적 접근을 제공합니다:
- **연속성**: 각 시스템 내에서 1부터 연속된 인덱스 할당
- **독립성**: 시스템 간 인덱스 값은 독립적으로 관리
- **확장성**: 새로운 주파수 추가 시 인덱스 확장 가능

주파수 인덱스 매핑의 수학적 표현:

$$\text{Fidx} = f(\text{code}, \text{sys})$$

여기서:
- $\text{Fidx}$: 주파수 인덱스 (1~N, 시스템별 가변)
- $\text{code}$: 관측 코드 ID
- $\text{sys}$: GNSS 시스템 ID

실제 주파수 계산:

$$f_{\text{carrier}} = \text{BaseFreq}(\text{band}, \text{sys}) + \Delta f(\text{FCN})$$

여기서:
- $f_{\text{carrier}}$: 실제 반송파 주파수
- $\text{BaseFreq}$: 시스템별 기준 주파수
- $\Delta f(\text{FCN})$: GLONASS 주파수 채널 번호 보정

---

## 2. 데이터 타입 구조

```
obs 모듈 데이터 계층
├── 관측 코드 체계
│   ├── 문자열 형태 ───────────── "LXX" (L+밴드+채널)
│   ├── 숫자 코드 ID ──────────── int 타입 고유 식별자
│   └── 밴드 ID ───────────────── 주파수 밴드 번호 (1~9)
├── 주파수 정보 구조
│   ├── 주파수 인덱스 (Fidx) ──── 시스템별 논리적 인덱스 (1~N)
│   ├── 실제 주파수 ────────────── double 타입 (MHz)
│   └── FCN 보정 ──────────────── GLONASS 주파수 채널 번호
├── 시스템별 매핑 테이블
│   ├── 코드→밴드 매핑 ─────────── 관측 코드에서 밴드 추출
│   ├── 코드→Fidx 매핑 ────────── 시스템별 주파수 인덱스
│   └── 밴드→주파수 매핑 ────────── 실제 주파수 계산
└── 채널 타입 분류
    ├── 공통 타입 ─────────────── I, Q, C, P, X
    ├── GPS 특화 ──────────────── S, L, W, N
    ├── Galileo/BDS 특화 ───────── D, P
    └── 기타 타입 ─────────────── A, B, E, Z
```

---

## 3. 데이터 타입 목록

### 3.1 관측 코드 구조
<details>
<summary>상세 설명</summary>

**목적**: GNSS 관측 신호의 표준화된 식별 체계

**정의**: `"LXX"` (L + 밴드번호 + 채널타입)

**값**:
- **L**: 반송파 신호 식별자 (고정)
- **밴드번호**: 1~9 (RINEX 표준)
- **채널타입**: I,Q,C,S,L,X,D,P,A,B,C,E,Z 등

**사용**: RINEX 표준 완전 호환, 모든 GNSS 시스템 지원

</details>

### 3.2 주파수 인덱스 (Fidx)
<details>
<summary>상세 설명</summary>

**목적**: 시스템별 주파수 밴드의 논리적 인덱스

**정의**: `int` (1~N, 시스템별 가변)

**값**:
- GPS: 1~5
- GLONASS: 1~5
- Galileo: 1~5
- BeiDou: 1~6
- QZSS: 1~6
- IRNSS: 1~2
- SBAS: 1~2

**사용**: 주파수별 관측 데이터 배열 인덱싱

</details>

### 3.3 채널 타입 분류
<details>
<summary>상세 설명</summary>

**목적**: 신호 내 세부 채널 구분

**정의**: 시스템별 채널 타입 문자

**값**:

#### 공통 채널 타입
| 타입 | 의미 | 설명 |
|------|------|------|
| **I** | In-phase | 동위상 성분 (주로 데이터) |
| **Q** | Quadrature | 직교위상 성분 (주로 파일럿) |
| **X** | Combined | 두 개 이상 채널 조합 |
| **C** | C/A | Coarse/Acquisition 코드 |
| **P** | Precision | 정밀 코드 |

#### GPS 특화 타입
| 타입 | 의미 | 설명 |
|------|------|------|
| **S** | L2C(M)/L1C(D) | Medium 또는 Data 성분 |
| **L** | L2C(L)/L1C(P) | Long 또는 Pilot 성분 |
| **W** | Z-tracking | Z-추적 방식 |
| **N** | Codeless | 코드 없는 추적 |

#### Galileo/BDS 특화 타입
| 타입 | 의미 | 설명 |
|------|------|------|
| **D** | Data | 데이터 성분 |
| **P** | Pilot | 파일럿 성분 |

#### 기타 타입
| 타입 | 의미 | 설명 |
|------|------|------|
| **A** | A-component | 시스템별 A 성분 |
| **B** | B-component | 시스템별 B 성분 |
| **E** | E-component | 시스템별 E 성분 |
| **Z** | Combined Multiple | 여러 성분 조합 |

**사용**: 시스템별 채널 타입에 따른 신호 처리 분기

</details>

### 3.4 주파수 정보 구조
<details>
<summary>상세 설명</summary>

**목적**: 관측 코드에서 실제 주파수 정보 추출

**정의**: 밴드 ID → 주파수 인덱스 → 실제 주파수 변환 체계

**값**:
- **밴드 ID**: RINEX 밴드 번호 (1~9)
- **주파수 인덱스**: 시스템별 논리 인덱스
- **실제 주파수**: Hz 단위 double 값
- **FCN 보정**: GLONASS 주파수 채널 보정

**사용**: 관측코드 → 밴드ID → 주파수인덱스 → 실제주파수 순차 변환

</details>

---

## 4. 함수 구조

```
obs 모듈 함수 계층
├── 관측 데이터 구조체 관리
│   ├── InitObss() ────────────── 관측 데이터셋 초기화
│   ├── FreeObss() ────────────── 메모리 해제
│   ├── AddObs() ──────────────── 관측 데이터 추가
│   └── SortObss() ────────────── 정렬 및 중복 제거
├── 관측 코드 변환
│   ├── Str2Code() ────────────── 문자열 → 코드 ID
│   ├── Code2Str() ────────────── 코드 ID → 문자열
│   ├── Code2Band() ───────────── 코드 ID → 밴드 ID
│   ├── Code2Freq() ───────────── 위성 + 코드 ID → 주파수
│   └── Code2Fidx() ───────────── 시스템 + 코드 ID → 주파수 인덱스
├── 밴드 관리
│   ├── Band2Str() ────────────── 밴드 ID → 밴드 문자
│   ├── Str2Band() ────────────── 밴드 문자 → 밴드 ID
│   ├── Band2Freq() ───────────── 위성 + 밴드 ID → 주파수
│   └── Fidx2Band() ───────────── 시스템 + 인덱스 → 밴드 ID
└── 시스템별 내부 함수 (static)
    ├── 주파수 인덱스 매핑
    │   ├── Code2Fidx_GPS() ───── GPS 주파수 인덱스 (static)
    │   ├── Code2Fidx_GLO() ───── GLONASS 주파수 인덱스 (static)
    │   ├── Code2Fidx_GAL() ───── Galileo 주파수 인덱스 (static)
    │   ├── Code2Fidx_BDS() ───── BeiDou 주파수 인덱스 (static)
    │   ├── Code2Fidx_QZS() ───── QZSS 주파수 인덱스 (static)
    │   ├── Code2Fidx_IRN() ───── IRNSS 주파수 인덱스 (static)
    │   └── Code2Fidx_SBS() ───── SBAS 주파수 인덱스 (static)
    └── 주파수 변환
        ├── Band2Freq_GPS() ───── GPS 주파수 변환 (static)
        ├── Band2Freq_GLO() ───── GLONASS 주파수 변환 (FCN 적용) (static)
        ├── Band2Freq_GAL() ───── Galileo 주파수 변환 (static)
        ├── Band2Freq_BDS() ───── BeiDou 주파수 변환 (static)
        ├── Band2Freq_QZS() ───── QZSS 주파수 변환 (static)
        ├── Band2Freq_IRN() ───── IRNSS 주파수 변환 (static)
        └── Band2Freq_SBS() ───── SBAS 주파수 변환 (static)
```

---

## 5. 함수 목록

### 5.1 관측 데이터 구조체 관리 함수

#### 5.1.1 InitObss() - 관측 데이터셋 초기화
<details>
<summary>상세 설명</summary>

**목적**: 관측 데이터셋 구조체를 초기 상태로 설정

**입력**:
- `obss_t *obss`: 관측 데이터셋 구조체 포인터

**출력**:
- `void`: 반환값 없음

**함수 로직**:
```c
if (obss == NULL) return;     // 포인터 유효성 검증
obss->n = 0;                  // 현재 관측 수 초기화
obss->nmax = 0;               // 최대 크기 초기화
obss->obs = NULL;             // 관측 배열 포인터 초기화
```

**사용 예시**:
```c
obss_t obsSet;
InitObss(&obsSet);
// obsSet.n = 0, obsSet.nmax = 0, obsSet.obs = NULL
```

</details>

#### 5.1.2 FreeObss() - 메모리 해제
<details>
<summary>상세 설명</summary>

**목적**: 관측 데이터셋에 할당된 메모리를 안전하게 해제

**입력**:
- `obss_t *obss`: 관측 데이터셋 구조체 포인터

**출력**:
- `void`: 반환값 없음

**함수 로직**:
```c
if (obss == NULL) return;     // 포인터 유효성 검증
if (obss->obs) {              // 관측 배열이 할당되어 있으면
    free(obss->obs);          // 메모리 해제
}
obss->n = 0;                  // 관측 수 초기화
obss->nmax = 0;               // 최대 크기 초기화
obss->obs = NULL;             // 포인터 초기화
```

**사용 예시**:
```c
obss_t obsSet;
InitObss(&obsSet);
// ... 관측 데이터 추가 작업들 ...
FreeObss(&obsSet);            // 메모리 안전 해제
```

</details>

#### 5.1.3 AddObs() - 관측 데이터 추가
<details>
<summary>상세 설명</summary>

**목적**: 관측 데이터셋에 새로운 관측 데이터를 추가

**입력**:
- `obss_t *obss`: 관측 데이터셋 구조체 포인터
- `const obs_t *obs`: 추가할 관측 데이터

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:
```c
if (!obss || !obs) return 0;      // 입력 유효성 검증

if (obss->n >= obss->nmax) {       // 배열 크기 확인
    int nmax_new = obss->nmax == 0 ? 2 : obss->nmax * 2;
    obs_t *obs_new = (obs_t*)realloc(obss->obs,
                                     sizeof(obs_t) * nmax_new);
    if (!obs_new) return 0;        // 메모리 할당 실패
    obss->obs = obs_new;
    obss->nmax = nmax_new;
}

obss->obs[obss->n] = *obs;         // 관측 데이터 복사
obss->n++;                         // 카운터 증가
return 1;                          // 성공
```

**사용 예시**:
```c
obss_t obsSet;
InitObss(&obsSet);

obs_t newObs = {0};
newObs.time = 123456.789;
newObs.sat = 5;
newObs.rcv = 1;

if (AddObs(&obsSet, &newObs)) {
    printf("관측 데이터 추가 성공: 총 %d개\n", obsSet.n);
}
```

</details>

#### 5.1.4 SortObss() - 정렬 및 중복 제거
<details>
<summary>상세 설명</summary>

**목적**: 관측 데이터를 시간, 수신기, 위성 순으로 정렬하고 중복 제거

**입력**:
- `obss_t *obss`: 관측 데이터셋 구조체 포인터

**출력**:
- `void`: 반환값 없음

**함수 로직**:
```c
if (!obss || obss->n == 0) return;             // 유효성 검증

// qsort로 정렬 (비교함수: time→rcv→sat 순)
qsort(obss->obs, obss->n, sizeof(obs_t), CompareObs);

// 중복 제거 (정확히 같은 time, rcv, sat 인 경우)
int n = 0;
for (int i = 0; i < obss->n; i++) {
    // 이전 관측과 중복인지 확인
    if (i > 0 &&
        obss->obs[i].time == obss->obs[i-1].time &&
        obss->obs[i].rcv  == obss->obs[i-1].rcv  &&
        obss->obs[i].sat  == obss->obs[i-1].sat) {
        continue;  // 중복이면 건너뜀
    }
    // 중복이 아니면 유지
    if (n != i) {
        obss->obs[n] = obss->obs[i];
    }
    n++;
}
obss->n = n;                                    // 압축된 크기로 업데이트
```

**사용 예시**:
```c
obss_t obsSet;
InitObss(&obsSet);

// 여러 관측 데이터 추가 (순서 무관)
obs_t obs1 = {.time = 100.0, .rcv = 1, .sat = 5};
obs_t obs2 = {.time = 50.0, .rcv = 1, .sat = 3};
obs_t obs3 = {.time = 100.0, .rcv = 1, .sat = 5};  // 중복

AddObs(&obsSet, &obs1);
AddObs(&obsSet, &obs2);
AddObs(&obsSet, &obs3);                         // 중복 데이터

printf("정렬 전: %d개\n", obsSet.n);            // "정렬 전: 3개"
SortObss(&obsSet);                              // 정렬 + 중복 제거
printf("정렬 후: %d개\n", obsSet.n);            // "정렬 후: 2개"
```

</details>

### 5.2 관측 코드 변환 함수

#### 5.2.1 Str2Code() - 문자열→코드ID
<details>
<summary>상세 설명</summary>

**목적**: 관측 코드 문자열을 고유 코드 ID로 변환

**입력**:
- `char *str`: 관측 코드 문자열 ("LXX" 형식)

**출력**:
- `int`: 코드 ID (고유 식별자), 오류 시 0

**함수 로직**:
```c
// OBSCODES 테이블에서 문자열 비교로 탐색
for (int i = 1; i <= NCODE; i++) {
    if (!strcmp(codeStr.str, OBSCODES[i])) {
        return i;                               // 매칭되는 코드 ID 반환
    }
}
return 0;                                       // 미발견 시 0 반환
```

**사용 예시**:
```c
codeStr_t l1c = {"L1C"};
codeStr_t l5i = {"L5I"};
codeStr_t invalid = {"ABC"};

int code1 = Str2Code(l1c);      // 정상: 고유 코드 ID
int code2 = Str2Code(l5i);      // 정상: 고유 코드 ID
int code3 = Str2Code(invalid);  // 오류: 0 반환
```

</details>

#### 5.2.2 Code2Str() - 코드ID→문자열
<details>
<summary>상세 설명</summary>

**목적**: 코드 ID를 관측 코드 문자열 구조체로 역변환

**입력**:
- `int code`: 코드 ID

**출력**:
- `codeStr_t`: 관측 코드 문자열 구조체, 오류 시 빈 구조체

**함수 로직**:
```c
codeStr_t result = {{0}};                       // 빈 구조체 초기화

if (code <= 0 || code >= NCODE) {               // 범위 검증
    return result;                              // 빈 구조체 반환
}

if (OBSCODES[code] && strlen(OBSCODES[code]) <= CODE_STR_SIZE-1) {
    strcpy(result.str, OBSCODES[code]);         // 문자열 복사
}
return result;                                  // 구조체 반환
```

**사용 예시**:
```c
int code = Str2Code((codeStr_t){"L1C"});        // 코드 ID 얻기
codeStr_t codeStr = Code2Str(code);             // 문자열로 역변환

if (strlen(codeStr.str) > 0) {
    printf("관측 코드: %s\n", codeStr.str);     // "L1C" 출력
} else {
    printf("유효하지 않은 코드 ID\n");
}
```

</details>

#### 5.2.3 Code2Band() - 코드ID→밴드ID
<details>
<summary>상세 설명</summary>

**목적**: 관측 코드에서 주파수 밴드 번호 추출

**입력**:
- `int code`: 코드 ID

**출력**:
- `int`: 밴드 ID (1~9), 오류 시 0

**함수 로직**:
```c
// 코드 유효성 검증
if (code <= 0 || code > NCODE) return 0;

// OBSCODES 배열에서 직접 접근하여 밴드 문자 추출
return Str2Band(OBSCODES[code][1]);            // 두 번째 문자를 밴드로 변환
```

**사용 예시**:
```c
int l1c_code = Str2Code((codeStr_t){"L1C"});   // L1C 코드 ID
int l5i_code = Str2Code((codeStr_t){"L5I"});   // L5I 코드 ID

int band1 = Code2Band(l1c_code);               // 1 (L1 밴드)
int band5 = Code2Band(l5i_code);               // 5 (L5 밴드)

printf("L1C 밴드: %d\n", band1);               // "L1C 밴드: 1"
printf("L5I 밴드: %d\n", band5);               // "L5I 밴드: 5"
```

</details>

#### 5.2.4 Code2Freq() - 위성+코드ID→주파수
<details>
<summary>상세 설명</summary>

**목적**: 위성 정보와 관측 코드에서 실제 주파수 계산

**입력**:
- `int sat`: 위성 인덱스
- `int code`: 코드 ID

**출력**:
- `double`: 주파수 (Hz), 오류 시 0.0

**함수 로직**:
```c
// 위성 인덱스 유효성 검증
if (sat <= 0 || sat > NSAT) return 0.0;

// 코드 인덱스 유효성 검증
if (code <= 0 || code > NCODE) return 0.0;

// 코드에서 밴드 추출
int band = Code2Band(code);

// 위성+밴드로 주파수 계산
return Band2Freq(sat, band);                    // Hz 단위 주파수 반환
```

**사용 예시**:
```c
int gps_sat = 5;                                // GPS 위성 인덱스
int l1c_code = Str2Code((codeStr_t){"L1C"});    // L1C 코드
int l5i_code = Str2Code((codeStr_t){"L5I"});    // L5I 코드

double l1_freq = Code2Freq(gps_sat, l1c_code); // 1575420000.0 Hz
double l5_freq = Code2Freq(gps_sat, l5i_code); // 1176450000.0 Hz

printf("GPS L1C: %.0f Hz\n", l1_freq);
printf("GPS L5I: %.0f Hz\n", l5_freq);
```

</details>

#### 5.2.5 Code2Fidx() - 시스템+코드ID→주파수인덱스
<details>
<summary>상세 설명</summary>

**목적**: 시스템과 관측 코드를 주파수 인덱스로 변환

**입력**:
- `int sys`: 시스템 ID (1~7)
- `int code`: 코드 ID

**출력**:
- `int`: 주파수 인덱스 (1~N), 오류 시 0

**함수 로직**:
```c
// 시스템을 문자로 변환 후 분기
switch (Sys2Str(sys)) {
    case STR_GPS: return Code2Fidx_GPS(code);   // GPS 매핑 테이블
    case STR_GLO: return Code2Fidx_GLO(code);   // GLONASS 매핑 테이블
    case STR_GAL: return Code2Fidx_GAL(code);   // Galileo 매핑 테이블
    case STR_BDS: return Code2Fidx_BDS(code);   // BeiDou 매핑 테이블
    case STR_QZS: return Code2Fidx_QZS(code);   // QZSS 매핑 테이블
    case STR_IRN: return Code2Fidx_IRN(code);   // IRNSS 매핑 테이블
    case STR_SBS: return Code2Fidx_SBS(code);   // SBAS 매핑 테이블
    default: return 0;                          // 미지원 시스템
}
```

**사용 예시**:
```c
int l5i_code = Str2Code((codeStr_t){"L5I"});    // L5I 코드 ID

int gps_fidx = Code2Fidx(SYS_GPS, l5i_code);   // GPS: 3
int gal_fidx = Code2Fidx(SYS_GAL, l5i_code);   // Galileo: 2
int bds_fidx = Code2Fidx(SYS_BDS, l5i_code);   // BeiDou: 5

printf("GPS L5I Fidx: %d\n", gps_fidx);        // 3
printf("Galileo L5I Fidx: %d\n", gal_fidx);    // 2
printf("BeiDou L5I Fidx: %d\n", bds_fidx);     // 5

// 주파수별 관측 배열 인덱싱에 활용
double gps_obs[6] = {0};
if (gps_fidx > 0) gps_obs[gps_fidx] = 123.456;
```

</details>

### 5.3 밴드 관리 함수

#### 5.3.1 Band2Str() - 밴드ID→밴드문자
<details>
<summary>상세 설명</summary>

**목적**: 밴드 ID를 밴드 문자로 변환

**입력**:
- `int band`: 밴드 ID (1~9)

**출력**:
- `char`: 밴드 문자 ('1'~'9'), 오류 시 '\0'

**함수 로직**:
```c
if (band < 1 || band > 9) return '\0';         // 범위 검증
return (char)(band + '0');                     // 숫자→문자 변환
```

**사용 예시**:
```c
char band1 = Band2Str(1);                     // '1'
char band5 = Band2Str(5);                     // '5'
char invalid = Band2Str(0);                   // '\0' (오류)
```

</details>

#### 5.3.2 Str2Band() - 밴드문자→밴드ID
<details>
<summary>상세 설명</summary>

**목적**: 밴드 문자를 밴드 ID로 변환

**입력**:
- `char str`: 밴드 문자 ('1'~'9')

**출력**:
- `int`: 밴드 ID (1~9), 오류 시 0

**함수 로직**:
```c
if (str < '1' || str > '9') return 0;          // 범위 검증
return (int)(str - '0');                       // 문자→숫자 변환
```

**사용 예시**:
```c
int band1 = Str2Band('1');                    // 1
int band5 = Str2Band('5');                    // 5
int invalid = Str2Band('A');                  // 0 (오류)
```

</details>

#### 5.3.3 Band2Freq() - 위성+밴드ID→주파수
<details>
<summary>상세 설명</summary>

**목적**: 위성 정보와 밴드 ID에서 실제 주파수 계산

**입력**:
- `int sat`: 위성 인덱스
- `int band`: 밴드 ID (1~9)

**출력**:
- `double`: 주파수 (Hz), 오류 시 0.0

**함수 로직**:
```c
// 위성 인덱스를 시스템과 PRN으로 변환
int sys, prn;
if ((sys = Sat2Prn(sat, &prn)) == 0) return 0.0;

// 밴드 유효성 검증
if (band <= 0 || band > NBAND) return 0.0;

// 시스템을 문자로 변환하여 분기
switch (Sys2Str(sys)) {
    case STR_GPS: return Band2Freq_GPS(band);
    case STR_GLO: return Band2Freq_GLO(band, prn);  // FCN 자동적용
    case STR_GAL: return Band2Freq_GAL(band);
    case STR_BDS: return Band2Freq_BDS(band);
    case STR_QZS: return Band2Freq_QZS(band);
    case STR_IRN: return Band2Freq_IRN(band);
    case STR_SBS: return Band2Freq_SBS(band);
    default: return 0.0;
}
```

**사용 예시**:
```c
int gps_sat = 5;                                // GPS 위성 인덱스
int glo_sat = 65;                               // GLONASS 위성 인덱스

double gps_l1 = Band2Freq(gps_sat, 1);         // GPS L1: 1575420000.0 Hz
double gps_l5 = Band2Freq(gps_sat, 5);         // GPS L5: 1176450000.0 Hz
double glo_l1 = Band2Freq(glo_sat, 1);         // GLO L1: FCN 적용된 주파수

printf("GPS L1: %.0f Hz\n", gps_l1);
printf("GPS L5: %.0f Hz\n", gps_l5);
printf("GLONASS L1: %.0f Hz\n", glo_l1);       // FCN에 따라 가변
```

</details>

#### 5.3.4 Fidx2Band() - 시스템+인덱스→밴드ID
<details>
<summary>상세 설명</summary>

**목적**: 시스템별 주파수 인덱스($f_{idx}$)를 해당 시스템의 밴드 ID($b$)로 변환

**입력**:
- `int sys`: 시스템 ID (1~7, GPS/GLONASS/Galileo/BeiDou/QZSS/IRNSS/SBAS)
- `int fidx`: 주파수 인덱스 (1~N, 시스템별 가변)

**출력**:
- `int`: 밴드 ID (1~9), 오류 시 0

**함수 로직**:
1. 시스템별 FMAP_XXX 테이블에서 fidx-1 위치의 band 값을 반환
2. 유효성 검사(범위 초과 시 0 반환)

**수식**:
$$
b = \text{FMAP}_{sys}[f_{idx}-1].\text{band}
$$

**예시**:
- GPS, fidx=3 → band=5 (L5)
- BeiDou, fidx=2 → band=6 (B3)

**사용**: 주파수 인덱스 기반 배열에서 실제 밴드 번호로 변환할 때 활용

</details>

### 5.4 시스템별 주파수 매핑

#### GPS 시스템 주파수 매핑
| Fidx | 밴드 | 주파수 (MHz) | 관측 코드 예시 | 특징 |
|------|------|-------------|---------------|------|
| 1    | 1    | 1575.42     | L1C           | Legacy C/A |
| 2    | 2    | 1227.60     | L2W           | Legacy P(Y) |
| 3    | 5    | 1176.45     | L5I, L5Q, L5X | L5 |
| 4    | 1    | 1575.42     | L1S, L1L, L1X | Modern L1C |
| 5    | 2    | 1227.60     | L2S, L2L, L2X | Modern L2C |

#### GLONASS 시스템 주파수 매핑
| Fidx | 밴드 | 기준 주파수 (MHz) | FCN 공식 | 관측 코드 예시 |
|------|------|------------------|----------|----------------|
| 1    | 1    | 1602.0           | +k×0.5625| L1C            |
| 2    | 2    | 1246.0           | +k×0.4375| L2C            |
| 3    | 3    | 1202.025         | CDMA     | L3I, L3Q, L3X  |
| 4    | 4    | 1600.995         | CDMA     | L4A, L4B, L4X  |
| 5    | 6    | 1248.06          | CDMA     | L6A, L6B, L6X  |

#### Galileo 시스템 주파수 매핑 (F/NAV)
| Fidx | 밴드 | 주파수 (MHz) | 관측 코드 예시 | 서비스 |
|------|------|-------------|---------------|---------|
| 1    | 1    | 1575.42     | L1B, L1C, L1X | E1 (OS/CS) |
| 2    | 5    | 1176.45     | L5I, L5Q, L5X | E5a (OS)  |
| 3    | 7    | 1207.14     | L7I, L7Q, L7X | E5b (OS)  |
| 4    | 6    | 1278.75     | L6B, L6C, L6X | E6 (CS/PRS) |
| 5    | 8    | 1191.795    | L8I, L8Q, L8X | E5ab (OS) |

#### Galileo 시스템 주파수 매핑 (I/NAV)
| Fidx | 밴드 | 주파수 (MHz) | 관측 코드 예시 | 서비스 |
|------|------|-------------|---------------|---------|
| 1    | 1    | 1575.42     | L1B, L1C, L1X | E1 (OS/CS) |
| 2    | 7    | 1207.14     | L7I, L7Q, L7X | E5b (OS)  |
| 3    | 5    | 1176.45     | L5I, L5Q, L5X | E5a (OS)  |
| 4    | 6    | 1278.75     | L6B, L6C, L6X | E6 (CS/PRS) |
| 5    | 8    | 1191.795    | L8I, L8Q, L8X | E5ab (OS) |

> ※ Galileo의 F/NAV, I/NAV 모드는 ephemeris type에 따라 자동 선택됨

#### BeiDou(BDS) 시스템 주파수 매핑
| Fidx | 밴드 | 주파수 (MHz) | 관측 코드 예시 | 설명 |
|------|------|-------------|---------------|------|
| 1    | 2    | 1561.098    | L2I           | B1 (BDS-2/3) |
| 2    | 6    | 1268.52     | L6I           | B3 (BDS-2/3) |
| 3    | 7    | 1207.14     | L7I           | B2 (BDS-2/3) |
| 4    | 1    | 1575.42     | L1D, L1P, L1X | B1C (BDS-3)  |
| 5    | 5    | 1176.45     | L5D, L5P, L5X | B2a (BDS-3)  |
| 6    | 7    | 1207.14     | L7D, L7P, L7Z | B2b (BDS-3)  |
| 7    | 8    | 1191.795    | L8D, L8P, L8X | B2ab (BDS-3) |

#### QZSS 시스템 주파수 매핑
| Fidx | 밴드 | 주파수 (MHz) | 관측 코드 예시 | 설명 |
|------|------|-------------|---------------|------|
| 1    | 1    | 1575.42     | L1C           | L1C/A (Legacy) |
| 2    | 2    | 1227.60     | L2S, L2L, L2X | L2C            |
| 3    | 5    | 1176.45     | L5I, L5Q, L5X | L5              |
| 4    | 6    | 1278.75     | L6S, L6L, L6E, L6X | L6 (LEX)  |
| 5    | 1    | 1575.42     | L1S, L1L, L1X | L1C (Modern)   |
| 6    | 5    | 1176.45     | L5D, L5P, L5Z | L5S (SBAS)     |

#### IRNSS 시스템 주파수 매핑
| Fidx | 밴드 | 주파수 (MHz) | 관측 코드 예시 | 설명 |
|------|------|-------------|---------------|------|
| 1    | 5    | 1176.45     | L5B, L5C, L5X | L5 |
| 2    | 9    | 2492.028    | L9B, L9C, L9X | S-band |

#### SBAS 시스템 주파수 매핑
| Fidx | 밴드 | 주파수 (MHz) | 관측 코드 예시 | 설명 |
|------|------|-------------|---------------|------|
| 1    | 1    | 1575.42     | L1C           | L1 SBAS |
| 2    | 5    | 1176.45     | L5I, L5Q, L5X | DFMC |

---

## 6. 사용 예시

### 6.1 관측 데이터 구조체 관리
```c
// 관측 데이터셋 초기화
obss_t obss;
InitObss(&obss);

// 관측 데이터 생성 및 추가
obs_t obs = {0};
obs.time = 123456.789;        // GPST 시간
obs.rcv = 1;                  // 수신기 1
obs.sat = 5;                  // GPS PRN 5
obs.code[0] = Str2Code((codeStr_t){"L1C"});  // L1C 코드
obs.P[0] = 23456789.123;      // 의사거리 [m]
obs.L[0] = 123456789.456;     // 반송파 위상 [m]
obs.D[0] = -1234.56;          // 도플러 주파수 [Hz]
obs.SNR[0] = 45.2;            // 신호강도 [dB-Hz]

// 관측 데이터 추가
if (AddObs(&obss, &obs)) {
    printf("관측 데이터 추가 성공\n");
}

// 여러 관측 데이터 추가 후 정렬
SortObss(&obss);
printf("총 %d개 관측 데이터 정렬 완료\n", obss.n);

// 메모리 해제
FreeObss(&obss);
```

---

### 6.2 기본 관측 코드 변환
```c
// 문자열을 코드 ID로 변환
codeStr_t l1c_str = {"L1C"};
codeStr_t l5i_str = {"L5I"};
int code_l1c = Str2Code(l1c_str);    // GPS L1 C/A 코드
int code_l5i = Str2Code(l5i_str);    // L5 In-phase 코드

// 코드 ID를 문자열로 역변환
codeStr_t result = Code2Str(code_l1c);    // "L1C"
printf("코드 문자열: %s\n", result.str);

// 코드에서 밴드 번호 추출
int band = Code2Band(code_l5i);    // 5 (L5 밴드)
printf("밴드 번호: %d\n", band);
```

### 6.3 주파수 정보 추출
```c
// 관측 코드 준비
codeStr_t l1c_str = {"L1C"};
int l1c_code = Str2Code(l1c_str);            // L1C 코드 ID

// GPS 위성의 L1C 주파수 계산 (가정: GPS 위성 인덱스 5)
int gps_sat = 5;                              // GPS 위성 인덱스
double freq = Code2Freq(gps_sat, l1c_code);  // 1575420000.0 Hz

printf("GPS 위성 L1C 주파수: %.0f Hz\n", freq);

// 밴드 기반 주파수 계산
int band = Code2Band(l1c_code);               // 1 (L1 밴드)
double band_freq = Band2Freq(gps_sat, band); // 1575420000.0 Hz

printf("GPS L1 밴드 주파수: %.0f Hz\n", band_freq);
```

### 6.4 주파수 인덱스 활용
```c
// 시스템별 주파수 인덱스 계산
codeStr_t l5i_str = {"L5I"};
int l5i_code = Str2Code(l5i_str);
int gps_fidx = Code2Fidx(1, l5i_code);    // GPS: 3
int gal_fidx = Code2Fidx(3, l5i_code);    // Galileo: 2

printf("GPS L5I 인덱스: %d\n", gps_fidx);
printf("Galileo L5I 인덱스: %d\n", gal_fidx);

// 주파수 인덱스로 배열 접근
double gps_obs_data[6] = {0};  // GPS 주파수별 관측값 배열
if (gps_fidx > 0 && gps_fidx < 6) {
    gps_obs_data[gps_fidx] = 25123456.789;  // L5I 관측값 저장
}
```

### 6.5 밴드 기반 주파수 변환
```c
// 밴드 번호로 직접 주파수 계산 (가정: GPS 위성 인덱스 10)
int gps_sat = 10;                         // GPS 위성 인덱스
double l1_freq = Band2Freq(gps_sat, 1);  // L1: 1575420000.0 Hz
double l5_freq = Band2Freq(gps_sat, 5);  // L5: 1176450000.0 Hz

printf("GPS L1 주파수: %.0f Hz\n", l1_freq);
printf("GPS L5 주파수: %.0f Hz\n", l5_freq);

// 밴드 문자 변환
char band_char = Band2Str(5);             // '5'
int band_id = Str2Band('7');              // 7
```

### 6.6 다중 시스템 처리
```c
// 여러 시스템의 L5 주파수 비교 (가정 위성 인덱스 사용)
int satellites[] = {5, 65, 100, 195, 250};  // 각 시스템 대표 위성 인덱스
char *sys_names[] = {"GPS", "GAL", "BDS", "QZS", "IRN"};

for (int i = 0; i < 5; i++) {
    double freq = Band2Freq(satellites[i], 5);    // L5 주파수

    if (freq > 0.0) {
        printf("%s L5 주파수: %.0f Hz\n", sys_names[i], freq);
    }
}
```

### 6.7 QZSS 신호 매핑 예시
```c
// QZSS Legacy와 Modern 신호 구분 (가정: QZSS 위성 인덱스 193)
int qzs_sat = 193;                        // QZSS 위성 인덱스

// 관측 코드 준비
codeStr_t l1c_str = {"L1C"};
codeStr_t l1s_str = {"L1S"};
codeStr_t l5i_str = {"L5I"};
codeStr_t l5d_str = {"L5D"};

// Legacy L1 신호 (Fidx 1)
int l1c_code = Str2Code(l1c_str);
int l1c_fidx = Code2Fidx(5, l1c_code);   // 1
double l1c_freq = Code2Freq(qzs_sat, l1c_code);

// Modern L1 신호 (Fidx 5)
int l1s_code = Str2Code(l1s_str);
int l1s_fidx = Code2Fidx(5, l1s_code);   // 5
double l1s_freq = Code2Freq(qzs_sat, l1s_code);

printf("QZSS L1C Legacy (Fidx %d): %.0f Hz\n", l1c_fidx, l1c_freq);
printf("QZSS L1S Modern (Fidx %d): %.0f Hz\n", l1s_fidx, l1s_freq);

// L5 기본과 SBAS 신호 구분
int l5i_code = Str2Code(l5i_str);
int l5d_code = Str2Code(l5d_str);
int l5i_fidx = Code2Fidx(5, l5i_code);   // 3 (기본 L5)
int l5d_fidx = Code2Fidx(5, l5d_code);   // 6 (SBAS L5)
```

---

## 7. 성능 특성

### 7.1 메모리 효율성
- **정적 테이블**: 모든 매핑 정보를 컴파일 타임 상수로 관리
- **캐시 친화적**: 순차 탐색 최소화, 직접 인덱스 접근 위주
- **최소 메모리**: 동적 할당 없는 고정 크기 데이터 구조

### 7.2 연산 성능
- **O(1) 밴드 변환**: 단순 문자-숫자 변환으로 상수 시간
- **O(n) 코드 검색**: 내부 테이블 선형 탐색 (n ≤ 100)
- **O(1) 주파수 계산**: 시스템별 고정 공식으로 직접 계산
- **O(1) 인덱스 매핑**: switch-case 기반 직접 매핑

### 7.3 정확도 보장
- **RINEX 표준 준수**: 모든 관측 코드가 RINEX 3.x 표준 완전 호환
- **주파수 정밀도**: double 타입으로 kHz 단위 정확도 보장
- **FCN 자동 적용**: GLONASS 주파수 채널 번호 자동 보정
- **시스템별 특성 반영**: 각 GNSS 시스템의 고유 특성 완전 지원

### 7.4 확장성
- **모듈형 설계**: 시스템별 독립적 함수로 새 시스템 추가 용이
- **테이블 기반**: 새로운 관측 코드 추가 시 테이블만 수정
- **인덱스 확장**: 주파수 인덱스 범위 확장 가능
- **호환성 보장**: 기존 코드 변경 없이 새 기능 추가 가능

### 7.5 GNSS 특화 최적화
- **실시간 처리**: 수신기 실시간 데이터 처리에 최적화
- **다중 시스템**: 모든 GNSS 시스템 동시 지원
- **주파수 다양성**: Legacy와 Modern 신호 모두 지원
- **채널 세분화**: 신호 내 세부 채널까지 완전 분류

---

**이 모듈은 GNSS 관측 데이터의 표준화된 처리를 위한 핵심 인터페이스입니다.**
