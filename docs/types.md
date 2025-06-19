# 데이터 타입 정의 모듈 (types)

GNSS 라이브러리의 모든 데이터 구조를 정의하는 중앙 집중형 타입 시스템입니다.

## 목차

1. [기본 개념](#1-기본-개념)
2. [데이터 타입 구조](#2-데이터-타입-구조)
3. [데이터 타입 목록](#3-데이터-타입-목록)
4. [함수 구조](#4-함수-구조)
5. [사용 예시](#5-사용-예시)
6. [구현 특성](#6-구현-특성)

---

## 1. 기본 개념

### 1.1 중앙 집중형 타입 시스템
본 모듈은 **GNSS 라이브러리 전체에서 사용되는 모든 데이터 구조를 중앙에서 정의**합니다. 다른 모든 모듈은 이 타입 정의를 참조하여 일관된 데이터 구조를 사용합니다.

**핵심 기능**:
- **타입 정의**: 모든 GNSS 처리용 구조체 및 열거형 정의
- **메모리 관리**: 동적 배열 구조체의 일관된 메모리 관리 패턴
- **타입 안전성**: 제네릭 데이터 타입의 안전한 사용 지원
- **호환성**: C99 표준 준수 및 크로스 플랫폼 호환성

### 1.2 계층적 데이터 구조 패턴
모든 데이터 타입은 **기본 구조체**와 **집합 구조체**의 2계층으로 설계됩니다:

**기본 구조체**: `xxx_t`
- 개별 데이터 항목을 표현
- 고정된 메모리 크기
- 스택 또는 힙에 할당 가능

**집합 구조체**: `xxxs_t`
- 동일 타입의 동적 배열 관리
- 현재 사용량(`n`)과 할당 크기(`nmax`) 분리
- 자동 메모리 확장 지원

### 1.3 시간 표현 통일 체계
모든 시간 데이터는 **GPS Time (GPST) 기준 표준 시간**으로 통일됩니다:

$$t_{GPST} = t_{GPST0} + \text{week} \times 604800 + \text{tow}$$

여기서:
- $t_{GPST}$: GPS Time 기준 연속 시간 [s] (UNIX Time 형식)
- $t_{GPST0}$: GPS 기준시간 (1980/1/6 00:00:00)의 UNIX Time
- $\text{week}$: GPS 주차 (week rollover 보정 포함)
- $\text{tow}$: 주내 초 (Time of Week) [s]
- $604800 = 7 \times 24 \times 3600$ (1주일의 초 수)

### 1.4 인덱스 매핑 최적화
위성, 수신기, 주파수 등의 식별을 위해 **1-based 연속 정수 인덱스 체계**를 사용합니다:
- **위성 인덱스**: 1~NSAT (PRN과 독립적, 1-based)
- **수신기 인덱스**: 1~NRCV (1-based)
- **주파수 인덱스**: 1~NFREQ (1-based)
- **시스템 인덱스**: 1~NSYS (1-based)

이를 통해 배열 접근 최적화와 메모리 효율성을 확보하며, 0은 항상 오류/무효 값을 나타냅니다.

---

## 2. 데이터 타입 구조

```
types 모듈 데이터 계층
├── 안테나 보정 데이터
│   ├── pcv_t ──────────────── 개별 안테나 위상중심 보정 데이터
│   └── pcvs_t ─────────────── 안테나 보정 데이터 집합
├── 관측소 및 옵션
│   ├── sta_t ──────────────── 관측소 매개변수 데이터
│   └── opt_t ──────────────── 처리 옵션 데이터
├── 궤도력 데이터
│   ├── eph_t ──────────────── 개별 위성 방송궤도력
│   └── ephs_t ─────────────── 궤도력 데이터 집합
├── 항법 통합 구조
│   └── nav_t ──────────────── 모든 항법 데이터 통합
├── 관측 데이터
│   ├── obs_t ──────────────── 개별 GNSS 관측 데이터
│   └── obss_t ─────────────── 관측 데이터 집합
├── 측위 결과 데이터
│   ├── sol_t ──────────────── 개별 측위 해
│   └── sols_t ─────────────── 측위 해 집합
├── 수신기 상태 데이터
│   ├── rcv_t ──────────────── 개별 수신기 상태
│   └── rcvs_t ─────────────── 수신기 상태 집합
├── 위성 상태 데이터
│   ├── sat_t ──────────────── 개별 위성 상태
│   └── sats_t ─────────────── 위성 상태 집합
├── 공통 유틸리티 타입
│   ├── satStr_t ───────────── 위성 문자열 ($\text{CXX}$)
│   ├── cal_t ──────────────── 달력 날짜/시간
│   ├── calStr_t ───────────── 달력 문자열
│   └── codeStr_t ──────────── 관측 코드 문자열
├── 파일 관리 타입
│   ├── files_t ────────────── 동적 파일명 배열
│   ├── file_t ─────────────── 파일 세트 관리
│   ├── lineinfo_t ─────────── 텍스트 라인 정보
│   └── buffer_t ───────────── 파일 버퍼 구조
└── 매트릭스 및 벡터 타입
    ├── type_t [enum] ──────── 데이터 타입 열거형
    ├── idx_t ──────────────── 인덱스 벡터
    └── mat_t ──────────────── 타입별 매트릭스
```

---

## 3. 데이터 타입 목록

### 3.1 안테나 보정 데이터 타입

#### 3.1.1 pcv_t - 안테나 위상중심 보정 데이터
<details>
<summary>상세 설명</summary>

**목적**: 위성 및 수신기 안테나의 위상중심 오프셋 및 변화 보정 데이터 저장

**정의**: `types.h`에 정의된 안테나 보정 구조체

**주요 멤버**:
- **위성 식별**: `sat` (위성 인덱스, 0이면 수신기 안테나)
- **안테나 정보**: `type[STA_STR_SIZE]` (안테나 타입 또는 위성 타입)
- **일련번호**: `serial[STA_STR_SIZE]` (안테나 일련번호 또는 위성 ID)
- **유효기간**: `ts`, `te` (시작/종료 시간 [GPST])
- **위상중심 오프셋**: `off[NSYS][NBAND][3]` [m]
  - 위성: xyz 좌표계
  - 수신기: enu 좌표계
- **위상중심 변화**: `var[NSYS][NBAND][19]` [m]
  - 위성: 천정각 0°,1°,...,18°
  - 수신기: 고도각 90°,85°,...,0°

**사용**: IGS ANTEX 파일 데이터 및 정밀 위치 보정에 활용

</details>

#### 3.1.2 pcvs_t - 안테나 보정 데이터 집합
<details>
<summary>상세 설명</summary>

**목적**: 다중 안테나 보정 데이터의 동적 배열 관리

**구조**: `{int n, nmax; pcv_t *pcv;}`

**메모리 관리**: 현재 개수(`n`)와 할당 크기(`nmax`) 분리 관리

**사용**: 안테나 타입/일련번호 기반 검색 및 시간 유효성 확인

</details>

### 3.2 관측소 및 옵션 타입

#### 3.2.1 sta_t - 관측소 매개변수 데이터
<details>
<summary>상세 설명</summary>

**목적**: 수신기 관측소의 위치, 안테나, 수신기 정보 및 GLONASS 특수 설정 저장

**주요 멤버**:
- **관측소 식별**: `name`, `marker` (관측소명, 마커 번호)
- **안테나 정보**: `antdes`, `antsno` (안테나 타입, 일련번호)
- **수신기 정보**: `rectype`, `recsno`, `recver` (타입, 일련번호, 버전)
- **위치 정보**: `pos[3]` (ECEF 위치 [m]), `del[3]` (델타 위치 [m])
- **좌표계**: `itrf` (ITRF 실현 연도), `deltype` (델타 타입: 0=enu, 1=xyz)
- **GLONASS**: `glo_align`, `glo_bias[4]` (정렬 플래그, GLONASS 주파수별 코드 바이어스 [m])
  - `glo_bias[0]`: C1C 코드 바이어스
  - `glo_bias[1]`: C1P 코드 바이어스
  - `glo_bias[2]`: C2C 코드 바이어스
  - `glo_bias[3]`: C2P 코드 바이어스

</details>

#### 3.2.2 opt_t - 처리 옵션 데이터
<details>
<summary>상세 설명</summary>

**목적**: GNSS 데이터 처리를 위한 모든 옵션 및 매개변수 설정

**주요 멤버**:
- **처리 설정**: `mode`, `engine`, `nrcv`, `nfreq` (모드, 엔진, 수신기 수, 주파수 수)
- **모델 옵션**: `ephopt`, `posopt`, `ionoopt`, `tropoopt` (궤도력, 위치, 전리층, 대류권)
- **모호정수**: `par`, `cascade`, `gloaropt`, `minlock` (부분 해결, 계단식, GLONASS, 최소 락)
- **필터링**: `dynamics`, `maxout`, `elmask`, `maxgdop` (역학, 최대 중단, 고도각, 최대 GDOP)
- **잡음 모델**: `err`, `errratio` (위상 오차 [m], 코드/위상 비율)
- **프로세스 잡음**: `procnoiseAmb`, `procnoiseTropo`, `procnoiseIono` 등
- **제외 위성**: `exsats[NSAT]` (제외 위성 배열)

</details>

### 3.3 궤도력 데이터 타입

#### 3.3.1 eph_t - 개별 위성 방송궤도력
<details>
<summary>상세 설명</summary>

**목적**: 모든 GNSS 시스템의 방송궤도력 매개변수 저장

**주요 멤버**:
- **위성 식별**: `sat` (위성 인덱스)
- **궤도력 ID**: `IODE`, `IODC`, `AODE`, `AODC` (시스템별 식별자)
- **건강/정확도**: `sva`, `svh` (URA/SISA 인덱스, 건강성)
- **시간 정보**: `week`, `toes`, `ttrs`, `toe`, `toc`, `ttr` (주차, 기준시간 등)
- **Keplerian 요소**: `A`, `e`, `i0`, `OMG0`, `omg`, `M0` (궤도 6요소)
- **섭동 계수**: `deln`, `OMGd`, `iodt`, `cuc`, `cus`, `crc`, `crs`, `cic`, `cis`
- **시계 매개변수**: `af0`, `af1`, `af2` (GPS/GAL/BDS/QZS/IRN), `taun`, `gamn` (GLONASS)
- **위치/속도**: `pos[3]`, `vel[3]`, `acc[3]` (GLONASS/SBAS 전용)
- **기타**: `code`, `flag`, `fit`, `data`, `frq`, `age`, `tgd[2]`

</details>

#### 3.3.2 ephs_t - 궤도력 데이터 집합
<details>
<summary>상세 설명</summary>

**목적**: 다중 궤도력 데이터의 동적 배열 관리

**구조**: `{int n, nmax; eph_t *eph;}`

**사용**: 위성별 궤도력 선택 및 시간 순 정렬

</details>

### 3.4 항법 통합 구조

#### 3.4.1 nav_t - 모든 항법 데이터 통합
<details>
<summary>상세 설명</summary>

**목적**: GNSS 처리에 필요한 모든 항법 관련 데이터의 중앙 집중 관리

**구조**:
- `ephs[NSAT]`: 위성별 궤도력 데이터 집합
- `pcvs`: 안테나 위상중심 보정 데이터 집합
- `sta[NRCV]`: 수신기별 관측소 매개변수
- `iono[NSYS][8]`: 시스템별 전리층 모델 매개변수
- `opt`: 처리 옵션 포인터

</details>

### 3.5 관측 데이터 타입

#### 3.5.1 obs_t - 개별 GNSS 관측 데이터
<details>
<summary>상세 설명</summary>

**목적**: 단일 시각, 단일 위성의 다중 주파수 관측 데이터 저장

**주요 멤버**:
- **시간/위치**: `time` (GPST [s]), `rcv`, `sat` (수신기/위성 인덱스)
- **관측 코드**: `code[NFREQ]` (관측 코드 인덱스)
- **관측값**: `P[NFREQ]` (의사거리 [m]), `L[NFREQ]` (반송파 위상 [m])
- **보조 데이터**: `D[NFREQ]` (도플러 [Hz]), `SNR[NFREQ]` (신호세기 [dB])
- **품질 지시자**: `LLI[NFREQ]` (사이클 슬립 지시자)

</details>

#### 3.5.2 obss_t - 관측 데이터 집합
<details>
<summary>상세 설명</summary>

**목적**: 다중 관측 데이터의 동적 배열 관리

**구조**: `{int n, nmax; obs_t *obs;}`

**사용**: 시간 순 정렬 및 대용량 데이터 처리

</details>

### 3.6 측위 결과 데이터 타입

#### 3.6.1 sol_t - 개별 측위 해
<details>
<summary>상세 설명</summary>

**목적**: 단일 시각의 측위 결과 및 품질 정보 저장

**주요 멤버**:
- **시간**: `time` (해 시간 [GPST])
- **모호정수**: `fix`, `Ps`, `ratio`, `namb`, `nfix` (해결 상태, 성공률, 비율, 개수)
- **품질**: `stat`, `nsat`, `dop[5]` (해 품질, 위성 수, DOP 값)
- **통계**: `lom`, `age` (모델 테스트, 차분 데이터 경과시간)

</details>

#### 3.6.2 sols_t - 측위 해 집합
<details>
<summary>상세 설명</summary>

**목적**: 시계열 측위 결과 관리

**구조**: `{int n, nmax; sol_t *sol;}`

**사용**: 연속 측위 해석 및 성능 평가

</details>

### 3.7 수신기 상태 데이터 타입

#### 3.7.1 rcv_t - 개별 수신기 상태
<details>
<summary>상세 설명</summary>

**목적**: 수신기의 위치, 시계, 바이어스 등 상태 매개변수 저장

**주요 멤버**:
- **시간/식별**: `time`, `rcv` (시간, 수신기 인덱스)
- **위치**: `rr[3]`, `rr_fix[3]` (ECEF 위치 [m], float/fixed)
- **대류권**: `zwd`, `zwd_fix` (천정 습지연 [m], float/fixed)
- **시계**: `dtr[NSYS]`, `dtr_fix[NSYS]` (시스템별 수신기 시계 [m])
- **바이어스**: `cbr[NSYS][NFREQ]`, `pbr[NSYS][NFREQ]` (코드/위상 바이어스 [m])

**`_fix` 필드 설명**: 모호정수 해결이 성공적으로 완료되었을 때, 측위 엔진에 의해 계산된 fixed 해가 `_fix` 접미사 필드들에 저장됩니다. 위성 시계와 바이어스의 float/fixed 해를 분리하여 정밀한 측위 품질 관리가 가능합니다.

</details>

#### 3.7.2 rcvs_t - 수신기 상태 집합
<details>
<summary>상세 설명</summary>

**목적**: 다중 수신기 상태 동적 배열 관리

**구조**: `{int n, nmax; rcv_t *rcv;}`

</details>

### 3.8 위성 상태 데이터 타입

#### 3.8.1 sat_t - 개별 위성 상태
<details>
<summary>상세 설명</summary>

**목적**: 위성의 시계, 바이어스 등 상태 매개변수 저장

**주요 멤버**:
- **시간/식별**: `time`, `sat`, `iode` (시간, 위성 인덱스, 궤도력 ID)
- **시계**: `dts`, `dts_fix` (위성 시계 [m], float/fixed)
- **바이어스**: `cbs[NFREQ]`, `pbs[NFREQ]` (코드/위상 바이어스 [m], float/fixed)

**`_fix` 필드 설명**: 모호정수 해결이 성공적으로 완료되었을 때, 측위 엔진에 의해 계산된 fixed 해가 `_fix` 접미사 필드들에 저장됩니다. 위성 시계와 바이어스의 float/fixed 해를 분리하여 정밀한 측위 품질 관리가 가능합니다.

</details>

#### 3.8.2 sats_t - 위성 상태 집합
<details>
<summary>상세 설명</summary>

**목적**: 다중 위성 상태 동적 배열 관리

**구조**: `{int n, nmax; sat_t *sat;}`

</details>

### 3.9 공통 유틸리티 타입

#### 3.9.1 satStr_t - 위성 문자열
<details>
<summary>상세 설명</summary>

**목적**: 위성 식별 문자열 ($\text{CXX}$ 형식) 저장

**구조**: `char str[SAT_STR_SIZE]`

**형식**: C01, G01, R01, E01, J01, I01, S01 등

**사용**: 사용자 인터페이스 및 로그 출력

</details>

#### 3.9.2 cal_t - 달력 날짜/시간
<details>
<summary>상세 설명</summary>

**목적**: 구조화된 달력 날짜 및 시간 표현

**구조**: `{int year, mon, day, hour, min; double sec;}`

**정밀도**: 밀리초 단위 초 정밀도 지원

**사용**: GPST ↔ 달력 시간 변환

</details>

#### 3.9.3 calStr_t - 달력 문자열
<details>
<summary>상세 설명</summary>

**목적**: 달력 시간의 문자열 표현

**구조**: `char str[CAL_STR_SIZE]`

**형식**: YYYY/MM/DD HH:MM:SS.sss (ISO 8601 호환)

**사용**: 시간 표시 및 파일 출력

</details>

#### 3.9.4 codeStr_t - 관측 코드 문자열
<details>
<summary>상세 설명</summary>

**목적**: RINEX 관측 코드 문자열 ($\text{LXX}$ 형식) 저장

**구조**: `char str[CODE_STR_SIZE]`

**형식**: L1C, L2W, C1C, S1C 등 RINEX v3 코드

**사용**: 관측 타입 식별 및 변환

</details>

### 3.10 파일 관리 타입

#### 3.10.1 files_t - 동적 파일명 배열
<details>
<summary>상세 설명</summary>

**목적**: 파일명 문자열 포인터의 동적 배열 관리

**구조**: `{char **names; int n, nmax;}`

**메모리**: 2차원 동적 할당 (파일 수 × 파일명 길이)

**사용**: 런타임 파일 추가/제거 지원

</details>

#### 3.10.2 file_t - 파일 세트 관리
<details>
<summary>상세 설명</summary>

**목적**: GNSS 처리용 다중 파일 타입의 통합 관리

**구조**:
- `obsfiles`: 관측 파일
- `navfiles`: 항법 파일
- `sp3files`: SP3 파일
- `clkfiles`: 시계 파일
- `dcbfiles`: DCB 파일
- `atxfiles`: 안테나 교환 파일

**사용**: 일괄 파일 처리 및 관리

</details>

#### 3.10.3 lineinfo_t - 텍스트 라인 정보
<details>
<summary>상세 설명</summary>

**목적**: 텍스트 파일 내 개별 라인의 위치 정보 저장

**구조**: `{size_t start, end, len;}`

**사용**: 고속 라인 단위 파싱 및 메모리 복사 없는 인덱싱

</details>

#### 3.10.4 buffer_t - 파일 버퍼 구조
<details>
<summary>상세 설명</summary>

**목적**: 전체 파일을 메모리에 로드하여 고속 처리 지원

**구조**: `{char *buff; lineinfo_t *lineinfo; size_t nline;}`

**성능**: 디스크 I/O 최소화를 통한 속도 향상

**사용**: 대용량 텍스트 파일 처리 최적화

</details>

### 3.11 매트릭스 및 벡터 타입

#### 3.11.1 type_t - 데이터 타입 열거형
<details>
<summary>상세 설명</summary>

**목적**: 제네릭 데이터 구조의 타입 안전성 보장

**값**: `BOOL`, `INT`, `DOUBLE`

**사용**: void 포인터 캐스팅 시 타입 검증 및 제네릭 함수 타입 관리

</details>

#### 3.11.2 idx_t - 인덱스 벡터
<details>
<summary>상세 설명</summary>

**목적**: 타입별 인덱스 벡터의 안전한 관리

**구조**: `{int n; type_t type; void *idx;}`

**타입**: `BOOL` (마스킹용), `INT` (인덱싱용)

**사용**: 조건부 데이터 선택 및 정렬 작업

</details>

#### 3.11.3 mat_t - 타입별 매트릭스
<details>
<summary>상세 설명</summary>

**목적**: 타입별 매트릭스 데이터의 안전한 관리

**구조**: `{int rows, cols; type_t type; void *data;}`

**저장 방식**: column-major 순서 (FORTRAN 호환)

**타입**: `INT`, `DOUBLE` 매트릭스 지원

**사용**: 매트릭스 라이브러리 함수를 통한 안전한 접근

</details>

---

## 4. 함수 구조

```
types 모듈 함수 계층
├── 데이터 구조체 관리
│   ├── 안테나 보정 데이터
│   │   ├── InitPcvs() ───────────── pcvs_t 초기화
│   │   ├── FreePcvs() ───────────── pcvs_t 해제
│   │   └── AddPcv() ─────────────── pcv_t 추가
│   ├── 궤도력 데이터
│   │   ├── InitEphs() ───────────── ephs_t 초기화
│   │   ├── FreeEphs() ───────────── ephs_t 해제
│   │   └── AddEph() ─────────────── eph_t 추가
│   ├── 관측 데이터
│   │   ├── InitObss() ───────────── obss_t 초기화
│   │   ├── FreeObss() ───────────── obss_t 해제
│   │   └── AddObs() ─────────────── obs_t 추가
│   ├── 측위 해 데이터
│   │   ├── InitSols() ───────────── sols_t 초기화
│   │   ├── FreeSols() ───────────── sols_t 해제
│   │   └── AddSol() ─────────────── sol_t 추가
│   ├── 수신기 상태 데이터
│   │   ├── InitRcvs() ───────────── rcvs_t 초기화
│   │   ├── FreeRcvs() ───────────── rcvs_t 해제
│   │   └── AddRcv() ─────────────── rcv_t 추가
│   └── 위성 상태 데이터
│       ├── InitSats() ───────────── sats_t 초기화
│       ├── FreeSats() ───────────── sats_t 해제
│       └── AddSat() ─────────────── sat_t 추가
├── 항법 데이터 관리
│   ├── InitNav() ────────────────── nav_t 초기화
│   └── FreeNav() ────────────────── nav_t 해제
├── 파일 관리
│   ├── InitFiles() ──────────────── files_t 초기화
│   ├── FreeFiles() ──────────────── files_t 해제
│   ├── AddFile() ────────────────── 파일명 추가
│   ├── InitBuffer() ─────────────── buffer_t 초기화
│   └── FreeBuffer() ─────────────── buffer_t 해제
└── 매트릭스 관리
    ├── Mat() ────────────────────── mat_t 생성
    ├── FreeMat() ────────────────── mat_t 해제
    ├── Idx() ────────────────────── idx_t 생성
    └── FreeIdx() ────────────────── idx_t 해제
```

---

## 5. 사용 예시

### 5.1 동적 배열 구조체 관리
```c
#include "gnsslib.h"

// 기본 초기화 패턴
obss_t obss = {0};          // 관측 데이터 집합
ephs_t ephs = {0};          // 궤도력 데이터 집합
sols_t sols = {0};          // 측위 해 집합

// 데이터 추가 (자동 메모리 확장)
obs_t newobs = {.time = 604800.0, .sat = 1, .rcv = 1};  // 1-based 인덱스
if (AddObs(&obss, &newobs)) {
    printf("관측 데이터 추가 성공: 총 %d개\n", obss.n);
}

// 메모리 해제
FreeObss(&obss);
FreeEphs(&ephs);
FreeSols(&sols);
```

### 5.2 항법 데이터 통합 관리
```c
// 항법 데이터 초기화
nav_t nav = {0};
InitNav(&nav);  // opt 포인터도 함께 초기화됨

// 처리 옵션 설정 (InitNav에서 이미 할당된 opt에 직접 설정)
nav.opt->mode = MODE_RTK;
nav.opt->nrcv = 2;
nav.opt->nfreq = 2;
nav.opt->elmask = 15.0 * D2R;    // 15도 고도각 마스크
nav.opt->err = 0.003;            // 3mm 위상 오차
nav.opt->errratio = 300.0;       // 코드/위상 오차 비율

// 관측소 정보 설정 (배열은 0-based, 하지만 수신기 ID는 1-based)
strcpy(nav.sta[0].name, "YONS");     // 첫 번째 수신기 (rcv ID = 1)
nav.sta[0].pos[0] = -3042956.0;      // ECEF X [m]
nav.sta[0].pos[1] =  4045491.0;      // ECEF Y [m]
nav.sta[0].pos[2] =  3858950.0;      // ECEF Z [m]

// 사용 후 해제
FreeNav(&nav);  // nav.opt도 함께 안전하게 해제됨
```

### 5.3 타입별 문자열 처리
```c
// 위성 문자열 생성
satStr_t satstr;
strcpy(satstr.str, "G01");
printf("위성: %s\n", satstr.str);

// 달력 시간 구조체
cal_t cal = {2025, 1, 15, 12, 30, 45.123};
printf("시간: %04d/%02d/%02d %02d:%02d:%06.3f\n",
       cal.year, cal.mon, cal.day, cal.hour, cal.min, cal.sec);

// 달력 문자열
calStr_t calstr;
sprintf(calstr.str, "%04d/%02d/%02d %02d:%02d:%06.3f",
        cal.year, cal.mon, cal.day, cal.hour, cal.min, cal.sec);
printf("시간 문자열: %s\n", calstr.str);

// 관측 코드 문자열
codeStr_t codestr;
strcpy(codestr.str, "L1C");
printf("관측 코드: %s\n", codestr.str);
```

### 5.4 파일 관리 예시
```c
#include "gnsslib.h"

// 파일 세트 관리 - 라이브러리 함수 사용
file_t files = {0};

// 안전한 초기화
InitFiles(&files.obsfiles);
InitFiles(&files.navfiles);

// 관측 파일 추가 (라이브러리 함수 사용)
if (AddFile(&files.obsfiles, "obs1.rnx")) {
    printf("관측 파일 1 추가 성공\n");
}
if (AddFile(&files.obsfiles, "obs2.rnx")) {
    printf("관측 파일 2 추가 성공\n");
}

// 항법 파일 추가
if (AddFile(&files.navfiles, "nav.rnx")) {
    printf("항법 파일 추가 성공\n");
}

printf("관측 파일 %d개, 항법 파일 %d개\n",
       files.obsfiles.n, files.navfiles.n);

// 안전한 메모리 해제 (라이브러리 함수 사용)
FreeFiles(&files.obsfiles);
FreeFiles(&files.navfiles);
```

### 5.5 매트릭스 타입 사용
```c
#include "gnsslib.h"

// 3x3 double 매트릭스 생성 (라이브러리 함수 사용)
mat_t *A = Mat(3, 3, DOUBLE);
if (A == NULL) {
    printf("매트릭스 생성 실패\n");
    return;
}

// 단위 행렬 설정 (안전한 인라인 함수 사용)
for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
        double value = (i == j) ? 1.0 : 0.0;
        MatSetD(A, i, j, value);  // 인라인 함수로 안전한 접근
    }
}

// 인덱스 벡터 생성 (라이브러리 함수 사용)
idx_t *idx = Idx(3, INT);
if (idx == NULL) {
    printf("인덱스 벡터 생성 실패\n");
    FreeMat(A);
    return;
}

// 인덱스 설정 (안전한 인라인 함수 사용)
IdxSetI(idx, 0, 0);  // 첫 번째 인덱스
IdxSetI(idx, 1, 2);  // 두 번째 인덱스
IdxSetI(idx, 2, 1);  // 세 번째 인덱스

printf("매트릭스 타입: %d, 크기: %dx%d\n", A->type, A->rows, A->cols);
printf("인덱스 타입: %d, 길이: %d\n", idx->type, idx->n);

// 값 확인 (안전한 인라인 함수 사용)
printf("A[0][0] = %.1f\n", MatGetD(A, 0, 0));
printf("idx[1] = %d\n", IdxGetI(idx, 1));

// 안전한 메모리 해제 (라이브러리 함수 사용)
FreeMat(A);
FreeIdx(idx);
```

### 5.6 구조체 상태 관리
```c
// 수신기 상태 데이터
rcv_t rcv = {
    .time = 604800.0,           // GPST [s]
    .rcv = 0,                   // 수신기 인덱스
    .rr = {-3042956.0, 4045491.0, 3858950.0},  // ECEF 위치 [m]
    .zwd = 0.12,                // 천정 습지연 [m]
};

// 시스템별 수신기 시계 설정
rcv.dtr[0] = 1.234e-6 * CLIGHT;  // GPS 시계 [m]
rcv.dtr[1] = 1.235e-6 * CLIGHT;  // GLONASS 시계 [m]

// 위성 상태 데이터
sat_t sat = {
    .time = 604800.0,           // GPST [s]
    .sat = 1,                   // 위성 인덱스 (GPS PRN 1)
    .iode = 100,                // IODE
    .dts = -2.345e-5 * CLIGHT,  // 위성 시계 [m]
};

printf("수신기 %d 위치: [%.1f, %.1f, %.1f] m\n",
       rcv.rcv, rcv.rr[0], rcv.rr[1], rcv.rr[2]);
printf("위성 %d 시계: %.6f m\n", sat.sat, sat.dts);
```

### 5.7 메모리 안전성 패턴
```c
#include "gnsslib.h"

// 안전한 구조체 초기화
pcvs_t pcvs = {0};              // 모든 멤버 0으로 초기화
InitPcvs(&pcvs);               // 라이브러리 초기화 함수 사용

// 데이터 추가 전 유효성 검사
pcv_t pcv = {
    .sat = 1,                   // GPS PRN 1
    .ts = 604800.0,             // 유효시작 시간
    .te = 608400.0              // 유효종료 시간
};
strcpy(pcv.type, "BLOCK IIF");
strcpy(pcv.serial, "G001");

// 라이브러리 함수를 통한 안전한 데이터 추가
if (AddPcv(&pcvs, &pcv)) {
    printf("PCV 데이터 추가 성공: 총 %d개\n", pcvs.n);
} else {
    printf("PCV 데이터 추가 실패\n");
}

// 라이브러리 함수를 통한 안전한 메모리 해제
FreePcvs(&pcvs);

printf("메모리 안전성 패턴 완료\n");
```

---

## 6. 구현 특성

### 6.1 메모리 관리 방식
- **동적 확장**: 모든 `xxxs_t` 구조체는 필요에 따라 자동으로 메모리를 확장합니다
- **2배 증가 전략**: 메모리 부족 시 현재 크기의 2배로 재할당하여 재할당 빈도를 최소화합니다
- **분리 관리**: 실제 사용량 (`n`)과 할당 크기 (`nmax`)를 분리하여 메모리 효율성을 확보합니다
- **메모리 안전**: realloc() 사용 시 실패 시에도 기존 데이터를 보존하는 안전한 패턴 적용

### 6.2 타입 안전성 체계
- **명시적 타입**: `type_t` 열거형을 통해 매트릭스와 인덱스의 데이터 타입을 명시합니다
- **void 포인터 관리**: 제네릭 구조체에서 void 포인터와 타입 필드를 결합한 안전한 캐스팅
- **컴파일 검증**: 구조체 멤버 타입 불일치를 컴파일 단계에서 감지 가능
- **타입 일관성**: 동일 모듈 내 모든 데이터 구조의 일관된 타입 정의

### 6.3 시간 표현 통일성
- **GPST 기준**: 모든 시간은 GPS Time 기준으로 통일하여 시간 변환 오차를 최소화합니다
- **double 정밀도**: IEEE 754 double 타입으로 나노초 단위 정밀도를 제공합니다
- **연속 시간**: 주차 롤오버 문제를 해결한 연속 시간 표현 사용
- **시간 일관성**: 모든 모듈에서 동일한 시간 기준 및 단위 적용

### 6.4 인덱싱 최적화
- **1-based 인덱스**: 위성(1~NSAT), 수신기(1~NRCV), 주파수(1~NFREQ)의 1-based 정수 관리
- **배열 최적화**: 연속 인덱스를 통해 배열 접근 성능을 최대화하고 메모리 지역성 향상
- **시스템 독립**: PRN 번호와 무관한 내부 인덱스로 다중 GNSS 시스템 통합 관리
- **인덱스 매핑**: 외부 식별자(PRN, 시스템 ID)와 내부 인덱스 간 효율적 변환
- **오류 표현**: 모든 인덱스에서 0은 오류/무효 값을 의미

### 6.5 설계 한계 및 상수
```c
// 시스템 제한 상수 (const.h에서 정의)
#define NSAT    150     // 최대 위성 수
#define NRCV     50     // 최대 수신기 수
#define NFREQ     5     // 최대 주파수 수
#define NSYS      7     // 최대 GNSS 시스템 수
#define NBAND     5     // 최대 밴드 수
```

- **확장성**: 상수 값 변경만으로 시스템 크기 조정 가능
- **메모리 효율**: 고정 크기 배열을 통한 예측 가능한 메모리 사용량
- **성능 최적화**: 컴파일 타임 상수를 통한 루프 언롤링 및 최적화 지원

### 6.6 플랫폼 호환성
- **표준 C99**: 모든 타입은 C99 표준을 준수하여 크로스 플랫폼 호환성 보장
- **엔디안 독립**: 네트워크 바이트 순서나 바이트 순서에 의존하지 않는 구조체 설계
- **패딩 일관성**: 구조체 멤버 정렬을 고려한 메모리 레이아웃으로 플랫폼별 차이 최소화
- **컴파일러 호환**: GCC, Clang, MSVC 등 주요 컴파일러에서 동일한 동작 보장

### 6.7 오류 처리 철학
- **방어적 프로그래밍**: 모든 공개 함수에서 입력 매개변수 NULL 체크 수행
- **경계 검사**: 배열 및 문자열 접근 시 범위 검증으로 버퍼 오버플로우 방지
- **초기화 보장**: 모든 구조체는 명시적 초기화 또는 0 초기화로 정의되지 않은 동작 방지
- **복구 가능성**: 오류 발생 시에도 기존 데이터 구조의 일관성 유지

### 6.8 확장성 설계
- **모듈형 구조**: 새로운 GNSS 시스템 추가 시 기존 구조체 재사용 가능
- **버전 호환성**: 구조체 확장 시 기존 코드와의 호환성 유지 가능한 설계
- **플러그인 지원**: 새로운 데이터 타입 추가를 위한 확장 포인트 제공
- **성능 확장**: 멀티스레딩 환경에서의 데이터 구조 안전성 고려

---

**이 모듈은 GNSS 라이브러리의 모든 데이터 구조를 중앙에서 관리하며, 타입 안전성과 메모리 효율성을 보장합니다.**
