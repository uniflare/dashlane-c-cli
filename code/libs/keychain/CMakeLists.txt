set(THIS_SDK_DIR ${OCT_SDKS_DIR}/keychain/_src)

oct_define_sources(
	PLATFORM ALL

	"CMakeLists.txt"
	
	GROUP "includes"
		"${THIS_SDK_DIR}/include/keychain/keychain.h"

	PLATFORM WINDOWS
	GROUP "sources"
		"${THIS_SDK_DIR}/src/keychain_win.cpp"

	PLATFORM APPLE
	GROUP "sources"
		"${THIS_SDK_DIR}/src/keychain_mac.cpp"

	PLATFORM LINUX
	GROUP "sources"
		"${THIS_SDK_DIR}/src/keychain_linux.cpp"
)

oct_project(keychain TYPE STATIC FOLDER "libs")

target_include_directories(${THIS_PROJECT}
	PUBLIC ${THIS_SDK_DIR}/include
	PRIVATE ${THIS_SDK_DIR}/include/keychain)

if ( WINDOWS )
    target_compile_definitions(${THIS_PROJECT} PUBLIC -DKEYCHAIN_WINDOWS=1)
    target_link_libraries(${THIS_PROJECT} PRIVATE crypt32)

elseif ( APPLE )
    target_compile_definitions(${THIS_PROJECT} PUBLIC -DKEYCHAIN_MACOS=1)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation REQUIRED)
    find_library(SECURITY_LIBRARY Security REQUIRED)
    target_link_libraries(${THIS_PROJECT} 
		PRIVATE ${COREFOUNDATION_LIBRARY}
		PRIVATE ${SECURITY_LIBRARY})

elseif ( LINUX )
    target_compile_definitions(${THIS_PROJECT} PUBLIC -DKEYCHAIN_LINUX=1)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLIB2 IMPORTED_TARGET glib-2.0)
    pkg_check_modules(LIBSECRET IMPORTED_TARGET libsecret-1)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE PkgConfig::GLIB2
        PRIVATE PkgConfig::LIBSECRET)
else ()
	message(FATAL_ERROR "Unsupported platform ${DDCLI_TARGET_PLATFORM}.")
endif ()