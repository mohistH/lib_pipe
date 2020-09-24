cmake_minimum_required(VERSION 3.1)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	SET(CMAKE_C_COMPILER /usr/bin/clang)
	SET(CMAKE_CXX_COMPILER /usr/bin/clang++)
endif()


# add_definitions(-std=c++11)



# ---------------------------------------------------------------------------------------------------
# 1. set name
project(lib_pipe)


# ---------------------------------------------------------------------------------------------------
# 2. to get all source files
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src pipe_src)



# ---------------------------------------------------------------------------------------------------
# 3. to attach to path of includes direcotries
include_directories(
"${CMAKE_CURRENT_SOURCE_DIR}/include"
)


# to show headerfiles in program
file(GLOB_RECURSE pipe_header_files  ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h )
source_group("Header Files" FILES ${pipe_header_files}) 





# ---------------------------------------------------------------------------------------------------
#4. set some options
option(example_shared 			"an example to show how to use dynamic library" ON)
option(lib_shared 				"to generate dynamic lbrary" 					ON)
#option(lib_mfc 					"to generate a dynamic library to support MFC" 	OFF)
#option(lib_static 				"to create an static library" 					OFF)



# ---------------------------------------------------------------------------------------------------
# 5. dynamic library program
if(lib_shared)

	# windows
 	IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
 		

				
		Add_Definitions(-DUNICODE -D_UNICODE)
		
		# to create static library
		if(lib_static)
			add_library( lib_pipe_static STATIC ${pipe_src} ${pipe_header_files})
		else()
			# to create dynamic library
			add_library( lib_pipe_shared SHARED ${pipe_src} ${pipe_header_files})
		endif(lib_static)

		

		# set_property(TARGET lib_udp_shared PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR})
 	ENDIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
endif(lib_shared)



# ---------------------------------------------------------------------------------------------------
#6. to support MFC 
IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		if(lib_mfc)

			# set charators unicode
			Add_Definitions(-DUNICODE -D_UNICODE)
			Add_Definitions(-D_AFC_DLL)
			SET(CMAKE_MFC_FLAG 2)	
			
			# 6.2 to create an tatic library 
			if(lib_static)
				add_library( mfc_lib_pipe_static STATIC ${pipe_src} )
			endif(lib_static)
			
			#6.3 to create dynamic library	
			add_library( mfc_lib_pipe_shared SHARED ${pipe_src} )
			
			# 6.4 set others 
			set_property(TARGET mfc_lib_pipe_shared PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR})

			#6.5 
			if(lib_static)
				SET_TARGET_PROPERTIES (mfc_lib_pipe_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
			endif(lib_static)
			
			SET_TARGET_PROPERTIES (mfc_lib_pipe_shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)

		endif(lib_mfc)
	endif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Windows")





# ---------------------------------------------------------------------------------------------------
#7.  an example , to show how to use dynamic libary
if (example_shared)
	#7.1 set src path
	set (exam_src 
		${CMAKE_CURRENT_SOURCE_DIR}/example/main.cpp
	)


	# 7.3 to get an executable program
	add_executable(exam_lib_dyn ${exam_src} ${pipe_header_files})

	target_include_directories(exam_lib_dyn PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
	
	#7.4 to link library
	IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
		target_link_libraries(exam_lib_dyn lib_pipe_shared)
	ELSE()
		target_link_libraries(exam_lib_dyn pipe_shared)
	ENDIF (CMAKE_SYSTEM_NAME MATCHES "Windows")

endif(example_shared)





#------------------------------------------------------------------------------------

if(WIN32)
	# 
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/build/Debug)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/build/Release)

	# 
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/build/lib/Debug)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/build/lib/Release)
endif(WIN32)



# 判断操作系统

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
	
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
	
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	
ELSE ()
	MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")


## 判断编译器
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
# using Clang
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
# using GCC
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
# using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
# using Visual Studio C++
endif()


