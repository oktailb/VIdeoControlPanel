cmake_minimum_required(VERSION 3.5)

project(VideoControlPanel VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
if (WIN32)
    set(CMAKE_PREFIX_PATH "C:\\Qt\\Qt-6.4.2\\lib\\cmake")
#set(CMAKE_PREFIX_PATH "C:\\Qt\\Qt5.12.12\\5.12.12\\mingw73_64\\lib\\cmake")
endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools network core gui Multimedia MultimediaWidgets OpenGL  REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools network core gui Multimedia MultimediaWidgets OpenGL  REQUIRED)
find_package(PkgConfig)

if (WIN32)
    set(LIBVLC_SDK_DIR C:\\Program\ Files\\VideoLAN\\VLC\\sdk)
else()
    set(LIBVLC_SDK_DIR /usr)
endif()

set(LIBVLC_LIBRARY_PATH ${LIBVLC_SDK_DIR}"\\..")
set(LIBVLC_INCLUDE_PATH ${LIBVLC_SDK_DIR}"\\include")

set(LIBVLC_LIBRARIES ${LIBVLC_LIBRARY_PATH})
set(LIBVLC_INCLUDE_DIRS ${LIBVLC_INCLUDE_PATH})

set(TS_FILES res/${PROJECT_NAME}_en_US.ts)

file(GLOB QT_RESOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} res/resources.qrc)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt6_add_resources(RESOURCES_RCC ${RESOURCE})
else()
    qt5_add_resources(RESOURCES_RCC ${RESOURCE})
endif()

set(PROJECT_SOURCES
        src/main.cpp
        src/cpanelmainwindow.cpp
        src/cpanelmainwindow.ui
        src/vlcmediaplayer.cpp
        include/cpanelmainwindow.h
        include/vlcmediaplayer.h
        res/resources.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    #qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(${PROJECT_NAME} SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(${PROJECT_NAME}
            ${PROJECT_SOURCES}
        )
    endif()

    #qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

include_directories(include
    ${LIBVLC_INCLUDE_DIRS}
)
link_directories(lib)

target_link_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_LINK_LIBRARY_FLAG}
    lib
    ${LIBVLC_LIBRARY_PATH}
)
target_link_libraries(${PROJECT_NAME}
                        PRIVATE Qt${QT_VERSION_MAJOR}::Core
                                Qt${QT_VERSION_MAJOR}::Widgets
                                Qt${QT_VERSION_MAJOR}::Multimedia
                                Qt${QT_VERSION_MAJOR}::MultimediaWidgets
                                #Qt${QT_VERSION_MAJOR}::OpenGLWidgets
                                libvlc
                     )

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER videoCOntrolPanel.oktail.mouling.corp
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()
