cmake_minimum_required(VERSION 3.5)

project(raycaster LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (UNIX)
	set(CMAKE_CXX_FLAGS "-lX11 -lGL -lpthread -lpng")
endif (UNIX)
add_executable(raycaster main.cpp)

if(WIN32)
	target_link_libraries(raycaster gdi32.lib)
	target_link_libraries(raycaster opengl32.lib)
	target_link_libraries(raycaster gdiplus.lib)
	target_link_libraries(raycaster Shlwapi.lib)
endif (WIN32)
