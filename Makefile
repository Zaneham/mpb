CC      = gcc
WARN    = -Wall -Wextra
CFLAGS  = $(WARN) -std=gnu99 -O2 -Iinclude
SRCS    = src/main.c src/mpb_cmd.c src/mpb_pkg.c src/mpb_http.c
OBJS    = $(SRCS:.c=.o)
TARGET  = mpb

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).exe

.PHONY: all clean
