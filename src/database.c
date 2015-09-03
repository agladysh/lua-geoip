#include <fcntl.h>
#include <unistd.h>

#include "lua-geoip.h"
#include "database.h"

/* Error message capture code inspired by code by Wolfgang Oertl. */
int luageoip_common_open_db(
    lua_State * L,
    const luaL_reg * M,
    int default_type,
    int default_flags,
    const char * mt_name,
    unsigned int bad_flags,
    size_t num_allowed_types,
    const int * allowed_types
  )
{
  /* First argument is checked later */
  int flags = luaL_optint(L, 2, default_flags);
  int charset = luaL_optint(L, 2, GEOIP_CHARSET_UTF8);

  GeoIP * pGeoIP = NULL;
  luageoip_DB * pResult = NULL;

  int error_reported = 0;

  if (bad_flags && (flags & bad_flags) == bad_flags)
  {
    /* TODO: Or is it concrete DB file problem? */
    return luaL_error(
        L,
        "%s error: can't open db with these flags",
        mt_name
      );
  }

  if (lua_isnoneornil(L, 1))
  {
    pGeoIP = GeoIP_open_type(default_type, flags);
  }
  else
  {
    const char * filename = luaL_checkstring(L, 1);
    pGeoIP = GeoIP_open(filename, flags);
  }

  if (pGeoIP)
  {
    int type = GeoIP_database_edition(pGeoIP);
    int found = 0;
    size_t i = 0;

    for (i = 0; i < num_allowed_types; ++i)
    {
      if (type == allowed_types[i])
      {
        found = 1;
        break;
      }
    }

    if (!found)
    {
      lua_pushnil(L);
      lua_pushfstring(
          L,
          "%s error: unexpected db type in that file (%s)",
          mt_name,
          GeoIP_database_info(pGeoIP)
        );
      error_reported = 1;

      GeoIP_delete(pGeoIP);
      pGeoIP = NULL;
    }
  }

  if (pGeoIP == NULL)
  {
    if (!error_reported)
    {
      lua_pushnil(L);
      lua_pushfstring(
          L,
          "%s error: failed to open database file",
          mt_name
        );
      error_reported = 1;
    }

    return 2; /* nil and error message already on stack */
  }

  GeoIP_set_charset(pGeoIP, charset);

  pResult = (luageoip_DB *)lua_newuserdata(L, sizeof(luageoip_DB));
  pResult->pGeoIP = pGeoIP;

  if (luaL_newmetatable(L, mt_name))
  {
    luaL_register(L, NULL, M);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
  }

  lua_setmetatable(L, -2);

  return 1;
}
