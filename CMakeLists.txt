cmake_minimum_required(VERSION 3.16)

project(rpi_dashboard VERSION 0.1 LANGUAGES CXX)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick WebSockets Core DBus)

qt_standard_project_setup(REQUIRES 6.8)


set(SOURCES main.cpp)
qt_add_big_resources(SOURCES assets/Assets.qrc)

list(APPEND SOURCES assets/Assets.qrc)
set_property(SOURCE assets/Assets.qrc PROPERTY SKIP_AUTORCC ON)

add_subdirectory(QtAwesome)

qt_add_executable(apprpi_dashboard
    ${SOURCES}
)


set_source_files_properties(assets/Style.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE)


qt_add_qml_module(apprpi_dashboard
    URI rpi_dashboard
    QML_FILES
    Dashboard.qml
        components/Gauge.qml
        components/InfoTile.qml
        components/WeatherTile.qml
        assets/Style.qml
        components/Uptime.qml
    SOURCES
        backend/backend.h
        backend/backend.cpp
        backend/prometheusclient.h backend/prometheusclient.cpp
        backend/weatherclient.h backend/weatherclient.cpp
        backend/wsupdator.h backend/wsupdator.cpp
        QML_FILES Spotify.qml
        RESOURCES
        QML_FILES Main.qml
        SOURCES backend/spotify.h backend/spotify.cpp
        QML_FILES PcOFF.qml
        QML_FILES components/Utils.js
        QML_FILES Osu.qml
        SOURCES backend/osu.h backend/osu.cpp
        SOURCES

)



# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(apprpi_dashboard PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.apprpi_dashboard
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(apprpi_dashboard
    PRIVATE Qt6::Quick
    PRIVATE Qt6::WebSockets
    PRIVATE Qt6::Core
    PRIVATE Qt6::DBus
    PRIVATE QtAwesome
)

include(GNUInstallDirs)
install(TARGETS apprpi_dashboard
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
