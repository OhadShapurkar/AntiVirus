CC = gcc
CFLAGS = -Wall -g
TARGET = AntiVirus
SRC = AntiVirus.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)