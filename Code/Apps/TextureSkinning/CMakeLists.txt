
link_directories( 
	${3RD_PARTY_LIBRARIES_DIRS}
)

# Within the project we add a simple executable program
ADD_EXECUTABLE(TextureSkinning
	TextureSkinning.cpp
)

include_directories( 
	${3RD_PARTY_INCLUDE_DIRS}
)

target_link_libraries (TextureSkinning
   ${3RD_PARTY_LIBRARIES}
)
target_link_libraries(TextureSkinning
	SklImp
	UI
	Utils
)
