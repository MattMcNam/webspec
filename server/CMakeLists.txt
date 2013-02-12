cmake_minimum_required(VERSION 2.6)

project(webspec)

set(PACKAGE "webspec")
set(PACKAGE_VERSION "0.1")
set(PACKAGE_BUGREPORT "https://github.com/SBlue/webspec/issues")
set(PACKAGE_NAME "${PACKAGE}")
set(PACKAGE_STRING "${PACKAGE} ${PACKAGE_VERSION}")
set(PACKAGE_TARNAME "${PACKAGE}_${PACKAGE_VERSION}")
set(PACKAGE_URL "https://github.com/SBlue/webspec")
set(VERSION "${PACKAGE_VERSION}")

include_directories(${PROJECT_BINARY_DIR})
# Put final product at the root of the build directory
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

# Include source, libwebsockets headers and hl2sdk
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${LWS_DIR}/lib)
include_directories(${HL2SDK}/common)
include_directories(${HL2SDK}/public)
include_directories(${HL2SDK}/public/tier0)
include_directories(${HL2SDK}/public/tier1)
include_directories(${HL2SDK}/game/server)
include_directories(${HL2SDK}/game/shared)

set(HDR_PUBLIC
	src/offsets.h
	src/webspec.h
	src/webspec-definitions.h
	src/webspec-dumping.h
	src/webspec-helpers.h
	src/webspec-vfuncs.h
	)

set(SOURCES
	src/offsets.cpp
	src/webspec.cpp
	)

source_group("Header Files" FILES ${HDR_PUBLIC})
source_group("Source FIles" FILES ${SOURCES})

# Setup defines, compiler options and linker options
set(DBG_DEFINES "-D_DEBUG -DDEBUG")
if (UNIX)
	add_definitions(-Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_snprintf=snprintf -D_vsnprintf=vsnprintf -DHAVE_STDINT_H -DGNUC -DNDEBUG -D_LINUX -DLINUX)
	add_definitions(-Usprintf -Ustrncpy -UPROTECTED_THINGS_ENABLE)
	set(COMPILER_FLAGS "-fno-strict-aliasing -Wall -Werror -Wno-uninitialized -Wno-unused -Wno-switch -msse -m32 -mfpmath=sse -O3 -fno-exceptions -fno-threadsafe-statics -Wno-non-virtual-dtor -Wno-overloaded-virtual")
	set(LINKER_FLAGS "-lm -m32 -static-libgcc -shared")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "$CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS}")

# Create the plugin
add_library(webspec MODULE
	${HDR_PUBLIC}
	${SOURCES})

# Remove 'lib' prefix
set_target_properties(webspec PROPERTIES PREFIX "")

# Link various libraries

# Various HL2SDK libs, websockets and zlib
if (WIN32)
	target_link_libraries(webspec ${HL2SDK}/lib/public/mathlib.lib)
	target_link_libraries(webspec ${HL2SDK}/lib/public/tier0.lib)
	target_link_libraries(webspec ${HL2SDK}/lib/public/tier1.lib)
	target_link_libraries(webspec ${HL2SDK}/lib/public/tier2.lib)
	target_link_libraries(webspec ${HL2SDK}/lib/public/vstdlib.lib)
	target_link_libraries(webspec ${PROJECT_SOURCE_DIR}/lib/websockets.lib)
	target_link_libraries(webspec ${PROJECT_SOURCE_DIR}/lib/ZLIB.lib)
else()
	target_link_libraries(webspec ${HL2SDK}/lib/linux/mathlib_i486.a)
	target_link_libraries(webspec ${HL2SDK}/lib/linux/libtier0_srv.so)
	target_link_libraries(webspec ${HL2SDK}/lib/linux/tier1_i486.a)
	target_link_libraries(webspec ${HL2SDK}/lib/linux/tier2_i486.a)
	target_link_libraries(webspec ${HL2SDK}/lib/linux/libvstdlib_srv.so)
	target_link_libraries(webspec ${PROJECT_SOURCE_DIR}/lib/linux/libwebsockets.a)
	find_package(ZLIB REQUIRED)
	target_link_libraries(webspec ${ZLIB_LIBRARIES})
endif()

# Platform specifics
if (WIN32)
	target_link_libraries(webspec ws2_32.lib)
else()
	target_link_libraries(webspec m)
endif()
