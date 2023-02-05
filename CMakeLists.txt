cmake_minimum_required(VERSION 3.24)
set(ignoreMe "${CMAKE_C_COMPILER}")
project(Tatsu VERSION 1.0.0 LANGUAGES CXX)

# error flags "-Wno-undefined-func-template -Wno-float-equal -Wno-switch-enum -Wno-padded -Wno-documentation-unknown-command -Wno-c++98-compat-pedantic -Wno-suggest-override -Wno-suggest-destructor-override -Wno-zero-as-null-pointer-constant -Wno-undef -Wno-c++98-compat-extra-semi -Wno-c++98-compat -Weverything -Werror"
set(CMAKE_CROSSCOMPILING ON)
if("${CMAKE_HOST_SYSTEM_NAME}" MATCHES "Darwin")
    set(PROJECT_CXX_FLAGS "-fembed-bitcode=off -pthread -mrelax-all")
elseif("${CMAKE_HOST_SYSTEM_NAME}" MATCHES "Linux")
    set(PROJECT_CXX_FLAGS "-Wl,--allow-multiple-definition -pthread -fPIC -mrelax-all")
elseif("${CMAKE_HOST_SYSTEM_NAME}" MATCHES "MSYS" OR "${CMAKE_HOST_SYSTEM_NAME}" MATCHES "Windows")
    set(PROJECT_CXX_FLAGS "-Wl,--allow-multiple-definition -pthread -fPIC -mrelax-all")
else()
    warning("[WARNING]: Detected unknown system!")
endif()

#SET(CMAKE_EXPORT_COMPILE_COMMANDS 1)
#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

if(ASAN)
    set(ASAN ON)
    set(ASAN_FLAGS "-fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer ${ASAN_FLAGS}")
    set(CMAKE_C_LINK_LIBRARY_FLAG "${CMAKE_C_LINK_LIBRARY_FLAG} ${ASAN_FLAGS}")
    set(CMAKE_MACOSX_RPATH 1)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH 1)
    set(CMAKE_BUILD_RPATH
            "/usr/local/lib"
            "/usr/lib"
            "/opt/procursus/lib"
            "/Library/Developer/CommandLineTools/usr/lib/clang/14.0.0/lib/darwin"
            "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/14.0.0/lib/darwin/"
            "/Applications/Xcode_14.1.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/14.0.0/lib/darwin/"
            "/Applications/Xcode_14.2.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/14.0.0/lib/darwin/")
    #hardcoded because actions never updates xcode
else()
    set(ASAN_FLAGS "")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON" FORCE)

set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT "-flto=thin ${ADDITIONAL_CXX_FLAGS} ${PROJECT_CXX_FLAGS} ${CMAKE_CXX_FLAGS_MINSIZEREL_INIT}")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-flto=thin ${ADDITIONAL_CXX_FLAGS} ${PROJECT_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE_INIT}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "-flto=thin ${ADDITIONAL_CXX_FLAGS} ${PROJECT_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT}")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-g -O0 ${ADDITIONAL_CXX_FLAGS} ${PROJECT_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG_INIT} ${ASAN_FLAGS}")

set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL_INIT}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE_INIT}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG_INIT}")

add_executable(Tatsu src/main.cpp src/Manifest.cpp src/Tatsu.cpp src/Requests.cpp src/Modern.cpp)

target_include_directories(Tatsu PRIVATE
        "${CMAKE_SOURCE_DIR}/dep_root/include"
        "${CMAKE_SOURCE_DIR}/include"
        "${CMAKE_SOURCE_DIR}/src")

target_link_directories(Tatsu PRIVATE
        "${CMAKE_SOURCE_DIR}/dep_root/lib")

if(NOT NO_PKGCFG)
    find_package(PkgConfig REQUIRED FATAL_ERROR)
    pkg_check_modules(DEPS REQUIRED
            libplist-2.0
            libplist++-2.0
            fmt
            libixwebsocket)
    target_include_directories(Tatsu PRIVATE "${DEPS_INCLUDE_DIRS}")
    target_link_directories(Tatsu PRIVATE "${DEPS_LIBRARY_DIRS}")
    target_link_libraries(Tatsu PRIVATE "${DEPS_LIBRARIES}")
else()
    target_link_libraries(Tatsu PRIVATE
            plist-2.0
            plist++-2.0
            ixwebsocket)
endif()
