PROJECT(RayTraceImplicit)

IF(WIN32)
  # Remove all the warnings about depreceated stuff from VS2005
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
  
  # Needed by GEL due to its fancy iterator work
  ADD_DEFINITIONS(-D_SECURE_SCL=0)
  ADD_DEFINITIONS(-D_HAS_ITERATOR_DEBUGGING=0)
ENDIF(WIN32)

# Try to locate the place where GEL was build
FIND_PACKAGE(GEL)

IF(GEL_FOUND)
  INCLUDE(${GEL_USE_FILE})
ENDIF(GEL_FOUND)

# Within the project we add a simple executable program
ADD_EXECUTABLE(RayTraceImplicit
	RayTraceImplicit.cpp

)


# We add all the GEL libraries
# The link directory is automagically set by the "GEL_USE_FILE"
# You can pick and choose
TARGET_LINK_LIBRARIES(RayTraceImplicit
  CGLA
  Geometry
  GLGraphics
  HMesh
  LinAlg
  Util
  )
