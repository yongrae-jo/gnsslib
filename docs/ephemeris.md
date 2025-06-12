# ★ 위성 궤도력 처리 모듈 (ephemeris)

<script>
  window.MathJax = {
    tex: {
      inlineMath: [['$', '$'], ['\\(', '\\)']]
    }
  };
</script>
<script async src="https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-mml-chtml.js"></script>


GNSS 위성의 궤도력 타입 관리를 위한 기초 모듈입니다.

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

### ◆ 궤도력 타입 관리 철학
본 모듈은 현재 **방송궤도력 타입 관리** 기능만 제공합니다. 각 GNSS 시스템별로 지원하는 방송궤도력 타입을 설정하고 조회하는 기본적인 인터페이스를 제공합니다.

**핵심 기능**:
- **타입 설정**: 시스템별 방송궤도력 타입 설정
- **타입 조회**: 현재 설정된 궤도력 타입 확인
- **범위 검증**: 시스템별 유효한 타입 범위 자동 검사

### ◆ 방송궤도력 개념
방송궤도력(Broadcast Ephemeris)은 위성이 주기적으로 송신하는 궤도 정보로, 실시간 위성 위치 계산에 사용됩니다:
- **실시간성**: 위성에서 실시간으로 방송
- **정확도**: 수 미터 수준의 위성 위치 정확도
- **유효기간**: 2-4시간 (시스템별 상이)

방송궤도력을 이용한 위성 위치 계산의 기본 수식:

$$\boldsymbol{r}^s(t) = f(\text{eph}, t - t_{\text{oe}})$$

여기서:
- $\boldsymbol{r}^s(t)$: 시각 $t$에서의 위성 위치 벡터
- $\text{eph}$: 방송궤도력 파라미터 집합
- $t_{\text{oe}}$: 궤도력 기준 시각 (Time of Ephemeris)

### ◆ 시스템별 궤도력 특성
각 GNSS 시스템은 고유한 방송궤도력 형식을 사용하며, 일부 시스템은 다중 타입을 지원합니다:
- **단일 타입**: GPS, GLONASS, BeiDou, QZSS, IRNSS, SBAS
- **다중 타입**: Galileo (I/NAV, F/NAV)

---

## ▲ 데이터 타입 구조

```
ephemeris 모듈 데이터 계층
├── 궤도력 타입 관리
│   ├── EPHTYPE[NSYS] ──────── 시스템별 궤도력 타입 배열
│   └── 타입 범위 ────────────── 시스템별 유효 타입 범위
├── 시스템별 타입 매핑
│   ├── GPS ────────────────── 0: LNAV
│   ├── GLONASS ────────────── 0: LNAV
│   ├── Galileo ────────────── 0: I/NAV, 1: F/NAV
│   ├── BeiDou ─────────────── 0: D1
│   ├── QZSS ───────────────── 0: LNAV
│   ├── IRNSS ──────────────── 0: LNAV
│   └── SBAS ───────────────── 0: RINEX SBAS
└── 타입 검증 체계
    ├── 시스템 ID 검증 ────────── 1~NSYS
    ├── 타입 범위 검증 ────────── 시스템별 0~1
    └── 기본값 제공 ───────────── 모든 시스템 기본 타입 0
```

---

## ▲ 데이터 타입 목록

#### ◆ 궤도력 타입 시스템
<details>
<summary>상세 설명</summary>

**목적**: 시스템별 방송궤도력 타입 구분

**정의**: `int` 타입 (0~2, 시스템별 가변 범위)

**값**:

##### GPS 시스템 궤도력 타입
| 타입 | 명칭 | 설명 |
|------|------|------|
| **0** | LNAV | Legacy Navigation (기본) |

##### GLONASS 시스템 궤도력 타입
| 타입 | 명칭 | 설명 |
|------|------|------|
| **0** | LNAV | Navigation Message (기본) |

##### Galileo 시스템 궤도력 타입
| 타입 | 명칭 | 설명 |
|------|------|------|
| **0** | I/NAV | Integrity Navigation (기본) |
| **1** | F/NAV | Freely accessible Navigation |

##### BeiDou 시스템 궤도력 타입
| 타입 | 명칭 | 설명 |
|------|------|------|
| **0** | D1 | Navigation Message D1 (기본) |

##### QZSS 시스템 궤도력 타입
| 타입 | 명칭 | 설명 |
|------|------|------|
| **0** | LNAV | Legacy Navigation (기본) |

##### IRNSS 시스템 궤도력 타입
| 타입 | 명칭 | 설명 |
|------|------|------|
| **0** | LNAV | Navigation Message (기본) |

##### SBAS 시스템 궤도력 타입
| 타입 | 명칭 | 설명 |
|------|------|------|
| **0** | RINEX | RINEX SBAS Ephemeris (기본) |

**사용**: 모든 시스템의 기본 타입은 0으로 통일

</details>

#### ◆ EPHTYPE 배열
<details>
<summary>상세 설명</summary>

**목적**: 시스템별 현재 설정된 궤도력 타입 저장

**정의**:
```c
static int EPHTYPE[NSYS] = {0}; // 모든 시스템 기본값 0
```

**값**:
- **크기**: NSYS (활성화된 시스템 개수)
- **인덱스**: 시스템 ID - 1 (0-based)
- **값**: 궤도력 타입 (0~2)

**사용**:
- 읽기: `GetEphType(sys)` 함수 사용
- 쓰기: `SetEphType(sys, type)` 함수 사용
- 초기값: 모든 시스템이 타입 0으로 초기화

</details>

#### ◆ 타입 범위 검증
<details>
<summary>상세 설명</summary>

**목적**: 시스템별 유효한 궤도력 타입 범위 정의

**정의**: 시스템별 하드코딩된 범위 검증 로직

**값**:
- **일반 시스템**: 0~1 (GPS, GLONASS, BeiDou, QZSS, IRNSS, SBAS)
- **확장 시스템**: 0~2 (Galileo)

**사용**:
```c
if (sys == 3) {  // Galileo
    valid_range = (type >= 0 && type <= 2);
} else {         // 기타 시스템
    valid_range = (type >= 0 && type <= 1);
}
```

</details>

---

## ▲ 함수 구조

```
ephemeris 모듈 함수 계층
└── 궤도력 타입 관리
    ├── GetEphType() ──────── 궤도력 타입 조회
    └── SetEphType() ──────── 궤도력 타입 설정
```

---

## ▲ 함수 목록

#### ◆ 궤도력 타입 관리 함수

##### ● GetEphType() - 궤도력 타입 조회
<details>
<summary>상세 설명</summary>

**목적**: 지정된 GNSS 시스템의 방송궤도력 타입 조회

**입력**:
- `int sys`: GNSS 시스템 ID (1~7)

**출력**:
- `int`: 방송궤도력 타입 (시스템별 0~2), 오류 시 -1

**함수 로직**:
1. 시스템 ID 유효성 검사: $1 \leq \text{sys} \leq \text{NSYS}$
2. EPHTYPE 배열에서 해당 시스템 타입 조회: $\text{type} = \text{EPHTYPE}[\text{sys}-1]$
3. 기본값 0 반환 (모든 시스템의 기본 타입)

</details>

##### ● SetEphType() - 궤도력 타입 설정
<details>
<summary>상세 설명</summary>

**목적**: 지정된 GNSS 시스템의 방송궤도력 타입 설정

**입력**:
- `int sys`: GNSS 시스템 ID (1~7)
- `int type`: 방송궤도력 타입

**출력**:
- `void`: 반환값 없음

**함수 로직**:
1. 시스템 ID 유효성 검사: $1 \leq \text{sys} \leq \text{NSYS}$
2. 시스템 문자 조회 (Sys2Str 호출)
3. 시스템별 타입 범위 검증: $0 \leq \text{type} \leq \text{max\_type}(\text{sys})$
4. 유효한 타입이면 EPHTYPE 배열에 저장: $\text{EPHTYPE}[\text{sys}-1] = \text{type}$
5. 잘못된 타입은 무시 (기존 설정 유지)

</details>

---

## ▲ 사용 예시

### ◆ 기본 타입 설정 및 조회
```c
// 시스템별 방송궤도력 타입 설정
SetEphType(1, 0);  // GPS LNAV 설정
SetEphType(3, 1);  // Galileo F/NAV 설정
SetEphType(4, 0);  // BeiDou D1 설정

// 현재 설정된 타입 조회
int gpsEphType = GetEphType(1);     // 0 (LNAV)
int galEphType = GetEphType(3);     // 1 (F/NAV)
int bdsEphType = GetEphType(4);     // 0 (D1)

printf("GPS 궤도력 타입: %d\n", gpsEphType);
printf("Galileo 궤도력 타입: %d\n", galEphType);
printf("BeiDou 궤도력 타입: %d\n", bdsEphType);
```

### ◆ 전체 시스템 초기화
```c
// 모든 시스템을 기본 타입으로 초기화
for (int sys = 1; sys <= NSYS; sys++) {
    SetEphType(sys, 0);  // 모든 시스템을 기본 타입으로 설정
}

// 시스템별 타입 확인
printf("현재 궤도력 타입 설정:\n");
if (SYS_GPS) printf("GPS: %d (LNAV)\n", GetEphType(1));
if (SYS_GLO) printf("GLONASS: %d (LNAV)\n", GetEphType(2));
if (SYS_GAL) printf("Galileo: %d (%s)\n", GetEphType(3),
                    GetEphType(3) == 0 ? "I/NAV" : "F/NAV");
if (SYS_BDS) printf("BeiDou: %d (D1)\n", GetEphType(4));
if (SYS_QZS) printf("QZSS: %d (LNAV)\n", GetEphType(5));
if (SYS_IRN) printf("IRNSS: %d (LNAV)\n", GetEphType(6));
if (SYS_SBS) printf("SBAS: %d (RINEX)\n", GetEphType(7));
```

### ◆ 에러 처리 및 검증
```c
// 잘못된 입력 처리
SetEphType(0, 0);   // 무효한 시스템 - 무시됨
SetEphType(1, 5);   // 무효한 타입 - 무시됨
SetEphType(10, 0);  // 범위 초과 - 무시됨

// 안전한 타입 설정 함수 예시
void SetSafeEphType(int sys, int type) {
    int oldType = GetEphType(sys);
    SetEphType(sys, type);
    int newType = GetEphType(sys);

    if (oldType == newType && type != oldType) {
        printf("경고: 시스템 %d에 타입 %d 설정 실패\n", sys, type);
    } else {
        printf("시스템 %d 타입을 %d로 변경\n", sys, newType);
    }
}

// 사용 예시
SetSafeEphType(1, 0);  // GPS LNAV 설정
SetSafeEphType(3, 1);  // Galileo F/NAV 설정
SetSafeEphType(3, 3);  // 잘못된 타입 - 경고 출력
```

### ◆ Galileo 다중 타입 처리
```c
// Galileo 시스템의 I/NAV와 F/NAV 전환
int galileo_sys = 3;

// I/NAV 모드 설정
SetEphType(galileo_sys, 0);
int current_type = GetEphType(galileo_sys);
printf("Galileo 모드: %s\n", current_type == 0 ? "I/NAV" : "F/NAV");

// F/NAV 모드로 전환
SetEphType(galileo_sys, 1);
current_type = GetEphType(galileo_sys);
printf("Galileo 모드: %s\n", current_type == 0 ? "I/NAV" : "F/NAV");

// 타입별 처리 분기
switch (GetEphType(galileo_sys)) {
    case 0:
        printf("I/NAV 궤도력 처리 로직\n");
        break;
    case 1:
        printf("F/NAV 궤도력 처리 로직\n");
        break;
    default:
        printf("알 수 없는 Galileo 궤도력 타입\n");
}
```

### ◆ 설정 백업 및 복원
```c
// 현재 설정 백업
int ephtype_backup[NSYS];
for (int sys = 1; sys <= NSYS; sys++) {
    ephtype_backup[sys-1] = GetEphType(sys);
}

// 임시 설정 변경
SetEphType(1, 0);  // GPS
SetEphType(3, 1);  // Galileo F/NAV

// 일부 처리 수행...

// 원래 설정 복원
for (int sys = 1; sys <= NSYS; sys++) {
    SetEphType(sys, ephtype_backup[sys-1]);
}

printf("궤도력 타입 설정이 복원되었습니다.\n");
```

---

## ▲ 성능 특성

### ◆ 메모리 효율성
- **정적 배열**: EPHTYPE[NSYS] 고정 크기로 메모리 예측 가능
- **최소 메모리**: 시스템당 int 1개 (4바이트)만 사용
- **동적 할당 없음**: 모든 데이터가 정적으로 관리

### ◆ 연산 성능
- **O(1) 접근**: 배열 직접 인덱스 접근으로 상수 시간
- **O(1) 설정**: 단순 배열 대입으로 상수 시간
- **최소 연산**: 범위 검증 외 추가 연산 없음

### ◆ 안전성 보장
- **범위 검증**: 모든 입력값에 대한 자동 유효성 검사
- **기존값 보존**: 잘못된 설정 시도 시 기존 설정 유지
- **기본값 제공**: 초기화되지 않은 시스템도 안전한 기본값 반환

### ◆ 확장성
- **시스템 추가**: NSYS 상수 변경으로 새 시스템 자동 지원
- **타입 확장**: 시스템별 타입 범위 조정으로 새 타입 추가 가능
- **호환성 유지**: 기존 코드 변경 없이 확장 가능

### ◆ 실용성
- **단순 인터페이스**: 2개 함수로 모든 기능 제공
- **직관적 사용**: 함수명과 매개변수가 직관적
- **에러 허용**: 잘못된 입력에도 안정적 동작

---

**■ 이 모듈은 GNSS 궤도력 타입 관리를 위한 기초 인터페이스를 제공합니다.**
