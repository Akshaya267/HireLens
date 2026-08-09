CC       = gcc
CFLAGS   = -Wall -Wextra -std=c11 -Iinclude -O2
SRC_DIR  = src
BIN_DIR  = bin
TARGET   = $(BIN_DIR)/hirelens_engine

SOURCES  = $(wildcard $(SRC_DIR)/*.c)
OBJECTS  = $(SOURCES:.c=.o)

.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)
	@echo "Build complete: $(TARGET)"

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(TARGET) data/jd/job_description.txt data/resumes output

test: all
	bash tests/run_tests.sh

clean:
	rm -f $(SRC_DIR)/*.o $(TARGET)
	rm -rf $(BIN_DIR)
