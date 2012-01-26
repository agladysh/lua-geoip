#! /bin/bash

set -e

echo "----> Going pedantic all over the source"

echo "--> c89..."
gcc -O2 -fPIC -I/usr/include/lua5.1 -c src/*.c -Isrc/ -Wall --pedantic -Werror --std=c89 -fms-extensions

echo "--> c99..."
gcc -O2 -fPIC -I/usr/include/lua5.1 -c src/*.c -Isrc/ -Wall --pedantic -Werror --std=c99 -fms-extensions

echo "--> c++98..."
gcc -xc++ -O2 -fPIC -I/usr/include/lua5.1 -c src/*.c -Isrc/ -Wall --pedantic -Werror --std=c++98

echo "----> Making rock"
sudo luarocks make rockspec/lua-geoip-scm-1.rockspec
