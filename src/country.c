/*
* country.c: Bindings for MaxMind's GeoIP library
*              See copyright information in file COPYRIGHT.
*/

#include <fcntl.h>

#include "lua-geoip.h"

#define LUAGEOIP_COUNTRY_VERSION     "lua-geoip.country 0.1"
#define LUAGEOIP_COUNTRY_COPYRIGHT   "Copyright (C) 2011, lua-geoip authors"
#define LUAGEOIP_COUNTRY_DESCRIPTION \
        "Bindings for MaxMind's GeoIP library (country database)"

GeoIP * check_country_db(lua_State * L, int idx)
{
  luageoip_DB * pDB = (luageoip_DB *)luaL_checkudata(L, 1, LUAGEOIP_COUNTRY_MT);
  if (pDB == NULL)
  {
    lua_pushstring(L, "lua-geoip error: country db is null");
    return NULL;
  }

  if (pDB->pGeoIP == NULL)
  {
    lua_pushstring(L, "lua-geoip error: attempted to use closed country db");
    return NULL;
  }

  if (
      pDB->pGeoIP->databaseType != GEOIP_COUNTRY_EDITION &&
      pDB->pGeoIP->databaseType != GEOIP_COUNTRY_EDITION_V6
    )
  {
    lua_pushstring(L, "lua-geoip error: object is not a country db");
    return NULL;
  }

  return pDB->pGeoIP;
}

/* TODO: Handle when id 0? */
int push_country_info_by_id(lua_State * L, int first_arg_idx, int id)
{
  static const int NUM_OPTS = 5;
  static const char * const opts[] =
  {
    /* order is important! */
    /* 0 */ "id",
    /* 1 */ "code",
    /* 2 */ "code3",
    /* 3 */ "continent",
    /* 4 */ "name",
    NULL
  };

  int nargs = lua_gettop(L) - first_arg_idx + 1;
  int need_all = (nargs == 0);

  int i = 0;

  if (need_all)
  {
    nargs = NUM_OPTS;
    lua_newtable(L);
  }

  for (i = 0; i < nargs; ++i)
  {
    int idx = (need_all)
      ? i
      : luaL_checkoption(L, first_arg_idx + i, NULL, opts)
      ;

    /* TODO: Ugly */
    switch (idx)
    {
      case 0: /* id */
        lua_pushinteger(L, id);
        break;

      case 1: /* code */
        lua_pushstring(L, GeoIP_code_by_id(id));
        break;

      case 2: /* code3 */
        lua_pushstring(L, GeoIP_code3_by_id(id));
        break;

      case 3: /* continent */
        lua_pushstring(L, GeoIP_continent_by_id(id));
        break;

      case 4: /* name */
        lua_pushstring(L, GeoIP_name_by_id(id));
        break;

      default:
        /* Hint: Did you synchronize switch cases with opts array? */
        return luaL_error(L, "lua-geoip error: bad implementation");
    }

    if (need_all)
    {
      lua_setfield(L, -2, opts[i]);
    }
  }

  return (need_all) ? 1 : nargs;
}

/* TODO: Remove copy-paste below! */

static int lcountry_query_by_name(lua_State * L)
{
  GeoIP * pGeoIP = check_country_db(L, 1);
  if (pGeoIP == NULL)
  {
    return lua_error(L); /* Error message already on stack */
  }

  const char * name = luaL_checkstring(L, 2);

  return push_country_info_by_id(
      L, 3, GeoIP_id_by_name(pGeoIP, name)
    );
}

static int lcountry_query_by_addr(lua_State * L)
{
  GeoIP * pGeoIP = check_country_db(L, 1);
  if (pGeoIP == NULL)
  {
    return lua_error(L); /* Error message already on stack */
  }

  const char * addr = luaL_checkstring(L, 2);

  return push_country_info_by_id(
      L, 3, GeoIP_id_by_addr(pGeoIP, addr)
    );
}

static int lcountry_query_by_ipnum(lua_State * L)
{
  GeoIP * pGeoIP = check_country_db(L, 1);
  if (pGeoIP == NULL)
  {
    return lua_error(L); /* Error message already on stack */
  }

  lua_Integer ipnum = luaL_checkinteger(L, 2); /* Hoping that value would fit */

  return push_country_info_by_id(
      L, 3, GeoIP_id_by_ipnum(pGeoIP, ipnum)
    );
}

static int lcountry_charset(lua_State * L)
{
  GeoIP * pGeoIP = check_country_db(L, 1);
  if (pGeoIP == NULL)
  {
    return lua_error(L); /* Error message already on stack */
  }

  lua_pushinteger(L, GeoIP_charset(pGeoIP));

  return 1;
}

static int lcountry_set_charset(lua_State * L)
{
  GeoIP * pGeoIP = check_country_db(L, 1);
  if (pGeoIP == NULL)
  {
    return lua_error(L); /* Error message already on stack */
  }

  int charset = luaL_checkint(L, 2);

  GeoIP_set_charset(pGeoIP, charset);

  return 0;
}

static int lcountry_close(lua_State * L)
{
  luageoip_DB * pDB = (luageoip_DB *)luaL_checkudata(L, 1, LUAGEOIP_COUNTRY_MT);

  if (pDB && pDB->pGeoIP != NULL)
  {
  	GeoIP_delete(pDB->pGeoIP);
  	pDB->pGeoIP = NULL;
  }

  return 0;
}

#define lcountry_gc lcountry_close

static int lcountry_tostring(lua_State * L)
{
  GeoIP * pGeoIP = check_country_db(L, 1);
  if (pGeoIP == NULL)
  {
    return lua_error(L); /* Error message already on stack */
  }

  lua_pushstring(L, GeoIP_database_info(pGeoIP));

  return 1;
}

static const luaL_reg M[] =
{
  { "query_by_name", lcountry_query_by_name },
  { "query_by_addr", lcountry_query_by_addr },
  { "query_by_ipnum", lcountry_query_by_ipnum },

  { "charset", lcountry_charset },
  { "set_charset", lcountry_set_charset },
  { "close", lcountry_close },
  { "__gc", lcountry_gc },
  { "__tostring", lcountry_tostring },

  { NULL, NULL }
};

/* Error message capture code inspired by code by Wolfgang Oertl. */
static int lcountry_open(lua_State * L)
{
  /* First argument is checked later */
  int flags = luaL_optint(L, 2, GEOIP_STANDARD);
  int charset = luaL_optint(L, 2, GEOIP_CHARSET_UTF8);

  GeoIP * pGeoIP = NULL;
  luageoip_DB * pResult = NULL;

  int old_stderr;
  int pipefd[2];
  char buf[256];

  int error_reported = 0;

  if ((flags & GEOIP_INDEX_CACHE) == GEOIP_INDEX_CACHE)
  {
    /* TODO: Or is it concrete DB file problem? */
    return luaL_error(
        L,
        "lua-geoip.country: can't open country db with INDEX_CACHE"
      );
  }

  /* Errors are printed to stderr, capture them */
  {
    pipe(pipefd);
    fcntl(pipefd[0], F_SETFL, O_NONBLOCK);
    fcntl(pipefd[1], F_SETFL, O_NONBLOCK);
    old_stderr = dup(2);
    dup2(pipefd[1], 2);
  }

  if (lua_isnoneornil(L, 1))
  {
    pGeoIP = GeoIP_open_type(GEOIP_COUNTRY_EDITION, flags);
  }
  else
  {
    const char * filename = luaL_checkstring(L, 1);
    if (filename == NULL)
    {
      lua_pushnil(L);
      lua_pushfstring(
          L,
          "bad argument #1: filename or db type expected, got %s",
          luaL_typename(L, 1)
        );
      error_reported = 1;
    }
    else
    {
      pGeoIP = GeoIP_open(filename, flags);
    }
  }

  /* Cleanup error handling */
  {
    int n = read(pipefd[0], buf, sizeof(buf));
    if (n >= 0)
    {
      buf[n] = 0;

      if (!pGeoIP) /* ?! What to do otherwise? */
      {
        lua_pushnil(L);
        lua_pushstring(L, buf);
        error_reported = 1;
      }
    }

    close(pipefd[0]);
    close(pipefd[1]);
    dup2(old_stderr, 2);
  }

  if (
      pGeoIP &&
      pGeoIP->databaseType != GEOIP_COUNTRY_EDITION &&
      pGeoIP->databaseType != GEOIP_COUNTRY_EDITION_V6
    )
  {
    lua_pushnil(L);
    lua_pushstring(L, "lua-geoip error: file is not a country db");
    error_reported = 1;

    pGeoIP = NULL;
  }

  if (pGeoIP == NULL)
  {
    if (!error_reported)
    {
      lua_pushnil(L);
      lua_pushliteral(L, "failed to open geoip database file");
      error_reported = 1;
    }

    return 2; /* nil and error message already on stack */
  }

  pResult = lua_newuserdata(L, sizeof(luageoip_DB));
  pResult->pGeoIP = pGeoIP;

  if (luaL_newmetatable(L, LUAGEOIP_COUNTRY_MT))
  {
    luaL_register(L, NULL, M);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
  }

  lua_setmetatable(L, -2);

  return 1;
}

/* Lua module API */
static const struct luaL_reg R[] =
{
  { "open", lcountry_open },

  { NULL, NULL }
};

#ifdef __cplusplus
extern "C" {
#endif

LUALIB_API int luaopen_geoip_country(lua_State * L)
{
  /*
  * Register module
  */
  luaL_register(L, "geoip.country", R);

  /*
  * Register module information
  */
  lua_pushliteral(L, LUAGEOIP_COUNTRY_VERSION);
  lua_setfield(L, -2, "_VERSION");

  lua_pushliteral(L, LUAGEOIP_COUNTRY_COPYRIGHT);
  lua_setfield(L, -2, "_COPYRIGHT");

  lua_pushliteral(L, LUAGEOIP_COUNTRY_DESCRIPTION);
  lua_setfield(L, -2, "_DESCRIPTION");

  return 1;
}

#ifdef __cplusplus
}
#endif
