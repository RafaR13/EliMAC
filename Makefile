CC = gcc
CFLAGS = -O3 -maes -msse4.2 -Wall -Isrc/headers -fopenmp
LDFLAGS = -fopenmp
SRC_DIR = src
HEADER_DIR = $(SRC_DIR)/headers
COMMON_SOURCES = $(SRC_DIR)/elimac.c $(SRC_DIR)/elihash.c $(SRC_DIR)/utils.c
OBJECTS = $(COMMON_SOURCES:.c=.o)

TEXT_TARGET = elimac_text
TEXT_MAIN = $(SRC_DIR)/main.c
TEXT_OBJECTS = $(TEXT_MAIN:.c=.o) $(OBJECTS)

CSV_TARGET = elimac_csv
CSV_MAIN = $(SRC_DIR)/main_csv.c
CSV_OBJECTS = $(CSV_MAIN:.c=.o) $(OBJECTS)

# Header dependencies
DEPS = $(HEADER_DIR)/elimac.h $(HEADER_DIR)/elihash.h $(HEADER_DIR)/utils.h $(HEADER_DIR)/main.h

# Default encoding (0: naive, 1: compact, 2: both)
ENCODING ?= 2

all: $(TEXT_TARGET) $(CSV_TARGET)

$(TEXT_TARGET): $(TEXT_OBJECTS)
	$(CC) $(TEXT_OBJECTS) -o $@ $(LDFLAGS)

$(CSV_TARGET): $(CSV_OBJECTS)
	$(CC) $(CSV_OBJECTS) -o $@ $(LDFLAGS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

out:
	@mkdir -p out

run_text: out $(TEXT_TARGET)
	./$(TEXT_TARGET) --test --encoding $(ENCODING)

run_csv: out $(CSV_TARGET)
	./$(CSV_TARGET) --test --encoding $(ENCODING)

run_all: out $(TEXT_TARGET) $(CSV_TARGET)
	./$(TEXT_TARGET) --test --encoding $(ENCODING)
	./$(CSV_TARGET) --test --encoding $(ENCODING)

clean:
	rm -f $(TEXT_TARGET) $(CSV_TARGET) $(SRC_DIR)/*.o
	rm -rf out

.PHONY: all run_text run_csv run_all clean