# GNSS 처리 라이브러리 (GNSSLIB)

[![License](https://img.shields.io/badge/license-All%20Rights%20Reserved-red.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-green.svg)]()
[![Language](https://img.shields.io/badge/language-C-orange.svg)]()

다중 위성항법시스템(GNSS) 신호 처리를 위한 고성능 C 라이브러리

## 🚀 빠른 시작

### 빌드

```bash
# 라이브러리 빌드
make

# 디버그 빌드
make debug

# 시스템에 설치
make install
```

### 사용 예시

```c
#include "gnsslib.h"

int main() {
    // GPS 위성 PRN 15 → 위성 인덱스 변환
    int sat = Prn2Sat(1, 15);

    // 현재 GPST 시간 획득
    double gpst = TimeGet();

    // 행렬 연산
    mat_t A = Mat(3, 3, DOUBLE);
    mat_t B = Eye(3);
    mat_t C = MatMul(A, B);

    // 메모리 해제
    FreeMat(&A); FreeMat(&B); FreeMat(&C);

    return 0;
}
```

## 📚 문서

- **[프로젝트 전체 가이드](Project.md)** - 상세한 프로젝트 문서
- **[Common 모듈](docs/common.md)** - 위성 변환, 시간 처리
- **[Matrix 모듈](docs/matrix.md)** - 행렬 연산, 수치 알고리즘
- **[Obs 모듈](docs/obs.md)** - 관측 데이터 처리

## 🛠️ 지원 기능

- ✅ **다중 GNSS**: GPS, Galileo, BeiDou
- ✅ **고성능 행렬 연산**: 메모리 정렬 최적화
- ✅ **정밀 측위**: LSQ, EKF 알고리즘
- ✅ **통합 시간 처리**: GPST 기준 시간 시스템
- ✅ **크로스 플랫폼**: Windows, Linux, macOS

## 📦 시스템 요구사항

- **컴파일러**: GCC 7+ 또는 Clang 10+
- **표준**: C17 (GNU17)
- **의존성**: POSIX.1-2008 (Linux/macOS)
- **메모리**: 64바이트 정렬 지원

## 📄 라이센스

© 2024 Yongrae Jo. All Rights Reserved.

## 👤 개발자

**Yongrae Jo**
📧 0727ggame@sju.ac.kr
🏫 세종대학교
