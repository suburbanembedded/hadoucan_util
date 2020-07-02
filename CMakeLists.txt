cmake_minimum_required(VERSION 3.10)

add_library(hadoucan_util

	src/lawicel_linux_conversions.cpp
    src/Hadoucan_IO_mgr.cpp
    src/Socketcan_IO_mgr.cpp
    src/lin_can_helper.cpp
    src/error_handling.cpp
    src/Gen_can.cpp
)


target_include_directories(hadoucan_util PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(hadoucan_util
    PUBLIC
        common_util
)

add_definitions(-DBOOST_ALL_DYN_LINK)
    find_package(
    Boost REQUIRED
    COMPONENTS
    chrono
    date_time
    thread
)
add_executable(hadoucan_util_test_script
    Examples/hadoucan_util_test_script.cpp
)


target_link_libraries(hadoucan_util_test_script
    PUBLIC
        common_util
        hadoucan_util
        ${Boost_LIBRARIES}
)

if(DEFINED Doxygen::doxygen)
	doxygen_add_docs(hadoucan_util_docs
		include/
		src/
	)
endif()

