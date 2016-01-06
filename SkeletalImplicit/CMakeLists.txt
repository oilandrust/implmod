PROJECT(SkeletalImplicitRay)

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


#set(Boost_FIND_VERSION 1.33.1 )
#set(Boost_FIND_VERSION_MAJOR 1 )
#set(Boost_FIND_VERSION_MINOR 33 )
#set(Boost_FIND_VERSION_PATCH 1 )

#find_package( Boost REQUIRED filesystem )

# Within the project we add a simple executable program
ADD_EXECUTABLE(SkeletalImplicitRay
	SkeletalImplicitRay.cpp
	Skeleton.cpp
	Skeleton.h
	MetaPrimitive.h
	MetaPrimitive.cpp
	MetaTube.cpp
	MetaTube.h
	RayCastUtils.h
	RayCastUtils.cpp
	DrawUtils.h
	DrawUtils.cpp
	Metaball.cpp
	Metaball.h
	BVH.h
	BVH.cpp
	BvhUtils.h
	BvhUtils.cpp
	Timer.h
	DisplayText.cpp
	DisplayText.h
	SkeletalImplicit.h
	SkeletalImplicit.cpp
	IEditor.h
	IEditor.cpp
	Input.h
	Input.cpp
	Animation.h
	Animation.cpp
	AnimationEditor.h
	AnimationEditor.cpp
	SkeletonEditor.h
	SkeletonEditor.cpp
	SurfaceEditor.h
	SurfaceEditor.cpp
	TextureProjector.h
	TextureProjector.cpp
	MainEditor.h
	MainEditor.cpp
	TextureEditor.h
	TextureEditor.cpp
)

include_directories( 
	${Boost_INCLUDE_DIRS}
)

# We add all the GEL libraries
# The link directory is automagically set by the "GEL_USE_FILE"
# You can pick and choose
TARGET_LINK_LIBRARIES(SkeletalImplicitRay
 # ${Boost_LIBRARIES}
  glew32
  CGLA
  Geometry
  GLGraphics
  HMesh
  LinAlg
  Util
  )
