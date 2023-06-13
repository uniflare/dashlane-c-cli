cmake_minimum_required( VERSION 3.26.0 )

include(${OCT_CMAKE_DIR}/common-macros.cmake)

oct_define_sources(
	PLATFORM ALL
	
	"${OCT_BASE_DIR}/CMakeLists.txt"
	"${OCT_BASE_DIR}/project.cmake"

	GROUP "tools"
	"CMakeLists.txt"
	"common-macros.cmake"

	GROUP "tools/toolchains"
	"toolchains/msvc-win_x64.cmake"
)

oct_project(CMAKE TYPE INTERFACE FOLDER "CMAKE")