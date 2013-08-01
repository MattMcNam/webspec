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
# Put final product in 'bin' folder
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

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
	src/callbacks.h
	src/definitions.h
	src/helpers.h
	src/win32_helpers.h
	src/offsets.h
	src/vfuncs.h
	src/webspec.h
	)

set(SOURCES
	src/callbacks.cpp
	src/definitions.cpp
	src/helpers.cpp
	src/offsets.cpp
	src/vfuncs.cpp
	src/webspec.cpp
	)

source_group("Header Files" FILES ${HDR_PUBLIC})
source_group("Source FIles" FILES ${SOURCES})

# Setup defines, compiler options and linker options for each platform
if(UNIX) # Common Unix settings
	add_definitions(-Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_snprintf=snprintf -D_vsnprintf=vsnprintf -DHAVE_STDINT_H -DGNUC -DNDEBUG)
	add_definitions(-Usprintf -Ustrncpy -UPROTECTED_THINGS_ENABLE)
	set(COMPILER_FLAGS "-fno-strict-aliasing -fvisibility=hidden -fvisibility-inlines-hidden -Wall -Werror -Wno-uninitialized -Wno-unused -Wno-switch -msse -m32 -fno-exceptions -fno-threadsafe-statics -Wno-non-virtual-dtor -Wno-overloaded-virtual")
	set(LINKER_FLAGS "-m32")
	if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		add_definitions(-DOSX -D_OSX)
		set(LINKER_FLAGS "${LINKER_FLAGS} -mmacosx-version-min=10.5 -arch i386 -dynamiclib")
	else() # Assume Linux
		add_definitions(-DLINUX -D_LINUX)
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -mfpmath=sse")
		set(LINKER_FLAGS "${LINKER_FLAGS} -shared")
	endif()
else() # Windows
	# Force use of static libs
	# Credit SteveL & brofield on StackOverflow
	foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINOFO)
		string(REGEX REPLACE "/MT" "/MD" ${flag_var} "${${flag_var}}")
	endforeach()
	
	add_definitions(/D_CRT_SECURE_NO_DEPRECATE /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_DEPRECATE)
	set(COMPILER_FLAGS "/EHsc /DR- /W3 /nologo /Zi /TP")
	set(LINKER_FLAGS "/MACHINE:X86 /subsystem:windows /NODEFAULTLIB:libc /NODEFAULTLIB:libcd")
	set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:libcmt")
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /NODEFAULTLIB:libcmtd")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LINKER_FLAGS}")

# Create the plugin
add_library(webspec SHARED
	${HDR_PUBLIC}
	${SOURCES})

# Remove 'lib' prefix
set_target_properties(webspec PROPERTIES PREFIX "")

# Link hl2, lws & system libraries
if(WIN32)
	target_link_libraries(webspec ${HL2SDK}/lib/public/mathlib.lib)
	target_link_libraries(webspec ${HL2SDK}/lib/public/tier0.lib)
	target_link_libraries(webspec ${HL2SDK}/lib/public/tier1.lib)
	target_link_libraries(webspec ${HL2SDK}/lib/public/vstdlib.lib)
	target_link_libraries(webspec ${PROJECT_SOURCE_DIR}/lib/win32/websockets.lib)
	target_link_libraries(webspec ${PROJECT_SOURCE_DIR}/lib/win32/ZLIB.lib)
	target_link_libraries(webspec ws2_32.lib)
else()
	if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		target_link_libraries(webspec ${HL2SDK}/lib/mac/mathlib_i486.a)
		target_link_libraries(webspec ${HL2SDK}/lib/mac/libtier0.dylib)
		target_link_libraries(webspec ${HL2SDK}/lib/mac/tier1_i486.a)
		target_link_libraries(webspec ${HL2SDK}/lib/mac/libvstdlib.dylib)
		target_link_libraries(webspec ${PROJECT_SOURCE_DIR}/lib/mac/libwebsockets.a)
	else()
		target_link_libraries(webspec ${HL2SDK}/lib/linux/mathlib_i486.a)
		target_link_libraries(webspec ${HL2SDK}/lib/linux/libtier0_srv.so)
		target_link_libraries(webspec ${HL2SDK}/lib/linux/tier1_i486.a)
		target_link_libraries(webspec ${HL2SDK}/lib/linux/libvstdlib_srv.so)
		target_link_libraries(webspec ${PROJECT_SOURCE_DIR}/lib/linux/libwebsockets.a)
	endif()
	
	find_package(ZLIB REQUIRED)
	target_link_libraries(webspec ${ZLIB_LIBRARIES})
	target_link_libraries(webspec m)
endif()
