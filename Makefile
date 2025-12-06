CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g
INCLUDES = -Iincludes -Ilib -Isrc

SOURCES = src/main.c \
          src/hal/hal-api.c \
		  src/peripherals/hex-display.c \
		  src/peripherals/switch.c \
		  src/peripherals/button.c \
		  src/peripherals/led.c \
		  src/peripherals/soft_timer.c

TARGET = M4_Timer

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) -o $(TARGET)
	@echo "Build complete: $(TARGET)"

clean:
	rm -f $(TARGET)
	@echo "Cleaned build files"
	
.PHONY: all clean install help



