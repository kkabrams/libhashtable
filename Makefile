LDFLAGS=-lhashtable -Llibhashtable
CFLAGS=-fpic -shared -pedantic -Wall
TARGET=libhashtable.so

all: $(TARGET)

$(TARGET):
	./genheader.sh
	$(CC) $(CFLAGS) -o $(TARGET) libhashtable.c

clean:
	rm -f libhashtable.so

install:
	cp $(TARGET) /usr/local/lib/$(TARGET)
	cp hashtable.h /usr/local/include/hashtable.h
