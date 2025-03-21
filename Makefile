CC = gcc
CFLAGS = $(SESSION_MACRO) `pkg-config --cflags gtk+-3.0 libwnck-3.0`
LDFLAGS = `pkg-config --libs gtk+-3.0 libwnck-3.0` -lxcb
SRC_DIR = src
BUILD_DIR = build
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC))
EXEC = gridflux  
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

all: $(BUILD_DIR) $(EXEC)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -rf $(BUILD_DIR) $(EXEC)

.PHONY: all clean
