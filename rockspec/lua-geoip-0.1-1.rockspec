package = "lua-geoip"
version = "0.1"
source = {
   url = "git://github.com/agladysh/lua-geoip.git",
   branch = "v0.1"
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
            "src/lua-geoip.c"
            "src/country.c"
         },
         incdirs = {
            "src/"
         },
         libraries = { "GeoIP" }
      }
   }
}
