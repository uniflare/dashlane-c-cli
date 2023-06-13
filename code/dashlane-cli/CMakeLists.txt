cmake_minimum_required( VERSION 3.26.0 )

if(EXISTS "${OCT_BASE_DIR}/LICENSE")
    set(CONTENTS "")
	file(READ "${OCT_BASE_DIR}/LICENSE" CONTENTS)
	string(REGEX REPLACE "([\\\$\"])" "\\\\\\1" CONTENTS "${CONTENTS}")
	string(REGEX REPLACE "[\r\n]+" "\\\\n\"\n\"" CONTENTS "${CONTENTS}")
    set(CONTENTS "#pragma once\ninline const std::string g_licenseText = \"${CONTENTS}\"\;")
	file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/generated/License-Gen.h" ${CONTENTS})
	set(GENERATED_LICENSE_HEADER "${CMAKE_CURRENT_BINARY_DIR}/generated/License-Gen.h")
else()
endif()

oct_define_sources(
	PLATFORM ALL

	"CMakeLists.txt"
	
	GROUP "Header Files"
		"Application.h"
		"CommandRegistry.h"
		"CoreConfig.h"
	
	GROUP "Source Files"
		"Application.cpp"
		"CommandRegistry.cpp"
		"main.cpp"
	
	GROUP "Commands"
		"Commands/Command.h"
		"Commands/Common.h"
		"Commands/Configure.cpp"
		"Commands/License.cpp"
		"Commands/Password.cpp"
		"Commands/Reset.cpp"
		"Commands/ResetAll.cpp"
		"Commands/Sync.cpp"
	
	GROUP "Utility"
		"Utility/DashlaneContext.h"
		"Utility/UserInput.h"
	
	GROUP "Generated"
		${GENERATED_LICENSE_HEADER}
)

oct_project(dashlane-cli TYPE EXECUTABLE FOLDER "Dashlane")

target_include_directories(${THIS_PROJECT}
	PUBLIC ${CMAKE_CURRENT_LIST_DIR}
	PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated
    PRIVATE ${OCT_SDKS_DIR}/CLI11/_src/include
    PRIVATE ${OCT_SDKS_DIR}/strutil/_src
    PRIVATE ${OCT_SDKS_DIR}/json/_src/include
)

target_link_libraries(${THIS_PROJECT}
	dashlane-lib
	clip
)

set_directory_properties(PROPERTIES CMAKE_CONFIGURE_DEPENDS "${OCT_BASE_DIR}/_private/AccessKeys.h")
if(EXISTS "${OCT_BASE_DIR}/_private/AccessKeys.h")
	target_compile_definitions(${THIS_PROJECT} PRIVATE DCCLI_ACCESS_KEYS)
target_include_directories(${THIS_PROJECT}
	PUBLIC "${OCT_BASE_DIR}/_private"
)
endif()

set_property(DIRECTORY ${OCT_BASE_DIR} PROPERTY VS_STARTUP_PROJECT ${THIS_PROJECT})