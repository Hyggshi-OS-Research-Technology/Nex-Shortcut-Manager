cmake_minimum_required(VERSION 3.16)
project(ShortcutManager VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/desktopentry.cpp
    src/desktopscanner.cpp
    src/shortcutmanager.cpp
    src/editdialog.cpp
    src/customshortcutdialog.cpp
    src/iconpickerdialog.cpp
    src/importexportmanager.cpp
    src/applistmodel.cpp
    src/aboutdialog.cpp
)

set(HEADERS
    src/mainwindow.h
    src/desktopentry.h
    src/desktopscanner.h
    src/shortcutmanager.h
    src/editdialog.h
    src/customshortcutdialog.h
    src/iconpickerdialog.h
    src/importexportmanager.h
    src/applistmodel.h
    src/aboutdialog.h
)

qt_add_resources(QRC_FILES resources/resources.qrc)

add_executable(ShortcutManager
    ${SOURCES}
    ${HEADERS}
    ${QRC_FILES}
)

target_link_libraries(ShortcutManager PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
)

target_include_directories(ShortcutManager PRIVATE src)

include(GNUInstallDirs)

install(TARGETS ShortcutManager
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES resources/shortcut-manager.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    OPTIONAL
)

# CPack DEB Packaging Configuration
set(CPACK_PACKAGE_NAME "shortcut-manager")
set(CPACK_PACKAGE_VENDOR "Nex")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Nex Shortcut Manager (NSM) - Linux Desktop Shortcut Manager")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "hyggshi")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)

