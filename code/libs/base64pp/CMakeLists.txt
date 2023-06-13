set(THIS_SDK_DIR ${OCT_SDKS_DIR}/base64pp/_src)

oct_define_sources(
	PLATFORM ALL

	"CMakeLists.txt"
	
	GROUP "includes"
		"${THIS_SDK_DIR}/base64pp/include/base64pp/base64pp.h"

	GROUP "sources"
		"${THIS_SDK_DIR}/base64pp/base64pp.cpp"
)

oct_project(base64pp TYPE STATIC FOLDER "libs")

target_include_directories(${THIS_PROJECT}
	PUBLIC ${THIS_SDK_DIR}/base64pp/include)

set_target_properties(${THIS_PROJECT} PROPERTIES
	CXX_STANDARD 20
	CXX_EXTENSIONS OFF
)