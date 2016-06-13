LUA_IMPL		?= lua5.1
DESTDIR			?= /
PKG_CONFIG		?= pkg-config
CC				?= gcc
CFLAGS			?= -O2 -fPIC -DPIC $(shell $(PKG_CONFIG) $(LUA_IMPL) --cflags)
INSTALL			?= install
LUA_CMOD_DIR	?= $(shell $(PKG_CONFIG) $(LUA_IMPL) --variable INSTALL_CMOD)

CF				+= $(CFLAGS) -Werror -pedantic -std=c99 -Isrc
LF				+= $(LDFLAGS) -shared -lGeoIP

all: prepare geoip.so geoip/country.so geoip/city.so

prepare:
	@mkdir -p geoip

geoip.so: src/database.o src/lua-geoip.o
geoip/country.so: src/database.o src/country.o
geoip/city.so: src/database.o src/city.o

.c.o:
	$(CC) $(CF) -c $^ -o $@

%.so:
	$(CC) $(LF) $^ -o $@

clean:
	@rm -f geoip.so geoip/country.so geoip/city.so
	@rm -f src/*.o
	@rm -rf geoip

install: all
	$(INSTALL) -d $(DESTDIR)/$(LUA_CMOD_DIR)/geoip
	$(INSTALL) geoip/* $(DESTDIR)/$(LUA_CMOD_DIR)/geoip
	$(INSTALL) geoip.so $(DESTDIR)/$(LUA_CMOD_DIR)

uninstall:
	@rm -f $(LUA_CMOD_DIR)/geoip.so
	@rm -rf $(LUA_CMOD_DIR)/geoip

.SUFFIXES: .c .o .so
