cmake_minimum_required(VERSION 3.10)
project(AuthServer)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
find_package(Boost 1.66 REQUIRED COMPONENTS system)
find_package(nlohmann_json 3.0 REQUIRED) 
find_package(pqxx QUIET)
if(NOT pqxx_FOUND)
    find_path(PQXX_INCLUDE_DIR
        NAMES pqxx/pqxx
        PATHS /usr/include /usr/local/include /opt/local/include
    )
    find_library(PQXX_LIBRARY
        NAMES pqxx
        PATHS /usr/lib /usr/local/lib /opt/local/lib
    )
    if(PQXX_INCLUDE_DIR AND PQXX_LIBRARY)
        message(STATUS "Found libpqxx manually")
        set(pqxx_FOUND TRUE)
        add_library(pqxx INTERFACE)
        target_include_directories(pqxx INTERFACE ${PQXX_INCLUDE_DIR})
        target_link_libraries(pqxx INTERFACE ${PQXX_LIBRARY})
    else()
        message(FATAL_ERROR "libpqxx not found. Install with: sudo apt-get install libpqxx-dev")
    endif()
endif()
add_executable(auth_server
    server.cpp
    SHA.cpp           
)
target_link_libraries(auth_server
    Boost::system
    pqxx
    nlohmann_json::nlohmann_json
    pthread
)
target_include_directories(auth_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}  
)
install(TARGETS auth_server DESTINATION bin)
