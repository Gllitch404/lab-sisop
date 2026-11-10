CC ?= gcc
CFLAGS ?= -Wall -O2
LDFLAGS ?= -pthread

TARGET = sched_profiler
SRC = sched_profiler.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
