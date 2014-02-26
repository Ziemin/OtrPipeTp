ENABLE_TESTING()

find_package(Qt5Core REQUIRED)
find_package(LibOTR REQUIRED)
find_package(Boost COMPONENTS thread system date_time unit_test_framework REQUIRED)
find_package(Threads REQUIRED)

add_definitions(${Qt5Core_DEFINITIONS})
ADD_DEFINITIONS(-DBOOST_ALL_DYN_LINK)
ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK)

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${LIBOTR_INCLUDE_DIR})

message("-- Adding test files:")
file(GLOB TEST_FILES "*.cpp")
set(TEST_NAMES "")
foreach(file ${TEST_FILES})

    get_filename_component(base_name ${file} NAME_WE)
    get_filename_component(full_name ${file} NAME)

    add_executable(${base_name} ${full_name})

    target_link_libraries (${base_name} ${Boost_LIBRARIES})
    target_link_libraries (${base_name} ${Boost_SYSTEM_LIBRARY})
    target_link_libraries (${base_name} ${CMAKE_THREAD_LIBS_INIT})

    qt5_use_modules(${base_name} Core )
    target_link_libraries (${base_name} OtrPipe)
    target_link_libraries(${base_name} ${LIBOTR_LIBRARY})

    add_test(NAME ${base_name} COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${base_name} ${TEST_PROPS})
    set_tests_properties(${base_name} PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data)

    LIST(APPEND TEST_NAMES ${base_name})
endforeach()

message("--   Tests found:")
foreach(test ${TEST_NAMES})
    message("--    " ${test})
endforeach()

ADD_CUSTOM_TARGET(check COMMAND ${CMAKE_CTEST_COMMAND} "-V" DEPENDS ${TEST_NAMES})
