### why the fuck is this here? LDFLAGS=-lhashtable -Llibhashtable
CFLAGS=-fpic -shared -pedantic -Wall
PREFIX=/usr/local
TARGET=libhashtable.so

all: $(TARGET)

libhashtable.h:
	./genheader.sh

$(TARGET): libhashtable.c libhashtable.h

clean:
	rm -f libhashtable.so

install:
	cp $(TARGET) $(PREFIX)/lib/$(TARGET)
	cp hashtable.h $(PREFIX)/include/hashtable.h

uninstall:
	rm $(PREFIX)/lib/$(TARGET)
	rm $(PREFIX)/include/hashtable.h
