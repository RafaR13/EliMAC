CC = gcc
CFLAGS = -O3 -maes -msse4.2 -Wall -Iheaders
SRC_DIR = src
COMMON_SOURCES = $(SRC_DIR)/elimac.c $(SRC_DIR)/elihash.c $(SRC_DIR)/utils.c
OBJECTS = $(COMMON_SOURCES:.c=.o)

TEXT_TARGET = elimac_text
TEXT_MAIN = $(SRC_DIR)/main.c
TEXT_OBJECTS = $(TEXT_MAIN:.c=.o) $(OBJECTS)

CSV_TARGET = elimac_csv
CSV_MAIN = $(SRC_DIR)/main_csv.c
CSV_OBJECTS = $(CSV_MAIN:.c=.o) $(OBJECTS)

all: $(TEXT_TARGET) $(CSV_TARGET)

$(TEXT_TARGET): $(TEXT_OBJECTS)
	$(CC) $(TEXT_OBJECTS) -o $@

$(CSV_TARGET): $(CSV_OBJECTS)
	$(CC) $(CSV_OBJECTS) -o $@

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

run_text: $(TEXT_TARGET)
	@mkdir -p out
	./$(TEXT_TARGET)

run_csv: $(CSV_TARGET)
	@mkdir -p out
	./$(CSV_TARGET)

run_all: $(TEXT_TARGET) $(CSV_TARGET)
	@mkdir -p out
	./$(TEXT_TARGET)
	./$(CSV_TARGET)

clean:
	rm -f $(TEXT_TARGET) $(CSV_TARGET) $(SRC_DIR)/*.o
	rm -rf out

.PHONY: all run_text run_csv run_all clean