---
layout: default
title: GNSS 처리 옵션 관리 모듈 (option)
---

# ■ GNSS 처리 옵션 관리 모듈 (option)

GNSS 측위 처리에 필요한 다양한 옵션과 매개변수를 관리하는 핵심 모듈입니다.

## ■ 목차

1. [기본 개념](#▲-기본-개념)
2. [데이터 타입 구조](#▲-데이터-타입-구조)
3. [데이터 타입 목록](#▲-데이터-타입-목록)
4. [함수 구조](#▲-함수-구조)
5. [함수 목록](#▲-함수-목록)
6. [사용 예시](#▲-사용-예시)
7. [성능 특성](#▲-성능-특성)

---

## ▲ 기본 개념

```
option 모듈 함수 계층
└── 옵션 관리
    └── InitOpt() ──────────── 옵션 데이터 초기화
```

---

## ▲ 데이터 타입 구조

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

## ▲ 데이터 타입 목록

#### ◆ opt_t 구조체
<details>
<summary>상세 설명</summary>

**목적**: GNSS 측위 처리에 필요한 모든 옵션과 매개변수 통합 관리

**구조**:
```c
typedef struct opt {
    // 처리 설정
    int mode;               // 처리 모드
    int engine;             // 처리 엔진
    int nrcv;               // 수신기 개수
    int nfreq;              // 주파수 개수

    // 보정 모델 설정
    int ephopt;             // 궤도력 옵션
    int posopt;             // 위치 옵션
    int ionoopt;            // 이온층 옵션
    int tropoopt;           // 대류층 옵션

    // 고급 처리 설정
    int par;                // 부분 모호정수 해결
    int cascade;            // 계단식 모호정수 해결
    int gloaropt;           // GLONASS 모호정수 해결
    int dynamics;           // 수신기 동역학 옵션

    // 필터링 매개변수
    int maxout;             // 최대 단절 횟수
    int minlock;            // 최소 록 횟수

    // 시간 범위 설정
    double ts;              // 처리 시작 시간 [s]
    double te;              // 처리 종료 시간 [s]

    // 측정 오차 설정
    double err;             // 반송파 위상 오차 [m]
    double errratio;        // 의사거리 오차 비율

    // EKF 프로세스 노이즈
    double procnoiseAmb;    // 위상 모호정수 [cycle]
    double procnoiseTropo;  // 천정 습윤 지연 [m]
    double procnoiseIono;   // 이온층 지연 [m]
    double procnoiseHacc;   // 수평 가속도 [m/s^2]
    double procnoiseVacc;   // 수직 가속도 [m/s^2]
    double procnoiseDtr;    // 수신기 시계 [m]
    double procnoiseDts;    // 위성 시계 [m]
    double procnoiseIsb;    // 시스템간 편향 [m]
    double procnoiseCbr;    // 수신기 코드 편향 [m]
    double procnoiseCbs;    // 위성 코드 편향 [m]
    double procnoisePbr;    // 수신기 위상 편향 [m]
    double procnoisePbs;    // 위성 위상 편향 [m]

    // 품질 제어 매개변수
    double elmask;          // 고도각 마스크 [rad]
    double maxgdop;         // 최대 GDOP 값

    // 위성 제외 관리
    int exsats[NSAT];       // 제외 위성 목록
} opt_t;
```

**특징**:
- 모든 GNSS 처리 옵션을 하나의 구조체로 통합
- 기본값으로 SPP 모드, LSQ 엔진 설정
- BeiDou GEO 위성을 기본 제외 처리
- EKF 필터링을 위한 상세한 프로세스 노이즈 설정

</details>

---

## ▲ 함수 구조

```
option 모듈 함수 계층
└── 옵션 관리
    └── InitOpt() ──────────── 옵션 데이터 초기화
```

---

## ▲ 함수 목록

#### ◆ 옵션 관리 함수

##### ● InitOpt() - 옵션 데이터 초기화
<details>
<summary>상세 설명</summary>

**목적**: 옵션 구조체의 모든 필드를 기본값으로 초기화

**입력**:
- `opt_t *opt`: 초기화할 옵션 구조체

**출력**:
- `void`: 반환값 없음

**함수 로직**:
1. 모든 정수형 옵션을 기본값으로 설정
2. 측정 오차 매개변수를 표준값으로 초기화
3. EKF 프로세스 노이즈를 적절한 수준으로 설정
4. BeiDou GEO 위성을 기본 제외 목록에 추가

**기본값 설정**:
- SPP 모드, LSQ 엔진, 단일 수신기/주파수
- 방송 궤도력 및 이온층/대류층 모델 사용
- 3mm 반송파 위상 오차, 100:1 의사거리 비율
- 10도 고도각 마스크, 최대 GDOP 30

</details>

---

## ▲ 사용 예시

```c
// 옵션 구조체 생성 및 초기화
opt_t opt;
InitOpt(&opt);

// 기본 설정 확인
printf("처리 모드: %d (SPP)\n", opt.mode);
printf("처리 엔진: %d (LSQ)\n", opt.engine);
printf("수신기 개수: %d\n", opt.nrcv);
printf("주파수 개수: %d\n", opt.nfreq);
printf("고도각 마스크: %.1f도\n", opt.elmask * R2D);
```

### ◆ 커스텀 설정 적용
```c
// EKF 모드로 변경
opt.engine = ENGINE_EKF;

// 다중 주파수 설정
opt.nfreq = 2;

// 고도각 마스크 15도로 변경
opt.elmask = 15.0 * D2R;

// 정밀 궤도력 사용
opt.ephopt = EPHOPT_PREC;

// 이온층 추정 모드
opt.ionoopt = IONOOPT_EST;

printf("업데이트된 설정 적용 완료\n");
```

### ◆ EKF 프로세스 노이즈 조정
```c
// 보다 보수적인 노이즈 설정
opt.procnoiseAmb = 1E-9;       // 위상 모호정수 노이즈 감소
opt.procnoiseTropo = 1E-5;     // 대류층 노이즈 감소
opt.procnoiseHacc = 1E-3;      // 수평 가속도 노이즈 감소

// 동적 플랫폼용 설정
opt.dynamics = DYNOPT_ON;      // 동역학 모델 활성화
opt.procnoiseHacc = 1E-1;      // 수평 가속도 노이즈 증가
opt.procnoiseVacc = 1E-2;      // 수직 가속도 노이즈 증가

printf("EKF 노이즈 매개변수 조정 완료\n");
```

### ◆ 위성 제외 관리
```c
// 특정 GPS 위성 제외 (예: PRN 1, PRN 15)
int gps1 = Prn2Sat(1, 1);     // GPS PRN 1
int gps15 = Prn2Sat(1, 15);   // GPS PRN 15

opt.exsats[gps1] = 1;         // 제외
opt.exsats[gps15] = 1;        // 제외

// 제외된 위성 확인
for (int i = 1; i <= NSAT; i++) {
    if (opt.exsats[i]) {
        int prn;
        int sys = Sat2Prn(i, &prn);
        printf("제외된 위성: %c%02d\n", Sys2Str(sys), prn);
    }
}
```

---

## ▲ 성능 특성

### ◆ 메모리 효율성
- **단일 구조체**: 모든 옵션을 하나의 구조체로 통합 관리
- **정적 할당**: 동적 메모리 할당 없는 고정 크기 구조체
- **배열 최적화**: 위성 제외 목록을 비트 배열로 효율적 관리

### ◆ 기본값 최적화
- **실용적 설정**: 일반적인 GNSS 처리에 적합한 기본값
- **BeiDou 최적화**: GEO 위성 자동 제외로 성능 향상
- **노이즈 균형**: EKF 수렴성과 정확도의 균형잡힌 설정

### ◆ 확장성
- **모듈화 설계**: 새로운 옵션 추가 시 구조체 확장 용이
- **시스템 독립**: GNSS 시스템별 독립적인 옵션 관리
- **호환성**: 기존 코드와의 하위 호환성 보장

**핵심 관리 영역**:
- **처리 모드**: SPP(Single Point Positioning) 설정
- **엔진 타입**: LSQ(Least Square) vs EKF(Extended Kalman Filter)
- **다중 주파수**: 주파수 개수 및 수신기 개수 설정
- **궤도력 옵션**: 방송궤도력 vs 정밀궤도력 선택
- **대기 보정**: 이온층 및 대류층 보정 모델 설정
- **프로세스 노이즈**: EKF 필터링을 위한 노이즈 매개변수
- **품질 제어**: 고도각 마스크, GDOP 임계값 설정
- **위성 제외**: 특정 위성 제외 목록 관리

### ● 옵션 데이터 구조
모든 GNSS 처리 옵션은 `opt_t` 구조체에서 통합 관리됩니다:
- **정수형 옵션**: 모드, 엔진, 수신기/주파수 개수 등
- **실수형 매개변수**: 측정 오차, 프로세스 노이즈, 임계값 등
- **배열형 데이터**: 위성별 제외 상태 관리

---

## ◆ 옵션 구조체

### ● opt_t 구조체 상세 분석

**처리 설정**
```c
int mode;           // 처리 모드 (PROCMODE_SPP, PROCMODE_RTK)
int engine;         // 처리 엔진 (ENGINE_LSQ, ENGINE_EKF)
int nrcv;           // 수신기 개수
int nfreq;          // 주파수 개수
```

**보정 모델 설정**
```c
int ephopt;         // 궤도력 옵션 (EPHOPT_BRDC, EPHOPT_PREC)
int posopt;         // 위치 옵션 (POSOPT_EST, POSOPT_FIX)
int ionoopt;        // 이온층 옵션 (IONOOPT_OFF, IONOOPT_BRDC, IONOOPT_EST)
int tropoopt;       // 대류층 옵션 (TROPOOPT_OFF, TROPOOPT_SAAS, TROPOOPT_EST)
```

**고급 처리 설정**
```c
int par;            // 부분 모호정수 해결
int cascade;        // 계단식 모호정수 해결
int gloaropt;       // GLONASS 모호정수 해결
int dynamics;       // 수신기 동역학 옵션 (DYNOPT_OFF, DYNOPT_ON, DYNOPT_STATIC)
```

**필터링 매개변수**
```c
int maxout;         // 상태 리셋을 위한 최대 단절 횟수
int minlock;        // 모호정수 고정을 위한 최소 록 횟수
```

**시간 범위 설정**
```c
double ts;          // 처리 시작 시간 [s] (0.0: 모든 시간)
double te;          // 처리 종료 시간 [s] (0.0: 모든 시간)
```

**측정 오차 설정**
```c
double err;         // 반송파 위상 측정 오차 표준편차 [m] (천정각 방향)
double errratio;    // 의사거리 측정 오차 비율
```

**EKF 프로세스 노이즈**
```c
double procnoiseAmb;    // 위상 모호정수 [cycle]
double procnoiseTropo;  // 천정 습윤 지연 [m]
double procnoiseIono;   // 이온층 지연 [m]
double procnoiseHacc;   // 수평 가속도 [m/s^2]
double procnoiseVacc;   // 수직 가속도 [m/s^2]
double procnoiseDtr;    // 수신기 시계 [m]
double procnoiseDts;    // 위성 시계 [m]
double procnoiseIsb;    // 시스템간 편향 [m]
double procnoiseCbr;    // 수신기 코드 편향 [m]
double procnoiseCbs;    // 위성 코드 편향 [m]
double procnoisePbr;    // 수신기 위상 편향 [m]
double procnoisePbs;    // 위성 위상 편향 [m]
```

**품질 제어 매개변수**
```c
double elmask;      // 고도각 마스크 [rad]
double maxgdop;     // 최대 GDOP 값
```

**위성 제외 관리**
```c
int exsats[NSAT];   // 제외 위성 목록 (!0: 제외됨)
```

---

## ◆ 함수 목록

<details>
<summary><strong>● 옵션 초기화 함수</strong></summary>

<ul>
<li><details>
<summary><strong>♦ void InitOpt(opt_t *opt)</strong></summary>

**목적**: 옵션 데이터 구조체 초기화

**입력**:
- `opt_t *opt`: 초기화할 옵션 구조체

**출력**:
- `void`: 반환값 없음

**초기화 설정값**:

**기본 처리 설정**:
- `mode = PROCMODE_SPP`: SPP 모드
- `engine = ENGINE_LSQ`: 최소제곱법 엔진
- `nrcv = 1`: 단일 수신기
- `nfreq = 1`: 단일 주파수

**보정 모델 기본값**:
- `ephopt = EPHOPT_BRDC`: 방송 궤도력 사용
- `posopt = POSOPT_EST`: 위치 추정 모드
- `ionoopt = IONOOPT_BRDC`: 방송 이온층 모델
- `tropoopt = TROPOOPT_SAAS`: Saastamoinen 대류층 모델

**고급 설정 기본값**:
- `par = 0`: 부분 모호정수 해결 비활성화
- `cascade = 0`: 계단식 모호정수 해결 비활성화
- `gloaropt = 0`: GLONASS 모호정수 해결 비활성화
- `dynamics = DYNOPT_OFF`: 동역학 모델 비활성화

**필터링 매개변수 기본값**:
- `maxout = 5`: 최대 5회 단절 시 상태 리셋
- `minlock = 0`: 모호정수 고정을 위한 최소 록 없음

**시간 범위 기본값**:
- `ts = 0.0`: 시작 시간 제한 없음
- `te = 0.0`: 종료 시간 제한 없음

**측정 오차 기본값**:
- `err = 3E-3`: 3mm 반송파 위상 오차 (천정각)
- `errratio = 100.0`: 100배 의사거리 오차 비율

**EKF 프로세스 노이즈 기본값**:
- `procnoiseAmb = 1E-8`: 위상 모호정수 노이즈
- `procnoiseTropo = 1E-4`: 대류층 노이즈
- `procnoiseIono = 0.0`: 이온층 노이즈 (비활성화)
- `procnoiseHacc = 1E-2`: 수평 가속도 노이즈
- `procnoiseVacc = 1E-3`: 수직 가속도 노이즈
- `procnoiseDtr = 0.0`: 수신기 시계 노이즈 (비활성화)
- `procnoiseDts = 0.0`: 위성 시계 노이즈 (비활성화)
- `procnoiseIsb = 0.0`: 시스템간 편향 노이즈 (비활성화)
- `procnoiseCbr = 1E-8`: 수신기 코드 편향 노이즈
- `procnoiseCbs = 1E-8`: 위성 코드 편향 노이즈
- `procnoisePbr = 1E-8`: 수신기 위상 편향 노이즈
- `procnoisePbs = 1E-8`: 위성 위상 편향 노이즈

**품질 제어 기본값**:
- `elmask = 10.0 * D2R`: 10도 고도각 마스크
- `maxgdop = 30.0`: 최대 GDOP 30

**위성 제외 설정**:
- 모든 위성을 기본적으로 사용 가능으로 초기화
- BeiDou 시스템이 활성화된 경우, GEO 위성들을 기본 제외:
  - **BDS-2 GEO**: C01, C02, C03, C04, C05
  - **BDS-3 GEO**: C59, C60, C61, C62

**로직**:
```c
1. 모든 옵션 필드를 기본값으로 설정
2. exsats 배열을 0으로 초기화 (모든 위성 사용)
3. BeiDou 시스템 활성화 확인
4. GEO 위성들을 안전하게 제외 목록에 추가
5. 위성 인덱스 유효성 검사 수행
```

</details></li>
</ul>

</details>

---

## ◆ 사용 예시

```c
// 기본 옵션 초기화
opt_t processingOpt;
InitOpt(&processingOpt);

// 기본 설정 확인
printf("Processing mode: %s\n",
       processingOpt.mode == PROCMODE_SPP ? "SPP" : "RTK");
printf("Engine: %s\n",
       processingOpt.engine == ENGINE_LSQ ? "LSQ" : "EKF");
printf("Elevation mask: %.1f deg\n",
       processingOpt.elmask * R2D);

// 사용자 정의 설정 적용
processingOpt.engine = ENGINE_EKF;          // EKF 엔진 사용
processingOpt.nfreq = 2;                    // 이중 주파수
processingOpt.ionoopt = IONOOPT_EST;        // 이온층 추정
processingOpt.elmask = 15.0 * D2R;          // 15도 고도각 마스크

// EKF 프로세스 노이즈 조정
processingOpt.procnoiseTropo = 5E-4;        // 대류층 노이즈 증가
processingOpt.procnoiseHacc = 5E-3;         // 수평 가속도 노이즈 증가

// 특정 위성 제외 (예: GPS PRN 1)
int gpsSat = Prn2Sat(1, 1);  // GPS 시스템(1), PRN 1
if (gpsSat > 0 && gpsSat < NSAT) {
    processingOpt.exsats[gpsSat] = 1;  // 제외
}

// 제외 위성 확인
for (int i = 1; i <= NSAT; i++) {
    if (processingOpt.exsats[i]) {
        satStr_t satStr = Sat2Str(i);
        printf("Excluded satellite: %s\n", satStr.str);
    }
}
```

---

## ◆ 성능 특성

### ● 시간 복잡도
- **InitOpt()**: $O(n)$ where $n = $ BeiDou GEO 위성 개수 (최대 9개)
- **옵션 접근**: $O(1)$ (직접 구조체 멤버 접근)
- **위성 제외 확인**: $O(1)$ (배열 인덱스 접근)

### ● 메모리 사용량
- **opt_t 구조체**: 약 $(20 \times 4 + 15 \times 8 + \text{NSAT} \times 4)$ 바이트
- **NSAT = 131일 때**: 약 $80 + 120 + 524 = 724$ 바이트
- **스택 기반**: 동적 할당 없음

### ● Thread Safety
- **읽기 안전**: 모든 옵션 읽기 연산이 thread-safe
- **쓰기 주의**: 옵션 수정 시 동기화 필요
- **초기화 안전**: `InitOpt()`는 단일 스레드에서만 호출 권장

### ● 확장성
- **새 옵션 추가**: 구조체에 필드 추가 시 `InitOpt()` 업데이트 필요
- **시스템별 설정**: 현재 BeiDou GEO 위성 제외 지원, 다른 시스템 확장 가능
- **동적 설정**: 런타임 옵션 변경 지원

---

## ⚠ 주의사항

1. **구조체 초기화**: 사용 전 반드시 `InitOpt()` 호출 필요
2. **위성 인덱스**: `exsats` 배열 접근 시 범위 검사 (1 ≤ index ≤ NSAT)
3. **각도 단위**: `elmask`는 라디안 단위, 도 단위 변환 시 `D2R` 사용
4. **프로세스 노이즈**: EKF 사용 시만 유효, 0.0 설정 시 해당 상태 비연동
5. **시간 범위**: `ts`, `te`가 0.0이면 시간 제한 없음을 의미
6. **메모리 관리**: 정적 구조체이므로 별도 메모리 해제 불필요
7. **시스템 의존성**: BeiDou 제외 설정은 `SYS_BDS` 활성화 상태에 의존
8. **복합 리터럴**: `(satStr_t){.str = "CXX"}` 형태로 위성 문자열 초기화
