# Compiler and flags
CC = gcc
CFLAGS = -O3 -maes -msse4.2 -Wall -Wextra -I$(HEADER_DIR)
LDFLAGS =

# Conditional OpenMP support
ifeq ($(PARALLEL), 1)
    CFLAGS += -fopenmp
    LDFLAGS += -fopenmp
endif

# Directories
SRC_DIR = src
HEADER_DIR = $(SRC_DIR)/headers
OUTDIR = out
GRAPH_DIR = graphs
TABLE_DIR = tables

# Source files
COMMON_SOURCES = $(SRC_DIR)/elimac.c $(SRC_DIR)/elihash.c $(SRC_DIR)/utils.c
MAIN_SOURCE = $(SRC_DIR)/main.c
SOURCES = $(MAIN_SOURCE) $(COMMON_SOURCES)
OBJECTS = $(SOURCES:.c=.o)

# Executable
TARGET = elimac

# Header dependencies
DEPS = $(HEADER_DIR)/elimac.h $(HEADER_DIR)/elihash.h $(HEADER_DIR)/utils.h $(HEADER_DIR)/main.h

# Default encoding (0: naive, 1: compact, 2: both)
ENCODING ?= 4

# Default output format (txt or csv)
OUTPUT_FORMAT ?= csv

all: $(OUTDIR) $(TARGET)

$(OUTDIR):
	@mkdir -p $(OUTDIR)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(OUTDIR) $(TARGET)
	taskset -c 0-7 ./$(TARGET) --test --encoding $(ENCODING) --parallel --output-format $(OUTPUT_FORMAT)

run_txt: $(OUTDIR) $(TARGET)
	taskset -c 0-7 ./$(TARGET) --test --encoding $(ENCODING) --parallel --output-format txt

run_csv: $(OUTDIR) $(TARGET)
	taskset -c 0-7 ./$(TARGET) --test --encoding $(ENCODING) --parallel --output-format csv

clean:
	rm -f $(TARGET) $(SRC_DIR)/*.o
	rm -rf $(OUTDIR) $(GRAPH_DIR) $(TABLE_DIR)
.PHONY: all run run_txt run_csv clean

# rm -rf $(OUTDIR) $(GRAPH_DIR) $(TABLE_DIR)
# TODO: Put this inside "clean:" to also remove the output folder
