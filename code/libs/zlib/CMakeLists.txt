set(THIS_SDK_DIR ${OCT_SDKS_DIR}/zlib/_src)

include(CheckTypeSize)
check_type_size(off64_t OFF64_T)

# /// SQLiteCpp
oct_define_sources(
	PLATFORM ALL

	"CMakeLists.txt"
	
	GROUP "includes"
		"${THIS_SDK_DIR}/zlib.h"
		"${THIS_SDK_DIR}/zconf.h"

	GROUP "sources"
		"${THIS_SDK_DIR}/crc32.h"
		"${THIS_SDK_DIR}/deflate.h"
		"${THIS_SDK_DIR}/gzguts.h"
		"${THIS_SDK_DIR}/inffast.h"
		"${THIS_SDK_DIR}/inffixed.h"
		"${THIS_SDK_DIR}/inflate.h"
		"${THIS_SDK_DIR}/inftrees.h"
		"${THIS_SDK_DIR}/trees.h"
		"${THIS_SDK_DIR}/zutil.h"
		"${THIS_SDK_DIR}/adler32.c"
		"${THIS_SDK_DIR}/compress.c"
		"${THIS_SDK_DIR}/crc32.c"
		"${THIS_SDK_DIR}/deflate.c"
		"${THIS_SDK_DIR}/gzclose.c"
		"${THIS_SDK_DIR}/gzlib.c"
		"${THIS_SDK_DIR}/gzread.c"
		"${THIS_SDK_DIR}/gzwrite.c"
		"${THIS_SDK_DIR}/inflate.c"
		"${THIS_SDK_DIR}/infback.c"
		"${THIS_SDK_DIR}/inftrees.c"
		"${THIS_SDK_DIR}/inffast.c"
		"${THIS_SDK_DIR}/trees.c"
		"${THIS_SDK_DIR}/uncompr.c"
		"${THIS_SDK_DIR}/zutil.c"
)

set(ZLIB_PUBLIC_HEADERS
    ${THIS_SDK_DIR}/zconf.h
    ${THIS_SDK_DIR}/zlib.h
)

oct_project(zlib LANGUAGE_C TYPE STATIC FOLDER "libs")

if(HAVE_OFF64_T)
   target_compile_definitions(${THIS_PROJECT}
	PUBLIC
		_LARGEFILE64_SOURCE=1
   )
endif()

target_include_directories(${THIS_PROJECT}
	PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include
	PRIVATE ${THIS_SDK_DIR})

target_compile_definitions(${THIS_PROJECT}
	PUBLIC
		_CRT_SECURE_NO_DEPRECATE
		_CRT_NONSTDC_NO_DEPRECATE
		ZLIB_CONST
)

# Copy to SDK directory to override defaults when building
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/zconf.h)
	file(COPY ${CMAKE_CURRENT_LIST_DIR}/zconf.h DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/zconf.h")
endif()

file(COPY ${ZLIB_PUBLIC_HEADERS} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/include/zlib")

message(STATUS "${ZLIB_PUBLIC_HEADERS} -> ${CMAKE_CURRENT_LIST_DIR}/include/zlib")