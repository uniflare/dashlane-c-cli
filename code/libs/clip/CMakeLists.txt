set(THIS_SDK_DIR ${OCT_SDKS_DIR}/clip/_src)

if ( APPLE )
	find_library(COCOA_LIBRARY Cocoa)
	if(COCOA_LIBRARY)
		set(CLIP_MAC_SRC clip_osx.mm)
	else()
		set(CLIP_MAC_SRC clip_none.cpp)
	endif()
elseif ( UNIX )
	include(CheckIncludeFiles)
	check_include_files(xcb/xcb.h HAVE_XCB_XLIB_H)

	if(HAVE_XCB_XLIB_H)
		set(CLIP_NIX_SRC clip_x11.cpp)
	else()
		set(CLIP_NIX_SRC clip_none.cpp)
	endif()
endif()

oct_define_sources(
	PLATFORM ALL

	"CMakeLists.txt"
	
	GROUP "includes"
		"${THIS_SDK_DIR}/clip.cpp"
		"${THIS_SDK_DIR}/image.cpp"

	PLATFORM WINDOWS
	GROUP "sources"
		"${THIS_SDK_DIR}/clip_win.cpp"

	PLATFORM APPLE
	GROUP "sources"
		"${THIS_SDK_DIR}/${CLIP_MAC_SRC}"

	PLATFORM LINUX
	GROUP "sources"
		"${THIS_SDK_DIR}/${CLIP_NIX_SRC}"
)

oct_project(clip TYPE STATIC FOLDER "libs")

if(MSVC)
	target_compile_definitions(${THIS_PROJECT} PRIVATE -D_SCL_SECURE_NO_WARNINGS)
endif()

target_include_directories(${THIS_PROJECT}
	PUBLIC ${THIS_SDK_DIR}
)

if ( WINDOWS )
	target_link_libraries(${THIS_PROJECT} PRIVATE shlwapi)
	if(MINGW)
		find_library(CLIP_WINDOWSCODECS_LIBRARY windowscodecs)
		
		if(CLIP_WINDOWSCODECS_LIBRARY)
			target_link_libraries(${THIS_PROJECT} PRIVATE ${CLIP_WINDOWSCODECS_LIBRARY})
		endif()
	endif()
elseif ( APPLE )
    target_compile_definitions(${THIS_PROJECT} PUBLIC -fobjc-arc)
	
	if(COCOA_LIBRARY)
		target_link_libraries(${THIS_PROJECT} PRIVATE ${COCOA_LIBRARY})
	endif()
elseif ( UNIX )
	if(HAVE_XCB_XLIB_H)
		target_compile_definitions(${THIS_PROJECT} PRIVATE -DHAVE_XCB_XLIB_H)
		target_link_libraries(${THIS_PROJECT} PRIVATE xcb pthread)

		if(CLIP_X11_WITH_PNG)
			check_include_files(png.h HAVE_PNG_H)
			
			if(CLIP_X11_PNG_LIBRARY)
				set(PNG_LIBRARY ${CLIP_X11_PNG_LIBRARY})
			else()
				find_library(PNG_LIBRARY png)
			endif()
			
			if(HAVE_PNG_H AND PNG_LIBRARY)
				target_compile_definitions(${THIS_PROJECT} PRIVATE -DHAVE_PNG_H)
				target_link_libraries(${THIS_PROJECT} PRIVATE ${PNG_LIBRARY})
			endif()
		endif()
	endif()
else ()
	message(FATAL_ERROR "Unsupported platform ${DDCLI_TARGET_PLATFORM}.")
endif ()