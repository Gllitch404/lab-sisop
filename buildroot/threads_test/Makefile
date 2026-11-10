CC ?= gcc
CFLAGS ?= -Wall -O2
LDFLAGS ?= -pthread

TARGET = threads_test
SRC = threads_test.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
