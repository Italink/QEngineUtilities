cmake_minimum_required(VERSION 3.12)

project(QEngineUtilities)

cmake_minimum_required(VERSION 3.12)

include(GenerateExportHeader)

set(GCC_DEBUG_FLAGS "-g -Wall")

if(CMAKE_CONFIGURATION_TYPES)
   list(APPEND CMAKE_CONFIGURATION_TYPES DebugEditor ReleaseEditor)
   list(REMOVE_DUPLICATES CMAKE_CONFIGURATION_TYPES)
   set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
       "Add the configurations that we need"
       FORCE)
endif()

set(CMAKE_MAP_IMPORTED_CONFIG_DEBUGEDITOR Debug Release)
set(CMAKE_CXX_FLAGS_DEBUGEDITOR "${CMAKE_CXX_FLAGS_DEBUG}" CACHE STRING "")
set(CMAKE_C_FLAGS_DEBUGEDITOR "${CMAKE_C_FLAGS_DEBUG}" CACHE STRING "")
set(CMAKE_EXE_LINKER_FLAGS_DEBUGEDITOR "${CMAKE_EXE_LINKER_FLAGS_DEBUG}" CACHE STRING "")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUGEDITOR "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}" CACHE STRING "")

set(CMAKE_CXX_FLAGS_RELEASEEDITOR "${CMAKE_CXX_FLAGS_RELEASE}" CACHE STRING "")
set(CMAKE_C_FLAGS_RELEASEEDITOR  "${CMAKE_C_FLAGS_RELEASE}" CACHE STRING "")
set(CMAKE_EXE_LINKER_FLAGS_RELEASEEDITOR "${CMAKE_EXE_LINKER_FLAGS_RELEASE}" CACHE STRING "")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASEEDITOR "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}" CACHE STRING "")

find_package(Qt6 COMPONENTS Core Widgets Gui Multimedia ShaderTools REQUIRED)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "Generated Files")

add_definitions(-DVK_NO_PROTOTYPES)

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set_property(GLOBAL PROPERTY QENGINE_TARGET_LIST "")
if(NOT QENGINE_SOURCE_GROUP_PREFIX)
    set(QENGINE_SOURCE_GROUP_PREFIX "QEngineUtilities" CACHE INTERNAL "")
endif()

macro(qengine_generate_export_header TARGET_NAME)
    set(EXPORT_HEADER_PATH ${CMAKE_CURRENT_LIST_DIR}/Source/Public/${TARGET_NAME}API.h)
    string(TOUPPER ${TARGET_NAME} API_NAME)
    file(WRITE ${EXPORT_HEADER_PATH} "#ifndef ${API_NAME}_API_H
#define ${API_NAME}_API_H

#include <QtCore/qglobal.h>

#if defined(${API_NAME}_LIBRARY)
#  define ${API_NAME}_API Q_DECL_EXPORT
#else
#  define ${API_NAME}_API Q_DECL_IMPORT
#endif

#endif // ${API_NAME}_API_H")
    set_property(TARGET ${TARGET_NAME} APPEND PROPERTY SOURCES ${EXPORT_HEADER_PATH})  
    target_compile_definitions(${TARGET_NAME} PRIVATE ${API_NAME}_LIBRARY)
endmacro()

macro(qengine_add_target TARGET_NAME)
    set(prefix QENGINE)
    set(options ENABLE_MOC)
    set(one_value_keywords SOURCE_GROUP QRC_FILE PLUGIN_TYPE)
    set(multi_value_keywords PUBLIC_DEPENDENCY PRIVATE_DEPENDENCY PUBLIC_INCLUDE PRIVATE_INCLUDE PUBLIC_DEFINITIONS PRIVATE_DEFINITIONS)
    cmake_parse_arguments(${prefix} "${options}" "${one_value_keywords}" "${multi_value_keywords}" ${ARGN})
    
    list(APPEND QENGINE_PUBLIC_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/Source/Public)
    list(APPEND QENGINE_PRIVATE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/Source/Private)
    
    file(GLOB_RECURSE PUBLIC_FILES LIST_DIRECTORIES TRUE ${CMAKE_CURRENT_SOURCE_DIR}/Source/Public/*)
    foreach(PUBLIC_FILE ${PUBLIC_FILES})
        if(IS_DIRECTORY ${PUBLIC_FILE})
            list(APPEND QENGINE_PRIVATE_INCLUDE ${PUBLIC_FILE})
        endif()
    endforeach()

    file(GLOB_RECURSE PROJECT_SOURCE FILES ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h  ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp)
    if(QENGINE_QRC_FILE)
        qt6_add_resources(QRC_FILE ${QENGINE_QRC_FILE})
    endif()

    add_library(${TARGET_NAME} SHARED ${PROJECT_SOURCE} ${QRC_FILE})
    
    if(QENGINE_ENABLE_MOC)
        set_property(TARGET ${TARGET_NAME} PROPERTY AUTOMOC ON)
    endif()

    if(NOT QENGINE_SOURCE_GROUP)
        if(QENGINE_PLUGIN_TYPE)
            set(QENGINE_SOURCE_GROUP "${QENGINE_SOURCE_GROUP_PREFIX}/Plugins")
        else()
            set(QENGINE_SOURCE_GROUP "${QENGINE_SOURCE_GROUP_PREFIX}")
        endif()
    endif()
    set_property(GLOBAL APPEND PROPERTY QENGINE_TARGET_LIST "${TARGET_NAME}")
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Source FILES ${PROJECT_SOURCE})
    target_include_directories(${TARGET_NAME} PUBLIC  ${QENGINE_PUBLIC_INCLUDE}) 
    target_include_directories(${TARGET_NAME} PRIVATE ${QENGINE_PRIVATE_INCLUDE}) 
    target_link_libraries(${TARGET_NAME} PUBLIC ${QENGINE_PUBLIC_DEPENDENCY})
    target_link_libraries(${TARGET_NAME} PRIVATE ${QENGINE_PRIVATE_DEPENDENCY})
    target_compile_definitions(${TARGET_NAME} PUBLIC ${QENGINE_PUBLIC_DEFINITIONS})
    target_compile_definitions(${TARGET_NAME} PRIVATE ${QENGINE_PRIVATE_DEFINITIONS})
    set_target_properties(${TARGET_NAME} PROPERTIES 
        FOLDER ${QENGINE_SOURCE_GROUP}
    )
    qengine_generate_export_header(${TARGET_NAME})
endmacro()

macro(qengine_copy_dll TARGET_NAME)
    get_property(QENGINE_TARGET_LIST GLOBAL PROPERTY QENGINE_TARGET_LIST)

    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E 
        make_directory ${CMAKE_CURRENT_BINARY_DIR}/Plugins/)

    foreach(QENGINE_TARGET ${QENGINE_TARGET_LIST})
        get_target_property(PLUGIN_TYPE ${QENGINE_TARGET} PLUGIN_TYPE)
        if(PLUGIN_TYPE)
            add_dependencies(${TARGET_NAME} ${QENGINE_TARGET})
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
              COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${QENGINE_TARGET}> ${CMAKE_CURRENT_BINARY_DIR}/Plugins/
              COMMAND_EXPAND_LISTS
            )
        else()
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
              COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${QENGINE_TARGET}> ${CMAKE_CURRENT_BINARY_DIR}
              COMMAND_EXPAND_LISTS
            )
        endif()
    endforeach()
endmacro()

add_subdirectory(3rdParty)
add_subdirectory(Source/Core)
add_subdirectory(Source/Editor)

file(GLOB PLUGIN_DIR_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} Plugins/*)
foreach(PLUGIN_DIR ${PLUGIN_DIR_LIST})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_DIR})
        add_subdirectory(${PLUGIN_DIR})
    endif()
endforeach()

add_subdirectory(Source/Launch)

if(PROJECT_IS_TOP_LEVEL)
    add_subdirectory(Examples)
endif()
