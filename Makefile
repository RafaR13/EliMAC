CC = gcc
CFLAGS = -O3 -maes -msse4.2 -Wall -Iheaders
COMMON_SOURCES = elimac.c elihash.c utils.c
OBJECTS = $(COMMON_SOURCES:.c=.o)

TEXT_TARGET = elimac_text
TEXT_MAIN = main.c
TEXT_OBJECTS = $(TEXT_MAIN:.c=.o) $(OBJECTS)

CSV_TARGET = elimac_csv
CSV_MAIN = main_csv.c
CSV_OBJECTS = $(CSV_MAIN:.c=.o) $(OBJECTS)

all: $(TEXT_TARGET) $(CSV_TARGET)

$(TEXT_TARGET): $(TEXT_OBJECTS)
	$(CC) $(TEXT_OBJECTS) -o $@

$(CSV_TARGET): $(CSV_OBJECTS)
	$(CC) $(CSV_OBJECTS) -o $@

%.o: %.c
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
	rm -f $(TEXT_TARGET) $(CSV_TARGET) *.o
	rm -rf out

.PHONY: all run_text run_csv run_all clean