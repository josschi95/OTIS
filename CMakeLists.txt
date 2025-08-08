cmake_minimum_required(VERSION 3.16)

project(OTIS VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Sql)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        logs_page.h
        logs_page.cpp
        rules_page.h
        rules_page.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(OTIS
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        syslog_receiver.h
        syslog_receiver.cpp

        log_parser.h
        log_parser.cpp

        database_manager.h
        database_manager.cpp

        settings_manager.h
        settings_manager.cpp

        tablefiltersdialog.h
        tablefiltersdialog.cpp
        tablefiltersdialog.ui

        logs_page.h
        logs_page.cpp

        rules.h
        rule_manager.h
        rule_manager.cpp

        new_rule_dialog.h
        new_rule_dialog.cpp
        new_rule_dialog.ui

        rules_page.h
        rules_page.cpp
        enums.h
        logs.h

        testwindow.h
        testwindow.cpp
        testwindow.ui

        log_format.json
        test_rules.json
        test_logs.txt
        TODO.txt
    )

# Define target properties for Android with Qt 6 as:
#    set_property(TARGET OTIS APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(OTIS SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(OTIS
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# Copy the JSON config file next to the binary
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/log_format.json ${CMAKE_CURRENT_BINARY_DIR}/log_format.json COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_logs.txt ${CMAKE_CURRENT_BINARY_DIR}/test_logs.txt COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_rules.json ${CMAKE_CURRENT_BINARY_DIR}/test_rules.json COPYONLY)

target_link_libraries(OTIS PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt6::Network Qt6::Sql)

# Should fix issue with promoted widgets
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.OTIS)
endif()
set_target_properties(OTIS PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS OTIS
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(OTIS)
endif()
