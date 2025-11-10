CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
INCLUDES = -Iincludes -Ilib

SOURCES = src/main.c \
          src/hal/hal-api.c \
		  src/peripherals/hex-display.c \
		  src/peripherals/switch.c \
		  src/peripherals/button.c 

TARGET = M2_SIM

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) -o $(TARGET)
	@echo "Build complete: $(TARGET)"

clean:
	rm -f $(TARGET)
	@echo "Cleaned build files"
	
.PHONY: all clean install help



