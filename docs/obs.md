# ★ GNSS 관측 데이터 처리 모듈 (obs)

GNSS 관측 데이터를 종합적으로 처리하는 핵심 모듈로, 관측 코드 변환부터 주파수 정보 추출까지 GNSS 측위에 필요한 모든 관측 데이터 처리 기능을 제공합니다.

## ■ 목차

1. [기본 개념](#-기본-개념)
2. [데이터 타입 트리 구조](#-데이터-타입-트리-구조)
3. [데이터 타입 목록](#-데이터-타입-목록)
4. [함수 트리 구조](#-함수-트리-구조)
5. [함수 목록](#-함수-목록)
6. [사용 예시](#-사용-예시)
7. [성능 특성](#-성능-특성)

---

## ▲ 기본 개념

### ◆ GNSS 관측 데이터 구조
obs 모듈은 다음과 같은 관측 데이터를 종합적으로 처리합니다:

**핵심 관측 타입**:
- **의사거리 (Pseudorange)**: 위성-수신기 간 거리 측정
- **반송파 위상 (Carrier Phase)**: 고정밀 거리 측정
- **도플러 주파수 (Doppler)**: 위성-수신기 상대속도
- **신호강도 (SNR)**: 신호 품질 지표

### ◆ GNSS 관측 코드 구조
GNSS 관측 코드는 **LXX** 형식으로 구성됩니다:
- **L**: 반송파 신호 식별자
- **첫 번째 X**: 주파수 밴드 (1~9)
- **두 번째 X**: 채널 타입 (I, Q, C, S, L, X, D, P, A, B, C, E, Z 등)

### ◆ RINEX 밴드 번호 체계
**중요**: RINEX에서 밴드 번호는 **시스템별로 독립적**으로 할당됩니다. 같은 밴드 번호라도 시스템에 따라 다른 주파수를 의미할 수 있습니다.

### ◆ 주파수 인덱스 매핑 철학
시스템별로 주파수 밴드를 논리적 인덱스(Fidx)로 매핑하여 배열 기반 효율적 접근을 제공합니다:
- **연속성**: 각 시스템 내에서 1부터 연속된 인덱스 할당
- **독립성**: 시스템 간 인덱스 값은 독립적으로 관리
- **확장성**: 새로운 주파수 추가 시 인덱스 확장 가능

주파수 인덱스 매핑의 수학적 표현:

$$\text{Fidx} = f(\text{code}, \text{sys})$$

여기서:
- $\text{Fidx}$: 주파수 인덱스 (1~N, 시스템별 가변)
- $\text{code}$: 관측 코드 ID
- $\text{sys}$: GNSS 시스템 ID

실제 주파수 계산:

$$f_{\text{carrier}} = \text{BaseFreq}(\text{band}, \text{sys}) + \Delta f(\text{FCN})$$

여기서:
- $f_{\text{carrier}}$: 실제 반송파 주파수
- $\text{BaseFreq}$: 시스템별 기준 주파수
- $\Delta f(\text{FCN})$: GLONASS 주파수 채널 번호 보정

---

## ▲ 데이터 타입 트리 구조

```
obs 모듈 데이터 계층
├── 관측 코드 체계
│   ├── 문자열 형태 ───────────── "LXX" (L+밴드+채널)
│   ├── 숫자 코드 ID ──────────── int 타입 고유 식별자
│   └── 밴드 ID ───────────────── 주파수 밴드 번호 (1~9)
├── 주파수 정보 구조
│   ├── 주파수 인덱스 (Fidx) ──── 시스템별 논리적 인덱스 (1~N)
│   ├── 실제 주파수 ────────────── double 타입 (MHz)
│   └── FCN 보정 ──────────────── GLONASS 주파수 채널 번호
├── 시스템별 매핑 테이블
│   ├── 코드→밴드 매핑 ─────────── 관측 코드에서 밴드 추출
│   ├── 코드→Fidx 매핑 ────────── 시스템별 주파수 인덱스
│   └── 밴드→주파수 매핑 ────────── 실제 주파수 계산
└── 채널 타입 분류
    ├── 공통 타입 ─────────────── I, Q, C, P, X
    ├── GPS 특화 ──────────────── S, L, W, N
    ├── Galileo/BDS 특화 ───────── D, P
    └── 기타 타입 ─────────────── A, B, E, Z
```

---

## ▲ 데이터 타입 목록

#### ◆ 관측 코드 구조
<details>
<summary>상세 설명</summary>

**목적**: GNSS 관측 신호의 표준화된 식별 체계

**정의**: `"LXX"` (L + 밴드번호 + 채널타입)

**값**:
- **L**: 반송파 신호 식별자 (고정)
- **밴드번호**: 1~9 (RINEX 표준)
- **채널타입**: I,Q,C,S,L,X,D,P,A,B,C,E,Z 등

**사용**: RINEX 표준 완전 호환, 모든 GNSS 시스템 지원

</details>

#### ◆ 주파수 인덱스 (Fidx)
<details>
<summary>상세 설명</summary>

**목적**: 시스템별 주파수 밴드의 논리적 인덱스

**정의**: `int` (1~N, 시스템별 가변)

**값**:
- GPS: 1~5
- GLONASS: 1~5
- Galileo: 1~5
- BeiDou: 1~6
- QZSS: 1~6
- IRNSS: 1~2
- SBAS: 1~2

**사용**: 주파수별 관측 데이터 배열 인덱싱

</details>

#### ◆ 채널 타입 분류
<details>
<summary>상세 설명</summary>

**목적**: 신호 내 세부 채널 구분

**정의**: 시스템별 채널 타입 문자

**값**:

##### 공통 채널 타입
| 타입 | 의미 | 설명 |
|------|------|------|
| **I** | In-phase | 동위상 성분 (주로 데이터) |
| **Q** | Quadrature | 직교위상 성분 (주로 파일럿) |
| **X** | Combined | 두 개 이상 채널 조합 |
| **C** | C/A | Coarse/Acquisition 코드 |
| **P** | Precision | 정밀 코드 |

##### GPS 특화 타입
| 타입 | 의미 | 설명 |
|------|------|------|
| **S** | L2C(M)/L1C(D) | Medium 또는 Data 성분 |
| **L** | L2C(L)/L1C(P) | Long 또는 Pilot 성분 |
| **W** | Z-tracking | Z-추적 방식 |
| **N** | Codeless | 코드 없는 추적 |

##### Galileo/BDS 특화 타입
| 타입 | 의미 | 설명 |
|------|------|------|
| **D** | Data | 데이터 성분 |
| **P** | Pilot | 파일럿 성분 |

##### 기타 타입
| 타입 | 의미 | 설명 |
|------|------|------|
| **A** | A-component | 시스템별 A 성분 |
| **B** | B-component | 시스템별 B 성분 |
| **E** | E-component | 시스템별 E 성분 |
| **Z** | Combined Multiple | 여러 성분 조합 |

**사용**: 시스템별 채널 타입에 따른 신호 처리 분기

</details>

#### ◆ 주파수 정보 구조
<details>
<summary>상세 설명</summary>

**목적**: 관측 코드에서 실제 주파수 정보 추출

**정의**: 밴드 ID → 주파수 인덱스 → 실제 주파수 변환 체계

**값**:
- **밴드 ID**: RINEX 밴드 번호 (1~9)
- **주파수 인덱스**: 시스템별 논리 인덱스
- **실제 주파수**: MHz 단위 double 값
- **FCN 보정**: GLONASS 주파수 채널 보정

**사용**: 관측코드 → 밴드ID → 주파수인덱스 → 실제주파수 순차 변환

</details>

---

## ▲ 함수 트리 구조

```
obs 모듈 함수 계층
├── 관측 코드 변환
│   ├── Str2Code() ────────────── 문자열 → 코드 ID
│   ├── Code2Str() ────────────── 코드 ID → 문자열
│   ├── Code2Band() ───────────── 코드 ID → 밴드 ID
│   ├── Code2Freq() ───────────── 코드 ID + 위성 → 주파수
│   └── Code2Fidx() ───────────── 코드 ID + 시스템 → 주파수 인덱스
├── 밴드 관리
│   ├── Band2Str() ────────────── 밴드 ID → 밴드 문자
│   ├── Str2Band() ────────────── 밴드 문자 → 밴드 ID
│   └── Band2Freq() ───────────── 밴드 ID + 위성 → 주파수
└── 시스템별 내부 함수
    ├── 주파수 인덱스 매핑
    │   ├── Code2Fidx_GPS() ───── GPS 주파수 인덱스
    │   ├── Code2Fidx_GLO() ───── GLONASS 주파수 인덱스
    │   ├── Code2Fidx_GAL() ───── Galileo 주파수 인덱스
    │   ├── Code2Fidx_BDS() ───── BeiDou 주파수 인덱스
    │   ├── Code2Fidx_QZS() ───── QZSS 주파수 인덱스
    │   ├── Code2Fidx_IRN() ───── IRNSS 주파수 인덱스
    │   └── Code2Fidx_SBS() ───── SBAS 주파수 인덱스
    └── 주파수 변환
        ├── Band2Freq_GPS() ───── GPS 주파수 변환
        ├── Band2Freq_GLO() ───── GLONASS 주파수 변환 (FCN 적용)
        ├── Band2Freq_GAL() ───── Galileo 주파수 변환
        ├── Band2Freq_BDS() ───── BeiDou 주파수 변환
        ├── Band2Freq_QZS() ───── QZSS 주파수 변환
        ├── Band2Freq_IRN() ───── IRNSS 주파수 변환
        └── Band2Freq_SBS() ───── SBAS 주파수 변환
```

---

## ▲ 함수 목록

#### ◆ 관측 코드 변환 함수

##### ● Str2Code() - 문자열→코드ID
<details>
<summary>상세 설명</summary>

**목적**: 관측 코드 문자열을 고유 코드 ID로 변환

**입력**:
- `char *str`: 관측 코드 문자열 ("LXX" 형식)

**출력**:
- `int`: 코드 ID (고유 식별자), 오류 시 0

**함수 로직**:
1. 문자열 형식 검증: $\text{pattern} = "L[1-9][A-Z]"$
2. 내부 코드 테이블에서 매칭 코드 검색: $\text{code} = \text{LookupTable}[\text{str}]$
3. 고유 코드 ID 반환: $\text{return } \text{code} \neq 0 \text{ ? code : 0}$

</details>

##### ● Code2Str() - 코드ID→문자열
<details>
<summary>상세 설명</summary>

**목적**: 코드 ID를 관측 코드 문자열로 역변환

**입력**:
- `int code`: 코드 ID

**출력**:
- `char *`: 관측 코드 문자열, 오류 시 NULL

**함수 로직**:
1. 코드 ID 유효성 검증: $\text{code} > 0$
2. 내부 코드 테이블에서 해당 문자열 조회: $\text{str} = \text{CodeTable}[\text{code}]$
3. 문자열 포인터 반환

</details>

##### ● Code2Band() - 코드ID→밴드ID
<details>
<summary>상세 설명</summary>

**목적**: 관측 코드에서 주파수 밴드 번호 추출

**입력**:
- `int code`: 코드 ID

**출력**:
- `int`: 밴드 ID (1~9), 오류 시 0

**함수 로직**:
1. 코드 ID를 문자열로 변환: $\text{str} = \text{Code2Str}(\text{code})$
2. 문자열에서 밴드 번호 추출: $\text{band} = \text{str}[1] - '0'$
3. 범위 검증 후 반환: $1 \leq \text{band} \leq 9$

</details>

##### ● Code2Freq() - 코드ID+위성→주파수
<details>
<summary>상세 설명</summary>

**목적**: 관측 코드와 위성 정보에서 실제 주파수 계산

**입력**:
- `int code`: 코드 ID
- `int sat`: 위성 인덱스

**출력**:
- `double`: 주파수 (MHz), 오류 시 0.0

**함수 로직**:
1. 코드에서 밴드 ID 추출: $\text{band} = \text{Code2Band}(\text{code})$
2. 위성에서 시스템 정보 추출: $\text{sys} = \text{Sat2Sys}(\text{sat})$
3. 시스템별 주파수 변환: $f = \text{Band2Freq}(\text{band}, \text{sat})$
4. GLONASS FCN 보정 적용: $f_{\text{corrected}} = f + \Delta f(\text{FCN})$

</details>

##### ● Code2Fidx() - 코드ID+시스템→주파수인덱스
<details>
<summary>상세 설명</summary>

**목적**: 관측 코드를 시스템별 주파수 인덱스로 변환

**입력**:
- `int code`: 코드 ID
- `int sys`: 시스템 ID (1~7)

**출력**:
- `int`: 주파수 인덱스 (1~N), 오류 시 0

**함수 로직**:
1. 시스템 유효성 검증: $1 \leq \text{sys} \leq 7$
2. 시스템별 내부 함수 호출: $\text{fidx} = \text{Code2Fidx\_SYS}(\text{code})$
3. 코드별 고정 매핑 테이블 조회
4. 주파수 인덱스 반환: $\text{fidx} \geq 1$

</details>

#### ◆ 밴드 관리 함수

##### ● Band2Str() - 밴드ID→밴드문자
<details>
<summary>상세 설명</summary>

**목적**: 밴드 ID를 밴드 문자로 변환

**입력**:
- `int band`: 밴드 ID (1~9)

**출력**:
- `char`: 밴드 문자 ('1'~'9'), 오류 시 '\0'

**함수 로직**: $\text{char} = \text{band} + '0'$ (단순 숫자-문자 변환)

</details>

##### ● Str2Band() - 밴드문자→밴드ID
<details>
<summary>상세 설명</summary>

**목적**: 밴드 문자를 밴드 ID로 변환

**입력**:
- `char str`: 밴드 문자 ('1'~'9')

**출력**:
- `int`: 밴드 ID (1~9), 오류 시 0

**함수 로직**: $\text{band} = \text{str} - '0'$ (단순 문자-숫자 변환)

</details>

##### ● Band2Freq() - 밴드ID+위성→주파수
<details>
<summary>상세 설명</summary>

**목적**: 밴드 ID와 위성 정보에서 실제 주파수 계산

**입력**:
- `int band`: 밴드 ID (1~9)
- `int sat`: 위성 인덱스

**출력**:
- `double`: 주파수 (MHz), 오류 시 0.0

**함수 로직**:
1. 위성에서 시스템 정보 추출: $\text{sys} = \text{Sat2Sys}(\text{sat})$
2. 시스템별 주파수 변환 함수 호출: $f = \text{Band2Freq\_SYS}(\text{band}, \text{sat})$
3. GLONASS FCN 자동 적용: $f_{\text{glo}} = f_{\text{base}} + k \times \Delta f$

</details>

#### ◆ 시스템별 주파수 매핑

##### GPS 시스템 주파수 매핑
| 밴드 | 주파수 (MHz) | Fidx | 관측 코드 | 특징 |
|------|-------------|------|-----------|------|
| **1** | 1575.42 | 1 | L1C | Legacy C/A |
| **1** | 1575.42 | 4 | L1S, L1L, L1X | Modern L1C |
| **2** | 1227.60 | 2 | L2W | Legacy P(Y) |
| **2** | 1227.60 | 5 | L2S, L2L, L2X | Modern L2C |
| **5** | 1176.45 | 3 | L5I, L5Q, L5X | L5 |

##### GLONASS 시스템 주파수 매핑
| 밴드 | 기준 주파수 (MHz) | FCN 공식 | Fidx | 관측 코드 |
|------|-----------------|----------|------|-----------|
| **1** | 1602.0 + k×0.5625 | FDMA | 1 | L1C |
| **2** | 1246.0 + k×0.4375 | FDMA | 2 | L2C |
| **3** | 1202.025 | CDMA | 3 | L3I, L3Q, L3X |
| **4** | 1600.995 | CDMA | 4 | L4A, L4B, L4X |
| **6** | 1248.06 | CDMA | 5 | L6A, L6B, L6X |

##### Galileo 시스템 주파수 매핑
| 밴드 | 주파수 (MHz) | Fidx | 관측 코드 | 서비스 |
|------|-------------|------|-----------|---------|
| **1** | 1575.42 | 1 | L1B, L1C, L1X | OS/CS |
| **5** | 1176.45 | 2 | L5I, L5Q, L5X | OS |
| **7** | 1207.14 | 3 | L7I, L7Q, L7X | OS |
| **8** | 1191.795 | 4 | L8I, L8Q, L8X | OS |
| **6** | 1278.75 | 5 | L6B, L6C, L6X | CS/PRS |

##### BeiDou 시스템 주파수 매핑
| 밴드 | 주파수 (MHz) | Fidx | 관측 코드 | 위성 타입 |
|------|-------------|------|-----------|----------|
| **2** | 1561.098 | 1 | L2I | BDS-2/3 |
| **1** | 1575.42 | 2 | L1D, L1P, L1X | BDS-3 |
| **5** | 1176.45 | 3 | L5D, L5P, L5X | BDS-3 |
| **7** | 1207.14 | 4 | L7I, L7X, L7D, L7P, L7Z | BDS-2/3 |
| **8** | 1191.795 | 5 | L8D, L8P, L8X | BDS-3 |
| **6** | 1268.52 | 6 | L6I | BDS-2/3 |

##### QZSS 시스템 주파수 매핑
| 밴드 | 주파수 (MHz) | Fidx | 관측 코드 | 서비스 |
|------|-------------|------|-----------|---------|
| **1** | 1575.42 | 1 | L1B, L1C | Legacy |
| **1** | 1575.42 | 5 | L1S, L1L, L1X | Modern |
| **2** | 1227.60 | 2 | L2S, L2L, L2X | L2C |
| **5** | 1176.45 | 3 | L5I, L5Q, L5X | L5 |
| **5** | 1176.45 | 6 | L5D, L5P, L5Z | SBAS |
| **6** | 1278.75 | 4 | L6S, L6L, L6E, L6X, L6Z | LEX |

##### IRNSS 시스템 주파수 매핑
| 밴드 | 주파수 (MHz) | Fidx | 관측 코드 | 특징 |
|------|-------------|------|-----------|------|
| **5** | 1176.45 | 1 | L5B, L5C, L5X | L5 |
| **9** | 2492.028 | 2 | L9B, L9C, L9X | S-band |

##### SBAS 시스템 주파수 매핑
| 밴드 | 주파수 (MHz) | Fidx | 관측 코드 | 서비스 |
|------|-------------|------|-----------|---------|
| **1** | 1575.42 | 1 | L1C | L1 SBAS |
| **5** | 1176.45 | 2 | L5I, L5Q, L5X | DFMC |

---

## ▲ 사용 예시

### ◆ 기본 관측 코드 변환
```c
// 문자열을 코드 ID로 변환
int code_l1c = Str2Code("L1C");    // GPS L1 C/A 코드
int code_l5i = Str2Code("L5I");    // L5 In-phase 코드

// 코드 ID를 문자열로 역변환
char *str = Code2Str(code_l1c);    // "L1C"
printf("코드 문자열: %s\n", str);

// 코드에서 밴드 번호 추출
int band = Code2Band(code_l5i);    // 5 (L5 밴드)
printf("밴드 번호: %d\n", band);
```

### ◆ 주파수 정보 추출
```c
// GPS 위성의 L1C 주파수 계산
int gps_sat = Prn2Sat(1, 5);              // GPS PRN 5
int l1c_code = Str2Code("L1C");            // L1C 코드 ID
double freq = Code2Freq(l1c_code, gps_sat); // 1575.42 MHz

printf("GPS PRN 5 L1C 주파수: %.2f MHz\n", freq);

// GLONASS 위성의 L1C 주파수 (FCN 적용)
int glo_sat = Prn2Sat(2, 1);              // GLONASS PRN 1
double glo_freq = Code2Freq(l1c_code, glo_sat); // FCN에 따라 변함

printf("GLONASS PRN 1 L1C 주파수: %.2f MHz\n", glo_freq);
```

### ◆ 주파수 인덱스 활용
```c
// 시스템별 주파수 인덱스 계산
int l5i_code = Str2Code("L5I");
int gps_fidx = Code2Fidx(l5i_code, 1);    // GPS: 3
int gal_fidx = Code2Fidx(l5i_code, 3);    // Galileo: 2

printf("GPS L5I 인덱스: %d\n", gps_fidx);
printf("Galileo L5I 인덱스: %d\n", gal_fidx);

// 주파수 인덱스로 배열 접근
double gps_obs_data[6] = {0};  // GPS 주파수별 관측값 배열
gps_obs_data[gps_fidx] = 25123456.789;  // L5I 관측값 저장
```

### ◆ 밴드 기반 주파수 변환
```c
// 밴드 번호로 직접 주파수 계산
int gps_sat = Prn2Sat(1, 10);             // GPS PRN 10
double l1_freq = Band2Freq(1, gps_sat);   // L1: 1575.42 MHz
double l5_freq = Band2Freq(5, gps_sat);   // L5: 1176.45 MHz

printf("GPS L1 주파수: %.2f MHz\n", l1_freq);
printf("GPS L5 주파수: %.2f MHz\n", l5_freq);

// 밴드 문자 변환
char band_char = Band2Str(5);             // '5'
int band_id = Str2Band('7');              // 7
```

### ◆ 다중 시스템 처리
```c
// 여러 시스템의 L5 주파수 비교
int systems[] = {1, 3, 4, 5, 6};  // GPS, GAL, BDS, QZS, IRN
char *sys_names[] = {"GPS", "GAL", "BDS", "QZS", "IRN"};

for (int i = 0; i < 5; i++) {
    int sat = Prn2Sat(systems[i], 1);  // 각 시스템 PRN 1
    double freq = Band2Freq(5, sat);    // L5 주파수

    if (freq > 0.0) {
        printf("%s L5 주파수: %.2f MHz\n", sys_names[i], freq);
    }
}
```

### ◆ QZSS 신호 매핑 예시
```c
// QZSS Legacy와 Modern 신호 구분
int qzs_sat = Prn2Sat(5, 193);            // QZSS PRN 193

// Legacy L1 신호 (Fidx 1)
int l1c_fidx = Code2Fidx(Str2Code("L1C"), 5);  // 1
double l1c_freq = Code2Freq(Str2Code("L1C"), qzs_sat);

// Modern L1 신호 (Fidx 5)
int l1s_fidx = Code2Fidx(Str2Code("L1S"), 5);  // 5
double l1s_freq = Code2Freq(Str2Code("L1S"), qzs_sat);

printf("QZSS L1C Legacy (Fidx %d): %.2f MHz\n", l1c_fidx, l1c_freq);
printf("QZSS L1S Modern (Fidx %d): %.2f MHz\n", l1s_fidx, l1s_freq);

// L5 기본과 SBAS 신호 구분
int l5i_fidx = Code2Fidx(Str2Code("L5I"), 5);  // 3 (기본 L5)
int l5d_fidx = Code2Fidx(Str2Code("L5D"), 5);  // 6 (SBAS L5)
```

---

## ▲ 성능 특성

### ◆ 메모리 효율성
- **정적 테이블**: 모든 매핑 정보를 컴파일 타임 상수로 관리
- **캐시 친화적**: 순차 탐색 최소화, 직접 인덱스 접근 위주
- **최소 메모리**: 동적 할당 없는 고정 크기 데이터 구조

### ◆ 연산 성능
- **O(1) 밴드 변환**: 단순 문자-숫자 변환으로 상수 시간
- **O(n) 코드 검색**: 내부 테이블 선형 탐색 (n ≤ 100)
- **O(1) 주파수 계산**: 시스템별 고정 공식으로 직접 계산
- **O(1) 인덱스 매핑**: switch-case 기반 직접 매핑

### ◆ 정확도 보장
- **RINEX 표준 준수**: 모든 관측 코드가 RINEX 3.x 표준 완전 호환
- **주파수 정밀도**: double 타입으로 kHz 단위 정확도 보장
- **FCN 자동 적용**: GLONASS 주파수 채널 번호 자동 보정
- **시스템별 특성 반영**: 각 GNSS 시스템의 고유 특성 완전 지원

### ◆ 확장성
- **모듈형 설계**: 시스템별 독립적 함수로 새 시스템 추가 용이
- **테이블 기반**: 새로운 관측 코드 추가 시 테이블만 수정
- **인덱스 확장**: 주파수 인덱스 범위 확장 가능
- **호환성 보장**: 기존 코드 변경 없이 새 기능 추가 가능

### ◆ GNSS 특화 최적화
- **실시간 처리**: 수신기 실시간 데이터 처리에 최적화
- **다중 시스템**: 모든 GNSS 시스템 동시 지원
- **주파수 다양성**: Legacy와 Modern 신호 모두 지원
- **채널 세분화**: 신호 내 세부 채널까지 완전 분류

---

**■ 이 모듈은 GNSS 관측 데이터의 표준화된 처리를 위한 핵심 인터페이스입니다.**
