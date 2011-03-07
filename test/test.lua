-- TODO: Scrap these hacks and write a proper test suite.

pcall(require, 'luarocks.require')

local socket = require 'socket'

local geoip = require 'geoip'
local geoip_country = require 'geoip.country'

print("TESTING lua-geoip")
print("")
print("VERSION: ", assert(geoip._VERSION))
print("DESCRIPTION: ", assert(geoip._DESCRIPTION))
print("COPYRIGHT: ", assert(geoip._COPYRIGHT))
print("")

-- Check that required files exist
-- See README on info on how to get them
assert(io.open("./GeoIP.dat", "r")):close()
assert(io.open("./GeoLiteCity.dat", "r")):close()

do
  local id = assert(geoip.id_by_code('RU'))
  assert(geoip.code_by_id(id) == 'RU')
  assert(geoip.code3_by_id(id) == 'RUS')
  assert(geoip.name_by_id(id) == 'Russian Federation')
  assert(geoip.continent_by_id(id) == 'EU')
  assert(geoip.region_name_by_code('RU', '77') == "Tver'") -- WTF? MSK?
  assert(geoip.time_zone_by_country_and_region('RU', '77') == 'Europe/Moscow')
end

do
  assert(geoip_country.open("./BADFILENAME") == nil)

  assert(pcall(geoip_country.open, nil, geoip.INDEX_CACHE) == false)

  --assert(geoip_country.open(nil, 2 ^ 10) == nil) -- TODO: This should fail
  --assert(geoip_country.open(nil, nil, -1) == nil) -- TODO: This should fail

  assert(geoip_country.open("./GeoLiteCity.dat") == nil)
end

do
  local flags =
  {
    geoip.STANDARD;
    geoip.MEMORY_CACHE;
    geoip.CHECK_CACHE;
    --geoip.INDEX_CACHE; -- not supported
    geoip.MMAP_CACHE;
  }

  for _, flag in ipairs(flags) do
    assert(geoip_country.open(nil, flag)):close()
    assert(geoip_country.open("./GeoIP.dat", flag)):close()
  end
end

do
  local geodb = assert(
      geoip_country.open("./GeoIP.dat")
    )
  geodb:close()
  geodb:close()
end

print()

do
  local check = function(db, method, arg)
    local id = assert(db[method](db, arg, "id"))
    assert(type(id) == "number")

    local expected =
    {
      id = id;
      code = assert(geoip.code_by_id(id));
      code3 = assert(geoip.code3_by_id(id));
      name = assert(geoip.name_by_id(id));
      continent = assert(geoip.continent_by_id(id));
    }

    local all = assert(db[method](db, arg))

    local keys = { }
    for k, v in pairs(expected) do
      assert(all[k] == expected[k])
      assert(db[method](db, arg, k) == expected[k])
      keys[#keys + 1] = k
    end

    local r = { db[method](db, arg, unpack(keys)) }
    assert(#r == #keys)
    for i = 1, #keys do
      assert(r[i] == expected[keys[i]])
    end
  end

  local geodb = assert(geoip_country.open("./GeoIP.dat"))

  check(geodb, "query_by_name", "google-public-dns-a.google.com")
  check(geodb, "query_by_addr", "8.8.8.8")
  check(geodb, "query_by_ipnum", 134744072) -- 8.8.8.8

  geodb:close()
end

-- TODO: Test two DBs in parallel
do
  local geodb = assert(geoip_country.open("./GeoIP.dat"))

  do
    print("profiling ipnum queries") -- slow due to dns resolution

    local num_queries = 1e5

    local time_start = socket.gettime()
    for i = 1, num_queries do
      if i % 1e4 == 0 then
        print("#", i, "of", num_queries)
      end
      assert(geodb:query_by_ipnum(134744072, "id")) -- 8.8.8.8
    end

    print(
        num_queries / (socket.gettime() - time_start),
        "ipnum queries per second"
      )
    print()
  end

  do
    print("profiling addr queries") -- slow due to dns resolution

    local num_queries = 1e5

    local time_start = socket.gettime()
    for i = 1, num_queries do
      if i % 1e4 == 0 then
        print("#", i, "of", num_queries)
      end
      assert(geodb:query_by_name("8.8.8.8", "id"))
    end

    print(
        num_queries / (socket.gettime() - time_start),
        "addr queries per second"
      )
    print()
  end

  do
    print("profiling name queries")

    local num_queries = 500 -- slow due to dns resolution

    local time_start = socket.gettime()
    for i = 1, num_queries do
      if i % 50 == 0 then
        print("#", i, "of", num_queries)
      end
      assert(geodb:query_by_name("ya.ru", "id"))
    end

    print(
        num_queries / (socket.gettime() - time_start),
        "name queries per second"
      )
    print()
  end
end

print("")
print("OK")
