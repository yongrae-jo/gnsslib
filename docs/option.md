---
layout: default
title: GNSS 처리 옵션 관리 모듈 (option)
---

# GNSS 처리 옵션 관리 모듈 (option)

GNSS 측위 처리에 필요한 다양한 옵션과 매개변수를 관리하는 핵심 모듈입니다.

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

`option` 모듈은 GNSS 데이터 처리에 사용되는 모든 설정 값을 `opt_t` 라는 단일 구조체를 통해 통합 관리합니다. 사용자는 이 구조체의 필드를 수정하여 측위 엔진의 동작 방식을 제어할 수 있습니다. `SetDefaultOpt()` 함수를 통해 모든 옵션을 표준적인 기본값으로 쉽게 초기화할 수 있으며, 이 값들은 일반적인 SPP(Single Point Positioning) 시나리오에 최적화되어 있습니다.

**핵심 관리 영역**:
- **처리 모드**: SPP(Single Point Positioning) 설정
- **엔진 타입**: LSQ(Least Square) vs EKF(Extended Kalman Filter)
- **다중 주파수**: 주파수 개수 및 수신기 개수 설정
- **궤도력 옵션**: 방송궤도력 vs 정밀궤도력 선택
- **대기 보정**: 이온층 및 대류층 보정 모델 설정
- **프로세스 노이즈**: EKF 필터링을 위한 노이즈 매개변수
- **품질 제어**: 고도각 마스크, GDOP 임계값 설정
- **위성 제외**: 특정 위성 제외 목록 관리

---

## 2. 데이터 타입 구조

```
option 모듈 데이터 계층
├── 처리 설정
│   ├── mode ──────────────── 처리 모드 (SPP, RTK)
│   ├── engine ────────────── 처리 엔진 (LSQ, EKF)
│   ├── nrcv ──────────────── 수신기 개수
│   └── nfreq ─────────────── 주파수 개수
├── 보정 모델 설정
│   ├── ephopt ────────────── 궤도력 옵션 (방송/정밀)
│   ├── posopt ────────────── 위치 옵션 (추정/고정)
│   ├── ionoopt ───────────── 이온층 옵션 (끄기/방송/추정)
│   └── tropoopt ──────────── 대류층 옵션 (끄기/Saas/추정)
├── 고급 처리 설정
│   ├── par ───────────────── 부분 모호정수 해결
│   ├── cascade ───────────── 계단식 모호정수 해결
│   ├── gloaropt ──────────── GLONASS 모호정수 해결
│   └── dynamics ──────────── 수신기 동역학 옵션
├── 필터링 매개변수
│   ├── maxout ────────────── 최대 단절 횟수
│   └── minlock ───────────── 최소 록 횟수
├── 시간 범위 설정
│   ├── ts ────────────────── 처리 시작 시간
│   └── te ────────────────── 처리 종료 시간
├── 측정 오차 설정
│   ├── err ───────────────── 반송파 위상 오차
│   └── errratio ──────────── 의사거리 오차 비율
├── EKF 프로세스 노이즈
│   ├── procnoiseAmb ──────── 위상 모호정수 노이즈
│   ├── procnoiseTropo ────── 대류층 노이즈
│   ├── procnoiseIono ─────── 이온층 노이즈
│   ├── procnoiseHacc ─────── 수평 가속도 노이즈
│   ├── procnoiseVacc ─────── 수직 가속도 노이즈
│   ├── procnoiseDtr ──────── 수신기 시계 노이즈
│   ├── procnoiseDts ──────── 위성 시계 노이즈
│   ├── procnoiseIsb ──────── 시스템간 편향 노이즈
│   ├── procnoiseCbr ──────── 수신기 코드 편향 노이즈
│   ├── procnoiseCbs ──────── 위성 코드 편향 노이즈
│   ├── procnoisePbr ──────── 수신기 위상 편향 노이즈
│   └── procnoisePbs ──────── 위성 위상 편향 노이즈
├── 품질 제어 매개변수
│   ├── elmask ────────────── 고도각 마스크
│   └── maxgdop ───────────── 최대 GDOP 값
└── 위성 제외 관리
    └── exsats[NSAT] ──────── 제외 위성 목록
```

---

## 3. 데이터 타입 목록

### 3.1 opt_t 구조체
<details>
<summary>상세 설명</summary>

**목적**: GNSS 측위 처리에 필요한 모든 옵션과 매개변수 통합 관리

**정의**: `types.h`
```c
typedef struct opt {
    // 처리 설정
    int mode;                           // Processing mode
    int engine;                         // Processing engine
    int nrcv;                           // Number of receivers
    int nfreq;                          // Number of frequencies

    // 보정 모델 설정
    int ephopt;                         // Ephemeris option
    int posopt;                         // Position option
    int ionoopt;                        // Ionospheric option
    int tropoopt;                       // Tropospheric option

    // 고급 처리 설정
    int par;                            // Partial ambiguity resolution
    int cascade;                        // Cascading ambiguity resolution
    int gloaropt;                       // GLONASS ambiguity resolution
    int dynamics;                       // Receiver dynamics option

    // 필터링 매개변수
    int maxout;                         // Maximum outage count to reset state
    int minlock;                        // Minimum lock count to fix ambiguity

    // 시간 범위 설정
    double ts;                          // Processing time start [s] (0.0: all)
    double te;                          // Processing time end [s] (0.0: all)

    // 측정 오차 설정
    double err;                         // Carrier phase measurement error std [m] (zenith direction)
    double errratio;                    // Pseudorange measurement error ratio

    // EKF 프로세스 노이즈
    double procnoiseAmb;                // Phase ambiguity [cycle]
    double procnoiseTropo;              // Zenith wet delay [m]
    double procnoiseIono;               // Ionospheric delay [m]
    double procnoiseHacc;               // Horizontal acceleration [m/s^2]
    double procnoiseVacc;               // Vertical acceleration [m/s^2]
    double procnoiseDtr;                // Receiver clock [m]
    double procnoiseDts;                // Satellite clock [m]
    double procnoiseIsb;                // Inter-system bias [m]
    double procnoiseCbr;                // Receiver code bias [m]
    double procnoiseCbs;                // Satellite code bias [m]
    double procnoisePbr;                // Receiver phase bias [m]
    double procnoisePbs;                // Satellite phase bias [m]

    // 품질 제어 매개변수
    double elmask;                      // Elevation mask angle [rad]
    double maxgdop;                     // Maximum GDOP

    // 위성 제외 관리
    int exsats[NSAT];                   // Excluded satellites (!0: excluded)
} opt_t;
```

**특징**:
- 모든 GNSS 처리 옵션을 하나의 구조체로 통합합니다.
- `SetDefaultOpt()` 함수를 통해 SPP 모드, LSQ 엔진 등 표준 설정으로 초기화됩니다.
- 성능 향상을 위해 BeiDou GEO 위성을 기본적으로 제외 처리합니다.
- EKF 필터링을 위한 상세한 프로세스 노이즈 설정이 가능합니다.

</details>

---

## 4. 함수 구조

```
option 모듈 함수 계층
└── 옵션 관리
    └── SetDefaultOpt() ──────── 옵션 데이터 초기화
```

---

## 5. 함수 목록

### 5.1 옵션 관리 함수

#### SetDefaultOpt() - 옵션 데이터 초기화
<details>
<summary>상세 설명</summary>

**목적**: 옵션 구조체의 모든 필드를 표준 기본값으로 초기화

**입력**:
- `opt_t *opt`: 초기화할 옵션 구조체

**출력**:
- `void`: 반환값 없음

**함수 로직**:
1. 모든 정수형 및 실수형 옵션을 표준 기본값으로 설정합니다.
2. 제외 위성 목록(`exsats`)을 모두 0으로 초기화하여 모든 위성을 사용 가능하게 합니다.
3. BeiDou 시스템이 활성화된 경우, 성능에 영향을 줄 수 있는 GEO 위성(C01-C05, C59-C62)을 기본적으로 제외 목록에 추가합니다.

**주요 기본값**:
- **처리 모드**: `PROCMODE_SPP` (SPP 모드)
- **처리 엔진**: `ENGINE_LSQ` (최소제곱법)
- **보정 모델**: 방송 궤도력(`EPHOPT_BRDC`), 방송 이온층 모델(`IONOOPT_BRDC`), Saastamoinen 대류층 모델(`TROPOOPT_SAAS`)
- **측정 오차**: `err` = 0.003m (반송파 위상), `errratio` = 100
- **품질 제어**: `elmask` = 10도, `maxgdop` = 30

</details>

---

## 6. 사용 예시

### 6.1 기본 옵션 초기화 및 확인
```c
// 옵션 구조체 생성 및 기본값 초기화
opt_t opt;
SetDefaultOpt(&opt);

// 기본 설정 확인
printf("처리 모드: %d (SPP)\n", opt.mode);
printf("처리 엔진: %d (LSQ)\n", opt.engine);
printf("고도각 마스크: %.1f도\n", opt.elmask * R2D);
```

### 6.2 사용자 정의 설정 적용
```c
// EKF 엔진 및 이중 주파수 설정
opt.engine = ENGINE_EKF;
opt.nfreq = 2;

// 고도각 마스크 15도로 변경
opt.elmask = 15.0 * D2R;

// 정밀 궤도력 및 이온층 추정 모드 사용
opt.ephopt = EPHOPT_PREC;
opt.ionoopt = IONOOPT_EST;

printf("사용자 정의 설정 적용 완료\n");
```

### 6.3 EKF 프로세스 노이즈 조정
```c
// 동적 플랫폼용 설정
opt.dynamics = DYNOPT_ON;      // 동역학 모델 활성화
opt.procnoiseHacc = 1E-1;      // 수평 가속도 노이즈 증가
opt.procnoiseVacc = 1E-2;      // 수직 가속도 노이즈 증가

printf("EKF 노이즈 매개변수 조정 완료\n");
```

### 6.4 위성 제외 관리
```c
// 특정 GPS 위성 제외 (예: G01)
int sat = Str2Sat((satStr_t){"G01"});
if (sat > 0) {
    opt.exsats[sat] = 1;
}

// 제외된 위성 확인
for (int i = 1; i <= NSAT; i++) {
    if (opt.exsats[i]) {
        satStr_t satStr = Sat2Str(i);
        printf("제외된 위성: %s\n", satStr.str);
    }
}
```

---

## 7. 성능 특성

### 7.1 메모리 효율성
- **단일 구조체**: 모든 옵션을 `opt_t` 구조체 하나로 통합하여 관리 효율성을 높입니다.
- **정적 할당**: 고정 크기 구조체를 사용하여 동적 메모리 할당 및 해제에 따른 오버헤드가 없습니다.
- **배열 최적화**: 위성 제외 목록(`exsats`)은 정수형 배열로 구현되어 메모리 사용량이 예측 가능하고 효율적입니다.

### 7.2 기본값 최적화
- **실용적 설정**: 기본값은 일반적인 GNSS 단독 측위(SPP) 환경에 적합하게 설정되어 있습니다.
- **BeiDou 최적화**: 측위 정확도에 영향을 줄 수 있는 BeiDou 시스템의 정지궤도(GEO) 위성을 기본적으로 제외하여 안정성을 높입니다.
- **노이즈 균형**: EKF 관련 프로세스 노이즈 기본값은 필터의 수렴성과 정확도 사이의 균형을 고려하여 설정되었습니다.

### 7.3 확장성
- **모듈화 설계**: 새로운 옵션이 필요할 경우 `opt_t` 구조체에 필드를 추가하고 `SetDefaultOpt` 함수만 수정하면 되므로 확장이 용이합니다.
- **시스템 독립성**: 대부분의 옵션은 특정 GNSS 시스템에 종속되지 않아 범용적으로 적용할 수 있습니다.
- **동적 설정**: 프로그램 실행 중에도 옵션 값을 변경하여 처리 방식을 동적으로 제어할 수 있습니다.
