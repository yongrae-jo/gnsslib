# ★ GNSSLIB: 다중 GNSS 신호 처리 라이브러리

GNSSLIB는 GPS, GLONASS, Galileo, BeiDou, QZSS, IRNSS, SBAS 등 다양한 위성항법시스템(GNSS) 신호를 정밀하게 처리하는 고성능 C 라이브러리입니다. 크로스 플랫폼(Windows, Linux, macOS) 환경에서 동작하며, 실시간 위치 결정, 항법, 위성 데이터 해석을 위한 표준화된 수치 연산 및 데이터 변환 기능을 제공합니다.

---

## ■ 프로젝트 목적

- **정밀 GNSS 측위** 및 항법 솔루션 제공
- **다중 GNSS 시스템** 지원 및 통합 데이터 처리
- **고성능 행렬/벡터 연산** 기반의 수치 알고리즘 제공
- **시간/위성/관측 데이터**의 표준화 및 변환 자동화
- **확장성/유지보수성**을 고려한 모듈화 설계

---

## ■ 전체 구성

```
GNSSLIB/
├── include/        # 공개 헤더 (API, 데이터 타입, 상수)
├── src/            # C 소스 코드 (모듈별 구현)
├── docs/           # 공식 모듈별 문서 및 AI 가이드라인
├── lib/            # 빌드된 라이브러리
├── data/           # 테스트 데이터 (RINEX 파일 등)
├── build/          # 빌드 임시 파일
├── .vscode/        # 개발 환경 설정
├── Makefile        # 빌드 시스템
└── Project.md      # 프로젝트 총괄 문서 (본 파일)
```

---

## ■ 주요 모듈 및 기능

### ◆ 1. Types 모듈 (`types.h`)

- **목적**: GNSS 라이브러리의 모든 데이터 구조를 정의하는 중앙 집중형 타입 시스템
- **주요 데이터 타입**:
  - **기본 구조체**: `obs_t`, `eph_t`, `sol_t`, `rcv_t`, `sat_t`
  - **집합 구조체**: `obss_t`, `ephs_t`, `sols_t`, `rcvs_t`, `sats_t`
  - **시간/문자열**: `cal_t`, `calStr_t`, `satStr_t`, `codeStr_t`
  - **파일 관리**: `files_t`, `file_t`, `buffer_t`, `lineinfo_t`
  - **매트릭스**: `mat_t`, `idx_t`, `type_t`
- **특징**:
  - 1-based 인덱스 체계 (위성, 수신기, 주파수)
  - GPST 기준 통일 시간 체계
  - 동적 메모리 관리 패턴 적용

### ◆ 2. Common 모듈 (`common.h`, `common.c`)

- **목적**: 위성 인덱스 변환, 시간 변환, GLONASS FCN 관리, 좌표 변환 등 GNSS 처리의 핵심 유틸리티 제공
- **주요 함수 트리**:
```
Common
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
│   └── 시간 체계 간 변환
│       ├── Gpst2Utc() ───── GPS Time → UTC
│       ├── Utc2Gpst() ───── UTC → GPS Time
│       ├── Gpst2Bdt() ───── GPS Time → BeiDou Time
│       └── Bdt2Gpst() ───── BeiDou Time → GPS Time
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
├── GNSS 분석 함수
│   └── Dops() ────────────── DOP 값 계산
└── GNSS 보정 모델
    ├── RcvAntModel() ─────── 수신기 안테나 보정
    ├── TropoMapF() ───────── 대류권 매핑함수
    ├── TropoModel() ──────── 대류권 지연 모델
    ├── IonoModel() ───────── 전리층 지연 모델
    └── MeasVar() ─────────── 관측값 분산 계산
```

### ◆ 3. Matrix 모듈 (`matrix.h`, `matrix.c`)

- **목적**: GNSS 수치해석을 위한 고성능 행렬/벡터 연산 및 필터/추정 알고리즘 제공
- **주요 함수 트리**:
```
Matrix
├── 생성/해제
│   ├── Mat() ─────────────── 행렬 생성
│   ├── FreeMat() ─────────── 행렬 해제
│   ├── Idx() ─────────────── 인덱스 벡터 생성
│   └── FreeIdx() ─────────── 인덱스 벡터 해제
├── 특수 행렬
│   ├── Eye() ─────────────── 단위 행렬
│   ├── Zeros() ───────────── 영 행렬
│   └── Ones() ────────────── 1 행렬
├── 기본 연산
│   ├── MatCopy() ─────────── 행렬 복사
│   ├── MatAdd() ──────────── 행렬 덧셈 (C = aA + bB)
│   ├── MatMul() ──────────── 행렬 곱셈 (C = aA*B + bC)
│   ├── MatInv() ──────────── 역행렬
│   └── MatTr() ───────────── 전치
├── 벡터 연산
│   ├── Dot() ─────────────── 내적 (출력 매개변수)
│   ├── Cross3() ──────────── 외적 (출력 매개변수)
│   └── Norm() ────────────── 노름 (반환값)
├── 행렬 분석
│   └── MatDet() ──────────── 행렬식
└── 고급 알고리즘
    ├── Lsq() ─────────────── 가중 최소제곱법
    ├── Ekf() ─────────────── Joseph 형태 확장칼만필터
    └── Interp() ──────────── 1차원 보간/외삽
```

### ◆ 4. Obs 모듈 (`obs.h`, `obs.c`)

- **목적**: GNSS 관측 코드, 주파수, 밴드, 인덱스 등 관측 데이터의 표준화 및 변환
- **특징**:
  - 67개 GNSS 관측 코드 완전 지원
  - 시스템별 밴드/주파수/인덱스 매핑
  - GLONASS FCN 자동 적용
  - RINEX v2/v3 표준 호환

### ◆ 5. Ephemeris 모듈 (`ephemeris.h`, `ephemeris.c`)

- **목적**: 방송궤도력을 이용한 위성 위치 및 시계 계산
- **주요 함수 트리**:
```
Ephemeris
├── 궤도력 데이터 구조체 관리
│   ├── InitEphs() ───────────── 궤도력 데이터셋 초기화
│   ├── FreeEphs() ───────────── 궤도력 데이터셋 해제
│   ├── AddEph() ─────────────── 궤도력 데이터 추가
│   └── SortEphs() ───────────── 궤도력 데이터 정렬/중복제거
├── 위성 위치/시계 계산
│   └── SatPosClkBrdc() ──────── 모든 시스템 위성 계산
├── 궤도력 데이터 관리
│   ├── TestEph() ────────────── 궤도력 유효성 검사
│   └── SelectEph() ──────────── 최적 궤도력 선택
├── 정확도 지수 변환
│   ├── Ura2Idx()/Idx2Ura() ──── URA 변환
│   └── Sisa2Idx()/Idx2Sisa() ── SISA 변환
└── 궤도력 타입 관리
    ├── GetEphType() ─────────── 궤도력 타입 조회
    └── SetEphType() ─────────── 궤도력 타입 설정
```

### ◆ 6. Files 모듈 (`files.h`, `files.c`)

- **목적**: GNSS 데이터 파일의 입출력, 버퍼링, 형식 변환을 위한 통합 파일 처리
- **주요 함수 트리**:
```
Files
├── 파일명 배열 관리
│   ├── AddFileName() ──────── 파일명 추가 (동적 확장)
│   └── GetFileName() ──────── 파일명 조회 (인덱스 기반)
├── 파일 세트 관리
│   ├── InitFile() ─────────── 모든 파일 타입 초기화
│   └── FreeFile() ─────────── 모든 파일 타입 해제
├── 버퍼 관리
│   ├── InitBuff() ─────────── 버퍼 구조체 초기화
│   ├── GetBuff() ──────────── 파일을 메모리 버퍼로 로드
│   ├── FreeBuff() ─────────── 버퍼 메모리 해제
│   └── GetLine() ──────────── 버퍼에서 라인 추출 (inline)
└── 파일 읽기 함수
    ├── ReadObsFiles() ──────── 관측 파일 읽기
    └── ReadFiles() ─────────── 모든 파일 타입 일괄 읽기
```

### ◆ 7. RINEX 모듈 (`rinex.h`, `rinex.c`)

- **목적**: RINEX(Receiver Independent Exchange Format) 파일을 읽고 처리
- **주요 함수**:
  - `IsRinexObs()`: RINEX 관측 파일명 검증
  - `ReadRnxObs()`: RINEX v2/v3 관측 파일 읽기
- **특징**:
  - RINEX v2/v3 자동 감지 및 처리
  - 2-pass 파싱으로 메모리 최적화
  - 다중 수신기 데이터 동시 처리

### ◆ 8. Option 모듈 (`option.h`, `option.c`)

- **목적**: GNSS 처리 옵션 및 파라미터 통합 관리
- **주요 설정**:
  - 처리 모드, 엔진 선택
  - 주파수 수, 고도각 마스크
  - 모호정수 해결 옵션
  - 프로세스 잡음 모델
  - 위성 제외 목록

### ◆ 9. Solution 모듈 (`solution.h`, `solution.c`)

- **목적**: 측위 결과 및 품질 정보 관리
- **주요 기능**:
  - 측위 해 저장 및 검색
  - 품질 지표 관리 (DOP, ratio 등)
  - 시계열 결과 분석

---

## ■ 개발/문서화 규칙

- **함수/타입 네이밍**: PascalCase(함수), camelCase+_t(타입), UPPER_SNAKE_CASE(상수)
- **문서화**: 각 모듈별 상세 마크다운 문서 제공 (docs/)
- **수식 표기**: 벡터 $\boldsymbol{a}$, 행렬 $\mathbf{A}$, 스칼라 $a$, 영어단어 $\text{week}$
- **코드/문서 일치**: 실제 소스/헤더에 없는 내용은 문서에 포함하지 않음
- **플랫폼 호환성**: Windows, Linux, macOS 완전 지원

---

## ■ 예시 코드

### 1. GNSS 데이터 파일 처리 및 분석
```c
#include "gnsslib.h"

int main() {
    // 데이터 구조체 초기화
    file_t file = {0};
    nav_t nav = {0};
    obss_t obs = {0};

    InitFile(&file);
    InitNav(&nav);
    InitObss(&obs);

    // RINEX 파일 추가
    AddFileName(&file.obsfiles, "YONS00KOR_R_20250010300_01H_01S_GO.rnx");
    AddFileName(&file.obsfiles, "SJU200KOR_R_20251522300_01H_01S_MO.rnx");

    // 파일 읽기 및 처리
    ReadFiles(&file, &nav, &obs);

    printf("총 관측 데이터: %d개\n", obs.n);
    printf("수신기 수: %d개\n", nav.nsta);

    // 첫 관측 데이터 정보
    if (obs.n > 0) {
        cal_t cal = Time2Cal(obs.obs[0].time);
        satStr_t satstr = Sat2Str(obs.obs[0].sat);
        printf("첫 관측: %04d/%02d/%02d %02d:%02d:%06.3f, 위성: %s\n",
               cal.year, cal.mon, cal.day, cal.hour, cal.min, cal.sec, satstr.str);
    }

    // 메모리 해제
    FreeObss(&obs);
    FreeNav(&nav);
    FreeFile(&file);

    return 0;
}
```

### 2. 행렬 연산 및 최소제곱법
```c
// 가중 최소제곱법 예시
mat_t *H = Mat(4, 3, DOUBLE);      // 설계행렬
mat_t *y = Mat(4, 1, DOUBLE);      // 관측벡터
mat_t *R = Eye(4, 4, DOUBLE);      // 가중행렬
mat_t *x = Mat(3, 1, DOUBLE);      // 추정 매개변수
mat_t *P = Mat(3, 3, DOUBLE);      // 공분산 행렬

// 가중행렬 설정 (관측 분산)
for (int i = 0; i < 4; i++) {
    MatSetD(R, i, i, 0.01);  // 0.01 m² 분산
}

// 최소제곱법 수행
if (Lsq(H, y, R, x, P, NULL)) {
    printf("추정 성공!\n");
    for (int i = 0; i < 3; i++) {
        printf("x[%d] = %.3f ± %.3f\n", i,
               MatGetD(x, i, 0), sqrt(MatGetD(P, i, i)));
    }
}

// 메모리 해제
FreeMat(H); FreeMat(y); FreeMat(R);
FreeMat(x); FreeMat(P);
```

### 3. 위성 위치 계산
```c
// 방송궤도력을 이용한 위성 위치 계산
nav_t nav = {0};
mat_t *rs = Mat(1, 6, DOUBLE);   // 위성 위치/속도
mat_t *dts = Mat(1, 2, DOUBLE);  // 시계 바이어스/드리프트
double var;
eph_t eph;

double ephtime = 604800.0;  // 궤도력 시각
double time = 604801.0;     // 신호 송신 시각
int sat = Prn2Sat(1, 5);    // GPS PRN 5

if (SatPosClkBrdc(ephtime, time, sat, &nav, -1, rs, dts, &var, &eph)) {
    printf("위성 위치: [%.1f, %.1f, %.1f] km\n",
           MatGetD(rs, 0, 0)/1000, MatGetD(rs, 0, 1)/1000, MatGetD(rs, 0, 2)/1000);
    printf("위성 시계: %.9f s\n", MatGetD(dts, 0, 0));
}

FreeMat(rs); FreeMat(dts);
```

---

## ■ 참고 문서

- [AI 작업 가이드라인](docs/AI_GUIDELINES.md)
- [Types 모듈](docs/types.md)
- [Common 모듈](docs/common.md)
- [Matrix 모듈](docs/matrix.md)
- [Obs 모듈](docs/obs.md)
- [Ephemeris 모듈](docs/ephemeris.md)
- [Files 모듈](docs/files.md)
- [RINEX 모듈](docs/rinex.md)
- [Option 모듈](docs/option.md)

---

**© 2024 Yongrae Jo. All Rights Reserved.**
**본 프로젝트의 모든 내용은 실제 소스 코드와 100% 일치하도록 관리됩니다.**

---
