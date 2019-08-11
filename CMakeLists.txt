cmake_minimum_required(VERSION 2.8)

project(masterarbeit CXX)

set(INC_DIR ${CMAKE_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

set(LIBS_DIR ${CMAKE_SOURCE_DIR}/libs)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

if ("${LEMON_DIR}" STREQUAL "")
else()
	set(LEMON_DIR ${LIBS_DIR}/${LEMON_DIR})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAS_LEMON")
endif()
if ("${BLOSSOM_IV_DIR}" STREQUAL "")
else()
	set(BLOSSOMIV_DIR ${LIBS_DIR}/${BLOSSOM_IV_DIR})
	set(BLOSSOMIV_LIB ${BLOSSOMIV_DIR}/blossom4.a)
	set(CONCORDE_DIR ${LIBS_DIR}/concorde-97)
	set(CONCORDE_LIB ${CONCORDE_DIR}/concorde.a)
	if(NOT EXISTS "${CONCORDE_LIB}") # Concorde is stable enough to only build once
		execute_process(COMMAND make all WORKING_DIRECTORY "${CONCORDE_DIR}")
	endif()
	set(BLOSSOM_IV_FLAGS "-O3")
	execute_process(COMMAND make CFLAGS=${BLOSSOM_IV_FLAGS} all WORKING_DIRECTORY "${BLOSSOMIV_DIR}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAS_BLOSSOM_IV")
endif()
if ("${BLOSSOM_V_DIR}" STREQUAL "")
else()
	set(BLOSSOMV_DIR ${LIBS_DIR}/${BLOSSOM_V_DIR})
	set(BLOSSOMV_LIB ${BLOSSOMV_DIR}/blossom5.a)
	set(BLOSSOM_V_FLAGS "-O3 -D_NDEBUG -DPM_TIMER_NONE")
	execute_process(COMMAND make CXXFLAGS=${BLOSSOM_V_FLAGS} all WORKING_DIRECTORY "${BLOSSOMV_DIR}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAS_BLOSSOM_V")
endif()
if ("${FADE_DIR}" STREQUAL "")
else()
	set(FADE_REL_DIR libs/${FADE_DIR})
	set(FADE_DIR ${CMAKE_SOURCE_DIR}/${FADE_REL_DIR})
	set(FADE_INC ${FADE_DIR}/include_fade2d)
	# FADE_LIB_NAME has to be provided externally
	set(FADE_LIB ${FADE_DIR}/${FADE_LIB_DIR}/lib${FADE_LIB_NAME}.so)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAS_FADE -l${FADE_LIB_NAME} -L${FADE_REL_DIR}/${FADE_LIB_DIR}")
endif()

find_package(Boost REQUIRED COMPONENTS
	system filesystem iostreams)

include_directories("${INC_DIR}") 

FILE(GLOB masterarbeit_files "${SRC_DIR}/*.cpp")
set_source_files_properties(${concorde} PROPERTIES LANGUAGE CXX)

add_executable(masterarbeit
	${masterarbeit_files})

target_link_libraries(masterarbeit
	Boost::system
	Boost::filesystem
	Boost::iostreams
	"${BLOSSOMIV_LIB}"
	"${BLOSSOMV_LIB}"
	"${CONCORDE_LIB}"
	"${FADE_LIB}")
include_directories(PUBLIC
	${Boost_INCLUDE_DIRS}
	${LEMON_DIR}
	${MV_DIR}
	${BLOSSOMIV_DIR}
	${CONCORDE_DIR}
	${BLOSSOMV_DIR}
	${FADE_INC})
