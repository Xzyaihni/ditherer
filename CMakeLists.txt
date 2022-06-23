cmake_minimum_required(VERSION 3.13)
project(dither)
set(CMAKE_CXX_STANDARD 20)

option(Y_DEBUG "build in debug mode" "OFF")
option(Y_SANITIZE "build with address sanitizer" "OFF")

set(YANDERELIBS "yanderegllib/yanconv.cpp")

set(SOURCE_FILES main.cpp
dither.cpp
${YANDERELIBS})

if(${Y_DEBUG})
	set(CMAKE_BUILD_TYPE Debug)
else()
	set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/yanderegllib")



if(${Y_DEBUG})
	add_definitions(-DDEBUG)
	target_link_libraries(${PROJECT_NAME} -O1 -pg -Wall -Werror -pedantic-errors)
	
	if(${Y_SANITIZE})
		target_link_libraries(${PROJECT_NAME} -fsanitize=address)
	endif()
else()
	target_link_libraries(${PROJECT_NAME} -O3)
endif()