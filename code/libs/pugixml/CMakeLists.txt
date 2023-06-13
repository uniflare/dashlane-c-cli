set(THIS_SDK_DIR ${OCT_SDKS_DIR}/pugixml/_src)


# /// SQLiteCpp
oct_define_sources(
	PLATFORM ALL

	"CMakeLists.txt"
	
	GROUP "includes"
		"${THIS_SDK_DIR}/src/pugiconfig.hpp"
		"${THIS_SDK_DIR}/src/pugixml.hpp"

	GROUP "sources"
		"${THIS_SDK_DIR}/src/pugixml.cpp"
)

oct_project(pugixml TYPE STATIC FOLDER "libs")

target_include_directories(${THIS_PROJECT}
	PUBLIC ${THIS_SDK_DIR}/src
)