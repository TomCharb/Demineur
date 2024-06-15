CC = gcc
CFLAGS = -lncurses

TARGET = demineur

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) -o $(TARGET) $(TARGET).c $(CFLAGS)

install: $(TARGET)
	install -m 0755 $(TARGET) /usr/local/bin/

clean:
	rm -f $(TARGET)
