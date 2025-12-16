cmake_minimum_required(VERSION 3.10)
project(AuthClient)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(PkgConfig REQUIRED)
# Ищем GTK4
pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)
pkg_check_modules(GLIBMM REQUIRED glibmm-2.68)
pkg_check_modules(CAIROMM REQUIRED cairomm-1.16)
# Ищем Boost
find_package(Boost 1.66 REQUIRED COMPONENTS system)
# Ищем nlohmann/json
find_package(nlohmann_json 3.0 REQUIRED)
include_directories(
    ${GTKMM_INCLUDE_DIRS}
    ${GLIBMM_INCLUDE_DIRS}
    ${CAIROMM_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)
link_directories(
    ${GTKMM_LIBRARY_DIRS}
    ${GLIBMM_LIBRARY_DIRS}
    ${CAIROMM_LIBRARY_DIRS}
)
set(CLIENT_SOURCES
    client_main.cpp
    NetworkClient.cpp
    SHA.cpp
    MainWindow.cpp
    MainMenuScreen.cpp
    LoginFormScreen.cpp
    RegisterFormScreen.cpp
    MainScreen.cpp
    TestScreen.cpp
    TestingScreen.cpp
    CustomTestScreen.cpp
    ResourcesScreen.cpp
    ProfileSettingsWidget.cpp
)
set(CLIENT_HEADERS
    NetworkClient.h
    SHA.h
    MainWindow.h
    MainMenuScreen.h
    LoginFormScreen.h
    RegisterFormScreen.h
    MainScreen.h
    TestScreen.h
    TestingScreen.h
    CustomTestScreen.h
    ResourcesScreen.h
    ProfileSettingsWidget.h
)
# Исполняемый файл клиента
add_executable(auth_client ${CLIENT_SOURCES} ${CLIENT_HEADERS})
target_compile_options(auth_client PRIVATE
    ${GTKMM_CFLAGS_OTHER}
    ${GLIBMM_CFLAGS_OTHER}
)
target_link_libraries(auth_client
    ${GTKMM_LIBRARIES}
    ${GLIBMM_LIBRARIES}
    ${CAIROMM_LIBRARIES}
    ${Boost_LIBRARIES}
    nlohmann_json::nlohmann_json
    pthread
)
# Установка
install(TARGETS auth_client DESTINATION bin)
