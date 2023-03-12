cmake_minimum_required(VERSION 3.12)

project(QEngineUtilities)

cmake_minimum_required(VERSION 3.12)

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

add_subdirectory(Core)
add_subdirectory(Editor)

file(GLOB_RECURSE PROJECT_SOURCE FILES Src/*.h Src/*.cpp)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Src FILES ${PROJECT_SOURCE})

add_library(QEngineUtilities STATIC
    ${PROJECT_SOURCE}
)

target_link_libraries(QEngineUtilities PUBLIC 
    QEngineCore
)

target_link_libraries(QEngineUtilities PRIVATE 
    $<$<OR:$<CONFIG:DebugEditor>,$<CONFIG:ReleaseEditor>>:QEngineEditor>
)

target_compile_definitions(QEngineUtilities PUBLIC "$<$<OR:$<CONFIG:DebugEditor>,$<CONFIG:ReleaseEditor>>:QENGINE_WITH_EDITOR>")
target_include_directories(QEngineUtilities PUBLIC Src/Public) 
target_include_directories(QEngineUtilities PRIVATE Src/Private) 

set_target_properties(QEngineUtilities PROPERTIES FOLDER "QEngineUtilities")

if(PROJECT_IS_TOP_LEVEL)
    add_subdirectory(Examples)
endif()

