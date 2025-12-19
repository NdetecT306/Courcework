cmake_minimum_required(VERSION 3.10)
project(AuthServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск библиотек
find_package(OpenSSL REQUIRED)
find_package(Boost 1.66 REQUIRED COMPONENTS system thread)
find_package(nlohmann_json 3.0 REQUIRED)

# Поиск libpqxx (PostgreSQL C++ библиотека)
find_library(PQXX_LIBRARY pqxx)
find_path(PQXX_INCLUDE_DIR pqxx)

if(NOT PQXX_LIBRARY OR NOT PQXX_INCLUDE_DIR)
    message(FATAL_ERROR "libpqxx не найден. Установите: sudo apt-get install libpqxx-dev")
endif()

include_directories(
    ${OPENSSL_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${PQXX_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(SERVER_SOURCES
    server.cpp
    bruteforceProtection.cpp
    clientSession.cpp
    userDatabase.cpp
    quizDatabase.cpp
    resourceDatabase.cpp
    SHA.cpp
)

set(SERVER_HEADERS
    bruteforceProtection.h
    clientSession.h
    server.h
    userDatabase.h
    quizDatabase.h
    resourceDatabase.h
    SHA.h
)

foreach(file ${SERVER_SOURCES})
    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${file})
        message(WARNING "Файл ${file} не найден в ${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
endforeach()

# Исполняемый файл сервера
add_executable(auth_server ${SERVER_SOURCES} ${SERVER_HEADERS})

target_compile_options(auth_server PRIVATE
    -Wall
    -Wextra
    -pthread
)

target_link_libraries(auth_server
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${PQXX_LIBRARY}
    nlohmann_json::nlohmann_json
    pthread
)

set_target_properties(auth_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
