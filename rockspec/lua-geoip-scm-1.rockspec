package = "lua-geoip"
version = "scm-1"
source = {
   url = "git://github.com/agladysh/lua-geoip.git",
   branch = "master"
}
external_dependencies = {
   GEOIP = {
     header = "GeoIP.h",
   }
}
description = {
   summary = "Bindings for MaxMind's GeoIP library",
   homepage = "http://github.com/agladysh/lua-geoip",
   license = "MIT/X11",
   maintainer = "Alexander Gladysh <agladysh@gmail.com>"
}
dependencies = {
   "lua >= 5.1"
}
build = {
   type = "builtin",
   modules = {
      geoip = {
         sources = {
            "src/lua-geoip.c",
            "src/database.c"
         },
         incdirs = {
            "src/"
         },
         libraries = { "GeoIP" }
      },
      ["geoip.country"] = {
         sources = {
            "src/database.c",
            "src/country.c"
         },
         incdirs = {
            "src/"
         },
         libraries = { "GeoIP" }
      },
      ["geoip.city"] = {
         sources = {
            "src/database.c",
            "src/city.c"
         },
         incdirs = {
            "src/"
         },
         libraries = { "GeoIP" }
      },                         
      ["geoip.asnum"] = {
         sources = {
            "src/database.c",
            "src/asnum.c"
         },
         incdirs = {
            "src/"
         },
         libraries = { "GeoIP" }
      } 
   }
}
