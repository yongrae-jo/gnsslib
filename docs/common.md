---
layout: default
title: GNSS 공통 유틸리티 모듈 (common)
---

# GNSS 공통 유틸리티 모듈 (common)

GNSS 라이브러리의 핵심 유틸리티 함수들을 제공하는 기반 모듈입니다.

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

### 1.1 위성 인덱스 시스템
GNSS 시스템별 PRN(Pseudo Random Noise) 번호를 연속적인 정수 인덱스로 매핑하여 배열 기반 효율적 접근을 제공합니다.

**통합 인덱스 계산 공식:**
$$\text{sat} = \text{BASE}[\text{sys}-1] + (\text{prn} - \text{MIN\_PRN}[\text{sys}] + 1)$$

### 1.2 시간 시스템 통합
다양한 GNSS 시간 체계를 UNIX Timestamp 기준으로 통합하여 상호 변환을 지원합니다.

**기준 시간**: UNIX Timestamp (1970/1/1 00:00:00 UTC)

**지원 시간 체계**:
- **UNIX Time**: 1970/1/1 00:00:00 UTC 기준
- **GPS Time**: 1980/1/6 00:00:00 UTC 기준
- **Galileo System Time**: 1999/8/22 00:00:00 UTC 기준 (GPS Time과 동일)
- **BeiDou Time**: 2006/1/1 00:00:00 UTC 기준

### 1.3 문자열 표현 통일
위성 식별자와 시간 정보를 표준화된 문자열 형식으로 표현하여 가독성과 호환성을 보장합니다.

**표준 형식**:
- 위성: `"CXX"` (C: 시스템 코드, XX: PRN 번호)
- 시간: `"YYYY/MM/DD HH:MM:SS.sss"` (소수점 자릿수 조절 가능)

### 1.4 고급 GNSS 좌표 변환
WGS84 타원체 기반의 정밀한 좌표 변환을 지원합니다:
- **ECEF ↔ 지리좌표**: 고정밀 타원체 변환
- **ECEF ↔ ENU**: 지역 좌표계 변환
- **회전행렬 생성**: 좌표변환용 회전행렬
- **위성 기하 계산**: 방위각/고도각, 기하거리
- **Sagnac 효과 보정**: 지구 자전에 의한 보정

### 1.5 정밀도 희석 지수 (DOP) 계산
위성 기하학적 배치가 측위 정확도에 미치는 영향을 정량화:
- **GDOP**: 기하학적 정밀도 희석 지수 (위치 + 시간)
- **PDOP**: 위치 정밀도 희석 지수 (3차원 위치)
- **HDOP**: 수평 정밀도 희석 지수 (2차원 위치)
- **VDOP**: 수직 정밀도 희석 지수 (고도)
- **TDOP**: 시간 정밀도 희석 지수 (클록 오차)

---

## 2. 데이터 타입 구조

```
common 모듈 타입 계층
├── 위성 관련
│   ├── satStr_t ──────────── 위성 문자열 구조체 (CXX 형식)
│   └── 위성 인덱스 ────────── int 타입 통합 인덱스
├── 시간 관련
│   ├── cal_t ─────────────── 달력 시간 구조체
│   ├── calStr_t ──────────── 달력 문자열 구조체
│   └── UNIX Time ─────────── double 타입 기준 시간
├── 안테나 보정
│   ├── pcv_t ─────────────── 개별 안테나 보정 데이터
│   └── pcvs_t ────────────── 안테나 보정 데이터 집합
├── 수신기 정보
│   └── sta_t ─────────────── 수신기 정보 구조체
├── 네비게이션 데이터
│   └── nav_t ─────────────── 통합 네비게이션 구조체
├── GLONASS FCN
│   └── FCN[NSAT_GLO] ─────── GLONASS 주파수 채널 번호
└── 글로벌 상수
    └── CHI2INV[200] ──────── 카이제곱 역분포 테이블
```

---

## 3. 데이터 타입 목록

### 3.1 satStr_t - 위성 문자열 구조체
<details>
<summary>상세 설명</summary>

**목적**: 위성을 문자열 형태로 표현

**구조**:
```c
typedef struct satStr {
    char str[SAT_STR_SIZE];    // 위성 문자열 (CXX 형식)
} satStr_t;
```

**사용**:
- GPS: `"G01"`, `"G32"`
- GLONASS: `"R01"`, `"R24"`
- Galileo: `"E01"`, `"E36"`
- BeiDou: `"C01"`, `"C63"`
- QZSS: `"J01"`, `"J07"`
- IRNSS: `"I01"`, `"I14"`
- SBAS: `"S20"`, `"S58"`

</details>

### 3.2 cal_t - 달력 시간 구조체
<details>
<summary>상세 설명</summary>

**목적**: 일반적인 달력 형태로 시간 표현

**구조**:
```c
typedef struct cal {
    int    year;    // 년도
    int    mon;     // 월 (1-12)
    int    day;     // 일 (1-31)
    int    hour;    // 시 (0-23)
    int    min;     // 분 (0-59)
    double sec;     // 초 (0.0-59.999...)
} cal_t;
```

</details>

### 3.3 calStr_t - 달력 문자열 구조체
<details>
<summary>상세 설명</summary>

**목적**: 달력 시간을 문자열 형태로 표현

**구조**:
```c
typedef struct calStr {
    char str[CAL_STR_SIZE];    // 달력 문자열
} calStr_t;
```

**사용**: `"YYYY/MM/DD HH:MM:SS.sss"` 형식
- 예시: `"2024/12/25 15:30:45.123"`

</details>

### 3.4 pcv_t - 안테나 위상 중심 보정 데이터
<details>
<summary>상세 설명</summary>

**목적**: 안테나 위상 중심 오프셋과 변이 보정 정보

**구조**:
```c
typedef struct pcv {
    int    sat;                        // 위성 인덱스
    char   type[MAX_STR_LEN];         // 안테나 타입 또는 SV 타입
    char   serial[MAX_STR_LEN];       // 안테나 시리얼 번호 또는 위성 ID
    double ts;                        // 유효 시작 시간
    double te;                        // 유효 끝 시간
    double off[NSYS][3][NBAND];       // 위상 중심 오프셋 [m]
    double var[NSYS][19][NBAND];      // 위상 중심 변이 [m]
} pcv_t;
```

</details>

### 3.5 pcvs_t - 안테나 보정 데이터 집합
<details>
<summary>상세 설명</summary>

**목적**: 다수의 안테나 보정 데이터 관리

**구조**:
```c
typedef struct pcvs {
    int    n, nmax;    // 데이터 개수, 할당된 메모리 크기
    pcv_t  *pcv;       // 안테나 보정 데이터 배열
} pcvs_t;
```

</details>

### 3.6 sta_t - 수신기 정보 구조체
<details>
<summary>상세 설명</summary>

**목적**: GNSS 수신기 및 안테나 정보 저장

**구조**:
```c
typedef struct sta {
    char   name[MAX_STR_LEN];      // 마커 이름
    char   marker[MAX_STR_LEN];    // 마커 번호
    char   antdes[MAX_STR_LEN];    // 안테나 설명
    char   antsno[MAX_STR_LEN];    // 안테나 시리얼 번호
    char   rectype[MAX_STR_LEN];   // 수신기 타입
    char   recsno[MAX_STR_LEN];    // 수신기 시리얼 번호
    int    antsetup;               // 안테나 설정 ID
    int    itrf;                   // ITRF 실현 년도
    int    deltype;                // 안테나 위치 델타 타입
    double pos[3];                 // 안테나 위치 (ECEF) [m]
    double del[3];                 // 안테나 델타 위치 [m]
    int    glo_align;              // GLONASS 코드-위상 정렬 여부
    double glo_bias[4];            // GLONASS 코드-위상 바이어스 [m]
} sta_t;
```

</details>

### 3.7 nav_t - 통합 네비게이션 구조체
<details>
<summary>상세 설명</summary>

**목적**: 모든 네비게이션 관련 정보 통합 관리

**구조**:
```c
typedef struct nav {
    ephs_t  ephs[NSAT];     // 방송궤도력 데이터
    pcvs_t  pcvs;           // 안테나 PCO/PCV 파라미터
    sta_t   sta[NRCV];      // 수신기 정보
    int     iono[NSYS][8];  // 이온층 모델 파라미터
    opt_t   *opt;           // 처리 옵션
} nav_t;
```

</details>

---

## 4. 함수 구조

```
common 모듈 함수 계층
├── 초기화 및 메모리 관리
│   ├── InitNav() ────────── 네비게이션 구조체 초기화
│   └── FreeNav() ────────── 네비게이션 구조체 메모리 해제
├── GLONASS FCN 관리
│   ├── GetFcn() ─────────── FCN 조회
│   ├── SetFcn() ─────────── FCN 설정
│   └── SetDefaultFcn() ──── 기본 FCN 설정
├── 위성 인덱스 변환
│   ├── Prn2Sat() ────────── PRN → 위성 인덱스
│   ├── Sat2Prn() ────────── 위성 인덱스 → PRN
│   ├── Str2Sat() ────────── 문자열 → 위성 인덱스
│   └── Sat2Str() ────────── 위성 인덱스 → 문자열
├── 시간 변환 함수
│   ├── 기본 시간 변환
│   │   ├── Cal2Time() ───── 달력 → UNIX Time
│   │   ├── Time2Cal() ───── UNIX Time → 달력
│   │   ├── Str2Cal() ────── 문자열 → 달력
│   │   └── Cal2Str() ────── 달력 → 문자열
│   ├── GNSS 시간 변환
│   │   ├── Gpst2Time() ──── GPS Time → UNIX Time
│   │   ├── Time2Gpst() ──── UNIX Time → GPS Time
│   │   ├── Bdt2Time() ───── BeiDou Time → UNIX Time
│   │   └── Time2Bdt() ───── UNIX Time → BeiDou Time
│   ├── 시간 체계 간 변환
│   │   ├── Gpst2Utc() ───── GPS Time → UTC
│   │   ├── Utc2Gpst() ───── UTC → GPS Time
│   │   ├── Gpst2Bdt() ───── GPS Time → BeiDou Time
│   │   └── Bdt2Gpst() ───── BeiDou Time → GPS Time
│   └── 기타 시간 함수
│       ├── TimeGet() ────── 현재 시간 조회
│       └── Time2Doy() ───── 시간 → 연중 일자
├── GNSS 좌표 변환
│   ├── 기본 좌표 변환
│   │   ├── Xyz2Llh() ─────── ECEF → 지리좌표
│   │   ├── Llh2Xyz() ─────── 지리좌표 → ECEF
│   │   └── Xyz2Rot() ─────── 회전행렬 생성
│   ├── 지역 좌표 변환
│   │   ├── Xyz2Enu() ─────── ECEF → ENU
│   │   └── Enu2Xyz() ─────── ENU → ECEF
│   └── 위성 기하
│       ├── SatAzEl() ─────── 위성 방위각/고도각
│       └── GeoDist() ─────── 기하거리 + Sagnac 보정
└── GNSS 분석 함수
    └── Dops() ────────────── DOP 값 계산
└── 인라인 유틸리티 함수
    ├── SQR() ───────────── 제곱 계산
    ├── Sys2Str() ───────── 시스템 인덱스 → 문자
    └── Str2Sys() ───────── 문자 → 시스템 인덱스
```

---

## 5. 함수 목록

### 5.1 초기화 및 메모리 관리 함수

#### 5.1.1 InitNav() - 네비게이션 구조체 초기화
<details>
<summary>상세 설명</summary>

**목적**: 네비게이션 구조체의 모든 멤버를 안전하게 초기화

**입력**:
- `nav_t *nav`: 초기화할 네비게이션 구조체 포인터

**출력**:
- `void`: 반환값 없음

**함수 로직**:
1. 방송궤도력 데이터 배열 초기화 (각 위성별 n=0, nmax=0, eph=NULL)
2. 안테나 보정 데이터 초기화 (n=0, nmax=0, pcv=NULL)
3. 처리 옵션 메모리 할당 및 기본값 설정
4. 메모리 할당 실패 시 에러 출력 후 프로그램 종료

**사용 예시**:
```c
nav_t nav;
InitNav(&nav);

// 네비게이션 구조체 사용
// ... 궤도력 로드, 관측 데이터 처리 등 ...

// 사용 완료 후 메모리 해제
FreeNav(&nav);
```

</details>

#### 5.1.2 FreeNav() - 네비게이션 구조체 메모리 해제
<details>
<summary>상세 설명</summary>

**목적**: 네비게이션 구조체에 동적 할당된 모든 메모리 안전 해제

**입력**:
- `nav_t *nav`: 메모리를 해제할 네비게이션 구조체 포인터

**출력**:
- `void`: 반환값 없음

**함수 로직**:
1. 각 위성의 방송궤도력 메모리 해제 (eph 배열)
2. 안테나 보정 데이터 메모리 해제 (pcv 배열)
3. 처리 옵션 메모리 해제
4. 포인터를 NULL로 설정하여 안전성 보장

**사용 예시**:
```c
nav_t nav;
InitNav(&nav);

// 네비게이션 데이터 사용...

// 프로그램 종료 전 메모리 해제
FreeNav(&nav);
printf("네비게이션 구조체 메모리 해제 완료\n");
```

</details>

### 5.2 GLONASS FCN 관리 함수

#### 5.2.1 GetFcn() - GLONASS FCN 조회
<details>
<summary>상세 설명</summary>

**목적**: GLONASS 위성의 주파수 채널 번호(FCN) 조회

**입력**:
- `int prn`: GLONASS PRN 번호
- `int *fcn`: FCN 값을 저장할 포인터

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:
1. PRN 유효성 검사 (1 ≤ prn ≤ MAX_PRN_GLO)
2. 내부 FCN 배열에서 값 조회 (FCN[prn-1] - 8)
3. FCN 유효성 검사 (-7 ≤ fcn ≤ 6)
4. 유효한 FCN이면 포인터에 저장 후 1 반환, 아니면 0 반환

**사용 예시**:
```c
int fcn;
if (GetFcn(1, &fcn)) {
    printf("GLONASS PRN 1의 FCN: %d\n", fcn);
} else {
    printf("GLONASS PRN 1의 FCN을 찾을 수 없습니다.\n");
}
```

</details>

#### 5.2.2 SetFcn() - GLONASS FCN 설정
<details>
<summary>상세 설명</summary>

**목적**: GLONASS 위성의 주파수 채널 번호(FCN) 설정

**입력**:
- `int prn`: GLONASS PRN 번호
- `int fcn`: 설정할 FCN 값 (-7 ≤ fcn ≤ 6)

**출력**:
- `void`: 반환값 없음

**함수 로직**:
1. PRN 유효성 검사 (1 ≤ prn ≤ MAX_PRN_GLO)
2. FCN 유효성 검사 (-7 ≤ fcn ≤ 6)
3. 유효한 값이면 내부 FCN 배열에 저장 (FCN[prn-1] = fcn + 8)
4. 유효하지 않으면 함수 종료

**사용 예시**:
```c
// GLONASS PRN 1에 FCN -7 설정
SetFcn(1, -7);

// GLONASS PRN 2에 FCN -6 설정
SetFcn(2, -6);

printf("GLONASS FCN 설정 완료\n");
```

</details>

#### 5.2.3 SetDefaultFcn() - 기본 FCN 설정
<details>
<summary>상세 설명</summary>

**목적**: 모든 GLONASS 위성에 기본 FCN 값 설정

**입력**:
- `void`: 입력 매개변수 없음

**출력**:
- `void`: 반환값 없음

**함수 로직**:
1. 사전 정의된 기본 FCN 테이블 참조
2. 각 GLONASS PRN에 대응하는 기본 FCN 값 설정
3. 모든 GLONASS 위성 (PRN 1-24)에 대해 순차적 설정

**사용 예시**:
```c
// 모든 GLONASS 위성에 기본 FCN 설정
SetDefaultFcn();
printf("모든 GLONASS 위성에 기본 FCN 설정 완료\n");

// 설정된 FCN 확인
int fcn;
for (int prn = 1; prn <= 24; prn++) {
    if (GetFcn(prn, &fcn)) {
        printf("GLONASS PRN %d: FCN %d\n", prn, fcn);
    }
}
```

</details>

### 5.3 위성 인덱스 변환 함수

#### 5.3.1 Prn2Sat() - PRN을 위성 인덱스로 변환
<details>
<summary>상세 설명</summary>

**목적**: 시스템별 PRN 번호를 통합 위성 인덱스로 변환

**입력**:
- `int sys`: GNSS 시스템 인덱스 (1~NSYS)
- `int prn`: PRN 번호

**출력**:
- `int`: 위성 인덱스 (1~NSAT), 오류 시 0

**함수 로직**:
1. 시스템 인덱스 유효성 검사 (1 ≤ sys ≤ NSYS)
2. 시스템별 PRN 범위 검사 (MIN_PRN ≤ prn ≤ MAX_PRN)
3. 베이스 인덱스 + 상대 PRN으로 위성 인덱스 계산
4. 계산 공식: `BASE[sys-1] + (prn - MIN_PRN[sys] + 1)`

**사용 예시**:
```c
// GPS PRN 5를 위성 인덱스로 변환
int gps_sat = Prn2Sat(1, 5);  // GPS 시스템(1), PRN 5
printf("GPS PRN 5 위성 인덱스: %d\n", gps_sat);

// GLONASS PRN 1을 위성 인덱스로 변환
int glo_sat = Prn2Sat(2, 1);  // GLONASS 시스템(2), PRN 1
printf("GLONASS PRN 1 위성 인덱스: %d\n", glo_sat);

// 오류 처리
int invalid_sat = Prn2Sat(1, 99);  // 잘못된 PRN
if (invalid_sat == 0) {
    printf("잘못된 PRN 번호입니다.\n");
}
```

</details>

#### 5.3.2 Sat2Prn() - 위성 인덱스를 PRN으로 변환
<details>
<summary>상세 설명</summary>

**목적**: 통합 위성 인덱스를 시스템별 PRN 번호로 변환

**입력**:
- `int sat`: 위성 인덱스 (1~NSAT)
- `int *prn`: PRN 번호를 저장할 포인터

**출력**:
- `int`: 시스템 인덱스 (1~NSYS), 오류 시 0

**함수 로직**:
1. 위성 인덱스 유효성 검사 (1 ≤ sat ≤ NSAT)
2. 베이스 인덱스 테이블을 순회하여 해당 시스템 찾기
3. 시스템 내 상대 인덱스에서 PRN 계산
4. PRN = 상대인덱스 + MIN_PRN[sys] - 1

**사용 예시**:
```c
// 위성 인덱스를 PRN으로 변환
int gps_sat = Prn2Sat(1, 5);  // GPS PRN 5 → 위성 인덱스
int prn;
int sys = Sat2Prn(gps_sat, &prn);  // 위성 인덱스 → PRN

if (sys > 0) {
    printf("위성 인덱스 %d → 시스템: %d, PRN: %d\n", gps_sat, sys, prn);
} else {
    printf("잘못된 위성 인덱스입니다.\n");
}

// 다양한 시스템 테스트
int test_sats[] = {5, 33, 69, 105};  // GPS, GLO, GAL, BDS 예시
for (int i = 0; i < 4; i++) {
    int sys = Sat2Prn(test_sats[i], &prn);
    if (sys > 0) {
        printf("위성 %d: 시스템 %d, PRN %d\n", test_sats[i], sys, prn);
    }
}
```

</details>

#### 5.3.3 Str2Sat() - 위성 문자열을 위성 인덱스로 변환
<details>
<summary>상세 설명</summary>

**목적**: "CXX" 형식의 위성 문자열을 위성 인덱스로 변환

**입력**:
- `satStr_t satStr`: 위성 문자열 구조체

**출력**:
- `int`: 위성 인덱스 (1~NSAT), 오류 시 0

**함수 로직**:
1. 문자열 형식 검증 (길이 3, 첫 글자는 시스템 문자)
2. 시스템 문자에서 시스템 인덱스 추출 (Str2Sys 호출)
3. 나머지 문자에서 PRN 번호 파싱
4. Prn2Sat() 호출하여 최종 위성 인덱스 계산

**사용 예시**:
```c
// 위성 문자열을 위성 인덱스로 변환
satStr_t sat_str = {"G05"};  // GPS PRN 5
int sat = Str2Sat(sat_str);
if (sat > 0) {
    printf("위성 문자열 %s → 위성 인덱스: %d\n", sat_str.str, sat);
} else {
    printf("잘못된 위성 문자열입니다: %s\n", sat_str.str);
}

// 다양한 시스템 테스트
char *test_strs[] = {"G01", "R01", "E01", "C01", "J01", "I01", "S20"};
for (int i = 0; i < 7; i++) {
    satStr_t str = {0};
    strcpy(str.str, test_strs[i]);
    int sat_idx = Str2Sat(str);
    printf("%s → 위성 인덱스: %d\n", test_strs[i], sat_idx);
}
```

</details>

#### 5.3.4 Sat2Str() - 위성 인덱스를 위성 문자열로 변환
<details>
<summary>상세 설명</summary>

**목적**: 위성 인덱스를 "CXX" 형식의 위성 문자열로 변환

**입력**:
- `int sat`: 위성 인덱스 (1~NSAT)

**출력**:
- `satStr_t`: 위성 문자열 구조체, 오류 시 빈 문자열

**함수 로직**:
1. Sat2Prn()으로 시스템과 PRN 추출
2. 시스템 인덱스에서 시스템 문자 변환 (Sys2Str 호출)
3. PRN을 2자리 문자열로 포맷팅
4. "CXX" 형식으로 최종 문자열 조합

**사용 예시**:
```c
// 위성 인덱스를 문자열로 변환
int gps_sat = Prn2Sat(1, 5);  // GPS PRN 5
satStr_t str = Sat2Str(gps_sat);
if (strlen(str.str) > 0) {
    printf("위성 인덱스 %d → 위성 문자열: %s\n", gps_sat, str.str);
} else {
    printf("잘못된 위성 인덱스입니다: %d\n", gps_sat);
}

// 완전한 변환 테스트 (인덱스 → 문자열 → 인덱스)
int original_sat = Prn2Sat(2, 10);  // GLONASS PRN 10
satStr_t sat_str = Sat2Str(original_sat);
int converted_sat = Str2Sat(sat_str);

printf("원본: %d → 문자열: %s → 변환: %d\n",
       original_sat, sat_str.str, converted_sat);
if (original_sat == converted_sat) {
    printf("변환 성공!\n");
}
```

</details>

### 5.4 시간 변환 함수

#### 5.4.1 Cal2Time() - 달력을 UNIX Time으로 변환
<details>
<summary>상세 설명</summary>

**목적**: 달력 형태의 시간을 UNIX Timestamp로 변환

**입력**:
- `cal_t cal`: 달력 시간 구조체

**출력**:
- `double`: UNIX Timestamp (초), 오류 시 0.0

**함수 로직**:
1. 달력 유효성 검사 (년, 월, 일, 시, 분, 초 범위)
2. 1970년 1월 1일부터의 일수 계산
3. 윤년 보정 및 월별 일수 누적
4. 초 단위로 변환하여 반환

**사용 예시**:
```c
// 달력 시간 구조체 생성
cal_t cal = {2024, 12, 25, 15, 30, 45.123};

// 달력을 UNIX Time으로 변환
double unix_time = Cal2Time(cal);
if (unix_time > 0.0) {
    printf("2024/12/25 15:30:45.123 → UNIX Time: %.3f\n", unix_time);
} else {
    printf("잘못된 달력 시간입니다.\n");
}

// 역변환으로 검증
cal_t cal_back = Time2Cal(unix_time);
printf("역변환: %04d/%02d/%02d %02d:%02d:%06.3f\n",
       cal_back.year, cal_back.mon, cal_back.day,
       cal_back.hour, cal_back.min, cal_back.sec);
```

</details>

#### 5.4.2 Time2Cal() - UNIX Time을 달력으로 변환
<details>
<summary>상세 설명</summary>

**목적**: UNIX Timestamp를 달력 형태로 변환

**입력**:
- `double time`: UNIX Timestamp (초)

**출력**:
- `cal_t`: 달력 시간 구조체

**함수 로직**:
1. 총 일수와 당일 초 분리
2. 1970년부터 순차적으로 년/월/일 계산
3. 윤년 고려한 월별 일수 처리
4. 당일 초를 시/분/초로 분해

**사용 예시**:
```c
// UNIX Time을 달력으로 변환
double unix_time = TimeGet();  // 현재 시간
cal_t cal = Time2Cal(unix_time);

printf("현재 시간: %04d/%02d/%02d %02d:%02d:%06.3f\n",
       cal.year, cal.mon, cal.day,
       cal.hour, cal.min, cal.sec);

// 특정 UNIX Time 변환
double test_time = 1735128645.123;  // 예시 시간
cal_t test_cal = Time2Cal(test_time);
printf("UNIX Time %.3f → %04d/%02d/%02d %02d:%02d:%06.3f\n",
       test_time, test_cal.year, test_cal.mon, test_cal.day,
       test_cal.hour, test_cal.min, test_cal.sec);
```

</details>

#### 5.4.3 Gpst2Time() - GPS Time을 UNIX Time으로 변환
<details>
<summary>상세 설명</summary>

**목적**: GPS 주차와 주내 초를 UNIX Timestamp로 변환

**입력**:
- `int week`: GPS 주차
- `double tow`: 주내 초 (Time of Week)

**출력**:
- `double`: UNIX Timestamp (초)

**GNSS 수식**:
GPS Time → UNIX Time 변환:
$$t_{\text{UNIX}} = t_{\text{GPS0}} + \text{week} \times 604800 + \text{tow}$$

여기서:
- $t_{\text{GPS0}}$: GPS 기준시간 (1980/1/6 00:00:00 UTC)의 UNIX Time
- $604800 = 7 \times 24 \times 3600$ (1주일의 초 수)

**함수 로직**:
1. GPS 시간 기준점 (1980/1/6) 계산
2. 주차에서 일수 계산 (week × 7)
3. 주내 초를 초 단위로 변환
4. UNIX 기준으로 최종 시간 계산

**사용 예시**:
```c
// GPS Time을 UNIX Time으로 변환
int gps_week = 2296;
double tow = 345678.5;
double unix_time = Gpst2Time(gps_week, tow);

printf("GPS 주차 %d, 주내초 %.1f → UNIX Time: %.3f\n",
       gps_week, tow, unix_time);

// 달력으로 확인
cal_t cal = Time2Cal(unix_time);
printf("달력 시간: %04d/%02d/%02d %02d:%02d:%06.3f\n",
       cal.year, cal.mon, cal.day, cal.hour, cal.min, cal.sec);

// 역변환으로 검증
int week_back;
double tow_back = Time2Gpst(unix_time, &week_back);
printf("역변환: 주차 %d, 주내초 %.1f\n", week_back, tow_back);
```

</details>

#### 5.4.4 Time2Gpst() - UNIX Time을 GPS Time으로 변환
<details>
<summary>상세 설명</summary>

**목적**: UNIX Time을 GPS Time으로 변환

**입력**:
- `double time`: UNIX Time (초)
- `int *week`: GPS 주차를 저장할 포인터

**출력**:
- `double`: 주내 초 (Time of Week)

**함수 로직**:
1. UNIX Time에서 GPS 기준시간 차이 계산
2. 주차 계산 (차이 / (7×24×3600))
3. 주내 초 계산 (차이 % (7×24×3600))
4. 주차는 포인터에 저장, 주내 초는 반환

</details>

#### 5.4.5 Bdt2Time() - BeiDou Time을 UNIX Time으로 변환
<details>
<summary>상세 설명</summary>

**목적**: BeiDou Time을 UNIX Time으로 변환

**입력**:
- `int week`: BeiDou 주차
- `double tow`: 주내 초

**출력**:
- `double`: UNIX Timestamp (초)

**함수 로직**:
1. BeiDou 시간 기준점 (2006/1/1) 계산
2. 주차와 주내 초를 총 초로 변환
3. UNIX 기준으로 최종 시간 계산

</details>

#### 5.4.6 Time2Bdt() - UNIX Time을 BeiDou Time으로 변환
<details>
<summary>상세 설명</summary>

**목적**: UNIX Timestamp를 BeiDou Time으로 변환

**입력**:
- `double time`: UNIX Timestamp (초)
- `int *week`: BeiDou 주차를 저장할 포인터

**출력**:
- `double`: 주내 초

**함수 로직**:
1. UNIX Time에서 BeiDou 기준시간 차이 계산
2. 주차와 주내 초 분리
3. 주차는 포인터에 저장, 주내 초는 반환

**사용 예시**:
```c
// UNIX Time을 BeiDou Time으로 변환
double unix_time = TimeGet();  // 현재 시간
int bdt_week;
double bdt_tow = Time2Bdt(unix_time, &bdt_week);

printf("UNIX Time %.3f → BeiDou 주차: %d, 주내초: %.1f\n",
       unix_time, bdt_week, bdt_tow);

// 역변환으로 검증
double unix_back = Bdt2Time(bdt_week, bdt_tow);
printf("역변환 검증: %.3f (차이: %.6f초)\n", unix_back, unix_time - unix_back);
```

</details>

#### 5.4.7 TimeGet() - 현재 시간 조회
<details>
<summary>상세 설명</summary>

**목적**: 시스템 현재 시간을 UNIX Timestamp로 조회

**입력**:
- `void`: 입력 매개변수 없음

**출력**:
- `double`: 현재 UNIX Timestamp (초)

**함수 로직**:
1. Windows: time() 함수 사용
2. Unix/Linux: gettimeofday() 함수 사용 (마이크로초 정밀도)
3. 시스템별 조건부 컴파일로 플랫폼 호환성 보장

**사용 예시**:
```c
// 현재 시간 조회
double current_time = TimeGet();
printf("현재 UNIX Time: %.3f\n", current_time);

// 달력 형태로 확인
cal_t cal = Time2Cal(current_time);
printf("현재 시간: %04d/%02d/%02d %02d:%02d:%06.3f\n",
       cal.year, cal.mon, cal.day, cal.hour, cal.min, cal.sec);

// 처리 시간 측정
double start_time = TimeGet();
// ... 어떤 처리 작업 ...
double end_time = TimeGet();
printf("처리 시간: %.3f초\n", end_time - start_time);
```

</details>

#### 5.4.8 Gpst2Utc() - GPS Time을 UTC로 변환
<details>
<summary>상세 설명</summary>

**목적**: GPS Time을 UTC로 변환 (윤초 보정 적용)

**입력**:
- `double gpst`: GPS Time (UNIX Timestamp)

**출력**:
- `double`: UTC Time (UNIX Timestamp)

**GNSS 수식**:
GPS Time → UTC 변환 (윤초 보정):
$$t_{\text{UTC}} = t_{\text{GPST}} + \Delta t_{\text{LS}}$$

여기서:
- $\Delta t_{\text{LS}}$: 윤초 보정값 (UTC - GPST)
- 윤초는 시간에 따라 계단 함수 형태로 변화

윤초 테이블에서 해당 시점의 보정값을 찾아 적용:
$$\Delta t_{\text{LS}} = \begin{cases}
-18.0 & \text{if } t \geq 2017/1/1 \\
-17.0 & \text{if } 2015/7/1 \leq t < 2017/1/1 \\
\vdots & \vdots \\
0.0 & \text{if } t < 1981/7/1
\end{cases}$$

**함수 로직**:
1. 입력 GPS Time에 해당하는 윤초 조회
2. 윤초 테이블에서 적절한 윤초 값 찾기
3. GPS Time에 윤초 적용하여 UTC 계산

**사용 예시**:
```c
// GPS Time을 UTC로 변환 (윤초 보정)
double gps_time = TimeGet();  // 현재 GPS Time
double utc_time = Gpst2Utc(gps_time);

printf("GPS Time: %.3f\n", gps_time);
printf("UTC Time: %.3f\n", utc_time);
printf("윤초 보정: %.1f초\n", utc_time - gps_time);

// 달력으로 확인
cal_t gps_cal = Time2Cal(gps_time);
cal_t utc_cal = Time2Cal(utc_time);
printf("GPS: %04d/%02d/%02d %02d:%02d:%06.3f\n",
       gps_cal.year, gps_cal.mon, gps_cal.day,
       gps_cal.hour, gps_cal.min, gps_cal.sec);
printf("UTC: %04d/%02d/%02d %02d:%02d:%06.3f\n",
       utc_cal.year, utc_cal.mon, utc_cal.day,
       utc_cal.hour, utc_cal.min, utc_cal.sec);
```

</details>

#### 5.4.9 Utc2Gpst() - UTC를 GPS Time으로 변환
<details>
<summary>상세 설명</summary>

**목적**: UTC를 GPS Time으로 변환 (윤초 보정 적용)

**입력**:
- `double utc`: UTC Time (UNIX Timestamp)

**출력**:
- `double`: GPS Time (UNIX Timestamp)

**함수 로직**:
1. 입력 UTC에 해당하는 윤초 조회
2. UTC에서 윤초를 차감하여 GPS Time 계산

**사용 예시**:
```c
// UTC를 GPS Time으로 변환
double utc_time = TimeGet();  // 현재 UTC 시간
double gps_time = Utc2Gpst(utc_time);

printf("UTC Time: %.3f\n", utc_time);
printf("GPS Time: %.3f\n", gps_time);
printf("윤초 보정: %.1f초\n", gps_time - utc_time);

// 역변환으로 검증
double utc_back = Gpst2Utc(gps_time);
printf("역변환 검증: %.3f (차이: %.6f초)\n", utc_back, utc_time - utc_back);
```

</details>

#### 5.4.10 Gpst2Bdt() - GPS Time을 BeiDou Time으로 변환
<details>
<summary>상세 설명</summary>

**목적**: GPS Time을 BeiDou Time으로 변환

**입력**:
- `double gpst`: GPS Time (UNIX Timestamp)

**출력**:
- `double`: BeiDou Time (UNIX Timestamp)

**함수 로직**:
1. GPS와 BeiDou 시간 기준점 차이 계산 (14초)
2. GPS Time에서 14초 차감하여 BeiDou Time 계산

**사용 예시**:
```c
// GPS Time을 BeiDou Time으로 변환
double gps_time = TimeGet();
double bdt_time = Gpst2Bdt(gps_time);

printf("GPS Time: %.3f\n", gps_time);
printf("BeiDou Time: %.3f\n", bdt_time);
printf("시간 차이: %.1f초\n", gps_time - bdt_time);

// 주차/주내초 형태로 확인
int gps_week, bdt_week;
double gps_tow = Time2Gpst(gps_time, &gps_week);
double bdt_tow = Time2Bdt(bdt_time, &bdt_week);

printf("GPS: 주차 %d, 주내초 %.1f\n", gps_week, gps_tow);
printf("BeiDou: 주차 %d, 주내초 %.1f\n", bdt_week, bdt_tow);
```

</details>

#### 5.4.11 Bdt2Gpst() - BeiDou Time을 GPS Time으로 변환
<details>
<summary>상세 설명</summary>

**목적**: BeiDou Time을 GPS Time으로 변환

**입력**:
- `double bdt`: BeiDou Time (UNIX Timestamp)

**출력**:
- `double`: GPS Time (UNIX Timestamp)

**함수 로직**:
1. BeiDou Time에 14초 가산하여 GPS Time 계산

**사용 예시**:
```c
// BeiDou Time을 GPS Time으로 변환
double bdt_time = TimeGet();  // 현재 BeiDou Time
double gps_time = Bdt2Gpst(bdt_time);

printf("BeiDou Time: %.3f\n", bdt_time);
printf("GPS Time: %.3f\n", gps_time);
printf("시간 차이: %.1f초\n", gps_time - bdt_time);

// 역변환으로 검증
double bdt_back = Gpst2Bdt(gps_time);
printf("역변환 검증: %.3f (차이: %.6f초)\n", bdt_back, bdt_time - bdt_back);
```

</details>

#### 5.4.12 Time2Doy() - 시간을 연중 일자로 변환
<details>
<summary>상세 설명</summary>

**목적**: UNIX Timestamp를 연중 일자(Day of Year)로 변환

**입력**:
- `double time`: UNIX Timestamp (초)

**출력**:
- `int`: 연중 일자 (1~366)

**함수 로직**:
1. UNIX Time을 달력으로 변환 (Time2Cal 호출)
2. 해당 년도의 1월 1일부터 일수 계산
3. 윤년 고려하여 정확한 연중 일자 반환

**사용 예시**:
```c
// 현재 시간의 연중 일자 계산
double current_time = TimeGet();
int doy = Time2Doy(current_time);
cal_t cal = Time2Cal(current_time);

printf("현재 시간: %04d/%02d/%02d\n", cal.year, cal.mon, cal.day);
printf("연중 일자: %d일째\n", doy);

// 특정 날짜들의 연중 일자
cal_t test_dates[] = {
    {2024, 1, 1, 0, 0, 0.0},    // 신정
    {2024, 12, 25, 0, 0, 0.0},  // 크리스마스
    {2024, 12, 31, 0, 0, 0.0}   // 대한민국 마지막 날
};

for (int i = 0; i < 3; i++) {
    double time = Cal2Time(test_dates[i]);
    int doy = Time2Doy(time);
    printf("%04d/%02d/%02d → 연중 %d일째\n",
           test_dates[i].year, test_dates[i].mon, test_dates[i].day, doy);
}
```

</details>

#### 5.4.13 Str2Cal() - 문자열을 달력으로 변환
<details>
<summary>상세 설명</summary>

**목적**: "YYYY/MM/DD HH:MM:SS.sss" 형식 문자열을 달력으로 변환

**입력**:
- `calStr_t calStr`: 달력 문자열 구조체

**출력**:
- `cal_t`: 달력 시간 구조체

**함수 로직**:
1. 문자열 형식 검증
2. sscanf를 사용하여 년, 월, 일, 시, 분, 초 파싱
3. 파싱된 값들로 cal_t 구조체 구성

**사용 예시**:
```c
// 문자열을 달력으로 변환
calStr_t time_str = {"2024/12/25 15:30:45.123"};
cal_t cal = Str2Cal(time_str);

printf("입력 문자열: %s\n", time_str.str);
printf("변환된 달력: %04d/%02d/%02d %02d:%02d:%06.3f\n",
       cal.year, cal.mon, cal.day, cal.hour, cal.min, cal.sec);

// 다양한 형식 테스트
char *test_strs[] = {
    "2024/01/01 00:00:00.000",
    "2024/06/15 12:30:45.500",
    "2024/12/31 23:59:59.999"
};

for (int i = 0; i < 3; i++) {
    calStr_t str = {0};
    strcpy(str.str, test_strs[i]);
    cal_t cal = Str2Cal(str);

    // UNIX Time으로 변환하여 검증
    double unix_time = Cal2Time(cal);
    printf("%s → UNIX Time: %.3f\n", test_strs[i], unix_time);
}
```

</details>

#### 5.4.14 Cal2Str() - 달력을 문자열로 변환
<details>
<summary>상세 설명</summary>

**목적**: 달력을 "YYYY/MM/DD HH:MM:SS.sss" 형식 문자열로 변환

**입력**:
- `cal_t cal`: 달력 시간 구조체
- `int dec`: 소수점 자릿수 (0~3)

**출력**:
- `calStr_t`: 달력 문자열 구조체

**함수 로직**:
1. 달력 유효성 검사
2. snprintf를 사용하여 지정된 포맷으로 문자열 생성
3. 소수점 자릿수에 따른 초 포맷팅

**사용 예시**:
```c
// 달력을 문자열로 변환 (다양한 소수점 자릿수)
cal_t cal = {2024, 12, 25, 15, 30, 45.123456};

// 소수점 자릿수별 변환
for (int dec = 0; dec <= 3; dec++) {
    calStr_t str = Cal2Str(cal, dec);
    printf("소수점 %d자리: %s\n", dec, str.str);
}

// 현재 시간을 문자열로 변환
double current_time = TimeGet();
cal_t current_cal = Time2Cal(current_time);
calStr_t current_str = Cal2Str(current_cal, 3);
printf("현재 시간: %s\n", current_str.str);

// 역변환으로 검증
cal_t cal_back = Str2Cal(current_str);
double time_back = Cal2Time(cal_back);
printf("역변환 검증: %.6f초 차이\n", current_time - time_back);
```

</details>

### 5.5 GNSS 좌표 변환 함수

#### 5.5.1 Xyz2Llh() - ECEF → 지리좌표 변환
<details>
<summary>상세 설명</summary>

**목적**: WGS84 타원체 기반 ECEF 좌표를 지리좌표로 변환

**입력**:
- `const mat_t *xyz`: ECEF 좌표 (1×3) [m]

**출력**:
- `mat_t *llh`: 지리좌표 (1×3) [rad, rad, m] (오류 시 NULL)

**GNSS 수식**:
WGS84 타원체 좌표 변환 (개선된 반복 알고리즘):

초기값:
$$p = \|\boldsymbol{r}\| = \sqrt{x^2 + y^2 + z^2}$$
$$L_0 = \frac{z}{1 - e^2}$$

반복 계산 ($|L - L_0| \geq 10^{-4}$까지):
$$\sin\phi = \frac{L_0}{\sqrt{p^2 + L_0^2}}$$
$$N = \frac{a}{\sqrt{1 - e^2\sin^2\phi}}$$
$$L = z + e^2 \cdot N \cdot \sin\phi$$

최종 결과:
$$\phi = \arctan2(L, p)$$
$$\lambda = \arctan2(y, x)$$
$$h = \sqrt{L^2 + p^2} - N$$

여기서:
- $e^2 = \text{WGS84\_E2} = FE\_WGS84 \cdot (2 - FE\_WGS84)$
- $a = \text{RE\_WGS84}$ (WGS84 장반축)
- 특수 경우: $p \leq 10^{-12}$일 때 $\phi = \pm\pi/2$, $\lambda = 0$

**함수 로직**:
- WGS84 타원체 매개변수 사용
- 반복 계산을 통한 고정밀 변환
- 위도, 경도, 타원체고 반환

</details>

#### 5.5.2 Llh2Xyz() - 지리좌표 → ECEF 변환
<details>
<summary>상세 설명</summary>

**목적**: 지리좌표를 WGS84 ECEF 좌표로 변환

**입력**:
- `const mat_t *llh`: 지리좌표 (1×3) [rad, rad, m]

**출력**:
- `mat_t *xyz`: ECEF 좌표 (1×3) [m] (오류 시 NULL)

**GNSS 수식**:
WGS84 지리좌표 → ECEF 변환:
$$x = (N + h) \cos\phi \cos\lambda$$
$$y = (N + h) \cos\phi \sin\lambda$$
$$z = (N(1-e^2) + h) \sin\phi$$

여기서:
- $N = \frac{a}{\sqrt{1-e^2\sin^2\phi}}$ (자오선 곡률반지름)
- $\phi$: 위도 [rad]
- $\lambda$: 경도 [rad]
- $h$: 타원체고 [m]

**함수 로직**:
- WGS84 타원체 매개변수 적용
- 곡률반지름 계산
- 직교 좌표 변환 공식 적용

**사용 예시**:
```c
// 지리좌표를 ECEF로 변환
mat_t *llh = Mat(1, 3, DOUBLE);
MatSetD(llh, 0, 0, 37.5665 * D2R);  // 위도 [rad] (서울)
MatSetD(llh, 0, 1, 126.9780 * D2R); // 경도 [rad] (서울)
MatSetD(llh, 0, 2, 100.0);          // 높이 [m]

mat_t *xyz = Llh2Xyz(llh);
if (xyz) {
    printf("서울 좌표 (위도: %.4f°, 경도: %.4f°, 높이: %.1fm)\n",
           37.5665, 126.9780, 100.0);
    printf("ECEF: X=%.3f, Y=%.3f, Z=%.3f [m]\n",
           MatGetD(xyz, 0, 0), MatGetD(xyz, 0, 1), MatGetD(xyz, 0, 2));

    // 역변환으로 검증
    mat_t *llh_back = Xyz2Llh(xyz);
    if (llh_back) {
        printf("역변환: 위도=%.6f°, 경도=%.6f°, 높이=%.3fm\n",
               MatGetD(llh_back, 0, 0) * R2D,
               MatGetD(llh_back, 0, 1) * R2D,
               MatGetD(llh_back, 0, 2));
        FreeMat(llh_back);
    }
}

FreeMat(llh); FreeMat(xyz);
```

</details>

#### 5.5.3 Xyz2Rot() - 회전행렬 생성
<details>
<summary>상세 설명</summary>

**목적**: ECEF 좌표에서 ENU 지역좌표로 변환하는 회전행렬 생성

**입력**:
- `const mat_t *xyz`: ECEF 좌표 (1×3) [m]

**출력**:
- `mat_t *rot`: 회전행렬 (3×3) (오류 시 NULL)

**함수 로직**:
- 먼저 LLH 좌표로 변환
- 위도/경도 기반 회전행렬 계산
- East, North, Up 방향 벡터 생성

**사용 예시**:
```c
// ECEF 좌표에서 회전행렬 생성
mat_t *xyz = Mat(1, 3, DOUBLE);
MatSetD(xyz, 0, 0, -2694685.473);  // X [m]
MatSetD(xyz, 0, 1, -4293642.366);  // Y [m]
MatSetD(xyz, 0, 2,  3857878.924);  // Z [m]

mat_t *rot = Xyz2Rot(xyz);
if (rot) {
    printf("ECEF → ENU 회전행렬:\n");
    for (int i = 0; i < 3; i++) {
        printf("[");
        for (int j = 0; j < 3; j++) {
            printf("%8.5f", MatGetD(rot, i, j));
            if (j < 2) printf(", ");
        }
        printf("]\n");
    }

    // 회전행렬의 직교성 검증 (R * R^T = I)
    mat_t *rot_t = MatTrans(rot);
    mat_t *identity = MatMul(rot, rot_t);
    printf("직교성 검증 (대각선 원소들): %.6f, %.6f, %.6f\n",
           MatGetD(identity, 0, 0), MatGetD(identity, 1, 1), MatGetD(identity, 2, 2));

    FreeMat(rot_t); FreeMat(identity);
}

FreeMat(xyz); FreeMat(rot);
```

</details>

#### 5.5.4 Xyz2Enu() - ECEF → ENU 변환
<details>
<summary>상세 설명</summary>

**목적**: ECEF 좌표를 지역 ENU 좌표로 변환

**입력**:
- `const mat_t *xyz`: ECEF 좌표 (1×3) [m]
- `const mat_t *org`: 원점 ECEF 좌표 (1×3) [m]

**출력**:
- `mat_t *enu`: ENU 지역좌표 (1×3) [m] (오류 시 NULL)

**함수 로직**:
- 원점 기준 ECEF 벡터 계산
- 회전행렬을 통한 ENU 변환
- East, North, Up 성분 반환

**사용 예시**:
```c
// 수신기 위치 (원점)
mat_t *origin = Mat(1, 3, DOUBLE);
MatSetD(origin, 0, 0, -2694685.473);  // X [m]
MatSetD(origin, 0, 1, -4293642.366);  // Y [m]
MatSetD(origin, 0, 2,  3857878.924);  // Z [m]

// 위성 위치
mat_t *sat_pos = Mat(1, 3, DOUBLE);
MatSetD(sat_pos, 0, 0, 12611434.0);   // X [m]
MatSetD(sat_pos, 0, 1, -13413103.0);  // Y [m]
MatSetD(sat_pos, 0, 2, 19062913.0);   // Z [m]

// ECEF → ENU 변환
mat_t *enu = Xyz2Enu(sat_pos, origin);
if (enu) {
    double east = MatGetD(enu, 0, 0);
    double north = MatGetD(enu, 0, 1);
    double up = MatGetD(enu, 0, 2);

    printf("ENU 좌표: E=%.1f, N=%.1f, U=%.1f [m]\n", east, north, up);
    printf("수평거리: %.1f m\n", sqrt(east*east + north*north));
    printf("3차원거리: %.1f m\n", sqrt(east*east + north*north + up*up));

    // 역변환으로 검증
    mat_t *xyz_back = Enu2Xyz(enu, origin);
    if (xyz_back) {
        printf("역변환 검증: X=%.3f, Y=%.3f, Z=%.3f [m]\n",
               MatGetD(xyz_back, 0, 0), MatGetD(xyz_back, 0, 1), MatGetD(xyz_back, 0, 2));
        FreeMat(xyz_back);
    }
}

FreeMat(origin); FreeMat(sat_pos); FreeMat(enu);
```

</details>

#### 5.5.5 Enu2Xyz() - ENU → ECEF 변환
<details>
<summary>상세 설명</summary>

**목적**: 지역 ENU 좌표를 ECEF 좌표로 변환

**입력**:
- `const mat_t *enu`: ENU 지역좌표 (1×3) [m]
- `const mat_t *org`: 원점 ECEF 좌표 (1×3) [m]

**출력**:
- `mat_t *xyz`: ECEF 좌표 (1×3) [m] (오류 시 NULL)

**함수 로직**:
- 회전행렬의 전치행렬 사용
- ENU → ECEF 역변환 수행
- 원점 좌표 더하여 최종 ECEF 계산

**사용 예시**:
```c
// 원점 ECEF 좌표 (수신기 위치)
mat_t *origin = Mat(1, 3, DOUBLE);
MatSetD(origin, 0, 0, -2694685.473);  // X [m]
MatSetD(origin, 0, 1, -4293642.366);  // Y [m]
MatSetD(origin, 0, 2,  3857878.924);  // Z [m]

// ENU 지역좌표 (상대 위치)
mat_t *enu = Mat(1, 3, DOUBLE);
MatSetD(enu, 0, 0, 1000.0);   // East [m]
MatSetD(enu, 0, 1, 2000.0);   // North [m]
MatSetD(enu, 0, 2, 500.0);    // Up [m]

// ENU → ECEF 변환
mat_t *xyz = Enu2Xyz(enu, origin);
if (xyz) {
    printf("ENU 좌표: E=%.1f, N=%.1f, U=%.1f [m]\n",
           MatGetD(enu, 0, 0), MatGetD(enu, 0, 1), MatGetD(enu, 0, 2));
    printf("ECEF 좌표: X=%.3f, Y=%.3f, Z=%.3f [m]\n",
           MatGetD(xyz, 0, 0), MatGetD(xyz, 0, 1), MatGetD(xyz, 0, 2));

    // 원점으로부터의 거리 확인
    double dx = MatGetD(xyz, 0, 0) - MatGetD(origin, 0, 0);
    double dy = MatGetD(xyz, 0, 1) - MatGetD(origin, 0, 1);
    double dz = MatGetD(xyz, 0, 2) - MatGetD(origin, 0, 2);
    double dist = sqrt(dx*dx + dy*dy + dz*dz);
    printf("원점으로부터 거리: %.1f m\n", dist);

    // 역변환으로 검증
    mat_t *enu_back = Xyz2Enu(xyz, origin);
    if (enu_back) {
        printf("역변환 검증: E=%.1f, N=%.1f, U=%.1f [m]\n",
               MatGetD(enu_back, 0, 0), MatGetD(enu_back, 0, 1), MatGetD(enu_back, 0, 2));
        FreeMat(enu_back);
    }
}

FreeMat(origin); FreeMat(enu); FreeMat(xyz);
```

</details>

#### 5.5.6 SatAzEl() - 위성 방위각/고도각 계산
<details>
<summary>상세 설명</summary>

**목적**: 수신기에서 본 위성의 방위각과 고도각 계산

**입력**:
- `const mat_t *rs`: 위성 위치 (1×3) [m]
- `const mat_t *rr`: 수신기 위치 (1×3) [m]

**출력**:
- `mat_t *azel`: 방위각/고도각 (1×2) [rad, rad] (오류 시 NULL)

**GNSS 수식**:
ENU 지역좌표에서 방위각/고도각 계산:
$$\text{방위각: } az = \arctan2(E, N)$$
$$\text{고도각: } el = \arctan2(U, \sqrt{E^2 + N^2})$$

여기서 ENU 좌표는:
$$\begin{bmatrix} E \\ N \\ U \end{bmatrix} = \mathbf{R} \begin{bmatrix} x^s - x_r \\ y^s - y_r \\ z^s - z_r \end{bmatrix}$$

회전행렬 $\mathbf{R}$:
$$\mathbf{R} = \begin{bmatrix}
-\sin\lambda & \cos\lambda & 0 \\
-\sin\phi\cos\lambda & -\sin\phi\sin\lambda & \cos\phi \\
\cos\phi\cos\lambda & \cos\phi\sin\lambda & \sin\phi
\end{bmatrix}$$

**함수 로직**:
- 수신기 위치가 원점인 경우: 방위각 0, 고도각 π/2 반환
- ENU 좌표로 변환 후 구면좌표 계산
- 방위각: `atan2(E, N)` → 0~2π 범위 정규화
- 고도각: `atan2(U, sqrt(E²+N²))` → -π/2~π/2 범위

**사용 예시**:
```c
// 위성 위치 (ECEF)
mat_t *sat_pos = Mat(1, 3, DOUBLE);
MatSetD(sat_pos, 0, 0, 12611434.0);  // X [m]
MatSetD(sat_pos, 0, 1, -13413103.0); // Y [m]
MatSetD(sat_pos, 0, 2, 19062913.0);  // Z [m]

// 수신기 위치 (ECEF)
mat_t *rcv_pos = Mat(1, 3, DOUBLE);
MatSetD(rcv_pos, 0, 0, -2694685.473); // X [m]
MatSetD(rcv_pos, 0, 1, -4293642.366); // Y [m]
MatSetD(rcv_pos, 0, 2, 3857878.924);  // Z [m]

// 방위각/고도각 계산
mat_t *azel = SatAzEl(sat_pos, rcv_pos);
if (azel) {
    double az_deg = MatGetD(azel, 0, 0) * R2D;  // 방위각 [도]
    double el_deg = MatGetD(azel, 0, 1) * R2D;  // 고도각 [도]

    printf("위성 방위각: %.1f°, 고도각: %.1f°\n", az_deg, el_deg);

    // 고도각 마스크 확인
    if (el_deg > 15.0) {
        printf("위성이 가시 범위에 있습니다.\n");
    } else {
        printf("위성이 고도각 마스크 아래에 있습니다.\n");
    }
}

FreeMat(sat_pos); FreeMat(rcv_pos); FreeMat(azel);
```

</details>

#### 5.5.7 GeoDist() - 기하거리 + Sagnac 보정
<details>
<summary>상세 설명</summary>

**목적**: 위성-수신기 간 기하거리 및 Sagnac 효과 보정 계산

**입력**:
- `const mat_t *rs`: 위성 위치 (1×3) [m]
- `const mat_t *rr`: 수신기 위치 (1×3) [m]
- `mat_t *e`: (선택적) 시선벡터 (1×3) (ECEF)

**출력**:
- `double`: 보정된 거리 [m] (오류 시 0.0)

**GNSS 수식**:
Sagnac 효과 보정이 적용된 기하거리:
$$\rho = \sqrt{(x^s - x_r)^2 + (y^s - y_r)^2 + (z^s - z_r)^2} + \frac{\omega_e}{c}(x^s y_r - x_r y^s)$$

여기서:
- $\boldsymbol{r}^s = [x^s, y^s, z^s]^T$: 위성 위치벡터 [m]
- $\boldsymbol{r}_r = [x_r, y_r, z_r]^T$: 수신기 위치벡터 [m]
- $\omega_e = 7.2921151467 \times 10^{-5}$ rad/s (지구 자전각속도)
- $c = 299792458.0$ m/s (광속)

시선벡터 (단위벡터):
$$\boldsymbol{e} = \frac{\boldsymbol{r}^s - \boldsymbol{r}_r}{|\boldsymbol{r}^s - \boldsymbol{r}_r|}$$

**함수 로직**:
- 유클리드 거리 계산
- Sagnac 효과 보정 적용
- 선택적 단위벡터 계산

**사용 예시**:
```c
// 위성 위치 (ECEF)
mat_t *sat_pos = Mat(1, 3, DOUBLE);
MatSetD(sat_pos, 0, 0, 12611434.0);   // X [m]
MatSetD(sat_pos, 0, 1, -13413103.0);  // Y [m]
MatSetD(sat_pos, 0, 2, 19062913.0);   // Z [m]

// 수신기 위치 (ECEF)
mat_t *rcv_pos = Mat(1, 3, DOUBLE);
MatSetD(rcv_pos, 0, 0, -2694685.473); // X [m]
MatSetD(rcv_pos, 0, 1, -4293642.366); // Y [m]
MatSetD(rcv_pos, 0, 2,  3857878.924); // Z [m]

// 시선벡터 저장용
mat_t *los_vector = Mat(1, 3, DOUBLE);

// 기하거리 + Sagnac 보정 계산
double range = GeoDist(sat_pos, rcv_pos, los_vector);
if (range > 0.0) {
    printf("보정된 거리: %.3f km\n", range / 1000.0);

    // 시선벡터 확인 (단위벡터)
    double ex = MatGetD(los_vector, 0, 0);
    double ey = MatGetD(los_vector, 0, 1);
    double ez = MatGetD(los_vector, 0, 2);
    double norm = sqrt(ex*ex + ey*ey + ez*ez);
    printf("시선벡터: [%.6f, %.6f, %.6f] (크기: %.6f)\n", ex, ey, ez, norm);

    // 유클리드 거리와 비교
    double dx = MatGetD(sat_pos, 0, 0) - MatGetD(rcv_pos, 0, 0);
    double dy = MatGetD(sat_pos, 0, 1) - MatGetD(rcv_pos, 0, 1);
    double dz = MatGetD(sat_pos, 0, 2) - MatGetD(rcv_pos, 0, 2);
    double euclidean = sqrt(dx*dx + dy*dy + dz*dz);

    printf("유클리드 거리: %.3f km\n", euclidean / 1000.0);
    printf("Sagnac 보정: %.3f m\n", range - euclidean);
}

FreeMat(sat_pos); FreeMat(rcv_pos); FreeMat(los_vector);
```

</details>

### 5.6 GNSS 분석 함수

#### 5.6.1 Dops() - DOP 값 계산
<details>
<summary>상세 설명</summary>

**목적**: 위성 기하를 기반으로 DOP(Dilution of Precision) 값들 계산

**입력**:
- `const mat_t *azels`: 방위각/고도각 배열 (n×2) [rad, rad]
- `double elmask`: 고도각 마스크 [rad]

**출력**:
- `mat_t *dops`: DOP 값들 (1×5) [GDOP, PDOP, HDOP, VDOP, TDOP] (오류 시 NULL)

**GNSS 수식**:
설계행렬 $\mathbf{H}$를 다음과 같이 구성:
$$\mathbf{H} = \begin{bmatrix}
\sin(az_1)\cos(el_1) & \cos(az_1)\cos(el_1) & \sin(el_1) & 1 \\
\sin(az_2)\cos(el_2) & \cos(az_2)\cos(el_2) & \sin(el_2) & 1 \\
\vdots & \vdots & \vdots & \vdots \\
\sin(az_n)\cos(el_n) & \cos(az_n)\cos(el_n) & \sin(el_n) & 1
\end{bmatrix}$$

공분산 행렬: $\mathbf{Q} = (\mathbf{H}^T\mathbf{H})^{-1}$

DOP 값들:
- $\text{GDOP} = \sqrt{q_{11} + q_{22} + q_{33} + q_{44}}$ (기하학적)
- $\text{PDOP} = \sqrt{q_{11} + q_{22} + q_{33}}$ (위치)
- $\text{HDOP} = \sqrt{q_{11} + q_{22}}$ (수평)
- $\text{VDOP} = \sqrt{q_{33}}$ (수직)
- $\text{TDOP} = \sqrt{q_{44}}$ (시간)

**함수 로직**:
- 고도각 마스크 적용하여 가시 위성 선별 (el > elmask)
- 설계행렬 H 구성 및 논리 인덱싱으로 마스크된 위성 제거
- Lsq 함수로 공분산 행렬 Q 계산
- Q의 대각선 원소들로 DOP 값들 직접 계산

**사용 예시**:
```c
// 8개 위성의 방위각/고도각 데이터 (예시)
mat_t *azels = Mat(8, 2, DOUBLE);
double azel_data[][2] = {
    {45.0 * D2R, 30.0 * D2R},   // 위성 1: 방위각 45°, 고도각 30°
    {135.0 * D2R, 45.0 * D2R},  // 위성 2: 방위각 135°, 고도각 45°
    {225.0 * D2R, 60.0 * D2R},  // 위성 3: 방위각 225°, 고도각 60°
    {315.0 * D2R, 35.0 * D2R},  // 위성 4: 방위각 315°, 고도각 35°
    {90.0 * D2R, 75.0 * D2R},   // 위성 5: 방위각 90°, 고도각 75°
    {180.0 * D2R, 25.0 * D2R},  // 위성 6: 방위각 180°, 고도각 25°
    {270.0 * D2R, 40.0 * D2R},  // 위성 7: 방위각 270°, 고도각 40°
    {0.0 * D2R, 85.0 * D2R}     // 위성 8: 방위각 0°, 고도각 85°
};

for (int i = 0; i < 8; i++) {
    MatSetD(azels, i, 0, azel_data[i][0]);  // 방위각 [rad]
    MatSetD(azels, i, 1, azel_data[i][1]);  // 고도각 [rad]
}

// DOP 계산 (고도각 마스크 15도)
double elmask = 15.0 * D2R;
mat_t *dops = Dops(azels, elmask);
if (dops) {
    double gdop = MatGetD(dops, 0, 0);  // GDOP
    double pdop = MatGetD(dops, 0, 1);  // PDOP
    double hdop = MatGetD(dops, 0, 2);  // HDOP
    double vdop = MatGetD(dops, 0, 3);  // VDOP
    double tdop = MatGetD(dops, 0, 4);  // TDOP

    printf("DOP 값들:\n");
    printf("  GDOP: %.2f (기하학적 정밀도)\n", gdop);
    printf("  PDOP: %.2f (위치 정밀도)\n", pdop);
    printf("  HDOP: %.2f (수평 정밀도)\n", hdop);
    printf("  VDOP: %.2f (수직 정밀도)\n", vdop);
    printf("  TDOP: %.2f (시간 정밀도)\n", tdop);

    // DOP 품질 평가
    if (hdop < 1.0) printf("수평 정밀도: 우수\n");
    else if (hdop < 2.0) printf("수평 정밀도: 양호\n");
    else if (hdop < 5.0) printf("수평 정밀도: 보통\n");
    else printf("수평 정밀도: 불량\n");
}

FreeMat(azels); FreeMat(dops);
```

</details>

### 5.7 인라인 유틸리티 함수

#### 5.7.1 SQR() - 제곱 계산
<details>
<summary>상세 설명</summary>

**목적**: double 값의 제곱 계산

**입력**:
- `double x`: 제곱할 값

**출력**:
- `double`: x의 제곱 (x²)

**함수 로직**: 단순 곱셈 연산 (x * x)

**사용 예시**:
```c
// 기본 제곱 계산
double x = 5.0;
double x_squared = SQR(x);
printf("%.1f의 제곱: %.1f\n", x, x_squared);

// 거리 계산에서 활용
double dx = 100.0, dy = 200.0, dz = 50.0;
double distance = sqrt(SQR(dx) + SQR(dy) + SQR(dz));
printf("3차원 거리: %.1f m\n", distance);

// 분산 계산에서 활용
double values[] = {1.0, 2.0, 3.0, 4.0, 5.0};
double mean = 3.0;
double variance = 0.0;
for (int i = 0; i < 5; i++) {
    variance += SQR(values[i] - mean);
}
variance /= 5.0;
printf("분산: %.2f\n", variance);
```

</details>

#### 5.7.2 Sys2Str() - 시스템 인덱스를 문자로 변환
<details>
<summary>상세 설명</summary>

**목적**: GNSS 시스템 인덱스를 시스템 문자로 변환

**입력**:
- `int sys`: 시스템 인덱스 (1~NSYS)

**출력**:
- `char`: 시스템 문자 ('G', 'R', 'E', 'C', 'J', 'I', 'S'), 오류 시 '\0'

**함수 로직**:
1. 활성화된 시스템별로 순차 확인
2. 해당 인덱스에 맞는 시스템 문자 반환

**사용 예시**:
```c
// 모든 시스템 인덱스를 문자로 변환
printf("GNSS 시스템 매핑:\n");
for (int sys = 1; sys <= NSYS; sys++) {
    char sys_char = Sys2Str(sys);
    if (sys_char != '\0') {
        printf("시스템 %d → '%c'\n", sys, sys_char);
    }
}

// 위성 문자열 생성에서 활용
int sys = 1;  // GPS
int prn = 5;
char sys_char = Sys2Str(sys);
if (sys_char != '\0') {
    printf("위성 문자열: %c%02d\n", sys_char, prn);  // "G05"
}

// 오류 처리 예시
int invalid_sys = 99;
char result = Sys2Str(invalid_sys);
if (result == '\0') {
    printf("잘못된 시스템 인덱스: %d\n", invalid_sys);
}
```

</details>

#### 5.7.3 Str2Sys() - 문자를 시스템 인덱스로 변환
<details>
<summary>상세 설명</summary>

**목적**: 시스템 문자를 GNSS 시스템 인덱스로 변환

**입력**:
- `char str`: 시스템 문자 ('G', 'R', 'E', 'C', 'J', 'I', 'S')

**출력**:
- `int`: 시스템 인덱스 (1~NSYS), 오류 시 0

**함수 로직**:
1. 입력 문자와 시스템 문자 매칭
2. 활성화된 시스템만 고려하여 인덱스 반환

**사용 예시**:
```c
// 모든 시스템 문자를 인덱스로 변환
char sys_chars[] = {'G', 'R', 'E', 'C', 'J', 'I', 'S'};
char *sys_names[] = {"GPS", "GLONASS", "Galileo", "BeiDou", "QZSS", "IRNSS", "SBAS"};

printf("GNSS 시스템 변환:\n");
for (int i = 0; i < 7; i++) {
    int sys_idx = Str2Sys(sys_chars[i]);
    if (sys_idx > 0) {
        printf("'%c' (%s) → 시스템 인덱스 %d\n", sys_chars[i], sys_names[i], sys_idx);
    } else {
        printf("'%c' (%s) → 비활성화 또는 오류\n", sys_chars[i], sys_names[i]);
    }
}

// 위성 문자열 파싱에서 활용
char sat_str[] = "G05";
int sys = Str2Sys(sat_str[0]);  // 첫 번째 문자에서 시스템 추출
if (sys > 0) {
    int prn = atoi(&sat_str[1]);  // 나머지 문자에서 PRN 추출
    printf("위성 문자열 '%s' → 시스템: %d, PRN: %d\n", sat_str, sys, prn);
}

// 역변환으로 검증
char test_char = 'G';
int sys_idx = Str2Sys(test_char);
char char_back = Sys2Str(sys_idx);
printf("변환 검증: '%c' → %d → '%c'\n", test_char, sys_idx, char_back);
```

</details>

---

## 6. 사용 예시

### 6.1 위성 인덱스 변환
```c
// PRN을 위성 인덱스로 변환
int gps_sat = Prn2Sat(1, 5);       // GPS PRN 5 → 위성 인덱스
int glo_sat = Prn2Sat(2, 1);       // GLONASS PRN 1 → 위성 인덱스

// 위성 인덱스를 PRN으로 변환
int prn;
int sys = Sat2Prn(gps_sat, &prn);  // 위성 인덱스 → GPS PRN 5

// 문자열 변환
satStr_t sat_str = {"G05"};
int sat = Str2Sat(sat_str);         // "G05" → 위성 인덱스
satStr_t str = Sat2Str(sat);        // 위성 인덱스 → "G05"

printf("GPS PRN 5 위성 인덱스: %d\n", gps_sat);
printf("위성 문자열: %s\n", str.str);
```

### 6.2 시간 변환
```c
// 현재 시간 조회
double current_time = TimeGet();
printf("현재 UNIX Time: %.3f\n", current_time);

// 달력 시간 변환
cal_t cal = {2024, 12, 25, 15, 30, 45.123};
double unix_time = Cal2Time(cal);
cal_t cal_back = Time2Cal(unix_time);

// GPS 시간 변환
int gps_week = 2296;
double tow = 345678.5;
double gps_time = Gpst2Time(gps_week, tow);
int week_back;
double tow_back = Time2Gpst(gps_time, &week_back);

printf("GPS 주차 %d, 주내초 %.1f → UNIX Time: %.3f\n",
       gps_week, tow, gps_time);
```

### 6.3 문자열 시간 처리
```c
// 문자열을 달력으로 변환
calStr_t time_str = {"2024/12/25 15:30:45.123"};
cal_t cal = Str2Cal(time_str);

// 달력을 문자열로 변환 (소수점 3자리)
calStr_t str_back = Cal2Str(cal, 3);
printf("변환된 시간: %s\n", str_back.str);

// 연중 일자 계산
double time = Cal2Time(cal);
int doy = Time2Doy(time);
printf("2024년 12월 25일은 연중 %d일째입니다.\n", doy);
```

### 6.4 GLONASS FCN 관리
```c
// GLONASS FCN 설정
SetFcn(1, -7);  // GLONASS PRN 1에 FCN -7 설정
SetFcn(2, -6);  // GLONASS PRN 2에 FCN -6 설정

// FCN 조회
int fcn;
if (GetFcn(1, &fcn)) {
    printf("GLONASS PRN 1의 FCN: %d\n", fcn);
}

// 기본 FCN 설정
SetDefaultFcn();
printf("모든 GLONASS 위성에 기본 FCN 설정 완료\n");
```

### 6.5 네비게이션 구조체 관리
```c
// 네비게이션 구조체 초기화
nav_t nav;
InitNav(&nav);

// 네비게이션 데이터 사용...
// (궤도력 로드, 관측 데이터 처리 등)

// 메모리 해제
FreeNav(&nav);
printf("네비게이션 구조체 메모리 해제 완료\n");
```

### 6.6 시간 체계 간 변환
```c
// GPS Time ↔ UTC 변환 (윤초 적용)
double gps_time = TimeGet();
double utc_time = Gpst2Utc(gps_time);
double gps_back = Utc2Gpst(utc_time);

printf("GPS Time: %.3f\n", gps_time);
printf("UTC Time: %.3f\n", utc_time);
printf("차이 (윤초): %.1f초\n", gps_time - utc_time);

// GPS Time ↔ BeiDou Time 변환
double bdt_time = Gpst2Bdt(gps_time);
double gps_from_bdt = Bdt2Gpst(bdt_time);

printf("BeiDou Time: %.3f\n", bdt_time);
printf("차이: %.1f초\n", gps_time - bdt_time);
```

### 6.7 GNSS 좌표 변환
```c
// ECEF → 지리좌표 변환
mat_t *xyz = Mat(1, 3, DOUBLE);
MatSetD(xyz, 0, 0, -2694685.473);  // X [m]
MatSetD(xyz, 0, 1, -4293642.366);  // Y [m]
MatSetD(xyz, 0, 2,  3857878.924);  // Z [m]

mat_t *llh = Xyz2Llh(xyz);
if (llh) {
    double lat_deg = MatGetD(llh, 0, 0) * R2D;
    double lon_deg = MatGetD(llh, 0, 1) * R2D;
    double hgt_m   = MatGetD(llh, 0, 2);

    printf("위도: %.6f°, 경도: %.6f°, 높이: %.3fm\n",
           lat_deg, lon_deg, hgt_m);
}

// 지리좌표 → ECEF 역변환
mat_t *xyz_back = Llh2Xyz(llh);

FreeMat(xyz); FreeMat(llh); FreeMat(xyz_back);
```

### 6.8 ENU 지역좌표 변환
```c
// 수신기 위치 (원점)
mat_t *origin = Mat(1, 3, DOUBLE);
MatSetD(origin, 0, 0, -2694685.473);
MatSetD(origin, 0, 1, -4293642.366);
MatSetD(origin, 0, 2,  3857878.924);

// 위성 위치
mat_t *sat_pos = Mat(1, 3, DOUBLE);
MatSetD(sat_pos, 0, 0, 12611434.0);
MatSetD(sat_pos, 0, 1, -13413103.0);
MatSetD(sat_pos, 0, 2, 19062913.0);

// ECEF → ENU 변환
mat_t *enu = Xyz2Enu(sat_pos, origin);
if (enu) {
    printf("ENU: E=%.1f, N=%.1f, U=%.1f [m]\n",
           MatGetD(enu, 0, 0), MatGetD(enu, 0, 1), MatGetD(enu, 0, 2));
}

FreeMat(origin); FreeMat(sat_pos); FreeMat(enu);
```

### 6.9 위성 방위각/고도각 계산
```c
// 위성-수신기 방위각/고도각 계산
mat_t *azel = SatAzEl(sat_pos, origin);
if (azel) {
    double az_deg = MatGetD(azel, 0, 0) * R2D;
    double el_deg = MatGetD(azel, 0, 1) * R2D;

    printf("방위각: %.1f°, 고도각: %.1f°\n", az_deg, el_deg);
}

// 기하거리 + Sagnac 보정
mat_t *los_vector = Mat(1, 3, DOUBLE);
double range = GeoDist(sat_pos, origin, los_vector);
printf("보정된 거리: %.3f km\n", range / 1000.0);

FreeMat(azel); FreeMat(los_vector);
```

### 6.10 DOP 값 계산
```c
// 다수 위성의 방위각/고도각 (예: 8개 위성)
mat_t *azels = Mat(8, 2, DOUBLE);
// ... 방위각/고도각 값 설정 ...

// DOP 계산 (고도각 마스크 15도)
double elmask = 15.0 * D2R;
mat_t *dops = Dops(azels, elmask);
if (dops) {
    // DOP 값들이 직접 계산되어 반환됨
    double gdop = MatGetD(dops, 0, 0);  // GDOP
    double pdop = MatGetD(dops, 0, 1);  // PDOP
    double hdop = MatGetD(dops, 0, 2);  // HDOP
    double vdop = MatGetD(dops, 0, 3);  // VDOP
    double tdop = MatGetD(dops, 0, 4);  // TDOP

    printf("GDOP: %.2f, PDOP: %.2f, HDOP: %.2f, VDOP: %.2f, TDOP: %.2f\n",
           gdop, pdop, hdop, vdop, tdop);
}

FreeMat(azels); FreeMat(dops);
```

---

## 7. 성능 특성

### 7.1 메모리 효율성
- **정적 테이블**: 윤초, FCN, 시스템 베이스 인덱스 등을 정적 배열로 관리
- **구조체 최적화**: 필요한 데이터만 포함하도록 구조체 설계
- **동적 할당 최소화**: 필수적인 경우만 동적 메모리 사용

### 7.2 연산 성능
- **O(1) 위성 변환**: 베이스 인덱스 테이블을 통한 직접 계산
- **O(1) 시간 변환**: 수학적 공식 기반 직접 변환
- **O(log n) 윤초 검색**: 이분 탐색으로 윤초 테이블 검색 최적화
- **인라인 함수**: 자주 사용되는 함수들을 인라인으로 성능 향상

### 7.3 정확도 보장
- **윤초 정확성**: 최신 윤초 테이블 유지로 GPS/UTC 변환 정확성
- **FCN 관리**: GLONASS 주파수 정확성을 위한 FCN 완전 지원
- **시간 정밀도**: double 타입으로 마이크로초 수준 정밀도 제공

### 7.4 플랫폼 호환성
- **조건부 컴파일**: Windows와 Unix/Linux 플랫폼 자동 지원
- **표준 라이브러리**: C 표준 함수만 사용하여 이식성 보장
- **크로스 플랫폼**: 동일한 코드로 다양한 플랫폼 지원

### 7.5 확장성 및 유지보수성
- **모듈화 설계**: 기능별 함수 그룹화로 유지보수 용이
- **상수 기반**: 시스템별 상수 정의로 새 시스템 추가 간편
- **에러 처리**: 모든 함수에서 입력 검증 및 안전한 실패 처리
- **문서화**: 헤더 파일에 상세한 함수 설명 포함

---

**이 모듈은 GNSS 라이브러리의 핵심 기반 기능을 제공하는 필수 모듈입니다.**
