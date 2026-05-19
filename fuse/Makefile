CC       = gcc
SYNCE_PREFIX = /usr/local

CFLAGS  = -Wall -Wextra -O2 \
           $(shell pkg-config --cflags fuse3) \
           $(shell pkg-config --cflags glib-2.0) \
           -I$(SYNCE_PREFIX)/include/synce \
           -DFUSE_USE_VERSION=31

LDFLAGS = $(shell pkg-config --libs fuse3) \
           -L$(SYNCE_PREFIX)/lib -lsynce \
           -Wl,-rpath,$(SYNCE_PREFIX)/lib \
           -lpthread

TARGET  = synce-fuse

all: $(TARGET)

$(TARGET): synce-fuse.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

install: $(TARGET)
	install -m 755 $(TARGET) $(SYNCE_PREFIX)/bin/$(TARGET)

clean:
	rm -f $(TARGET)
