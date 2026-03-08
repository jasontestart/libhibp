TARGET  = libhibp.so.1.0.0
SONAME  = libhibp.so.1
DEVLINK = libhibp.so
PCFILE  = libhibp.pc
SOURCES = src/log.c src/pp.c src/hibp.c
OBJECTS = $(SOURCES:.c=.o)

PREFIX  ?= /usr/local
LIBDIR  = $(PREFIX)/lib
INCDIR  = $(PREFIX)/include
PCDIR   = $(LIBDIR)/pkgconfig
VERSION = 1.0.0

# Dependency Discovery
PKG_CONFIG ?= pkg-config
DEP_CFLAGS := -Iinclude $(shell $(PKG_CONFIG) --cflags libcurl openssl)
DEP_LIBS   := $(shell $(PKG_CONFIG) --libs libcurl openssl)

CFLAGS  += -fPIC -fvisibility=hidden -Wall -Wextra -O2 $(DEP_CFLAGS)
LDFLAGS = -shared -Wl,-soname,$(SONAME)
LIBS    = $(DEP_LIBS)

all: $(TARGET) $(PCFILE)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
	ln -sf $(TARGET) $(SONAME)
	ln -sf $(SONAME) $(DEVLINK)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(PCFILE):
	@echo "Generating $(PCFILE)..."
	@echo "prefix=$(PREFIX)" > $(PCFILE)
	@echo "libdir=\$${prefix}/lib" >> $(PCFILE)
	@echo "includedir=\$${prefix}/include" >> $(PCFILE)
	@echo "" >> $(PCFILE)
	@echo "Name: libhibp" >> $(PCFILE)
	@echo "Description: C library for Have I Been Pwned API" >> $(PCFILE)
	@echo "Version: $(VERSION)" >> $(PCFILE)
	@echo "Libs: -L\$${libdir} -lhibp" >> $(PCFILE)
	@echo "Requires.private: libcurl openssl" >> $(PCFILE)
	@echo "Cflags: -I\$${includedir}" >> $(PCFILE)

install: all
	install -d $(DESTDIR)$(LIBDIR)
	install -d $(DESTDIR)$(INCDIR)
	install -d $(DESTDIR)$(PCDIR)
	install -m 0755 $(TARGET) $(DESTDIR)$(LIBDIR)/
	ln -sf $(TARGET) $(DESTDIR)$(LIBDIR)/$(SONAME)
	ln -sf $(SONAME) $(DESTDIR)$(LIBDIR)/$(DEVLINK)
	install -m 0644 hibp.h $(DESTDIR)$(INCDIR)/
	install -m 0644 $(PCFILE) $(DESTDIR)$(PCDIR)/
	@echo "Installation complete. Run 'sudo ldconfig' if installing to system paths."

uninstall:
	rm -f $(DESTDIR)$(LIBDIR)/$(TARGET) $(DESTDIR)$(LIBDIR)/$(SONAME) $(DESTDIR)$(LIBDIR)/$(DEVLINK)
	rm -f $(DESTDIR)$(INCDIR)/hibp.h
	rm -f $(DESTDIR)$(PCDIR)/$(PCFILE)

clean:
	rm -f $(OBJECTS) $(TARGET) $(SONAME) $(DEVLINK) $(PCFILE)

.PHONY: all clean install uninstall

examples: all
	$(MAKE) -C examples
