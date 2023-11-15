cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project(VulkanTutorial VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 设置指定的C++编译器版本是必须的，如果不设置，或者为OFF，则指定版本不可用时，会使用上一版本
set(BUILD_SHARED_LIBS OFF) # 不构建共享库

# 收集对应目录的文件到对应变量
file(GLOB HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

# 在IDE里面给编译需要的文件归类
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${HEADERS} ${SOURCES})

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})

# 设置目录变量
set(TUTORIAL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}") # 定义变量
set(LIBRARY_DIR "${TUTORIAL_ROOT_DIR}/library")

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

# 链接依赖库
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)
link_libraries(glfw)
target_link_libraries(${PROJECT_NAME} PUBLIC ${vulkan_lib})

# begin Vulkan
target_include_directories(
  ${PROJECT_NAME}
  PUBLIC $<BUILD_INTERFACE:${vulkan_include}>)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC $<BUILD_INTERFACE:${LIBRARY_DIR}/VulkanSDK/include/vma>
)

# end Vulkan

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