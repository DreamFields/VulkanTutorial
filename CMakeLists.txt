cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project(VulkanTutorial VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 设置指定的C++编译器版本是必须的，如果不设置，或者为OFF，则指定版本不可用时，会使用上一版本
set(BUILD_SHARED_LIBS OFF) # 不构建共享库

# 收集对应目录的文件到对应变量
file(GLOB HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.h)
file(GLOB SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

# 在IDE里面给编译需要的文件归类
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${HEADERS} ${SOURCES})

# 设置目录变量
set(TUTORIAL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}") # 定义变量
set(LIBRARY_DIR "${TUTORIAL_ROOT_DIR}/library")

# begin imgui
set(IMGUI_HEADERS
  ${LIBRARY_DIR}/imgui/imconfig.h
  ${LIBRARY_DIR}/imgui/imgui.h
  ${LIBRARY_DIR}/imgui/imgui_internal.h
  ${LIBRARY_DIR}/imgui/imstb_rectpack.h
  ${LIBRARY_DIR}/imgui/imstb_textedit.h
  ${LIBRARY_DIR}/imgui/imstb_truetype.h
  ${LIBRARY_DIR}/imgui/backends/imgui_impl_glfw.h
  ${LIBRARY_DIR}/imgui/backends/imgui_impl_vulkan.h
)
set(IMGUI_SRC
  ${LIBRARY_DIR}/imgui/imgui.cpp
  ${LIBRARY_DIR}/imgui/imgui_demo.cpp
  ${LIBRARY_DIR}/imgui/imgui_draw.cpp
  ${LIBRARY_DIR}/imgui/imgui_widgets.cpp
  ${LIBRARY_DIR}/imgui/imgui_tables.cpp
  ${LIBRARY_DIR}/imgui/backends/imgui_impl_glfw.cpp
  ${LIBRARY_DIR}/imgui/backends/imgui_impl_vulkan.cpp
)

include_directories(${LIBRARY_DIR}/imgui ${LIBRARY_DIR}/imgui/backends)
# end imgui

# add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})
add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${IMGUI_HEADERS} ${IMGUI_SRC})


# 设置vulkan的include目录
set(vulkan_include ${LIBRARY_DIR}/VulkanSDK/include)

# 对于不同平台拥有不同设置
if(WIN32)
  set(vulkan_lib ${LIBRARY_DIR}/VulkanSDK/lib/Win32/vulkan-1.lib)
  set(glslangValidator_executable ${LIBRARY_DIR}/VulkanSDK/bin/Win32/glslangValidator.exe)
  add_compile_definitions("VK_LAYER_PATH=${LIBRARY_DIR}/VulkanSDK/bin/Win32")
elseif(UNIX)
  if(APPLE)
    set(vulkan_lib ${LIBRARY_DIR}/VulkanSDK/lib/MacOS/libvulkan.1.dylib)
    set(glslangValidator_executable ${LIBRARY_DIR}/VulkanSDK/bin/MacOS/glslangValidator)
    add_compile_definitions("VK_LAYER_PATH=${LIBRARY_DIR}/VulkanSDK/bin/MacOS")
    add_compile_definitions("VK_ICD_FILENAMES=${LIBRARY_DIR}/VulkanSDK/bin/MacOS/MoltenVK_icd.json")
  else()
    set(vulkan_lib ${LIBRARY_DIR}/VulkanSDK/lib/Linux/libvulkan.so.1)
    set(glslangValidator_executable ${LIBRARY_DIR}/VulkanSDK/bin/Linux/glslangValidator)
    add_compile_definitions("VK_LAYER_PATH=${LIBRARY_DIR}/VulkanSDK/bin/Linux")
  endif()
else()
  message(FATAL_ERROR "Unknown Platform")
endif()

# shader编译，不作为库编译，而是通过precompile.cmake手动预编译
set(SHADER_COMPILE_TARGET ShaderCompile)
add_subdirectory(shader)

add_subdirectory(library)

# begin glfw
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)
link_libraries(glfw)

# end glfw

# begin stb
target_include_directories(${PROJECT_NAME} PUBLIC ${LIBRARY_DIR}/stb)

# end stb

# begin Vulkan
target_link_libraries(${PROJECT_NAME} PUBLIC ${vulkan_lib})
target_include_directories(
  ${PROJECT_NAME}
  PUBLIC $<BUILD_INTERFACE:${vulkan_include}>)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC $<BUILD_INTERFACE:${LIBRARY_DIR}/VulkanSDK/include/vma>
)

# end Vulkan

# begin DCMTK
set(DCMTK_DIR ${LIBRARY_DIR}/dcmtk)

# Add include directories
include_directories(${DCMTK_DIR}/include)

# Add link directories
link_directories(${DCMTK_DIR}/lib)

# Add libraries
add_library(ofstd SHARED IMPORTED)
set_target_properties(ofstd PROPERTIES
  IMPORTED_LOCATION ${DCMTK_DIR}/bin/ofstd.dll
  IMPORTED_IMPLIB ${DCMTK_DIR}/lib/ofstd.lib)

add_library(oflog SHARED IMPORTED)
set_target_properties(oflog PROPERTIES
  IMPORTED_LOCATION ${DCMTK_DIR}/bin/oflog.dll
  IMPORTED_IMPLIB ${DCMTK_DIR}/lib/oflog.lib)

add_library(dcmdata SHARED IMPORTED)
set_target_properties(dcmdata PROPERTIES
  IMPORTED_LOCATION ${DCMTK_DIR}/bin/dcmdata.dll
  IMPORTED_IMPLIB ${DCMTK_DIR}/lib/dcmdata.lib)

add_library(zlib SHARED IMPORTED)
set_target_properties(zlib PROPERTIES
  IMPORTED_IMPLIB ${DCMTK_DIR}/lib/zlib.lib)

add_library(dcmimgle SHARED IMPORTED)
set_target_properties(dcmimgle PROPERTIES
  IMPORTED_LOCATION ${DCMTK_DIR}/bin/dcmimgle.dll
  IMPORTED_IMPLIB ${DCMTK_DIR}/lib/dcmimgle.lib)
target_link_libraries(${PROJECT_NAME} PUBLIC ofstd dcmdata dcmimgle)

# 将.dll文件复制到输出目录
configure_file(${DCMTK_DIR}/bin/ofstd.dll ${CMAKE_CURRENT_BINARY_DIR}/ofstd.dll COPYONLY)
configure_file(${DCMTK_DIR}/bin/oflog.dll ${CMAKE_CURRENT_BINARY_DIR}/oflog.dll COPYONLY)
configure_file(${DCMTK_DIR}/bin/dcmdata.dll ${CMAKE_CURRENT_BINARY_DIR}/dcmdata.dll COPYONLY)
configure_file(${DCMTK_DIR}/bin/dcmimgle.dll ${CMAKE_CURRENT_BINARY_DIR}/dcmimgle.dll COPYONLY)

# end DCMTK

# begin glm
target_include_directories(${PROJECT_NAME} PUBLIC ${LIBRARY_DIR}/glm)

# end glm

# 将目录包含到对应目标里，不写这句话可就找不到.h文件了
target_include_directories(
  ${PROJECT_NAME}
  PUBLIC $<BUILD_INTERFACE:${TUTORIAL_ROOT_DIR}/source>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

# shader 显式要求cmake先编译${SHADER_COMPILE_TARGET}
add_dependencies(${PROJECT_NAME} ${SHADER_COMPILE_TARGET})
target_include_directories(
  ${PROJECT_NAME}
  PUBLIC $<BUILD_INTERFACE:${TUTORIAL_ROOT_DIR}/shader/generated/cpp>)

# 设置启动项目
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})