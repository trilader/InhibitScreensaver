cmake_minimum_required(VERSION 3.28)

project(InhibitScreensaver)

# First, find sdbus-c++
find_package(sdbus-c++ REQUIRED)

# Use the sdbus-c++ target in SDBusCpp namespace
add_executable(inhibit-screensaver main.cpp)
target_link_libraries(inhibit-screensaver PRIVATE SDBusCpp::sdbus-c++)

install(TARGETS inhibit-screensaver)
install(FILES inhibit-screensaver.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
install(FILES inhibit-screensaver.svg DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps)
