set(THIS_SDK_DIR ${OCT_SDKS_DIR}/argon2/_src)

oct_define_sources(
	PLATFORM ALL

	"CMakeLists.txt"
	
	GROUP "includes"
		"${THIS_SDK_DIR}/src/argon2.c"
		"${THIS_SDK_DIR}/src/core.c"
		"${THIS_SDK_DIR}/src/blake2/blake2b.c"
		"${THIS_SDK_DIR}/src/thread.c"
		"${THIS_SDK_DIR}/src/encoding.c"
		"${THIS_SDK_DIR}/src/opt.c"
)

oct_project(argon2 LANGUAGE_C TYPE STATIC FOLDER "libs")

target_include_directories(${THIS_PROJECT}
	PUBLIC ${THIS_SDK_DIR}/include)

target_compile_definitions(${THIS_PROJECT}
	PRIVATE
		-Werror=declaration-after-statement
		-D_FORTIFY_SOURCE=2
		-Wextra
		-Wno-type-limits
		-Werror
		-coverage
		-DTEST_LARGE_RAM
)