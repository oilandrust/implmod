
link_directories( 
	${3RD_PARTY_LIBRARIES_DIRS}
)

# Within the project we add a simple executable program
ADD_EXECUTABLE(Viewer
	Viewer.cpp
)

include_directories( 
	${3RD_PARTY_INCLUDE_DIRS}
)

target_link_libraries (Viewer
   ${3RD_PARTY_LIBRARIES}
)
target_link_libraries(Viewer
	SklImp
	UI
	Utils
)
