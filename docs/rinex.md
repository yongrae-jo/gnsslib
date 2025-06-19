# RINEX 파일 처리 모듈 (rinex)

GNSS 관측 데이터의 표준 교환 형식인 RINEX(Receiver Independent Exchange Format) 파일을 읽고 처리하는 모듈입니다.

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

### 1.1 RINEX 버전 지원
본 모듈은 **RINEX v2.xx 및 v3.xx** 형식을 자동으로 감지하고 처리합니다.

**버전별 특징**:
- **RINEX v2**: `# / TYPES OF OBSERV` 헤더로 관측 타입 정의
- **RINEX v3**: `SYS / # / OBS TYPES` 헤더로 시스템별 관측 타입 정의

### 1.2 파일명 패턴 인식
RINEX 관측 파일을 자동으로 식별하는 패턴:
- **압축 형식**: `.XXo` 또는 `.XXO` (XX는 2자리 숫자)
- **표준 형식**: `.rnx`

### 1.3 관측 타입 변환 시스템
RINEX v2의 2글자 관측 코드를 v3의 3글자 형식으로 내부 변환:
- **v2 → v3 변환**: `P1` → `C1W`, `L1` → `L1C` 등
- **시스템별 매핑**: GPS, GLONASS, Galileo, BDS 시스템별 차별화

### 1.4 메모리 관리 체계
- **버퍼 관리**: `buffer_t` 구조체를 통한 파일 전체 메모리 로딩
- **동적 할당**: 관측 데이터 개수에 따른 메모리 동적 할당
- **안전한 해제**: `FreeRnxObs()` 함수로 메모리 누수 방지

### 1.5 데이터 처리 흐름도

RINEX 파일 파싱은 다음 3단계로 진행됩니다:

```
[RINEX 파일 (텍스트)]
         ↓ ReadRnxObsHeader() + ReadRnxObsBody()
[rnxObs_t 구조체]
    ├── header (rnxObsHeader_t): 파일 메타정보
    └── body[] (rnxObsBody_t): 원시 관측 데이터 배열
         ↓ ArrangeObs()
[obss_t 구조체]
    └── obs[] (obs_t): GNSS 라이브러리 표준 형식
```

**단계별 설명**:
1. **파일 → rnxObs_t**: RINEX 텍스트를 내부 구조체로 파싱
2. **rnxObs_t → obss_t**: 관측 데이터를 라이브러리 표준 형식으로 변환
3. **최종 정렬**: 시간, 수신기, 위성 순으로 데이터 정렬

### 1.6 Column-major 저장 방식
관측 데이터는 시간순으로 정렬된 선형 배열에 저장:
- **시간 기준 정렬**: 모든 관측 데이터가 시간순으로 배치
- **수신기별 구분**: `rcvidx` 필드로 다중 수신기 데이터 구분
- **효율적 검색**: `SortObss()` 함수로 정렬된 데이터 구조

---

## 2. 데이터 타입 구조

```
rinex 모듈 타입 계층
├── rnxObsHeader_t (static struct)
│   ├── ver ────────────────── RINEX 버전 번호 (double)
│   ├── sys ────────────────── 시스템 문자 (char)
│   ├── obsType[NSYS][MAX_OBSTYPE][4] ─── 시스템별 관측 타입 배열
│   ├── nObsType[NSYS] ────── 시스템별 관측 타입 개수
│   └── sta ────────────────── 관측소 정보 (sta_t)
├── rnxObsBody_t (static struct)
│   ├── cal ────────────────── 관측 시간 (cal_t)
│   ├── satStr ─────────────── 위성 ID 문자열 (satStr_t)
│   ├── obs[MAX_OBSTYPE] ──── 관측값 배열 (double)
│   └── lli[MAX_OBSTYPE] ──── 신호 손실 지시자 (int)
└── rnxObs_t (static struct)
    ├── header ─────────────── RINEX 관측 헤더 (rnxObsHeader_t)
    ├── body ───────────────── 관측 데이터 배열 포인터 (rnxObsBody_t*)
    └── n ───────────────────── 관측 데이터 개수 (int)
```

---

## 3. 데이터 타입 목록

### 3.1 rnxObsHeader_t (static struct)
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

### 3.2 rnxObsBody_t (static struct)
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

### 3.3 rnxObs_t (static struct)
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

---

## 4. 함수 구조

```
rinex 모듈 함수 계층
├── 파일 검증
│   └── IsRinexObs() ─────── RINEX 관측 파일명 검증
├── 파일 읽기
│   └── ReadRnxObs() ────── RINEX 관측 파일 읽기 (v2/v3 지원)
├── 내부 헤더 처리 (static)
│   ├── ReadRnxObsHeader() ── RINEX 헤더 파싱 (static)
│   ├── ReadRnxObsBody() ─── RINEX 본문 읽기 총괄 (static)
│   ├── ReadRnxObsBodyV2() ─ RINEX v2 본문 파싱 (static)
│   └── ReadRnxObsBodyV3() ─ RINEX v3 본문 파싱 (static)
├── 데이터 변환 (static)
│   ├── ArrangeObs() ─────── 관측 데이터 정렬 및 변환 (static)
│   └── ConvCode() ───────── v2→v3 관측 코드 변환 (static)
├── 구조체 관리 (static)
│   ├── InitRnxObs() ─────── RINEX 구조체 초기화 (static)
│   └── FreeRnxObs() ─────── RINEX 구조체 메모리 해제 (static)
└── 유틸리티 함수 (static)
    ├── LineContains() ────── 문자열 포함 검사 (static)
    └── Deblank() ─────────── 공백 제거 (static)
```

---

## 5. 함수 목록

### 5.1 공개 함수

#### 5.1.1 IsRinexObs() - RINEX 관측 파일명 검증
<details>
<summary>상세 설명</summary>

**목적**: 파일명이 RINEX 관측 파일 형식인지 검증

**입력**:
- `const char *filename`: 검증할 파일명

**출력**:
- `int`: RINEX 관측 파일이면 1, 아니면 0

**함수 로직**:
1. **압축 형식 검사**: `.XXo` 또는 `.XXO` 패턴 (XX는 2자리 숫자)
2. **표준 형식 검사**: `.rnx` 확장자
3. **대소문자 구분**: 'o'와 'O' 모두 지원

**사용 예시**:
```c
// 다양한 RINEX 파일명 테스트
const char *files[] = {
    "YONS00KOR_R_20250010300_01H_01S_GO.25o",  // 압축 형식 (소문자)
    "SJU200KOR_R_20251522300_01H_01S_MO.25O",  // 압축 형식 (대문자)
    "station1.22o",                            // 간단한 압축 형식
    "obs_data.rnx",                            // 표준 형식
    "invalid.txt",                             // 비 RINEX 파일
    "test.21x"                                 // 잘못된 확장자
};

printf("RINEX 파일명 검증 결과:\n");
for (int i = 0; i < 6; i++) {
    int result = IsRinexObs(files[i]);
    printf("%-40s → %s\n", files[i], result ? "RINEX" : "비RINEX");
}
```

</details>

#### 5.1.2 ReadRnxObs() - RINEX 관측 파일 읽기
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

**사용 예시**:
```c
#include "rinex.h"

void rinex_reading_example() {
    // 데이터 구조체 초기화
    nav_t nav = {0};
    obss_t obs = {0};

    // RINEX v2 파일 읽기
    const char *rinex_v2 = "data/obs/YONS00KOR_R_20250010300_01H_01S_GO.25o";
    int success_v2 = ReadRnxObs(&nav, &obs, 1, rinex_v2);

    if (success_v2) {
        printf("RINEX v2 파일 읽기 성공!\n");
        printf("  관측 데이터 개수: %d\n", obs.n);
        printf("  관측소명: %s\n", nav.sta[0].name);
        printf("  수신기 타입: %s\n", nav.sta[0].rectype);

        // 첫 번째 관측 데이터 출력
        if (obs.n > 0) {
            cal_t cal = Time2Cal(obs.obs[0].time);
            calStr_t calstr = Cal2Str(cal, 3);
            satStr_t satstr = Sat2Str(obs.obs[0].sat);

            printf("  첫 관측: %s, 위성: %s\n", calstr.str, satstr.str);
            printf("  의사거리 L1: %.3f m\n", obs.obs[0].P[0]);
            printf("  반송파 L1: %.3f cycles\n", obs.obs[0].L[0]);
        }
    }

    // RINEX v3 파일 추가 읽기 (같은 obs 구조체에)
    const char *rinex_v3 = "data/obs/YONS00KOR_R_20250010300_01H_01S_GO.rnx";
    int success_v3 = ReadRnxObs(&nav, &obs, 2, rinex_v3);

    if (success_v3) {
        printf("\nRINEX v3 파일 추가 읽기 성공!\n");
        printf("  총 관측 데이터 개수: %d\n", obs.n);
        printf("  수신기 2 정보: %s\n", nav.sta[1].name);

        // 수신기별 데이터 개수 확인
        int count_rcv1 = 0, count_rcv2 = 0;
        for (int i = 0; i < obs.n; i++) {
            if (obs.obs[i].rcv == 1) count_rcv1++;
            else if (obs.obs[i].rcv == 2) count_rcv2++;
        }
        printf("  수신기 1 데이터: %d개\n", count_rcv1);
        printf("  수신기 2 데이터: %d개\n", count_rcv2);
    }

    // 메모리 해제
    FreeObss(&obs);

    printf("\n파일 읽기 완료 및 메모리 해제\n");
}
```

</details>

### 5.2 내부 함수 (static)

#### 5.2.1 ConvCode() - 관측 코드 변환 (static)
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

#### 5.2.2 ReadRnxObsHeader() - RINEX 헤더 파싱 (static)
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

#### 5.2.3 ReadRnxObsBodyV2() - RINEX v2 본문 파싱 (static)
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

#### 5.2.4 ReadRnxObsBodyV3() - RINEX v3 본문 파싱 (static)
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

#### 5.2.5 ArrangeObs() - 관측 데이터 정렬 및 변환 (static)
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

### 6.1 기본 RINEX 파일 읽기

```c
#include "rinex.h"
#include <stdio.h>

int main() {
    // 데이터 구조체 초기화
    nav_t nav = {0};
    obss_t obs = {0};

    // RINEX 파일 읽기
    const char *filename = "data/obs/station.25o";
    int success = ReadRnxObs(&nav, &obs, 1, filename);

    if (success) {
        printf("RINEX 파일 읽기 성공!\n");
        printf("관측 데이터 개수: %d\n", obs.n);

        // 첫 번째와 마지막 관측 시간 출력
        if (obs.n > 0) {
            cal_t first_time = Time2Cal(obs.obs[0].time);
            cal_t last_time = Time2Cal(obs.obs[obs.n-1].time);

            calStr_t first_str = Cal2Str(first_time, 0);
            calStr_t last_str = Cal2Str(last_time, 0);

            printf("관측 시작: %s\n", first_str.str);
            printf("관측 종료: %s\n", last_str.str);
        }
    } else {
        printf("RINEX 파일 읽기 실패\n");
    }

    // 메모리 해제
    FreeObss(&obs);
    return 0;
}
```

### 6.2 다중 RINEX 파일 처리

```c
void process_multiple_rinex_files() {
    nav_t nav = {0};
    obss_t obs = {0};

    // 파일 목록
    const char *files[] = {
        "data/obs/station1.25o",  // 수신기 1
        "data/obs/station2.25o",  // 수신기 2
        "data/obs/station3.rnx"   // 수신기 3 (v3)
    };
    int nfiles = 3;

    // 각 파일을 순차적으로 읽기
    for (int i = 0; i < nfiles; i++) {
        int rcvidx = i + 1;  // 수신기 인덱스 (1부터 시작)

        printf("\n파일 %d 처리 중: %s\n", i+1, files[i]);

        if (ReadRnxObs(&nav, &obs, rcvidx, files[i])) {
            printf("  수신기 %d 읽기 성공\n", rcvidx);
            printf("  관측소명: %s\n", nav.sta[rcvidx-1].name);
            printf("  수신기 타입: %s\n", nav.sta[rcvidx-1].rectype);
        } else {
            printf("  수신기 %d 읽기 실패\n", rcvidx);
        }
    }

    printf("\n=== 전체 통계 ===\n");
    printf("총 관측 데이터: %d개\n", obs.n);

    // 수신기별 데이터 개수 집계 (NRCV는 types.h에 정의)
    int count_per_rcv[NRCV] = {0};
    for (int i = 0; i < obs.n; i++) {
        if (obs.obs[i].rcv > 0 && obs.obs[i].rcv <= NRCV) {
            count_per_rcv[obs.obs[i].rcv - 1]++;
        }
    }

    for (int i = 0; i < nfiles; i++) {
        printf("수신기 %d: %d개 관측\n", i+1, count_per_rcv[i]);
    }

    // 메모리 해제
    FreeObss(&obs);
}
```

### 6.3 관측 데이터 분석

```c
#include "rinex.h"
#include "types.h"  // NSYS, NFREQ, NRCV 상수 정의

void analyze_observation_data() {
    nav_t nav = {0};
    obss_t obs = {0};

    // RINEX 파일 읽기
    if (!ReadRnxObs(&nav, &obs, 1, "data/obs/multi_gnss.rnx")) {
        printf("파일 읽기 실패\n");
        return;
    }

    // 시스템별 위성 개수 집계 (NSYS는 types.h에 정의)
    int sat_count[NSYS] = {0};  // GPS, GLO, GAL, BDS, QZS, SBS
    int total_epochs = 0;
    double prev_time = 0.0;

    for (int i = 0; i < obs.n; i++) {
        obs_t *o = &obs.obs[i];

        // 시스템 분류
        int prn;
        int sys = Sat2Prn(o->sat, &prn);
        if (sys > 0 && sys <= NSYS) {
            sat_count[sys - 1]++;
        }

        // Epoch 개수 계산
        if (fabs(o->time - prev_time) > 1e-6) {
            total_epochs++;
            prev_time = o->time;
        }
    }

    printf("\n=== GNSS 시스템별 관측 통계 ===\n");
    const char *sys_names[] = {"GPS", "GLO", "GAL", "BDS", "QZS", "SBS"};
    for (int i = 0; i < NSYS; i++) {
        if (sat_count[i] > 0) {
            printf("%s: %d개 관측\n", sys_names[i], sat_count[i]);
        }
    }

    printf("\n총 Epoch 수: %d\n", total_epochs);
    printf("평균 위성 수: %.1f개/epoch\n", (double)obs.n / total_epochs);

    // 주파수별 관측 가능성 분석 (NFREQ는 types.h에 정의)
    int freq_count[NFREQ] = {0};
    for (int i = 0; i < obs.n; i++) {
        obs_t *o = &obs.obs[i];
        for (int f = 0; f < NFREQ; f++) {
            if (o->P[f] != 0.0 || o->L[f] != 0.0) {
                freq_count[f]++;
            }
        }
    }

    printf("\n=== 주파수별 관측 통계 ===\n");
    for (int f = 0; f < NFREQ; f++) {
        if (freq_count[f] > 0) {
            printf("L%d: %d개 관측 (%.1f%%)\n",
                   f+1, freq_count[f],
                   100.0 * freq_count[f] / obs.n);
        }
    }

    FreeObss(&obs);
}
```

### 6.4 품질 검사 및 필터링

```c
void quality_check_observations() {
    nav_t nav = {0};
    obss_t obs = {0};

    if (!ReadRnxObs(&nav, &obs, 1, "data/obs/noisy_data.rnx")) {
        printf("파일 읽기 실패\n");
        return;
    }

    printf("=== 관측 데이터 품질 검사 ===\n");

    int total_obs = obs.n;
    int valid_obs = 0;
    int cycle_slip_count = 0;
    int low_snr_count = 0;

    // 관측 데이터 품질 검사
    for (int i = 0; i < obs.n; i++) {
        obs_t *o = &obs.obs[i];
        bool is_valid = false;

        // 주파수별 품질 검사
        for (int f = 0; f < NFREQ; f++) {
            // 의사거리와 반송파 위상 모두 있는 경우
            if (o->P[f] != 0.0 && o->L[f] != 0.0) {
                is_valid = true;

                // Cycle slip 검사 (LLI bit 0 또는 1)
                if (o->LLI[f] & 0x01 || o->LLI[f] & 0x02) {
                    cycle_slip_count++;
                }

                // 낮은 SNR 검사 (30 dB-Hz 미만)
                if (o->SNR[f] != 0.0 && o->SNR[f] < 30.0) {
                    low_snr_count++;
                }
            }
        }

        if (is_valid) valid_obs++;
    }

    printf("전체 관측: %d개\n", total_obs);
    printf("유효 관측: %d개 (%.1f%%)\n",
           valid_obs, 100.0 * valid_obs / total_obs);
    printf("Cycle slip: %d건\n", cycle_slip_count);
    printf("낮은 SNR: %d건\n", low_snr_count);

    // 연속성 검사 (시간 간격)
    int time_gap_count = 0;
    double expected_interval = 1.0;  // 1초 간격 예상

    for (int i = 1; i < obs.n; i++) {
        double time_diff = obs.obs[i].time - obs.obs[i-1].time;
        if (fabs(time_diff - expected_interval) > 0.1) {
            time_gap_count++;
        }
    }

    printf("시간 간격 이상: %d건\n", time_gap_count);

    // 기하학적 분산 검사 (DOP 계산을 위한 위성 분포)
    double elevation_sum = 0.0;
    int elevation_count = 0;

    // 실제로는 위성 위치 계산이 필요하지만, 여기서는 간략화
    printf("\n품질 검사 완료 - 데이터 사용 가능\n");

    FreeObss(&obs);
}
```

---

## 7. 구현 특성

### 7.1 메모리 관리

#### 7.1.1 메모리 할당 방식
- **파일 버퍼**: 전체 파일을 한 번에 메모리에 로딩
- **관측 데이터**: 2-pass 파싱으로 필요한 메모리 크기 사전 계산 후 할당
- **동적 할당**: `malloc/free` 기반, 파일 크기에 비례한 메모리 사용

#### 7.1.2 메모리 구성 요소
```
전체 메모리 = 파일 버퍼 + 파싱된 데이터 + 구조체 오버헤드
- 파일 버퍼: 원본 파일 크기와 동일
- 파싱된 데이터: obs_t 구조체 × 관측 데이터 개수
- 구조체 오버헤드: 헤더 정보 및 기타 메타데이터
```

### 7.2 파싱 구조

#### 7.2.1 RINEX 버전별 처리 방식
| RINEX 버전 | 파싱 방식 | 특징 |
|------------|-----------|------|
| v2.xx | 고정 필드 형식 | Epoch 라인 + 위성 목록 + 관측 데이터 |
| v3.xx | 가변 필드 형식 | Epoch 라인(`>`) + 위성별 개별 라인 |

#### 7.2.2 시스템별 지원 현황
| 위성 시스템 | v2 지원 | v3 지원 | 특징 |
|-------------|---------|---------|------|
| GPS | ✓ | ✓ | 전체 주파수 대역 지원 |
| GLONASS | ✓ | ✓ | 전체 주파수 대역 지원 |
| Galileo | 제한적 | ✓ | v3에서 완전 지원 |
| BDS | 제한적 | ✓ | v3에서 완전 지원 |
| QZSS | 제한적 | ✓ | v3에서 완전 지원 |
| SBAS | 제한적 | ✓ | v3에서 완전 지원 |

### 7.3 데이터 정확도

#### 7.3.1 시간 처리
- **시간 분해능**: 달력 시간 구조체(`cal_t`) 기반
- **시간 범위**: GPS 시간 시스템 기준
- **변환**: `Cal2Time()` 함수로 GPS 시간으로 통일

#### 7.3.2 데이터 무결성
- **입력 검증**: 파일명, 매개변수 유효성 사전 검사
- **위성 ID 검증**: `Str2Sat()` 함수로 유효한 위성만 처리
- **메모리 안전성**: NULL 포인터 검사 및 경계 조건 확인

### 7.4 확장성 및 제한사항

#### 7.4.1 설계 한계
| 항목 | 현재 설정 | 제한 상수 | 확장 가능성 |
|------|-----------|-----------|-------------|
| 관측 타입 수 | 32개/시스템 | `MAX_OBSTYPE` | 상수 변경으로 확장 |
| 수신기 수 | - | `NRCV` | 다중 수신기 동시 처리 |
| 주파수 대역 | 5개 | `NFREQ` | 다중 주파수 처리 |

#### 7.4.2 구현 제약사항

**1. 메모리 기반 파싱**:
- 전체 파일을 메모리에 로딩하는 방식
- 대용량 파일에서 메모리 사용량 증가
- 시스템 메모리에 의존적

**2. 순차적 처리**:
- 단일 스레드 파싱
- 파일별 순차 처리 구조

### 7.5 오류 처리

#### 7.5.1 오류 처리 전략
- **계층적 오류 처리**: 파일 → 헤더 → 본문 순차 검증
- **부분 실패 허용**: 일부 데이터 오류 시에도 가능한 데이터 복구
- **기본값 사용**: 누락된 정보는 0 또는 빈 문자열로 처리

#### 7.5.2 복구 메커니즘
- **헤더 오류**: 필수 정보 누락 시 기본값으로 대체
- **데이터 오류**: 개별 관측 epoch 단위로 오류 격리
- **메모리 오류**: 할당 실패 시 즉시 정리 후 오류 반환

---
