cmake_minimum_required (VERSION 2.8)
Project(ktech CXX)

set(PACKAGE_VERSION_MAJOR 1)
set(PACKAGE_VERSION_MINOR 0)
set(PACKAGE_VERSION "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}")

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/out_of_source_build.cmake)
ENSURE_OUT_OF_SOURCE_BUILD( "${PROJECT_SOURCE_DIR}/build" )

ENABLE_LANGUAGE(C)


set( INCLUDE_DIRS "" )
set( LINK_DIRS "" )
set( ktech_LIBS "" )


include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckTypeSize)
include(CheckStructHasMember)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/thirdparty/LibFindMacros.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/integer_types.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/warnings.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/compiler_specific.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/thirdparty/OptimizeForArchitecture.cmake)


set(CMAKE_BUILD_TYPE Release)
OptimizeForArchitecture()


add_subdirectory(lib)


FIND_PACKAGE(ImageMagick COMPONENTS Magick++ MagickWand MagickCore REQUIRED)
LIBFIND_PKG_CHECK_MODULES(Magick++_PKGCONF Magick++)
if("${Magick++_PKGCONF_CFLAGS}" STREQUAL "")
	if("${ImageMagick_LIBRARIES}" MATCHES "HDRI")
		add_definitions( "-DMAGICKCORE_HDRI_ENABLE=1" )
	else()
		add_definitions( "-DMAGICKCORE_HDRI_ENABLE=0" )
	endif()
	if("${ImageMagick_LIBRARIES}" MATCHES "Q16")
		add_definitions( "-DMAGICKCORE_QUANTUM_DEPTH=16" )
	else()
		add_definitions( "-DMAGICKCORE_QUANTUM_DEPTH=8" )
	endif()
else()
	foreach(f ${Magick++_PKGCONF_CFLAGS})
		set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${f}" )
	endforeach()
endif()
foreach(f ${Magick++_PKGCONF_LDFLAGS})
	set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${f}" )
endforeach()
list( APPEND INCLUDE_DIRS "${ImageMagick_INCLUDE_DIRS}" )
list( APPEND ktech_LIBS "${ImageMagick_LIBRARIES}" )


CHECK_LIBRARY_EXISTS(m floor "" HAVE_MATH_LIB) 
if(HAVE_MATH_LIB)
	set( ktech_LIBS ${ktech_LIBS} "m" )
endif(HAVE_MATH_LIB)


CHECK_INCLUDE_FILES (cstddef HAVE_CSTDDEF)
#
CHECK_INCLUDE_FILES (stddef.h HAVE_STDDEF_H)
CHECK_INCLUDE_FILES (stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILES (inttypes.h HAVE_INTTYPES_H)
CHECK_INCLUDE_FILES (sys/types.h HAVE_SYS_TYPES_H)
#
CHECK_INCLUDE_FILES (strstream HAVE_STRSTREAM)
CHECK_INCLUDE_FILES (strstream.h HAVE_STRSTREAM_H)
#
if(HAVE_STRSTREAM)
	set(CMAKE_EXTRA_INCLUDE_FILES "strstream")
elseif(HAVE_STRSTREAM_H)
	set(CMAKE_EXTRA_INCLUDE_FILES "strstream.h")
endif()
CHECK_TYPE_SIZE("class std::ostrstream" CLASS_STRSTREAM)
set(CMAKE_EXTRA_INCLUDE_FILES)
#
set(CMAKE_EXTRA_INCLUDE_FILES "sstream")
CHECK_TYPE_SIZE("class std::stringstream" SSTREAM)
set(CMAKE_EXTRA_INCLUDE_FILES)
#
CHECK_TYPE_SIZE("long long" LONG_LONG)
#
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )


add_subdirectory(src)


# add_executable(<name> [WIN32] [MACOSX_BUNDLE] ...
include_directories( src lib ${INCLUDE_DIRS} )
link_directories( ${LINK_DIRS} )
add_executable(ktech ${ktech_SOURCES} ${ktech_HEADERS})
target_link_libraries( ktech ${ktech_LIBS} )


INSTALL(TARGETS ktech RUNTIME DESTINATION bin)