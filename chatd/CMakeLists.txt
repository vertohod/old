cmake_minimum_required(VERSION 2.8)

project(chatd)

find_package(Git REQUIRED)

execute_process(
	COMMAND ${GIT_EXECUTABLE} log HEAD~1..HEAD --pretty=oneline 
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	OUTPUT_VARIABLE GIT_HASH_TEMP
)

string(REGEX MATCH [0-9a-f]+ GIT_HASH "${GIT_HASH_TEMP}")

add_definitions(-DDAEMON=\"chatd\" -DGIT_HASH=\"${GIT_HASH}\")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

include_directories(../include)

set(SOURCE_EXE 
	../include/serialization.cpp
	../include/config_base.cpp
	../include/http_server.cpp
	../include/http_proto.cpp
	../include/stat_base.cpp
	../include/get_html.cpp
	../include/response.cpp
	../include/request.cpp
	../include/crypto.cpp
	../include/format.cpp
	../include/stream.cpp
	../include/config.cpp
	../include/socket.cpp
	../include/types.cpp
	../include/admin.cpp
	../include/util.cpp
	../include/log.cpp
	service_messages.cpp
	object.cpp
#	stat_daemon.cpp
#	admin_chatd.cpp
#	audio_tags.cpp
#	sessions.cpp
	blizzard.cpp
	message.cpp
#	service.cpp
#	archive.cpp
#	answer.cpp
#	dumper.cpp
#	writer.cpp
	domain.cpp
#	invite.cpp
#	avatar.cpp
	table.cpp
	smile.cpp
#	front.cpp
	error.cpp
	image.cpp
	audio.cpp
#	video.cpp
#	room.cpp
)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)

find_package(Boost COMPONENTS system regex thread date_time filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(CMAKE_SKIP_INSTALL_RULES TRUE)

# Crypto++ =======================================
if (NOT EXISTS "${CMAKE_SOURCE_DIR}/cryptopp")
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" clone https://github.com/weidai11/cryptopp.git
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
endif()

add_subdirectory(cryptopp)

include_directories(${CMAKE_SOURCE_DIR}/cryptopp)
#=================================================


# Taglib =======================================
if (NOT EXISTS "${CMAKE_SOURCE_DIR}/taglib")
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" clone https://github.com/taglib/taglib.git
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
endif()

add_subdirectory(taglib)

include_directories(${CMAKE_SOURCE_DIR}/taglib)
include_directories(${CMAKE_SOURCE_DIR}/taglib/taglib)
include_directories(${CMAKE_SOURCE_DIR}/taglib/taglib/toolkit)
#=================================================

link_directories(${Boost_LIBRARY_DIRS} /usr/local/lib)

#message(STATUS "*****")

# Google test =======================================
if (NOT EXISTS "${CMAKE_SOURCE_DIR}/googletest")
	execute_process(
		COMMAND "${GIT_EXECUTABLE}" clone https://github.com/google/googletest.git
		WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	)
endif()

add_subdirectory(googletest)

include_directories(${CMAKE_SOURCE_DIR}/googletest/googletest/include)
#=================================================

add_executable(${PROJECT_NAME} ${SOURCE_EXE} chatd.cpp)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} cryptopp-static tag z)

add_executable(tests ${SOURCE_EXE} unit_tests.cpp)
target_link_libraries(tests ${Boost_LIBRARIES} cryptopp-static tag z gtest)
