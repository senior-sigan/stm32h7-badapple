cmake_minimum_required(VERSION 3.10)
project(lua C)

set(LUA_VERSION 5.4.7)
set(CMAKE_C_STANDARD 11)
set(LUA_INCLUDE_DIR .)
set(LUA_SRC
    lapi.c
    lcode.c
    lctype.c
    ldebug.c
    ldo.c
    ldump.c
    lfunc.c
    lgc.c
    llex.c
    lmem.c
    lobject.c
    lopcodes.c
    lparser.c
    lstate.c
    lstring.c
    ltable.c
    ltm.c
    lundump.c
    lvm.c
    lzio.c
    lauxlib.c
    lbaselib.c
    lcorolib.c
    ldblib.c
    liolib.c
    lmathlib.c
    loslib.c
    lstrlib.c
    ltablib.c
    lutf8lib.c
    loadlib.c
    linit.c
)

add_library(lua STATIC ${LUA_SRC})

target_include_directories(lua PRIVATE ${LUA_INCLUDE_DIR})
