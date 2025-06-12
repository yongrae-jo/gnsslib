# ★ GNSSLIB: 다중 GNSS 신호 처리 라이브러리

<script>
  window.MathJax = {
    tex: {
      inlineMath: [['$', '$'], ['\\(', '\\)']]
    }
  };
</script>
<script async src="https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-mml-chtml.js"></script>


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
├── include/      # 공개 헤더 (API, 데이터 타입, 상수)
├── src/          # C 소스 코드 (모듈별 구현)
├── docs/         # 공식 모듈별 문서 및 AI 가이드라인
├── lib/          # 빌드된 라이브러리
├── build/        # 빌드 임시 파일
├── .vscode/      # 개발 환경 설정
├── Makefile      # 빌드 시스템
└── Project.md    # 프로젝트 총괄 문서 (본 파일)
```

---

## ■ 주요 모듈 및 기능

### ◆ 1. Common 모듈 (`common.h`, `common.c`)

- **목적**: 위성 인덱스 변환, 시간 변환, GLONASS FCN 관리 등 GNSS 처리의 핵심 유틸리티 제공
- **주요 데이터 타입**: `satStr_t`, `cal_t`, `calStr_t`, `nav_t`, `pcv_t`, `pcvs_t`, `sta_t`
- **주요 함수 트리**
```
Common
├── 위성 인덱스 변환
│   ├── Prn2Sat()
│   ├── Sat2Prn()
│   ├── Str2Sat()
│   └── Sat2Str()
├── 시간 변환
│   ├── Cal2Time()
│   ├── Time2Cal()
│   ├── Gpst2Time()
│   ├── Time2Gpst()
│   ├── Bdt2Time()
│   └── Time2Bdt()
├── 시간 체계 변환
│   ├── Gpst2Utc()
│   ├── Utc2Gpst()
│   ├── Gpst2Bdt()
│   └── Bdt2Gpst()
├── 기타
│   ├── TimeGet()
│   ├── Time2Doy()
│   ├── Str2Cal()
│   └── Cal2Str()
├── GLONASS FCN
│   ├── GetFcn()
│   ├── SetFcn()
│   └── SetDefaultFcn()
└── 네비게이션 데이터 관리
    ├── InitNav()
    └── FreeNav()
```
- **특징**
  - 모든 시간 데이터는 GPST 기준 UNIX timestamp로 통합
  - 위성/PRN/문자열 간 상호 변환 완전 지원
  - 윤초, FCN 등 GNSS 특화 테이블 내장

---

### ◆ 2. Matrix 모듈 (`matrix.h`, `matrix.c`)

- **목적**: GNSS 수치해석을 위한 고성능 행렬/벡터 연산 및 필터/추정 알고리즘 제공
- **주요 데이터 타입**: `mat_t`, `idx_t`, `type_t`
- **주요 함수 트리**
```
Matrix
├── 생성/해제
│   ├── Mat()
│   ├── FreeMat()
│   ├── Idx()
│   └── FreeIdx()
├── 특수 행렬
│   ├── Eye()
│   ├── Zeros()
│   └── Ones()
├── 연산
│   ├── MatCopy()
│   ├── MatCopyIn()
│   ├── MatAdd()
│   ├── MatAddIn()
│   ├── MatMul()
│   ├── MatMulIn()
│   ├── MatInv()
│   └── MatInvIn()
├── 변환
│   ├── MatTr()
│   └── MatTrIn()
├── 벡터 연산 (mat_t, cols=1)
│   ├── Dot()
│   ├── Cross3()
│   └── Norm()
├── 분석
│   └── MatDet()
└── 고급 알고리즘
    ├── Lsq()   // 최소제곱법
    └── Ekf()   // 확장 칼만필터
```
- **특징**
  - 32바이트 정렬 메모리, column-major 저장, SIMD 최적화
  - 벡터 연산은 `mat_t`(cols=1)만 지원, 별도 vec_t 없음
  - 인덱스 벡터는 `idx_t` 타입 사용
  - LSQ: $\mathbf{L} = (\mathbf{H}^T \mathbf{R}^{-1} \mathbf{H})^{-1} \mathbf{H}^T \mathbf{R}^{-1}$, $\mathbf{x} = \mathbf{L}\mathbf{y}$, $\mathbf{P} = \mathbf{L}\mathbf{R}\mathbf{L}^T$
  - EKF: $\mathbf{K} = \mathbf{P}\mathbf{H}^T(\mathbf{H}\mathbf{P}\mathbf{H}^T+\mathbf{R})^{-1}$, $\mathbf{x} = \mathbf{x} + \mathbf{K}\mathbf{v}$, $\mathbf{P} = (\mathbf{I}-\mathbf{K}\mathbf{H})\mathbf{P}$

---

### ◆ 3. Obs 모듈 (`obs.h`, `obs.c`)

- **목적**: GNSS 관측 코드, 주파수, 밴드, 인덱스 등 관측 데이터의 표준화 및 변환
- **주요 데이터 타입**: 관측 코드 ID, 밴드 ID, 주파수 인덱스, 코드 문자열
- **주요 함수 트리**
```
Obs
├── 코드 변환
│   ├── Str2Code()
│   ├── Code2Str()
│   ├── Code2Band()
│   ├── Code2Freq()
│   └── Code2Fidx()
├── 밴드 변환
│   ├── Band2Str()
│   ├── Str2Band()
│   └── Band2Freq()
└── 시스템별 매핑
    ├── Code2Fidx_GPS()
    └── Band2Freq_GLO()
```
- **특징**
  - 67개 GNSS 관측 코드, 시스템별 밴드/주파수/인덱스 완전 지원
  - GLONASS FCN 자동 적용, RINEX 표준 완전 호환

---

### ◆ 4. Ephemeris 모듈 (`ephemeris.h`, `ephemeris.c`)

- **목적**: GNSS 시스템별 방송궤도력 타입 관리
- **주요 데이터 타입**: 시스템별 궤도력 타입 배열
- **주요 함수 트리**
```
Ephemeris
├── GetEphType()
└── SetEphType()
```
- **특징**
  - GPS, GLONASS, Galileo, BeiDou, QZSS, IRNSS, SBAS 등 모든 시스템 지원
  - Galileo 다중 타입(I/NAV, F/NAV) 지원

---

### ◆ 5. Option 모듈 (`option.h`, `option.c`)

- **목적**: GNSS 처리 옵션 및 파라미터 통합 관리
- **주요 데이터 타입**: `opt_t`
- **주요 함수 트리**
```
Option
└── SetDefaultOpt()
```
- **특징**
  - 처리 모드, 엔진, 주파수, 위성 제외 목록 등 다양한 옵션 제공

---

## ■ 개발/문서화 규칙

- **함수/타입 네이밍**: PascalCase(함수), camelCase+_t(타입), UPPER_SNAKE_CASE(상수)
- **문서화**: 각 모듈별 상세 마크다운 문서 제공 (docs/)
- **수식 표기**: 벡터 $\boldsymbol{a}$, 행렬 $\mathbf{A}$, 스칼라 $a$, 영어단어 $\text{week}$
- **코드/문서 일치**: 실제 소스/헤더에 없는 내용은 문서에 포함하지 않음
- **플랫폼 호환성**: Windows, Linux, macOS 완전 지원

---

## ■ 예시 코드

```c
// 행렬 및 벡터(1열) 생성 및 연산
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
{
    double dot;
    if (Dot(v1, v2, &dot)) {
        printf("dot = %f\n", dot);
    }
}

// 벡터 외적
{
    mat_t *cross = Mat(3, 1, DOUBLE);
    if (Cross3(v1, v2, cross)) {
        // cross 사용
    }
    FreeMat(cross);
}

// 벡터 노름
{
    double norm = Norm(v1);
}

// 메모리 해제
FreeMat(A); FreeMat(B); FreeMat(v1); FreeMat(v2);
```

---

## ■ 참고 문서

- [AI 작업 가이드라인](docs/AI_GUIDELINES.md)
- [Common 모듈](docs/common.md)
- [Matrix 모듈](docs/matrix.md)
- [Obs 모듈](docs/obs.md)
- [Ephemeris 모듈](docs/ephemeris.md)
- [Option 모듈](docs/option.md)

---

**© 2024 Yongrae Jo. All Rights Reserved.**
**본 프로젝트의 모든 내용은 실제 소스 코드와 100% 일치하도록 관리됩니다.**

---
