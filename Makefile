SRC = src
OUT = build
TARGET = clipmanager 

CC = gcc
CCFLAGS = -Wall -g -Iinclude
CLIBS = -lX11

CFILES := $(wildcard $(SRC)/**/*.c)
CFILES += $(wildcard $(SRC)/*.c) 

all: $(TARGET)

$(TARGET): 
	mkdir -p $(OUT)
	$(CC) $(CCFLAGS) $(CLIBS) -o $(OUT)/$@ $(CFILES) 

clean:
	rm -rf build 
