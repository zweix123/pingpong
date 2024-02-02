if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)

if("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
    add_compile_options(-std=c++11 -DLINUX -Wall -g -fPIC -Wl,-rpath,${CMAKE_CURRENT_SOURCE_DIR}:${CMAKE_CURRENT_SOURCE_DIR}/lib)
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
    add_compile_options(-std=c++11 -DLINUX -Wall -O3 -fPIC -Wl,-rpath,${CMAKE_CURRENT_SOURCE_DIR}:${CMAKE_CURRENT_SOURCE_DIR}/lib)
else()
    message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}")
endif()

# include_directories(
# /home/weixin/Work/api-cplusplus/include
# )
# link_directories(
# /home/weixin/Work/api-cplusplus/build/
# )
file(GLOB files *.cpp)

foreach(file ${files})
    string(REGEX MATCH "src/.*" relative_path ${file})
    string(REGEX REPLACE "src/" "" target_name ${relative_path})
    string(REGEX REPLACE ".cpp" "" target_name ${target_name})

    add_executable(${target_name} ${file})
    target_link_libraries(${target_name} ${LINK_LIBS})
endforeach()
