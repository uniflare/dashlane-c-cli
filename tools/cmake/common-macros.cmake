macro(oct_define_sources)
	set(single_args PLATFORM)
	set(multi_args GROUP)
	cmake_parse_arguments(X "" "${single_args}" "${multi_args}" ${ARGN})
	
	set (SOURCES)
	set (current_platform)
	set (current_group)
	set (expect_platform)
	set (expect_group)
	foreach(arg ${ARGN})
		if ( ${expect_platform} )
			set(expect_platform)
			set(current_platform ${arg})
		elseif( ${expect_group} )
			set(expect_group)
			set(current_group ${arg})
		elseif( arg STREQUAL "PLATFORM" )
			set(expect_platform TRUE)
		elseif( arg STREQUAL "GROUP" )
			set(expect_group TRUE)
		else()
			if ( current_platform STREQUAL "ALL" OR current_platform STREQUAL OCT_TARGET_PLATFORM )
				source_group("${current_group}" FILES ${arg})
				list(APPEND SOURCES ${arg})
			endif()
		endif()
	endforeach()
endmacro()

macro(oct_project name)
	set(single_args TYPE FOLDER LANGUAGE_C)
	cmake_parse_arguments(PROJECT "${optional_args}" "${single_args}" "" ${ARGN})
	
	project(name ${PROJECT_LANGUAGE_C})
	set(THIS_PROJECT ${name} PARENT_SCOPE)
	set(THIS_PROJECT ${name})

	if(NOT ${THIS_PROJECT}_SOURCES)
		set(${THIS_PROJECT}_SOURCES ${SOURCES})
	endif()

	string(MAKE_C_IDENTIFIER "${THIS_PROJECT}" safe_libname)

	if( PROJECT_TYPE STREQUAL "INTERFACE" ) # Should be true interface, make this 'utility' or 'container'
		add_custom_target(${THIS_PROJECT} SOURCES ${${THIS_PROJECT}_SOURCES})
	elseif( PROJECT_TYPE STREQUAL "EXECUTABLE" )
		add_executable(${THIS_PROJECT} ${${THIS_PROJECT}_SOURCES})
	elseif( PROJECT_TYPE STREQUAL "SHARED" )
		add_library(${THIS_PROJECT} SHARED ${${THIS_PROJECT}_SOURCES})
	elseif( PROJECT_TYPE STREQUAL "STATIC" )
		add_library(${THIS_PROJECT} STATIC ${${THIS_PROJECT}_SOURCES})
		target_compile_definitions(${THIS_PROJECT} PUBLIC "${safe_libname}_STATIC")
	elseif( PROJECT_TYPE STREQUAL "MODULE" )
		add_library(${THIS_PROJECT} MODULE ${${THIS_PROJECT}_SOURCES})
	else()
		message(FATAL_ERROR "Missing project type. (INTERFACE/EXECUTABLE/SHARED/STATIC/MODULE)")
	endif()
	
	set_property(TARGET ${THIS_PROJECT} PROPERTY FOLDER "${PROJECT_FOLDER}")
endmacro()

macro(oct_setup_output_dirs)
	foreach(config_type in ${CMAKE_CONFIGURATION_TYPES})
		string(TOUPPER ${config_type} config_type_upper)
		if( ${config_type_upper} STREQUAL "RELEASE" )
			set(suffix "_release")
		elseif( ${config_type_upper} STREQUAL "DEBUG" )
			set(suffix "_debug")
		else()
			set(suffix "")
		endif()
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_type_upper} ${OCT_BASE_DIR}/bin/${OCT_OUTPUT_DIR}${suffix})
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${config_type_upper} ${OCT_BASE_DIR}/lib/${config_type}/${OCT_OUTPUT_DIR})
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${config_type_upper} ${OCT_BASE_DIR}/lib/${config_type}/${OCT_OUTPUT_DIR})
	endforeach()
endmacro()