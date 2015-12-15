/*
* asnum.c: Bindings for MaxMind's GeoIP library
*              See copyright information in file COPYRIGHT.
*/

#include "lua-geoip.h"
#include "database.h"

#define LUAGEOIP_ASNUM_VERSION     "lua-geoip.asnum 0.1.1"
#define LUAGEOIP_ASNUM_COPYRIGHT   "Copyright (C) 2011, lua-geoip authors"
#define LUAGEOIP_ASNUM_DESCRIPTION \
        "Bindings for MaxMind's GeoIP library (asnum database)"

static GeoIP * check_asnum_db(lua_State * L, int idx)
{
  int type = 0;
  luageoip_DB * pDB = (luageoip_DB *)luaL_checkudata(
      L,
      idx,
      LUAGEOIP_ASNUM_MT
    );
  if (pDB == NULL)
  {
    lua_pushstring(L, "lua-geoip error: asnum db is null");
    return NULL;
  }

  if (pDB->pGeoIP == NULL)
  {
    lua_pushstring(L, "lua-geoip error: attempted to use closed asnum db");
    return NULL;
  }

  type = GeoIP_database_edition(pDB->pGeoIP);
  if (
      type != GEOIP_ASNUM_EDITION &&
      type != GEOIP_ASNUM_EDITION_V6
    )
  {
    lua_pushstring(L, "lua-geoip error: object is not a asnum db");
    return NULL;
  }

  return pDB->pGeoIP;
}


static int lasnum_query_by_name(lua_State * L)
{
  GeoIP * pGeoIP = check_asnum_db(L, 1);
  GeoIPLookup gl;
  const char * name = luaL_checkstring(L, 2);

  if (pGeoIP == NULL)
  {
    return lua_error(L); /* Error message already on stack */
  }

   lua_pushstring(L, GeoIP_name_by_name_gl(pGeoIP, name, &gl));
   return 1;
}


static int lasnum_query_by_addr(lua_State * L)
{
  GeoIP * pGeoIP = check_asnum_db(L, 1);
  GeoIPLookup gl;
  const char * addr = luaL_checkstring(L, 2);

  if (pGeoIP == NULL)
  {
    return lua_error(L); /* Error message already on stack */
  }

   lua_pushstring(L, GeoIP_name_by_addr_gl(pGeoIP, addr, &gl));
   return 1;
}


static int lasnum_query_by_ipnum(lua_State * L)
{
  GeoIP * pGeoIP = check_asnum_db(L, 1);
  GeoIPLookup gl;
  const char * ipnum = luaL_checkstring(L, 2);

  if (pGeoIP == NULL)
  {
    return lua_error(L); /* Error message already on stack */
  }

   lua_pushstring(L, GeoIP_name_by_ipnum_gl(pGeoIP, ipnum, &gl));
   return 1;
}


static int lasnum_charset(lua_State * L)
{
  GeoIP * pGeoIP = check_asnum_db(L, 1);
  if (pGeoIP == NULL)
  {
    return lua_error(L); /* Error message already on stack */
  }

  lua_pushinteger(L, GeoIP_charset(pGeoIP));

  return 1;
}

static int lasnum_set_charset(lua_State * L)
{
  GeoIP * pGeoIP = check_asnum_db(L, 1);
  int charset = luaL_checkint(L, 2);

  if (pGeoIP == NULL)
  {
    return lua_error(L); /* Error message already on stack */
  }

  GeoIP_set_charset(pGeoIP, charset);

  return 0;
}

static int lasnum_close(lua_State * L)
{
  luageoip_DB * pDB = (luageoip_DB *)luaL_checkudata(L, 1, LUAGEOIP_ASNUM_MT);

  if (pDB && pDB->pGeoIP != NULL)
  {
    GeoIP_delete(pDB->pGeoIP);
    pDB->pGeoIP = NULL;
  }

  return 0;
}

#define lasnum_gc lasnum_close

static int lasnum_tostring(lua_State * L)
{
  GeoIP * pGeoIP = check_asnum_db(L, 1);
  if (pGeoIP == NULL)
  {
    return lua_error(L); /* Error message already on stack */
  }

  lua_pushstring(L, GeoIP_database_info(pGeoIP));

  return 1;
}

static const luaL_reg M[] =
{
  { "query_by_name", lasnum_query_by_name },
  { "query_by_addr", lasnum_query_by_addr },
  { "charset", lasnum_charset },
  { "set_charset", lasnum_set_charset },
  { "close", lasnum_close },
  { "__gc", lasnum_gc },
  { "__tostring", lasnum_tostring },

  { NULL, NULL }
};

/* Error message capture code inspired by code by Wolfgang Oertl. */
static int lasnum_open(lua_State * L)
{
  static const int allowed_types[] =
  {
    GEOIP_ASNUM_EDITION,
    GEOIP_ASNUM_EDITION_V6
  };

  return luageoip_common_open_db(
      L,
      M,
      GEOIP_ASNUM_EDITION,
      GEOIP_MEMORY_CACHE,
      LUAGEOIP_ASNUM_MT,
      GEOIP_INDEX_CACHE, /* not allowed */
      2,
      allowed_types
    );
}

/* Lua module API */
static const struct luaL_reg R[] =
{
  { "open", lasnum_open },

  { NULL, NULL }
};

#ifdef __cplusplus
extern "C" {
#endif

LUALIB_API int luaopen_geoip_asnum(lua_State * L)
{
  /*
  * Register module
  */

#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 502
  luaL_register(L, "geoip.asnum", R);
#else
  lua_newtable(L);
  luaL_setfuncs(L, R, 0);
#endif

  /*
  * Register module information
  */
  lua_pushliteral(L, LUAGEOIP_ASNUM_VERSION);
  lua_setfield(L, -2, "_VERSION");

  lua_pushliteral(L, LUAGEOIP_ASNUM_COPYRIGHT);
  lua_setfield(L, -2, "_COPYRIGHT");

  lua_pushliteral(L, LUAGEOIP_ASNUM_DESCRIPTION);
  lua_setfield(L, -2, "_DESCRIPTION");

  return 1;
}

#ifdef __cplusplus
}
#endif