# CMakeList.txt : CMake project for RayTracedChess, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.14)

PROJECT(RayTracedChess)

SET(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1 ) 

find_package(OpenGL REQUIRED)

add_compile_definitions(IMGUI_USER_CONFIG="imgui/imconfig.h")

file(GLOB_RECURSE RayTracedChess_SRC
    "src/*.h"
    "src/*.cpp"
    "src/*.c"
)

file(GLOB ImGUI_SRC
    "libs/imgui/*.h"
    "libs/imgui/*.cpp"
    "libs/imgui/examples/imgui_impl_glfw.cpp"
    "libs/imgui/examples/imgui_impl_glfw.h"
    "libs/imgui/examples/imgui_impl_opengl3.cpp"
    "libs/imgui/examples/imgui_impl_opengl3.h"
)

#target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
add_subdirectory(libs/geGL)
add_subdirectory(libs/Vars)
add_subdirectory(libs/glm)
add_subdirectory(libs/glfw)

# Add source to this project's executable.
add_executable (${PROJECT_NAME} ${RayTracedChess_SRC} ${ImGUI_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/glm>)
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui>)


target_link_libraries(${PROJECT_NAME} geGL::geGL Vars::Vars glfw ${OPENGL_gl_LIBRARY})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# link_directories("lib")
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})



function(getSharedLibraries out lib cfg)
  if(TARGET ${lib})
    get_target_property(target_type ${lib} TYPE)
    if(${target_type} STREQUAL "INTERFACE_LIBRARY")
      return()
    endif()
  else()
    return()
  endif()


  get_target_property(dll ${lib} IMPORTED_LOCATION_${cfg})
  list(APPEND dlls ${dll})

  get_target_property(interfaceLibs ${lib} INTERFACE_LINK_LIBRARIES)
  if(NOT "${interfaceLibs}" STREQUAL "interfaceLibs-NOTFOUND")
    foreach(interfaceLib ${interfaceLibs})
      getSharedLibraries(ilibs ${interfaceLib} ${cfg})
      list(APPEND dlls ${ilibs})
    endforeach()
  endif()
  list(REMOVE_DUPLICATES dlls)
  set(${out} ${dlls} PARENT_SCOPE)
endfunction()


function(getAllSharedLibraries allLibraries app cfg)
  get_target_property(libs ${app} LINK_LIBRARIES)
  foreach(lib ${libs})
    getSharedLibraries(libList ${lib} ${cfg})
    list(APPEND allLibs ${libList})
  endforeach()
  list(REMOVE_DUPLICATES allLibs)
  foreach(ll ${allLibs})
    string(REGEX MATCH "\.dll" isDll ${ll})
    if(NOT "${isDll}" STREQUAL "")
      list(APPEND outputAllLibs ${ll})
    endif()
  endforeach()
  set(${allLibraries} ${outputAllLibs} PARENT_SCOPE)
endfunction()

getAllSharedLibraries(allDebugSharedLibraries ${PROJECT_NAME} DEBUG)
getAllSharedLibraries(allReleaseSharedLibraries ${PROJECT_NAME} RELEASE)

message("${allDebugSharedLibraries}")

if(MSVC)
  foreach(lib ${allDebugSharedLibraries})
    file(COPY ${lib} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
  endforeach()
  foreach(lib ${allReleaseSharedLibraries})
    file(COPY ${lib} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
  endforeach()
endif() 

