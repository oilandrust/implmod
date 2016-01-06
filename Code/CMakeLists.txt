PROJECT(SkeletalImplicitRayTracing)


#************************ MACRO DECLARATIONS
macro( cache_set var )
set ( ${var} ${ARGN} CACHE INTERNAL "" )  
set ( ${var} ${ARGN} CACHE INTERNAL "" )  
endmacro()
macro( cache_get var )
set ( ${var} )  
endmacro()
macro( add_to_cached_list listname )
cache_get ( ${listname} )
set( _ATC_${listname}_tmp  ${${listname}} )
if ( NOT "${ARGN}" STREQUAL "" )
  list( APPEND _ATC_${listname}_tmp ${ARGN} )
endif()
cache_set ( ${listname} ${_ATC_${listname}_tmp} )
endmacro()
#************************ 


IF(WIN32)
  # Remove all the warnings about depreceated stuff from VS2005
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
  # Needed by GEL due to its fancy iterator work
  ADD_DEFINITIONS(-D_SECURE_SCL=0)
  ADD_DEFINITIONS(-D_HAS_ITERATOR_DEBUGGING=0)
ENDIF(WIN32)


#*********************** CACHE FOR INCLUDE AND LIBS
cache_set(3RD_PARTY_INCLUDE_DIRS   "" )
cache_set(3RD_PARTY_LIBRARIES      "" )
cache_set(3RD_PARTY_LIBRARIES_DIRS "" )


#************************ LOCATE OPENGL/GLUT/GLEW 
find_package(OpenGL)
find_library(GLEW_LIBRARY glew)
add_to_cached_list(3RD_PARTY_LIBRARIES   ${GLEW_LIBRARY} )
find_path(GL_ADDITIONAL_INCLUDE glut.h GL)
add_to_cached_list(3RD_PARTY_INCLUDE_DIRS   ${GL_ADDITIONAL_INCLUDE} )

#************************ LOCATE SOIL
find_path(SOIL_INCLUDE_DIR SOIL.h)
add_to_cached_list(3RD_PARTY_INCLUDE_DIRS   ${SOIL_INCLUDE_DIR} )

#************************ LOCATE GEL 
find_package(GEL)
if(GEL_FOUND)
add_to_cached_list(3RD_PARTY_INCLUDE_DIRS   ${GEL_INCLUDE_DIRS} )
add_to_cached_list(3RD_PARTY_LIBRARIES_DIRS ${GEL_LIBRARY_DIRS} )
add_to_cached_list(3RD_PARTY_LIBRARIES      CGLA HMesh GLGraphics Util Geometry    )
endif(GEL_FOUND)

#************************ LOCATE BOOST 
set(Boost_FIND_VERSION 1.33.1 )
set(Boost_FIND_VERSION_MAJOR 1 )
set(Boost_FIND_VERSION_MINOR 33 )
set(Boost_FIND_VERSION_PATCH 1 )
find_package( Boost REQUIRED filesystem )
add_to_cached_list(3RD_PARTY_INCLUDE_DIRS   ${Boost_INCLUDE_DIRS} )
add_to_cached_list(3RD_PARTY_LIBRARIES_DIRS ${Boost_LIBRARY_DIRS} )


SET(LIBRARY_OUTPUT_PATH ${SkeletalImplicitRayTracing_BINARY_DIR}/lib)
SET(EXECUTABLE_OUTPUT_PATH ${SkeletalImplicitRayTracing_BINARY_DIR}/bin)

include_directories(
	src
)

SUBDIRS(
	src/SklImp
	src/UI
	src/Utils
#	Apps/Demo
	Apps/Editor
	Apps/Viewer
	Apps/TextureSkinning
	Apps/LateSkinning
	Apps/MetaBallEditor
	Apps/AnimationEditor
	Tests/PrimitiveSplitting
	Apps/GekkoSkinnedAnimation
)


