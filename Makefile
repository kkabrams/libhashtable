### why the fuck is this here? LDFLAGS=-lhashtable -Llibhashtable
CFLAGS=-pedantic -Wall -g3
PREFIX:=/usr/local
TARGET=libhashtable.so

all: $(TARGET)

libhashtable.h:
	./genheader.sh

$(TARGET): CFLAGS+=-fpic -shared
$(TARGET): libhashtable.o
	ld -shared -o $(TARGET) libhashtable.o

clean:
	rm -f libhashtable.so
	rm *.o

install: all
	mkdir -p $(PREFIX)/lib
	mkdir -p $(PREFIX)/include
	install $(TARGET) $(PREFIX)/lib/$(TARGET)
	install hashtable.h $(PREFIX)/include/hashtable.h

uninstall:
	rm $(PREFIX)/lib/$(TARGET)
	rm $(PREFIX)/include/hashtable.h
