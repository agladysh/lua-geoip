LUA_VERSION	?= 5.1
PREFIX		?= /usr
CFLAGS		+= -O2 -Werror -pedantic -fPIC
LDFLAGS		+= -shared -lGeoIP
LUA_INCLUDE_DIR	?= $(PREFIX)/include
LUA_LIB_DIR	?= $(PREFIX)/lib
LUA_CMODULE_DIR	?= $(DESTDIR)$(PREFIX)/lib/lua/$(LUA_VERSION)
LUA_MODULE_DIR	?= $(DESTDIR)$(PREFIX)/share/lua/$(LUA_VERSION)
LUA_BIN_DIR	?= $(DESTDIR)$(PREFIX)/bin
INCLUDES	= -I$(LUA_INCLUDE_DIR) -Isrc
INC_LIBS	= -L$(LUA_LIB_DIR)
CC		= gcc
INSTALL		= install

all: prepare geoip.so geoip/country.so geoip/city.so

prepare:
	@mkdir -p geoip

geoip.so: src/database.o src/lua-geoip.o
geoip/country.so: src/database.o src/country.o
geoip/city.so: src/database.o src/city.o

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $^ -o $@

%.so:
	$(CC) $(INC_LIBS) $^ -o $@ $(LDFLAGS)

clean:
	@rm -f geoip.so geoip/country.so geoip/city.so
	@rm -f src/*.o
	@rm -rf geoip

install: all
	$(INSTALL) -d $(LUA_CMODULE_DIR)/geoip
	$(INSTALL) geoip/* $(LUA_CMODULE_DIR)/geoip
	$(INSTALL) geoip.so $(LUA_CMODULE_DIR)

uninstall:
	@rm -f $(LUA_CMODULE_DIR)/geoip.so
	@rm -rf $(LUA_CMODULE_DIR)/geoip

.SUFFIXES: .c .o .so
