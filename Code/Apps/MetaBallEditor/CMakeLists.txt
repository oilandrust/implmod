
link_directories( 
	${3RD_PARTY_LIBRARIES_DIRS}
)

# Within the project we add a simple executable program
ADD_EXECUTABLE(MetaBallEditor
	MetaBallEditor.cpp
)

include_directories( 
	${3RD_PARTY_INCLUDE_DIRS}
)

target_link_libraries (MetaBallEditor
   ${3RD_PARTY_LIBRARIES}
)
target_link_libraries(MetaBallEditor
	SklImp
	UI
	Utils
)
