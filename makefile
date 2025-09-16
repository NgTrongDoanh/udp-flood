CC = gcc
CFLAGS = -Wall -I./include -pthread
LDFLAGS = -pthread

BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include

TARGET = udp_flood_sender
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all clean run mkdir_build

all: mkdir_build ./$(TARGET)

mkdir_build:
	@mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -rf $(BUILD_DIR) $(TARGET)
