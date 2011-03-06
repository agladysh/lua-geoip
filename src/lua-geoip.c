/*
* lua-geoip.c: Bindings for MaxMind's GeoIP library
*              See copyright information in file COPYRIGHT.
*/

#define LUAGEOIP_VERSION     "lua-geoip 0.1"
#define LUAGEOIP_COPYRIGHT   "Copyright (C) 2011, lua-geoip authors"
#define LUAGEOIP_DESCRIPTION "Bindings for MaxMind's GeoIP library"

#if defined (__cplusplus)
extern "C" {
#endif

#include <lua.h>
#include <lauxlib.h>

#if defined (__cplusplus)
}
#endif


/* Lua module API */
static const struct luaL_reg R[] =
{
  { NULL, NULL }
};

#ifdef __cplusplus
extern "C" {
#endif

LUALIB_API int luaopen_geoip(lua_State * L)
{
  /*
  * Register module
  */
  luaL_register(L, "geoip", R);

  /*
  * Register module information
  */
  lua_pushliteral(L, LUAGEOIP_VERSION);
  lua_setfield(L, -2, "_VERSION");

  lua_pushliteral(L, LUAGEOIP_COPYRIGHT);
  lua_setfield(L, -2, "_COPYRIGHT");

  lua_pushliteral(L, LUAGEOIP_DESCRIPTION);
  lua_setfield(L, -2, "_DESCRIPTION");

  return 1;
}

#ifdef __cplusplus
}
#endif
