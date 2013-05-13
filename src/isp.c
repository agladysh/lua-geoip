/*
* isp.c: Bindings for MaxMind's GeoIP library
*        See copyright information in file COPYRIGHT.
*/

#include "lua-geoip.h"
#include "database.h"

#define LUAGEOIP_ISP_VERSION     "lua-geoip.isp 0.1.0"
#define LUAGEOIP_ISP_COPYRIGHT   "Copyright (C) 2013, lua-geoip authors"
#define LUAGEOIP_ISP_DESCRIPTION \
        "Bindings for MaxMind's GeoIP library (ISP database)"

static GeoIP * check_isp_db(lua_State * L, int idx)
{
  int type = 0;
  luageoip_DB * pDB = (luageoip_DB *)luaL_checkudata(
      L,
      idx,
      LUAGEOIP_ISP_MT
    );
  if (pDB == NULL)
  {
    lua_pushstring(L, "lua-geoip error: isp db is null");
    return NULL;
  }

  if (pDB->pGeoIP == NULL)
  {
    lua_pushstring(L, "lua-geoip error: attempted to use closed isp db");
    return NULL;
  }

  type = GeoIP_database_edition(pDB->pGeoIP);
  if (
      type != GEOIP_ISP_EDITION &&
      type != GEOIP_ISP_EDITION_V6
    )
  {
    lua_pushstring(L, "lua-geoip error: object is not an isp db");
    return NULL;
  }

  return pDB->pGeoIP;
}

static int push_isp_info(lua_State * L, char *orgName)
{
  if (orgName == NULL)
  {
    lua_pushnil(L);
    lua_pushliteral(L, "not found");
    return 2;
  }

  lua_newtable(L);
  lua_pushstring(L, orgName);
  lua_setfield(L, -2, "isp_name");

  return 1;
}

static int lisp_query_by_name(lua_State * L)
{
  GeoIP * pGeoIP = check_isp_db(L, 1);
  const char * name = luaL_checkstring(L, 2);

  if (pGeoIP == NULL)
  {
    return lua_error(L); /* Error message already on stack */
  }

  return push_isp_info(L, GeoIP_org_by_name(pGeoIP, name));
}

static int lisp_query_by_addr(lua_State * L)
{
  GeoIP * pGeoIP = check_isp_db(L, 1);
  const char * addr = luaL_checkstring(L, 2);

  if (pGeoIP == NULL)
  {
    return lua_error(L); /* Error message already on stack */
  }

  return push_isp_info(L, GeoIP_org_by_addr(pGeoIP, addr));
}

static int lisp_query_by_ipnum(lua_State * L)
{
  GeoIP * pGeoIP = check_isp_db(L, 1);
  lua_Integer ipnum = luaL_checkinteger(L, 2); /* Hoping that value would fit */

  if (pGeoIP == NULL)
  {
    return lua_error(L); /* Error message already on stack */
  }

  return push_isp_info(L, GeoIP_org_by_ipnum(pGeoIP, ipnum));
}

static int lisp_charset(lua_State * L)
{
  GeoIP * pGeoIP = check_isp_db(L, 1);
  if (pGeoIP == NULL)
  {
    return lua_error(L); /* Error message already on stack */
  }

  lua_pushinteger(L, GeoIP_charset(pGeoIP));

  return 1;
}

static int lisp_set_charset(lua_State * L)
{
  GeoIP * pGeoIP = check_isp_db(L, 1);
  int charset = luaL_checkint(L, 2);

  if (pGeoIP == NULL)
  {
    return lua_error(L); /* Error message already on stack */
  }

  GeoIP_set_charset(pGeoIP, charset);

  return 0;
}

static int lisp_close(lua_State * L)
{
  luageoip_DB * pDB = (luageoip_DB *)luaL_checkudata(L, 1, LUAGEOIP_ISP_MT);

  if (pDB && pDB->pGeoIP != NULL)
  {
  	GeoIP_delete(pDB->pGeoIP);
  	pDB->pGeoIP = NULL;
  }

  return 0;
}

#define lisp_gc lisp_close

static int lisp_tostring(lua_State * L)
{
  GeoIP * pGeoIP = check_isp_db(L, 1);
  if (pGeoIP == NULL)
  {
    return lua_error(L); /* Error message already on stack */
  }

  lua_pushstring(L, GeoIP_database_info(pGeoIP));

  return 1;
}

static const luaL_reg M[] =
{
  { "query_by_name", lisp_query_by_name },
  { "query_by_addr", lisp_query_by_addr },
  { "query_by_ipnum", lisp_query_by_ipnum },
  { "charset", lisp_charset },
  { "set_charset", lisp_set_charset },
  { "close", lisp_close },
  { "__gc", lisp_gc },
  { "__tostring", lisp_tostring },

  { NULL, NULL }
};

/* Error message capture code inspired by code by Wolfgang Oertl. */
static int lisp_open(lua_State * L)
{
  static const int allowed_types[] =
  {
    GEOIP_ISP_EDITION,
    GEOIP_ISP_EDITION_V6
  };

  return luageoip_common_open_db(
      L,
      M,
      GEOIP_ISP_EDITION,
      GEOIP_MEMORY_CACHE,
      LUAGEOIP_ISP_MT,
      GEOIP_INDEX_CACHE, /* not allowed */
      2,
      allowed_types
    );
}

/* Lua module API */
static const struct luaL_reg R[] =
{
  { "open", lisp_open },

  { NULL, NULL }
};

#ifdef __cplusplus
extern "C" {
#endif

LUALIB_API int luaopen_geoip_isp(lua_State * L)
{
  /*
  * Register module
  */
  luaL_register(L, "geoip.isp", R);

  /*
  * Register module information
  */
  lua_pushliteral(L, LUAGEOIP_ISP_VERSION);
  lua_setfield(L, -2, "_VERSION");

  lua_pushliteral(L, LUAGEOIP_ISP_COPYRIGHT);
  lua_setfield(L, -2, "_COPYRIGHT");

  lua_pushliteral(L, LUAGEOIP_ISP_DESCRIPTION);
  lua_setfield(L, -2, "_DESCRIPTION");

  return 1;
}

#ifdef __cplusplus
}
#endif
