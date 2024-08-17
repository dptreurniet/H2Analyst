cmake_minimum_required(VERSION 3.5)

project(h2analyst VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets PrintSupport Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets PrintSupport Concurrent)

find_package(Boost 1.77.0 COMPONENTS algorithm)
include_directories(${Boost_INCLUDE_DIRS})


set(PROJECT_SOURCES
        main.cpp
        h2analyst.cpp
        h2analyst.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(h2analyst
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        data_parser.h data_parser.cpp
        datafile.h
        datafile.cpp
        data_store.h data_store.cpp
        tree_item.h tree_item.cpp
        namespace.h
        qcustomplot.h qcustomplot.cpp
        plot_base.h plot_base.cpp
        plot_manager.h plot_manager.cpp
        layout_selector.h layout_selector.cpp
        resources.qrc
        plot_time.h plot_time.cpp
        dataset.h dataset.cpp
        plot_tracer.h plot_tracer.cpp
        dataset_operators.h dataset_operators.cpp
        post_pro_dialog.h post_pro_dialog.cpp
        data_explorer.h data_explorer.cpp
        post_pro_expression_edit.h post_pro_expression_edit.cpp

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET h2analyst APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(h2analyst SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(h2analyst
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(h2analyst PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(h2analyst PRIVATE Qt${QT_VERSION_MAJOR}::PrintSupport)
target_link_libraries(h2analyst PRIVATE Qt${QT_VERSION_MAJOR}::Concurrent)
target_link_libraries(h2analyst ${Boost_LIBRARIES})

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.h2analyst)
endif()
set_target_properties(h2analyst PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS h2analyst
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(h2analyst)
endif()
