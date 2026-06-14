CC := clang

SRC := $(wildcard src/*.c)

TARGET := build/vmm

CFLAGS := \
		-D_GNU_SOURCE \
    -std=c17 \
    -Wall \
    -Wextra \
    -Wpedantic \
    -Wshadow \
    -Wconversion \
    -Iinclude \
    -g

SANITIZERS := \
    -fsanitize=address,undefined

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p build
	$(CC) $(CFLAGS) $(SANITIZERS) $^ -o $@

run: all
	./$(TARGET)

debug: all
	lldb ./$(TARGET)

clean:
	rm -rf build

format:
	clang-format -i src/*.c include/*.h

tidy:
	clang-tidy src/*.c -- -Iinclude

bear:
	bear -- make

check:
	@echo "SRC     = $(SRC)"
	@echo "TARGET  = $(TARGET)"
	@which clang
	@clang --version | head -1

.PHONY: \
    all \
    run \
    debug \
    clean \
    format \
    tidy \
    bear
