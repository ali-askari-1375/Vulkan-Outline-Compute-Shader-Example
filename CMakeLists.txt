cmake_minimum_required(VERSION 3.5)

project(VkOutlinePostProcess LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

add_executable(VkOutlinePostProcess WIN32
	Main.cpp
	external/imgui/imgui.cpp
	external/imgui/imgui_draw.cpp
	external/imgui/imgui_demo.cpp
	external/imgui/imgui_widgets.cpp
	external/imgui/imgui_tables.cpp
	external/imgui/backends/imgui_impl_win32.cpp
	external/imgui/backends/imgui_impl_vulkan.cpp
)

target_include_directories(VkOutlinePostProcess PUBLIC "${Vulkan_INCLUDE_DIR}")
target_include_directories(VkOutlinePostProcess PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui")
target_include_directories(VkOutlinePostProcess PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/external/tinygltf")
target_link_libraries(VkOutlinePostProcess PUBLIC "Shcore.lib")

target_compile_definitions(VkOutlinePostProcess PUBLIC APP_SOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(VkOutlinePostProcess PUBLIC VK_NO_PROTOTYPES)
