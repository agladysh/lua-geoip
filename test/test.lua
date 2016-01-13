-- TODO: Scrap these hacks and write a proper test suite.

pcall(require, 'luarocks.require')

local math = require 'math'
local socket = require 'socket'

local geoip = require 'geoip'
local geoip_country = require 'geoip.country'
local geoip_city = require 'geoip.city'
local geoip_asnum = require 'geoip.asnum'

local geoip_country_filename = select(1, ...) or "./GeoIP.dat"
local geoip_city_filename = select(2, ...) or "./GeoLiteCity.dat"
local geoip_asnum_filename = select(3, ...) or "./GeoIPASNum.dat"

print("TESTING lua-geoip")
print("")
print("VERSION: ", assert(geoip._VERSION))
print("DESCRIPTION: ", assert(geoip._DESCRIPTION))
print("COPYRIGHT: ", assert(geoip._COPYRIGHT))
print("")
print("VERSION: ", assert(geoip_country._VERSION))
print("DESCRIPTION: ", assert(geoip_country._DESCRIPTION))
print("COPYRIGHT: ", assert(geoip_country._COPYRIGHT))
print("")
print("VERSION: ", assert(geoip_city._VERSION))
print("DESCRIPTION: ", assert(geoip_city._DESCRIPTION))
print("COPYRIGHT: ", assert(geoip_city._COPYRIGHT))
print("")
print("VERSION: ", assert(geoip_asnum._VERSION))
print("DESCRIPTION: ", assert(geoip_asnum._DESCRIPTION))
print("COPYRIGHT: ", assert(geoip_asnum._COPYRIGHT))
print("")

-- Check that required files exist
-- See README on info on how to get them
assert(io.open(geoip_country_filename, "r")):close()
assert(io.open(geoip_city_filename, "r")):close()
assert(io.open(geoip_asnum_filename, "r")):close()

do
  local id = assert(geoip.id_by_code('RU'))

  assert(geoip.code_by_id(id) == 'RU')
  assert(geoip.code3_by_id(id) == 'RUS')
  assert(geoip.name_by_id(id) == 'Russian Federation')

  -- Depends on libgeoip version o_O
  assert(geoip.continent_by_id(id) == 'EU' or geoip.continent_by_id(id) == 'AS')

  assert(geoip.region_name_by_code('RU', '77') == "Tver'") -- WTF? MSK?
  assert(geoip.time_zone_by_country_and_region('RU', '77') == 'Europe/Moscow')
end

do
  assert(geoip_country.open("./BADFILENAME") == nil)

  assert(pcall(geoip_country.open, nil, geoip.INDEX_CACHE) == false)

  --assert(geoip_country.open(nil, 2 ^ 10) == nil) -- TODO: This should fail
  --assert(geoip_country.open(nil, nil, -1) == nil) -- TODO: This should fail

  assert(geoip_country.open(geoip_asnum_filename) == nil)
end

do
  assert(geoip_city.open("./BADFILENAME") == nil)

  --assert(geoip_city.open(nil, 2 ^ 10) == nil) -- TODO: This should fail
  --assert(geoip_city.open(nil, nil, -1) == nil) -- TODO: This should fail

  assert(geoip_city.open(geoip_country_filename) == nil)
end

do
  assert(geoip_asnum.open("./BADFILENAME") == nil)

  --assert(geoip_asnum.open(nil, 2 ^ 10) == nil) -- TODO: This should fail
  --assert(geoip_asnum.open(nil, nil, -1) == nil) -- TODO: This should fail

  assert(geoip_asnum.open(geoip_country_filename) == nil)
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

  for _, flag in ipairs(flags) do
    if flag ~= geoip.INDEX_CACHE then
      assert(geoip_country.open(geoip_country_filename, flag)):close()
    end
    assert(geoip_asnum.open(geoip_asnum_filename, flag)):close()
    assert(geoip_city.open(geoip_city_filename, flag)):close()
  end
end

do
  local geodb = assert(
      geoip_country.open(geoip_country_filename)
    )
  geodb:close()
  geodb:close()
end

do
  local geodb = assert(
      geoip_city.open(geoip_city_filename)
    )
  geodb:close()
  geodb:close()
end

do
  local geodb = assert(
      geoip_asnum.open(geoip_asnum_filename)
    )
  geodb:close()
  geodb:close()
end

do
  local check_country = function(db, method, arg)
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

  local apack = function(...)
    return select("#", ...), { ... }
  end

  local check_city = function(db, method, arg)
    local keys =
    {
      "country_code";
      "country_code3";
      "country_name";
      "region";
      "city";
      "postal_code";
      "latitude";
      "longitude";
      "metro_code";
      "dma_code";
      "area_code";
      "charset";
      "continent_code";
    }

    local all = assert(db[method](db, arg))

    local nret, r = apack(db[method](db, arg, unpack(keys)))
    assert(nret == #keys)

    for i = 1, #keys do
      assert(r[i] == all[keys[i]])
      assert(r[i] == db[method](db, arg, keys[i]))
    end
  end

  local check_asnum = function(db, method, arg)
    local org = assert(db[method](db, arg))
  end

  local geodb_country = assert(geoip_country.open(geoip_country_filename))
  local geodb_city = assert(geoip_city.open(geoip_city_filename))
  local geodb_asnum = assert(geoip_asnum.open(geoip_asnum_filename))

  local checkers =
  {
    [geodb_country] = check_country;
    [geodb_city] = check_city;
    [geodb_asnum] = check_asnum;
  }

  for _, geodb in ipairs { geodb_country, geodb_city, geodb_asnum } do
    local checker = checkers[geodb]

    checker(geodb, "query_by_name", "google-public-dns-a.google.com")
    checker(geodb, "query_by_addr", "8.8.8.8")
    checker(geodb, "query_by_ipnum", 134744072) -- 8.8.8.8
  end

  geodb_country:close()
  geodb_city:close()
  geodb_asnum:close()
end

-- TODO: Test two different DBs open in parallel work properly

local profiles =
{
  {
    name = "country";
    module = geoip_country;
    file = geoip_country_filename;
    field = "id";
  };
  {
    name = "city";
    module = geoip_city;
    file = geoip_city_filename;
    field = "country_code";
  };
  {
    name = "asnum";
    module = geoip_asnum;
    file = geoip_asnum_filename;
    field = "org";
  };
}

for i = 1, #profiles do
  local p = profiles[i]

  local ok, geodb = pcall(p.module.open, p.file)

  do
    print(p.name, "profiling ipnum queries")

    local num_queries = 1e5

    local cases = { }
    for i = 1, num_queries do
      cases[i] = math.random(0x60FFFFFF)
    end

    local time_start = socket.gettime()
    for i = 1, num_queries do
      if i % 1e4 == 0 then
        print("#", i, "of", num_queries)
      end
      if p.name ~= "asnum" then
        local result, err = geodb:query_by_ipnum(cases[i], p.field)
        if not result and err ~= "not found" then
          error(err)
        end
      else
        local ok, err = geodb:query_by_ipnum(cases[i])
        if not ok and err ~= nil then
          error(err)
        end
      end
    end

    print(
        p.name,
        num_queries / (socket.gettime() - time_start),
        "ipnum queries per second"
      )
    print()
  end

  do
    print(p.name, "profiling addr queries") -- slow due to dns resolution

    local num_queries = 1e5

    local cases = { }
    for i = 1, num_queries do
      cases[i] = ('%d.%d.%d.%d'):format(
          math.random(255),
          math.random(255),
          math.random(255),
          math.random(255)
        )
    end

    local time_start = socket.gettime()
    for i = 1, num_queries do
      if i % 1e4 == 0 then
        print("#", i, "of", num_queries)
      end
      if p.name ~= "asnum" then
        local ok, err = geodb:query_by_addr(cases[i], p.field)
        if not ok and err ~= "not found" then
          error(err)
        end
      else
        local ok, err = geodb:query_by_addr(cases[i])
        if not ok and err ~= nil then
          error(err)
        end
      end
    end

    print(
        p.name,
        num_queries / (socket.gettime() - time_start),
        "addr queries per second"
      )
    print()
  end

  do
    print(p.name, "profiling name queries")

    local num_queries = 500 -- slow due to dns resolution

    local time_start = socket.gettime()
    for i = 1, num_queries do
      if i % 50 == 0 then
        print("#", i, "of", num_queries)
      end

      if p.name == "asnum" then
        assert(geodb:query_by_name("ya.ru"))
      else
        assert(geodb:query_by_name("ya.ru", p.field))
      end
    end

    print(
        p.name,
        num_queries / (socket.gettime() - time_start),
        "name queries per second"
      )
    print()
  end

  geodb:close()
end

print("")
print("OK")
