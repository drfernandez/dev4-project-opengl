cmake_minimum_required(VERSION 3.10)

project(Project_OpenGL)

# currently using unicode in some libraries on win32 but will change soon
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

if (UNIX AND NOT APPLE)
     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lX11 -lGL")
     find_package(X11)
     find_package(OpenGL)
     include_directories(${X11_INCLUDE_DIR})
     link_libraries(${X11_LIBRARIES})
     include_directories(${OPENGL_INCLUDE_DIRS})
     link_libraries(${OPENGL_LIBRARIES})
endif()

if(WIN32)

	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
		PROPERTY VS_STARTUP_PROJECT Project_OpenGL
	)
	set(VERTEX_SHADERS
		shaders/vs.glsl
	)
	set(FRAGMENT_SHADERS
		shaders/fs.glsl
	)
	set(IMGUI
		imgui/imconfig.h
		imgui/imgui_demo.cpp
		imgui/imgui_draw.cpp
		imgui/imgui_internal.h
		imgui/imgui_tables.cpp
		imgui/imgui_widgets.cpp
		imgui/imgui.h
		imgui/imgui.cpp
		imgui/imgui_impl_opengl3.h
		imgui/imgui_impl_opengl3.cpp
		imgui/imgui_impl_win32.h
		imgui/imgui_impl_win32.cpp
	)
	add_executable (Project_OpenGL 
		source/main.cpp 
		source/renderer.h
		source/gateware.h
		source/model.h
		source/model.cpp
		source/level.h
		source/level.cpp
		source/h2bparser.h
		${VERTEX_SHADERS}
		${FRAGMENT_SHADERS}
		${IMGUI}
	)
	#set_property(GLOBAL PROPERTY USE_FOLDERS ON)
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VERTEX_SHADERS})
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${FRAGMENT_SHADERS})
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${IMGUI})
endif(WIN32)

