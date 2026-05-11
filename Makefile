CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -Wpedantic -O2 -Iinclude
LDFLAGS = -lm

BUILD   = build
TARGET  = $(BUILD)/mathvista
LIB     = $(BUILD)/libmathvista.a

# Source lists (all implementation files except main)
SRCS_CORE = \
    src/core/math_engine.c \
    src/core/memory_pool.c \
    src/core/common.c

SRCS_DISC = \
    src/discrete/stirling.c \
    src/discrete/catalan.c

SRCS_CALC = \
    src/calculus/gradient.c \
    src/calculus/lagrange.c \
    src/calculus/transform3d.c

SRCS_DS = \
    src/ds/skiplist.c \
    src/ds/radix.c

SRCS_ALGO = \
    src/algo/sort.c \
    src/algo/graph.c

SRCS_VIZ = \
    src/viz/viz_dot.c \
    src/viz/viz_ascii.c \
    src/viz/viz_csv.c

LIB_SRCS = \
    $(SRCS_CORE) $(SRCS_DISC) $(SRCS_CALC) \
    $(SRCS_DS)   $(SRCS_ALGO) $(SRCS_VIZ)

LIB_OBJS = $(patsubst src/%.c, $(BUILD)/%.o, $(LIB_SRCS))
MAIN_OBJ = $(BUILD)/main.o

# Default target
.PHONY: all clean test lib dirs

all: dirs $(TARGET)

# Executable
$(TARGET): $(LIB_OBJS) $(MAIN_OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)
	@echo "  linked  → $@"

# Static library (optional, usable by external projects)
lib: dirs $(LIB)

$(LIB): $(LIB_OBJS)
	ar rcs $@ $^
	@echo "  archive → $@"

# Pattern rule: src/a/b.c → build/a/b.o
$(BUILD)/main.o: src/main.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Create build sub-directories
dirs:
	@mkdir -p \
	    $(BUILD)/core \
	    $(BUILD)/discrete \
	    $(BUILD)/calculus \
	    $(BUILD)/ds \
	    $(BUILD)/algo \
	    $(BUILD)/viz \
	    output

# Run all demos
test: all
	./$(TARGET) all

# Quick per-module debug build (add -g -fsanitize=address)
debug: CFLAGS += -g -fsanitize=address -fno-omit-frame-pointer
debug: LDFLAGS += -fsanitize=address
debug: all

# Clean
clean:
	rm -rf $(BUILD)
	@echo "  cleaned build/"