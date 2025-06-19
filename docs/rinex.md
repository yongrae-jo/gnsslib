# RINEX 파일 처리 모듈 (rinex)

GNSS 관측 데이터의 표준 교환 형식인 RINEX(Receiver Independent Exchange Format) 파일을 읽고 처리하는 모듈입니다. 관측(Observation) 파일과 항법(Navigation) 파일을 모두 지원합니다.

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

### 1.1 RINEX 파일 타입
본 모듈은 두 가지 주요 RINEX 파일 타입을 처리합니다.
- **관측 파일 (Observation)**: 위성으로부터 수신한 의사거리, 반송파 위상 등의 원시 측정치를 포함합니다.
- **항법 파일 (Navigation)**: 위성의 정확한 위치를 계산하는 데 필요한 궤도력 정보를 포함합니다.

### 1.2 RINEX 버전 지원
본 모듈은 **RINEX v2.xx 및 v3.xx** 형식을 자동으로 감지하고 처리합니다.

**버전별 특징**:
- **RINEX v2**: `# / TYPES OF OBSERV` 헤더로 관측 타입 정의
- **RINEX v3**: `SYS / # / OBS TYPES` 헤더로 시스템별 관측 타입 정의

### 1.3 항법 데이터 파싱의 견고성
RINEX 항법 파일(v2 및 v3)에서는 Fortran에서 사용되던 지수 표기법 `D` 또는 `d` (예: `0.12345D+01`, `-1.23d-05`)가 혼용됩니다. C 표준 라이브러리는 이 형식을 직접 지원하지 않아 파싱 오류를 유발할 수 있습니다.

본 모듈의 `ReadRnxNav` 함수는 다음과 같은 방식으로 이 문제를 해결하여 파싱의 견고성을 확보했습니다:
1.  **헤더 파싱**: ION ALPHA, ION BETA, IONOSPHERIC CORR 라인에서 D/d를 E로 치환
2.  **본문 파싱**: 궤도력 클록 파라미터와 궤도 파라미터 필드에서 D/d를 E로 치환
3.  **안전한 변환**: `strtod` 함수를 사용하여 double 타입으로 변환

이를 통해 RINEX v2와 v3 모든 버전의 항법 파일을 안정적으로 처리할 수 있습니다.

### 1.4 관측 타입 변환 시스템
RINEX v2의 2글자 관측 코드를 v3의 3글자 형식으로 내부 변환합니다.
- **v2 → v3 변환**: `P1` → `C1W`, `L1` → `L1C` 등
- **시스템별 매핑**: GPS, GLONASS, Galileo, BDS 시스템별 차별화

### 1.5 메모리 관리 체계
- **버퍼 관리**: `buffer_t` 구조체를 통한 파일 전체 메모리 로딩
- **동적 할당**: 관측 데이터 개수에 따른 메모리 동적 할당
- **안전한 해제**: `FreeRnxObs()` 함수로 메모리 누수 방지

### 1.6 데이터 처리 흐름도

**관측 파일 처리**:
```
[RINEX OBS 파일 (텍스트)]
         ↓ ReadRnxObsHeader() + ReadRnxObsBody()
[rnxObs_t 구조체]
    ├── header (rnxObsHeader_t): 파일 메타정보
    └── body[] (rnxObsBody_t): 원시 관측 데이터 배열
         ↓ ArrangeObs()
[obss_t 구조체]
    └── obs[] (obs_t): GNSS 라이브러리 표준 형식
```

**항법 파일 처리**:
```
[RINEX NAV 파일 (텍스트)]
         ↓ ReadRnxNav()
[nav_t 구조체]
    └── ephs[] (ephs_t): 위성별 궤도력 데이터
```

---

## 2. 데이터 타입 구조

```
rinex 모듈 타입 계층
├── 관측 파일 관련 구조체
│   ├── rnxObsHeader_t (static struct)
│   │   ├── ver ────────────────── RINEX 버전 번호 (double)
│   │   ├── sys ────────────────── 시스템 문자 (char)
│   │   ├── obsType[NSYS][MAX_OBSTYPE][4] ─── 시스템별 관측 타입 배열
│   │   ├── nObsType[NSYS] ────── 시스템별 관측 타입 개수
│   │   └── sta ────────────────── 관측소 정보 (sta_t)
│   ├── rnxObsBody_t (static struct)
│   │   ├── cal ────────────────── 관측 시간 (cal_t)
│   │   ├── satStr ─────────────── 위성 ID 문자열 (satStr_t)
│   │   ├── obs[MAX_OBSTYPE] ──── 관측값 배열 (double)
│   │   └── lli[MAX_OBSTYPE] ──── 신호 손실 지시자 (int)
│   └── rnxObs_t (static struct)
│       ├── header ─────────────── RINEX 관측 헤더 (rnxObsHeader_t)
│       ├── body ───────────────── 관측 데이터 배열 포인터 (rnxObsBody_t*)
│       └── n ───────────────────── 관측 데이터 개수 (int)
└── 항법 파일 관련 구조체
    ├── rnxNavHeader_t (static struct)
    │   ├── ver ────────────────── RINEX 버전 번호 (double)
    │   ├── sys ────────────────── 시스템 문자 (char)
    │   └── iono[NSYS][8] ─────── 전리층 파라미터 배열 (double)
    ├── rnxNavBody_t (static struct)
    │   ├── cal ────────────────── 시계 기준 시간 (cal_t)
    │   ├── satStr ─────────────── 위성 ID 문자열 (satStr_t)
    │   ├── clock[3] ───────────── 클록 파라미터 배열 (double)
    │   └── orbit[28] ──────────── 궤도 파라미터 배열 (double)
    └── rnxNav_t (static struct)
        ├── header ─────────────── RINEX 항법 헤더 (rnxNavHeader_t)
        ├── body ───────────────── 항법 데이터 배열 포인터 (rnxNavBody_t*)
        └── n ───────────────────── 항법 데이터 개수 (int)
```

---

## 3. 데이터 타입 목록

### 3.1 관측 파일 관련 구조체

#### 3.1.1 rnxObsHeader_t (static struct)
<details>
<summary>상세 설명</summary>

**목적**: RINEX 관측 파일 헤더 정보 저장

**정의**:
```c
typedef struct rnxObsHeader {
    double ver;                             // RINEX 버전 번호
    char   sys;                             // 시스템 문자
    char   obsType[NSYS][MAX_OBSTYPE][4];   // 관측 타입 배열
    int    nObsType[NSYS];                  // 시스템별 관측 타입 개수
    sta_t  sta;                             // 관측소 정보
} rnxObsHeader_t;
```

**특징**:
- RINEX v2/v3 헤더 정보 통합 저장
- 시스템별 관측 타입 분리 관리
- 관측소 메타데이터 포함

**사용**: `ReadRnxObsHeader()` 함수에서 파싱되어 저장

</details>

#### 3.1.2 rnxObsBody_t (static struct)
<details>
<summary>상세 설명</summary>

**목적**: 개별 관측 데이터 저장

**정의**:
```c
typedef struct rnxObsBody {
    cal_t    cal;                           // 관측 시간
    satStr_t satStr;                        // 위성 ID 문자열
    double   obs[MAX_OBSTYPE];              // 관측값 배열
    int      lli[MAX_OBSTYPE];              // 신호 손실 지시자
} rnxObsBody_t;
```

**특징**:
- 시간, 위성, 관측값을 하나의 구조체로 관리
- 최대 32개 관측 타입 지원
- LLI(Loss of Lock Indicator) 정보 포함

**사용**: RINEX 파일의 각 관측 epoch 데이터를 저장

</details>

#### 3.1.3 rnxObs_t (static struct)
<details>
<summary>상세 설명</summary>

**목적**: 전체 RINEX 관측 파일 데이터 통합 관리

**정의**:
```c
typedef struct rnxObs {
    rnxObsHeader_t header;                  // RINEX 관측 헤더
    rnxObsBody_t   *body;                   // 관측 데이터 배열
    int            n;                       // 관측 데이터 개수
} rnxObs_t;
```

**특징**:
- 헤더와 본문 데이터를 통합 관리
- 동적 메모리 할당으로 파일 크기에 적응
- 메모리 효율적인 구조체 설계

**메모리 관리**:
- `InitRnxObs()`: 구조체 초기화
- `FreeRnxObs()`: 메모리 안전 해제

</details>

### 3.2 항법 파일 관련 구조체

#### 3.2.1 rnxNavHeader_t (static struct)
<details>
<summary>상세 설명</summary>

**목적**: RINEX 항법 파일 헤더 정보 저장

**정의**:
```c
typedef struct rnxNavHeader {
    double ver;                             // RINEX 버전 번호
    char   sys;                             // 시스템 문자
    double iono[NSYS][8];                   // 전리층 파라미터 배열
} rnxNavHeader_t;
```

**특징**:
- RINEX v2/v3 항법 헤더 정보 통합 저장
- 시스템별 전리층 보정 파라미터 관리
- ION ALPHA/BETA, IONOSPHERIC CORR 정보 포함

**사용**: `ReadRnxNavHeader()` 함수에서 파싱되어 저장

</details>

#### 3.2.2 rnxNavBody_t (static struct)
<details>
<summary>상세 설명</summary>

**목적**: 개별 위성 궤도력 데이터 저장

**정의**:
```c
typedef struct rnxNavBody {
    cal_t    cal;                           // 시계 기준 시간 (Toc)
    satStr_t satStr;                        // 위성 ID 문자열
    double   clock[3];                      // 클록 파라미터 배열
    double   orbit[28];                     // 궤도 파라미터 배열
} rnxNavBody_t;
```

**특징**:
- TOC (Time of Clock): 클록 보정 기준 시간
- 시스템별 클록 및 궤도 파라미터 통합 저장
- GPS, Galileo, GLONASS, BDS 등 모든 시스템 지원

**사용**: RINEX 파일의 각 궤도력 블록 데이터를 저장

</details>

#### 3.2.3 rnxNav_t (static struct)
<details>
<summary>상세 설명</summary>

**목적**: 전체 RINEX 항법 파일 데이터 통합 관리

**정의**:
```c
typedef struct rnxNav {
    rnxNavHeader_t header;                  // RINEX 항법 헤더
    rnxNavBody_t   *body;                   // 항법 데이터 배열
    int            n;                       // 항법 데이터 개수
} rnxNav_t;
```

**특징**:
- 헤더와 본문 데이터를 통합 관리
- 동적 메모리 할당으로 파일 크기에 적응
- 메모리 효율적인 구조체 설계

**메모리 관리**:
- `InitRnxNav()`: 구조체 초기화
- `FreeRnxNav()`: 메모리 안전 해제

</details>

---

## 4. 함수 구조

```
rinex 모듈 함수 계층
├── 파일 검증
│   ├── IsRinexObs() ─────── RINEX 관측 파일명 검증
│   └── IsRinexNav() ─────── RINEX 항법 파일명 검증
├── 관측 파일 읽기
│   ├── ReadRnxObs() ─────── RINEX 관측 파일 읽기 (v2/v3 지원)
│   ├── 내부 헤더 처리 (static)
│   │   ├── ReadRnxObsHeader() ─ RINEX 헤더 파싱
│   │   └── ReadRnxObsBody() ─── RINEX 본문 읽기 총괄
│   └── 내부 본문 처리 (static)
│       ├── ReadRnxObsBodyV2() ─ RINEX v2 본문 파싱
│       └── ReadRnxObsBodyV3() ─ RINEX v3 본문 파싱
├── 항법 파일 읽기
│   ├── ReadRnxNav() ─────── RINEX 항법 파일 읽기 (v2/v3 지원)
│   └── 내부 처리 (static)
│       ├── ReadRnxNavHeader() ─ RINEX 항법 헤더 파싱
│       └── ReadRnxNavBody() ─── RINEX 항법 본문 파싱
├── 데이터 변환 (static)
│   ├── ArrangeObs() ─────── 관측 데이터 정렬 및 변환
│   ├── ArrangeNav() ─────── 항법 데이터 정렬 및 변환
│   └── ConvCode() ───────── v2→v3 관측 코드 변환
├── 구조체 관리 (static)
│   ├── InitRnxObs() ─────── RINEX 관측 구조체 초기화
│   ├── FreeRnxObs() ─────── RINEX 관측 구조체 메모리 해제
│   ├── InitRnxNav() ─────── RINEX 항법 구조체 초기화
│   └── FreeRnxNav() ─────── RINEX 항법 구조체 메모리 해제
├── 시간 조정 함수 (static)
│   ├── AdjWeek() ────────── 주 전환 시간 조정
│   └── AdjDay() ─────────── 일 전환 시간 조정
└── 유틸리티 함수 (static)
    ├── LineContains() ────── 문자열 포함 검사
    └── Deblank() ─────────── 공백 제거
```

---

## 5. 함수 목록

#### 5.1 공개 함수

##### 5.1.1 IsRinexObs() - RINEX 관측 파일명 검증
<details>
<summary>상세 설명</summary>

**목적**: 파일명이 RINEX 관측 파일 형식인지 검증

**입력**:
- `const char *filename`: 검증할 파일명

**출력**:
- `int`: RINEX 관측 파일이면 1, 아니면 0

**함수 로직**:
1. **압축 형식 검사**: `.YYo` 또는 `.YYO` 패턴 (YY는 2자리 연도)
2. **표준 형식 검사**: `.rnx` 확장자
3. **대소문자 구분**: 'o'와 'O' 모두 지원

**사용 예시**:
```c
const char *file1 = "YONS00KOR_R_20250010300_01H_01S_GO.25o";
const char *file2 = "obs_data.rnx";
printf("%s is RINEX OBS: %d\n", file1, IsRinexObs(file1)); // 1
printf("%s is RINEX OBS: %d\n", file2, IsRinexObs(file2)); // 1
```

</details>

##### 5.1.2 IsRinexNav() - RINEX 항법 파일명 검증
<details>
<summary>상세 설명</summary>

**목적**: 파일명이 RINEX 항법 파일 형식인지 검증

**입력**:
- `const char *filename`: 검증할 파일명

**출력**:
- `int`: RINEX 항법 파일이면 1, 아니면 0

**함수 로직**:
1.  **고전 형식 검사**: `.YYn`, `.YYg`, `.YYl` 패턴 (YY는 2자리 연도, 대/소문자 모두 허용)
2.  **표준 형식 검사**: `.rnx` 확장자 (소문자 고정)

> 위 로직은 `src/rinex.c`의 `IsRinexNav()` 구현과 동일하며, `.YYp/.nav` 확장자는 현재 코드에서 지원하지 않습니다.

**사용 예시**:
```c
const char *file1 = "brdc0010.25n"; // GPS 항법 파일 (.n)
const char *file2 = "brdc0010.25g"; // GLONASS 항법 파일 (.g)
const char *file3 = "GALI0010.25L"; // Galileo 항법 파일 (.L, 대문자 허용)
printf("%s is RINEX NAV: %d\n", file1, IsRinexNav(file1)); // 1
printf("%s is RINEX NAV: %d\n", file2, IsRinexNav(file2)); // 1
printf("%s is RINEX NAV: %d\n", file3, IsRinexNav(file3)); // 1
```

</details>

##### 5.1.3 ReadRnxObs() - RINEX 관측 파일 읽기
<details>
<summary>상세 설명</summary>

**목적**: RINEX v2/v3 관측 파일을 읽어 관측 데이터 구조체에 저장

**입력**:
- `nav_t *nav`: 내비게이션 데이터 구조체 (관측소 정보 저장용)
- `obss_t *obs`: 관측 데이터 구조체 (출력)
- `int rcvidx`: 수신기 인덱스 (1부터 시작)
- `const char *filename`: RINEX 파일명

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:

**1. 파일 검증 및 버퍼 로딩**:
- `IsRinexObs()` 함수로 파일명 검증
- `GetBuff()` 함수로 파일 전체를 메모리에 로딩

**2. RINEX 헤더 파싱**:
- `ReadRnxObsHeader()` 함수 호출
- 버전 정보, 관측 타입, 관측소 정보 추출

**3. 버전별 본문 파싱**:
- RINEX v2: `ReadRnxObsBodyV2()` 함수
- RINEX v3: `ReadRnxObsBodyV3()` 함수

**4. 데이터 변환 및 정렬**:
- `ArrangeObs()` 함수로 GNSS 라이브러리 형식으로 변환
- `SortObss()` 함수로 시간순 정렬

</details>

##### 5.1.4 ReadRnxNav() - RINEX 항법 파일 읽기
<details>
<summary>상세 설명</summary>

**목적**: RINEX v2/v3 항법 파일을 읽어 궤도력 데이터(`nav_t`)에 저장

**입력**:
- `nav_t *nav`: 궤도력 데이터를 저장할 네비게이션 구조체 (출력)
- `const char *filename`: RINEX 항법 파일명

**출력**:
- `int`: 성공 시 1, 실패 시 0

**함수 로직**:

**1. 파일 검증 및 버퍼 로딩**:
- `IsRinexNav()` 함수로 파일명 검증
- `GetBuff()` 함수로 파일 전체를 메모리에 로딩

**2. 헤더 파싱**:
- `ION ALPHA`, `ION BETA`, `DELTA-UTC` 등 헤더 정보 파싱
- `LEAP SECONDS` 파싱하여 윤초 정보 업데이트

**3. 본문 파싱 (궤도력 레코드)**:
- 각 라인을 읽어 시스템과 PRN을 식별
- **Fortran D-지수 처리**: 궤도 파라미터 필드의 `D`를 `E`로 치환하여 `strtod`로 안전하게 파싱
- 파싱된 궤도력 데이터를 `eph_t` 구조체에 저장
- `AddEph()` 함수를 호출하여 `nav_t` 구조체에 궤도력 추가 및 정렬

**4. 오류 처리**:
- 잘못된 형식의 라인이 발견되면 건너뛰고 다음 레코드 처리
- 파일 끝에 도달할 때까지 파싱 계속

</details>

#### 5.2 내부 함수 (static)

##### 시간 조정 함수

##### 5.2.1 AdjWeek() - 주 전환 시간 조정 (static)
<details>
<summary>상세 설명</summary>

**목적**: GPS 주 전환을 고려한 시간 조정

**입력**:
- `double t`: 조정할 시간
- `double t0`: 기준 시간

**출력**:
- `double`: 조정된 시간

**함수 로직**:
주 전환 경계(±302400초 = ±3.5일)를 검사하여 필요시 ±604800초(1주) 조정

</details>

##### 5.2.2 AdjDay() - 일 전환 시간 조정 (static)
<details>
<summary>상세 설명</summary>

**목적**: 일 전환을 고려한 시간 조정

**입력**:
- `double t`: 조정할 시간
- `double t0`: 기준 시간

**출력**:
- `double`: 조정된 시간

**함수 로직**:
일 전환 경계(±43200초 = ±12시간)를 검사하여 필요시 ±86400초(1일) 조정

</details>

##### 항법 파일 처리 함수

##### 5.2.3 ReadRnxNavHeader() - RINEX 항법 헤더 파싱 (static)
<details>
<summary>상세 설명</summary>

**목적**: RINEX 항법 파일 헤더를 파싱하여 메타데이터 추출

**함수 로직**:

**1. 필수 헤더 라인 파싱**:
- `RINEX VERSION / TYPE`: 버전 및 파일 타입
- 시스템 식별자 설정 (v2: N=GPS, G=GLONASS, v3: 40번째 문자)

**2. 전리층 파라미터 파싱**:
- **v2**: `ION ALPHA`, `ION BETA` (GPS용)
- **v3**: `IONOSPHERIC CORR` (시스템별 구분)
- **Fortran D-지수 처리**: 헤더 라인에서 D/d를 E로 치환 후 파싱

**3. 헤더 종료 감지**:
- `END OF HEADER` 라인 검출

</details>

##### 5.2.4 ReadRnxNavBody() - RINEX 항법 본문 파싱 (static)
<details>
<summary>상세 설명</summary>

**목적**: RINEX 항법 파일 본문의 궤도력 데이터 파싱

**함수 로직**:

**1. 2-Pass 파싱**:
- **1차**: 궤도력 메시지 개수 계산
- **2차**: 실제 데이터 파싱 및 저장

**2. Epoch 라인 파싱**:
- **v2**: PRN 번호와 시간 정보
- **v3**: 시스템 식별자 + PRN과 시간 정보

**3. 클록 파라미터 파싱**:
- af0, af1, af2 파라미터
- **Fortran D-지수 처리**: D/d를 E로 치환

**4. 궤도 파라미터 파싱**:
- 시스템별 라인 수 결정 (GPS/GAL/BDS: 7라인, GLO/SBS: 3라인)
- 각 필드에서 **Fortran D-지수 처리** 적용
- 19문자 필드 단위로 파싱

</details>

##### 5.2.5 ArrangeNav() - 항법 데이터 정렬 및 변환 (static)
<details>
<summary>상세 설명</summary>

**목적**: RINEX 항법 데이터를 GNSS 라이브러리 형식(`nav_t`)으로 변환

**변환 과정**:

**1. 위성 정보 변환**:
- `Str2Sat()`: 위성 문자열을 위성 번호로 변환
- `Sat2Prn()`: 위성 번호에서 시스템과 PRN 추출

**2. 시간 변환**:
- **GPS/GAL/QZS/IRN**: `Cal2Time()` → `Gpst2Time()`
- **BDS**: BDT → GPS 시간 변환
- **GLO**: UTC → GPS 시간 변환

**3. 시스템별 파라미터 매핑**:
- **Keplerian 궤도 시스템**: GPS, Galileo, BDS, QZSS, NavIC
- **ECEF 좌표 시스템**: GLONASS (km → m 단위 변환)
- **SBAS**: 위성 기반 보강 시스템

**4. 시간 조정**:
- `AdjWeek()`: TOE, TTR 주 전환 조정
- `AdjDay()`: GLONASS UTC 시간 조정

**5. 궤도력 추가**:
- `AddEph()`: 위성별 궤도력 배열에 추가

</details>

##### 구조체 관리 함수

##### 5.2.6 InitRnxNav() - RINEX 항법 구조체 초기화 (static)
<details>
<summary>상세 설명</summary>

**목적**: `rnxNav_t` 구조체를 안전하게 초기화

**초기화 내용**:
- 헤더 정보 초기화 (버전, 시스템, 전리층 파라미터)
- 본문 데이터 포인터를 `NULL`로 설정
- 데이터 개수를 0으로 설정

</details>

##### 5.2.7 FreeRnxNav() - RINEX 항법 구조체 메모리 해제 (static)
<details>
<summary>상세 설명</summary>

**목적**: `rnxNav_t` 구조체의 동적 할당 메모리를 안전하게 해제

**해제 과정**:
- 본문 데이터 배열 메모리 해제
- 포인터를 `NULL`로 설정
- 데이터 개수를 0으로 재설정

</details>

##### 5.2.8 ConvCode() - 관측 코드 변환 (static)
<details>
<summary>상세 설명</summary>

**목적**: RINEX v2의 2글자 관측 코드를 v3의 3글자 형식으로 변환

**입력**:
- `const char *type2`: v2 관측 코드 (2글자)
- `double ver`: RINEX 버전
- `char sys`: 위성 시스템 ('G', 'R', 'E', 'C' 등)
- `char *type3`: 변환된 v3 관측 코드 (출력, 3글자)

**출력**:
- `int`: 변환 성공 시 1, 실패 시 0

**변환 규칙**:

**1. 의사거리 코드 변환**:
- GPS: `P1` → `C1W`, `P2` → `C2W`
- GLONASS: `P1` → `C1P`, `P2` → `C2P`

**2. 시스템별 주파수 매핑**:
- GPS L1: `L1` → `L1C`
- Galileo E1: `L1` → `L1X`
- BDS B1I: `L1` → `L2I` (RINEX v2의 L1 코드가 BDS에서는 B1I 주파수를 나타내는 L2I 코드로 변환)

**3. 버전별 차이 처리**:
- v2.12 이전: 기본 매핑 규칙
- v2.12 이후: 확장된 관측 코드 지원

**4. BDS 특별 처리** (RINEX v3.02):
- v3.02에서만 BDS 시스템의 관측 코드에서 `B1` → `B2` 문자 변환 수행
- 예: `C1I` → `C2I`, `L1I` → `L2I` (BDS B1I 주파수 대역을 B2 표기로 통일)

</details>

##### 5.2.9 ReadRnxObsHeader() - RINEX 헤더 파싱 (static)
<details>
<summary>상세 설명</summary>

**목적**: RINEX 파일 헤더를 파싱하여 메타데이터 추출

**함수 로직**:

**1. 필수 헤더 라인 파싱**:
- `RINEX VERSION / TYPE`: 버전 및 파일 타입
- `MARKER NAME`: 관측소명
- `REC # / TYPE / VERS`: 수신기 정보
- `ANT # / TYPE`: 안테나 정보

**2. 위치 정보 파싱**:
- `APPROX POSITION XYZ`: 근사 위치 (m)
- `ANTENNA: DELTA H/E/N`: 안테나 오프셋

**3. 관측 타입 파싱**:
- **v2**: `# / TYPES OF OBSERV` (모든 시스템 공통)
- **v3**: `SYS / # / OBS TYPES` (시스템별 분리)

**4. 헤더 종료 감지**:
- `END OF HEADER` 라인 검출

</details>

##### 5.2.10 ReadRnxObsBodyV2() - RINEX v2 본문 파싱 (static)
<details>
<summary>상세 설명</summary>

**목적**: RINEX v2 형식의 관측 데이터 본문 파싱

**v2 형식 특징**:
- **Epoch 라인**: 시간 정보 + 위성 개수
- **위성 목록**: 최대 12개씩 한 라인에 배치
- **관측 데이터**: 위성별로 여러 라인에 걸쳐 배치

**파싱 알고리즘**:

**1. 2-Pass 파싱**:
- **1차**: epoch 개수와 총 관측 데이터 개수 계산
- **2차**: 실제 데이터 파싱 및 저장

**2. 위성 목록 처리**:
- 추가 라인 개수: `nles = (nsat + 11) / 12 - 1`
- 위성 ID 인덱스: `satidx = 12 * i + j`

**3. 관측 데이터 레코드 처리**:
- 16문자 필드 단위 파싱
- LLI(Loss of Lock Indicator) 추출
- 빈 필드는 0.0으로 처리

</details>

##### 5.2.11 ReadRnxObsBodyV3() - RINEX v3 본문 파싱 (static)
<details>
<summary>상세 설명</summary>

**목적**: RINEX v3 형식의 관측 데이터 본문 파싱

**v3 형식 특징**:
- **Epoch 라인**: `>` 시작, 시간 정보 + 위성 개수
- **관측 라인**: 위성 ID + 관측 데이터 (한 라인에 배치)
- **시스템별 구분**: 각 위성별로 개별 라인

**파싱 알고리즘**:

**1. Epoch 라인 감지**:
- `line[0] == '>'` 조건으로 epoch 시작 감지
- 시간 정보 파싱: `YYYY MM DD HH MM SS.SSS`

**2. 위성별 관측 데이터**:
- 위성 ID: 처음 3글자 (예: `G01`, `R22`, `E15`)
- 관측 데이터: 4번째 글자부터 16문자 필드 단위

**3. 데이터 검증**:
- 소수점 위치 검증으로 형식 확인
- 잘못된 형식은 0으로 처리

</details>

##### 5.2.12 ArrangeObs() - 관측 데이터 정렬 및 변환 (static)
<details>
<summary>상세 설명</summary>

**목적**: RINEX 원시 데이터를 GNSS 라이브러리 형식으로 변환

**변환 과정**:

**1. 위성 정보 변환**:
- `Str2Sat()`: 위성 문자열을 위성 번호로 변환
- `Sat2Prn()`: 위성 번호에서 시스템과 PRN 추출

**2. 시간 변환**:
- `Cal2Time()`: 달력 시간을 GPS 시간으로 변환

**3. 관측 타입별 처리**:
- **C (의사거리)**: `newObs.P[fidx-1]` 배열에 저장
- **L (반송파 위상)**: `newObs.L[fidx-1]` 배열에 저장
- **D (도플러)**: `newObs.D[fidx-1]` 배열에 저장
- **S (신호강도)**: `newObs.SNR[fidx-1]` 배열에 저장

**4. 주파수 인덱스 매핑**:
- `Str2Code()`: 관측 타입 문자열을 코드로 변환
- `Code2Fidx()`: 코드를 주파수 인덱스로 변환

**5. 최종 정렬**:
- `SortObss()`: 시간, 수신기, 위성 순으로 정렬

</details>

---

## 6. 사용 예시

### 6.1 관측 파일 읽기
```c
#include "rinex.h"
#include <stdio.h>

void read_obs_file_example() {
    nav_t nav = {0};
    obss_t obs = {0};
    InitNav(&nav); // nav 구조체 초기화

    const char *filename = "data/obs/SJU200KOR_R_20251522300_01H_01S_MO.rnx";
    if (ReadRnxObs(&nav, &obs, 1, filename)) {
        printf("관측 파일 읽기 성공!\n");
        printf("관측소: %s, 관측 데이터 수: %d\n", nav.sta[0].name, obs.n);
    } else {
        printf("관측 파일 읽기 실패\n");
    }

    FreeObss(&obs);
    FreeNav(&nav);
}
```

### 6.2 항법 파일 읽기
```c
#include "rinex.h"
#include "ephemeris.h" // SortEph, UniqEph 사용
#include <stdio.h>

void read_nav_file_example() {
    nav_t nav = {0};
    InitNav(&nav);

    // 레거시 v2 항법 파일 (Fortran D-지수 포함)
    const char *filename = "data/nav/6024R40092202506012300A.25N";
    if (ReadRnxNav(&nav, filename)) {
        printf("항법 파일 읽기 성공!\n");

        // 후처리: 정렬 (중복 제거는 AddEph에서 자동 처리)
        for (int i = 0; i < NSAT; i++) {
            if (nav.ephs[i].n <= 0) continue;
            SortEph(&nav.ephs[i]);
        }

        // GPS(G)와 Galileo(E) 위성 궤도력 개수 출력
        int gps_count = nav.ephs[Prn2Sat(SYS_GPS, 1)-1].n;
        for (int i = 1; i < MAX_PRN_GPS; i++) {
            gps_count += nav.ephs[Prn2Sat(SYS_GPS, i+1)-1].n;
        }

        int gal_count = nav.ephs[Prn2Sat(SYS_GAL, 1)-1].n;
        for (int i = 1; i < MAX_PRN_GAL; i++) {
            gal_count += nav.ephs[Prn2Sat(SYS_GAL, i+1)-1].n;
        }

        printf("  - GPS 궤도력 수: %d\n", gps_count);
        printf("  - Galileo 궤도력 수: %d\n", gal_count);
        printf("  - 전리층 파라미터 (ALPHA): %.4e, %.4e\n", nav.iono[SYS_GPS][0], nav.iono[SYS_GPS][1]);

    } else {
        printf("항법 파일 읽기 실패\n");
    }

    FreeNav(&nav);
}
```

### 6.3 다중 RINEX 파일 처리
```c
void process_multiple_rinex_files() {
    nav_t nav = {0};
    obss_t obs = {0};
    InitNav(&nav);

    const char *files[] = {
        "data/obs/station1.25o",
        "data/obs/station2.25o",
        "data/obs/station3.rnx"
    };
    int nfiles = 3;

    for (int i = 0; i < nfiles; i++) {
        int rcvidx = i + 1;
        printf("\n파일 %d 처리 중: %s\n", i+1, files[i]);
        if (ReadRnxObs(&nav, &obs, rcvidx, files[i])) {
            printf("  수신기 %d (%s) 읽기 성공\n", rcvidx, nav.sta[rcvidx-1].name);
        }
    }

    printf("\n=== 전체 통계 ===\n");
    printf("총 관측 데이터: %d개\n", obs.n);

    FreeObss(&obs);
    FreeNav(&nav);
}
```

---

## 7. 구현 특성

### 7.1 메모리 관리
- **파일 버퍼**: 전체 파일을 한 번에 메모리에 로딩하여 파싱 속도 향상.
- **동적 할당**: 관측/항법 데이터 개수에 따른 필요한 만큼만 메모리 할당.
- **구조체 관리**: `InitNav`/`FreeNav`, `InitObss`/`FreeObss`와 같은 쌍으로 메모리 관리를 명확하게 하여 누수 방지.

### 7.2 파싱 구조
- **RINEX 버전 자동 감지**: 파일 헤더를 분석하여 v2와 v3를 자동으로 구분하고 그에 맞는 파싱 로직 적용.
- **견고한 수치 파싱**: RINEX v2 항법 파일의 Fortran `D` 지수 표기법을 `E`로 변환하여 C 표준 라이브러리와의 호환성 및 파싱 안정성 확보.
- **시스템별 처리**: GPS, GLONASS, Galileo 등 각 위성 시스템의 고유한 데이터 형식을 올바르게 해석.

### 7.3 데이터 정확도
- **시간 처리**: `cal_t` 구조체와 `double` 타입의 UNIX 시간을 기반으로 마이크로초 수준의 시간 정밀도 유지.
- **데이터 무결성**: 파일 읽기 전 유효성 검사를 수행하고, 파싱 중 발생할 수 있는 형식 오류에 대해 방어적으로 설계하여 프로그램 안정성 확보.

### 7.4 확장성 및 제한사항
- **상수 기반 설계**: `MAX_OBSTYPE`, `NRCV`, `NFREQ` 등 주요 한계값들이 상수로 정의되어 있어 필요시 쉽게 확장 가능.
- **메모리 의존성**: 파일을 통째로 메모리에 로드하므로, 시스템 메모리 크기를 초과하는 매우 큰 파일 처리 시 성능 저하 또는 실패 가능성 존재.

### 7.5 오류 처리
- **계층적 검증**: 파일 존재 여부 → 헤더 유효성 → 본문 데이터 형식 순으로 단계적 검증 수행.
- **부분 실패 허용**: 파일 내 일부 데이터 라인에 오류가 있더라도, 유효한 데이터는 최대한 파싱하여 복구.
- **안전한 실패**: 메모리 할당 실패 등 심각한 오류 발생 시, 할당된 자원을 모두 해제하고 오류 코드를 반환하여 안전하게 종료.

---
