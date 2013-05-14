-- TODO: Scrap these hacks and write a proper test suite.

pcall(require, 'luarocks.require')

local subject  = require 'geoip.isp'
local filename = select(1, ...) or "./GeoIPISP.dat"

print("TESTING lua-geoip.isp")
print("")
print("VERSION: ", assert(subject._VERSION))
print("DESCRIPTION: ", assert(subject._DESCRIPTION))
print("COPYRIGHT: ", assert(subject._COPYRIGHT))
print("")

-- Check that required files exist
assert(io.open(filename, "r")):close()

do
  assert(subject.open("./BADFILENAME") == nil)
end

do
  local flags =
  {
    geoip.STANDARD;
    geoip.MEMORY_CACHE;
    geoip.CHECK_CACHE;
    geoip.INDEX_CACHE;
    geoip.MMAP_CACHE;
  }

  for i=1,#flags do
    assert(subject.open(filename, flags[i])):close()
  end
end

do
  local geodb = assert(subject.open(filename))
  geodb:close()
  geodb:close()
end

do
  local geodb = assert(subject.open(filename))
  local check_isp = function(db, method, arg)
    local res = assert(db[method](db, arg))
    assert(res["isp_name"])
  end

  check_isp(geodb, "query_by_name", "google-public-dns-a.google.com")
  check_isp(geodb, "query_by_addr", "8.8.8.8")
  check_isp(geodb, "query_by_ipnum", 134744072) -- 8.8.8.8
  geodb:close()
end

print("")
print("OK")
