/*
 * netspeed.c:
 *   LUA bindings for MaxMind's GeoIP library.
 *   Abstracts accessors to the NetSpeed database:
 *     https://www.maxmind.com/en/netspeed.
 *   See copyright information in file COPYRIGHT.
 */

#include "lua-geoip.h"
#include "database.h"

#define LUAGEOIP_NETSPEED_VERSION     "lua-geoip.netspeed 0.1.0"
#define LUAGEOIP_NETSPEED_COPYRIGHT   "Copyright (C) 2013, lua-geoip authors"
#define LUAGEOIP_NETSPEED_DESCRIPTION \
        "Bindings for MaxMind's GeoIP library (netspeed database)"

static GeoIP * check_netspeed_db(lua_State * L, int idx)
{
  int type = 0;
  luageoip_DB * pDB = (luageoip_DB *)luaL_checkudata(
      L,
      idx,
      LUAGEOIP_NETSPEED_MT
    );
  if (pDB == NULL)
  {
    lua_pushstring(L, "lua-geoip error: netspeed db is null");
    return NULL;
  }

  if (pDB->pGeoIP == NULL)
  {
    lua_pushstring(L, "lua-geoip error: attempted to use closed netspeed db");
    return NULL;
  }

  type = GeoIP_database_edition(pDB->pGeoIP);
  if (
      type != GEOIP_NETSPEED_EDITION &&
      type != GEOIP_NETSPEED_EDITION_REV1 &&
      type != GEOIP_NETSPEED_EDITION_REV1_V6
    )
  {
    lua_pushstring(L, "lua-geoip error: object is not an netspeed db");
    return NULL;
  }

  return pDB->pGeoIP;
}

static int push_netspeed_info(lua_State * L, char *orgName)
{
  if (orgName == NULL)
  {
    lua_pushnil(L);
    lua_pushliteral(L, "not found");
    return 2;
  }

  lua_newtable(L);
  lua_pushstring(L, orgName);
  lua_setfield(L, -2, "net_speed");

  return 1;
}

static int lnetspeed_query_by_name(lua_State * L)
{
  GeoIP * pGeoIP = check_netspeed_db(L, 1);
  const char * name = luaL_checkstring(L, 2);

  if (pGeoIP == NULL)
  {
    return lua_error(L); /* Error message already on stack */
  }

  return push_netspeed_info(L, GeoIP_org_by_name(pGeoIP, name));
}

static int lnetspeed_query_by_addr(lua_State * L)
{
  GeoIP * pGeoIP = check_netspeed_db(L, 1);
  const char * addr = luaL_checkstring(L, 2);

  if (pGeoIP == NULL)
  {
    return lua_error(L); /* Error message already on stack */
  }

  return push_netspeed_info(L, GeoIP_org_by_addr(pGeoIP, addr));
}

static int lnetspeed_query_by_ipnum(lua_State * L)
{
  GeoIP * pGeoIP = check_netspeed_db(L, 1);
  lua_Integer ipnum = luaL_checkinteger(L, 2); /* Hoping that value would fit */

  if (pGeoIP == NULL)
  {
    return lua_error(L); /* Error message already on stack */
  }

  return push_netspeed_info(L, GeoIP_org_by_ipnum(pGeoIP, ipnum));
}

static int lnetspeed_charset(lua_State * L)
{
  GeoIP * pGeoIP = check_netspeed_db(L, 1);
  if (pGeoIP == NULL)
  {
    return lua_error(L); /* Error message already on stack */
  }

  lua_pushinteger(L, GeoIP_charset(pGeoIP));

  return 1;
}

static int lnetspeed_set_charset(lua_State * L)
{
  GeoIP * pGeoIP = check_netspeed_db(L, 1);
  int charset = luaL_checkint(L, 2);

  if (pGeoIP == NULL)
  {
    return lua_error(L); /* Error message already on stack */
  }

  GeoIP_set_charset(pGeoIP, charset);

  return 0;
}

static int lnetspeed_close(lua_State * L)
{
  luageoip_DB * pDB = (luageoip_DB *)luaL_checkudata(L, 1, LUAGEOIP_NETSPEED_MT);

  if (pDB && pDB->pGeoIP != NULL)
  {
    GeoIP_delete(pDB->pGeoIP);
    pDB->pGeoIP = NULL;
  }

  return 0;
}

#define lnetspeed_gc lnetspeed_close

static int lnetspeed_tostring(lua_State * L)
{
  GeoIP * pGeoIP = check_netspeed_db(L, 1);
  if (pGeoIP == NULL)
  {
    return lua_error(L); /* Error message already on stack */
  }

  lua_pushstring(L, GeoIP_database_info(pGeoIP));

  return 1;
}

static const luaL_reg M[] =
{
  { "query_by_name", lnetspeed_query_by_name },
  { "query_by_addr", lnetspeed_query_by_addr },
  { "query_by_ipnum", lnetspeed_query_by_ipnum },
  { "charset", lnetspeed_charset },
  { "set_charset", lnetspeed_set_charset },
  { "close", lnetspeed_close },
  { "__gc", lnetspeed_gc },
  { "__tostring", lnetspeed_tostring },

  { NULL, NULL }
};

/* Error message capture code inspired by code by Wolfgang Oertl. */
static int lnetspeed_open(lua_State * L)
{
  static const int allowed_types[] =
  {
    GEOIP_NETSPEED_EDITION,
    GEOIP_NETSPEED_EDITION_REV1,
    GEOIP_NETSPEED_EDITION_REV1_V6
  };

  return luageoip_common_open_db(
      L,
      M,
      GEOIP_NETSPEED_EDITION_REV1,
      GEOIP_MEMORY_CACHE,
      LUAGEOIP_NETSPEED_MT,
      GEOIP_INDEX_CACHE, /* not allowed */
      3,
      allowed_types
    );
}

/* Lua module API */
static const struct luaL_reg R[] =
{
  { "open", lnetspeed_open },

  { NULL, NULL }
};

#ifdef __cplusplus
extern "C" {
#endif

LUALIB_API int luaopen_geoip_netspeed(lua_State * L)
{
  /*
  * Register module
  */
  luaL_register(L, "geoip.netspeed", R);

  /*
  * Register module information
  */
  lua_pushliteral(L, LUAGEOIP_NETSPEED_VERSION);
  lua_setfield(L, -2, "_VERSION");

  lua_pushliteral(L, LUAGEOIP_NETSPEED_COPYRIGHT);
  lua_setfield(L, -2, "_COPYRIGHT");

  lua_pushliteral(L, LUAGEOIP_NETSPEED_DESCRIPTION);
  lua_setfield(L, -2, "_DESCRIPTION");

  return 1;
}

#ifdef __cplusplus
}
#endif
