# =============================================================================
# GNSS Library Makefile
# =============================================================================

# 프로젝트 설정
PROJECT_NAME = gnsslib
VERSION = 1.0.0

# 디렉토리 설정
SRCDIR = src
INCDIR = include
OBJDIR = build
LIBDIR = lib

# 파일 설정
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
HEADERS = $(wildcard $(INCDIR)/*.h)

# 라이브러리 설정
STATIC_LIB = $(LIBDIR)/lib$(PROJECT_NAME).a
SHARED_LIB = $(LIBDIR)/lib$(PROJECT_NAME).so

# 컴파일러 설정
CC = gcc
CFLAGS = -Wall -Wextra -std=gnu17 -O2 -fPIC
CPPFLAGS = -I$(INCDIR) -D_POSIX_C_SOURCE=200809L
LDFLAGS = -lm

# 디버그 빌드 설정
DEBUG_CFLAGS = -g -O0 -DDEBUG

# 기본 타겟
.PHONY: all clean debug install uninstall help

all: directories $(STATIC_LIB) $(SHARED_LIB)

# 디렉토리 생성
directories:
	@mkdir -p $(OBJDIR) $(LIBDIR)

# 정적 라이브러리 생성
$(STATIC_LIB): $(OBJECTS)
	@echo "Creating static library: $@"
	@ar rcs $@ $^

# 공유 라이브러리 생성
$(SHARED_LIB): $(OBJECTS)
	@echo "Creating shared library: $@"
	@$(CC) -shared -o $@ $^ $(LDFLAGS)

# 오브젝트 파일 컴파일
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS)
	@echo "Compiling: $<"
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# 디버그 빌드
debug: CFLAGS += $(DEBUG_CFLAGS)
debug: all

# 설치
install: all
	@echo "Installing GNSS Library..."
	@sudo cp $(STATIC_LIB) /usr/local/lib/
	@sudo cp $(SHARED_LIB) /usr/local/lib/
	@sudo cp $(INCDIR)/*.h /usr/local/include/
	@sudo ldconfig
	@echo "Installation complete"

# 제거
uninstall:
	@echo "Uninstalling GNSS Library..."
	@sudo rm -f /usr/local/lib/lib$(PROJECT_NAME).a
	@sudo rm -f /usr/local/lib/lib$(PROJECT_NAME).so
	@sudo rm -f /usr/local/include/gnsslib.h
	@sudo rm -f /usr/local/include/const.h
	@sudo rm -f /usr/local/include/common.h
	@sudo rm -f /usr/local/include/matrix.h
	@sudo rm -f /usr/local/include/obs.h
	@sudo ldconfig
	@echo "Uninstallation complete"

# 정리
clean:
	@echo "Cleaning build files..."
	@rm -rf $(OBJDIR) $(LIBDIR)

# 도움말
help:
	@echo "GNSS Library Build System"
	@echo "========================="
	@echo "Available targets:"
	@echo "  all      - Build static and shared libraries (default)"
	@echo "  debug    - Build with debug symbols"
	@echo "  install  - Install libraries and headers to system"
	@echo "  uninstall- Remove libraries and headers from system"
	@echo "  clean    - Remove build files"
	@echo "  help     - Show this help message"
	@echo ""
	@echo "Project: $(PROJECT_NAME) v$(VERSION)"

# 의존성 정보 출력
info:
	@echo "Build Configuration:"
	@echo "==================="
	@echo "CC       = $(CC)"
	@echo "CFLAGS   = $(CFLAGS)"
	@echo "CPPFLAGS = $(CPPFLAGS)"
	@echo "LDFLAGS  = $(LDFLAGS)"
	@echo "SOURCES  = $(SOURCES)"
	@echo "OBJECTS  = $(OBJECTS)"
