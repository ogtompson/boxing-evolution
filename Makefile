# ============================================================
#  Makefile — Boxing Evolution
#  Compilação com GCC e Raylib
# ============================================================

CC      = gcc
TARGET  = boxing_evolution
SRC_DIR = src
BUILD_DIR = build

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# Flags de compilação
CFLAGS  = -Wall -Wextra -std=c99 -I$(SRC_DIR)
# Flags de link para Raylib (Linux)
LFLAGS  = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Para macOS, substitua LFLAGS por:
# LFLAGS = -lraylib -framework OpenGL -framework Cocoa -framework IOKit

# Para Windows com MinGW:
# LFLAGS = -lraylib -lopengl32 -lgdi32 -lwinmm

# ============================================================
all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LFLAGS)
	@echo ""
	@echo "  Build OK! Execute: ./$(TARGET)"
	@echo ""

clean:
	rm -rf $(BUILD_DIR) $(TARGET) *.dat

run: all
	./$(TARGET)

.PHONY: all clean run
