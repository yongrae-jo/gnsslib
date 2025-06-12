# ★ GNSS 공통 유틸리티 모듈 (common)

GNSS 라이브러리의 핵심 유틸리티 함수들을 제공하는 기반 모듈입니다.

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

### ◆ 위성 인덱스 시스템
GNSS 시스템별 PRN(Pseudo Random Noise) 번호를 연속적인 정수 인덱스로 매핑하여 배열 기반 효율적 접근을 제공합니다.

**통합 인덱스 계산 공식:**
$$\text{sat} = \text{BASE}[\text{sys}-1] + (\text{prn} - \text{MIN\_PRN}[\text{sys}] + 1)$$

### ◆ 시간 시스템 통합
다양한 GNSS 시간 체계를 UNIX Timestamp 기준으로 통합하여 상호 변환을 지원합니다.

**기준 시간**: UNIX Timestamp (1970/1/1 00:00:00 UTC)

**지원 시간 체계**:
- **UNIX Time**: 1970/1/1 00:00:00 UTC 기준
- **GPS Time**: 1980/1/6 00:00:00 UTC 기준
- **Galileo System Time**: 1999/8/22 00:00:00 UTC 기준
- **BeiDou Time**: 2006/1/1 00:00:00 UTC 기준

### ◆ 문자열 표현 통일
위성 식별자와 시간 정보를 표준화된 문자열 형식으로 표현하여 가독성과 호환성을 보장합니다.

**표준 형식**:
- 위성: `"CXX"` (C: 시스템 코드, XX: PRN 번호)
- 시간: `"YYYY/MM/DD HH:MM:SS.sss"`

---

## ▲ 데이터 타입 구조

```
common 모듈 타입 계층
├── 위성 관련
│   ├── satStr_t ──────────── 위성 문자열 구조체
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
└── GLONASS FCN
    └── FCN[NSAT_GLO] ─────── GLONASS 주파수 채널 번호
```

---

## ▲ 데이터 타입 목록

#### ◆ satStr_t - 위성 문자열 구조체
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

#### ◆ cal_t - 달력 시간 구조체
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

#### ◆ calStr_t - 달력 문자열 구조체
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

#### ◆ pcv_t - 안테나 위상 중심 보정 데이터
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

#### ◆ pcvs_t - 안테나 보정 데이터 집합
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

#### ◆ sta_t - 수신기 정보 구조체
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

#### ◆ nav_t - 통합 네비게이션 구조체
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

## ▲ 함수 구조

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
└── 인라인 유틸리티 함수
    ├── SQR() ───────────── 제곱 계산
    ├── Sys2Str() ───────── 시스템 인덱스 → 문자
    └── Str2Sys() ───────── 문자 → 시스템 인덱스
```

---

## ▲ 함수 목록

#### ◆ 초기화 및 메모리 관리 함수

##### ● InitNav() - 네비게이션 구조체 초기화
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

</details>

##### ● FreeNav() - 네비게이션 구조체 메모리 해제
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

</details>

#### ◆ GLONASS FCN 관리 함수

##### ● GetFcn() - GLONASS FCN 조회
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

</details>

##### ● SetFcn() - GLONASS FCN 설정
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

</details>

##### ● SetDefaultFcn() - 기본 FCN 설정
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

</details>

#### ◆ 위성 인덱스 변환 함수

##### ● Prn2Sat() - PRN을 위성 인덱스로 변환
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

</details>

##### ● Sat2Prn() - 위성 인덱스를 PRN으로 변환
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

</details>

##### ● Str2Sat() - 위성 문자열을 위성 인덱스로 변환
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

</details>

##### ● Sat2Str() - 위성 인덱스를 위성 문자열로 변환
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

</details>

#### ◆ 시간 변환 함수

##### ● Cal2Time() - 달력을 UNIX Time으로 변환
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

</details>

##### ● Time2Cal() - UNIX Time을 달력으로 변환
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

</details>

##### ● Gpst2Time() - GPS Time을 UNIX Time으로 변환
<details>
<summary>상세 설명</summary>

**목적**: GPS 주차와 주내 초를 UNIX Timestamp로 변환

**입력**:
- `int week`: GPS 주차
- `double tow`: 주내 초 (Time of Week)

**출력**:
- `double`: UNIX Timestamp (초)

**함수 로직**:
1. GPS 시간 기준점 (1980/1/6) 계산
2. 주차에서 일수 계산 (week × 7)
3. 주내 초를 초 단위로 변환
4. UNIX 기준으로 최종 시간 계산

</details>

##### ● Time2Gpst() - UNIX Time을 GPS Time으로 변환
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

##### ● Bdt2Time() - BeiDou Time을 UNIX Time으로 변환
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

##### ● Time2Bdt() - UNIX Time을 BeiDou Time으로 변환
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

</details>

##### ● TimeGet() - 현재 시간 조회
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

</details>

##### ● Gpst2Utc() - GPS Time을 UTC로 변환
<details>
<summary>상세 설명</summary>

**목적**: GPS Time을 UTC로 변환 (윤초 보정 적용)

**입력**:
- `double gpst`: GPS Time (UNIX Timestamp)

**출력**:
- `double`: UTC Time (UNIX Timestamp)

**함수 로직**:
1. 입력 GPS Time에 해당하는 윤초 조회
2. 윤초 테이블에서 적절한 윤초 값 찾기
3. GPS Time에 윤초 적용하여 UTC 계산

</details>

##### ● Utc2Gpst() - UTC를 GPS Time으로 변환
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

</details>

##### ● Gpst2Bdt() - GPS Time을 BeiDou Time으로 변환
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

</details>

##### ● Bdt2Gpst() - BeiDou Time을 GPS Time으로 변환
<details>
<summary>상세 설명</summary>

**목적**: BeiDou Time을 GPS Time으로 변환

**입력**:
- `double bdt`: BeiDou Time (UNIX Timestamp)

**출력**:
- `double`: GPS Time (UNIX Timestamp)

**함수 로직**:
1. BeiDou Time에 14초 가산하여 GPS Time 계산

</details>

##### ● Time2Doy() - 시간을 연중 일자로 변환
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

</details>

##### ● Str2Cal() - 문자열을 달력으로 변환
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

</details>

##### ● Cal2Str() - 달력을 문자열로 변환
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

</details>

#### ◆ 인라인 유틸리티 함수

##### ● SQR() - 제곱 계산
<details>
<summary>상세 설명</summary>

**목적**: double 값의 제곱 계산

**입력**:
- `double x`: 제곱할 값

**출력**:
- `double`: x의 제곱 (x²)

**함수 로직**: 단순 곱셈 연산 (x * x)

</details>

##### ● Sys2Str() - 시스템 인덱스를 문자로 변환
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

</details>

##### ● Str2Sys() - 문자를 시스템 인덱스로 변환
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

</details>

---

## ▲ 사용 예시

### ◆ 위성 인덱스 변환
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

### ◆ 시간 변환
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

### ◆ 문자열 시간 처리
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

### ◆ GLONASS FCN 관리
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

### ◆ 네비게이션 구조체 관리
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

### ◆ 시간 체계 간 변환
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

---

## ▲ 성능 특성

### ◆ 메모리 효율성
- **정적 테이블**: 윤초, FCN, 시스템 베이스 인덱스 등을 정적 배열로 관리
- **구조체 최적화**: 필요한 데이터만 포함하도록 구조체 설계
- **동적 할당 최소화**: 필수적인 경우만 동적 메모리 사용

### ◆ 연산 성능
- **O(1) 위성 변환**: 베이스 인덱스 테이블을 통한 직접 계산
- **O(1) 시간 변환**: 수학적 공식 기반 직접 변환
- **O(log n) 윤초 검색**: 이분 탐색으로 윤초 테이블 검색 최적화
- **인라인 함수**: 자주 사용되는 함수들을 인라인으로 성능 향상

### ◆ 정확도 보장
- **윤초 정확성**: 최신 윤초 테이블 유지로 GPS/UTC 변환 정확성
- **FCN 관리**: GLONASS 주파수 정확성을 위한 FCN 완전 지원
- **시간 정밀도**: double 타입으로 마이크로초 수준 정밀도 제공

### ◆ 플랫폼 호환성
- **조건부 컴파일**: Windows와 Unix/Linux 플랫폼 자동 지원
- **표준 라이브러리**: C 표준 함수만 사용하여 이식성 보장
- **크로스 플랫폼**: 동일한 코드로 다양한 플랫폼 지원

### ◆ 확장성 및 유지보수성
- **모듈화 설계**: 기능별 함수 그룹화로 유지보수 용이
- **상수 기반**: 시스템별 상수 정의로 새 시스템 추가 간편
- **에러 처리**: 모든 함수에서 입력 검증 및 안전한 실패 처리
- **문서화**: 헤더 파일에 상세한 함수 설명 포함

---

**■ 이 모듈은 GNSS 라이브러리의 핵심 기반 기능을 제공하는 필수 모듈입니다.**
