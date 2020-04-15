set (lib_common_SRCS
./ieeeC37_118/ieeeC37_118.c
./common/lib_memory.c
)

set (lib_linux_SRCS
./pal/socket/linux/socket_linux.c
./pal/thread/linux/thread_linux.c
./pal/time/unix/time.c
)

set (lib_windows_SRCS
./pal/socket/win32/socket_win32_1.c
./pal/thread/win32/thread_win32.c
./pal/time/win32/time.c
)

set (lib_bsd_SRCS
./pal/socket/bsd/socket_bsd.c
./pal/thread/linux/thread_linux.c
./pal/time/unix/time.c
)

IF(WIN32)

set_source_files_properties(${lib_common_SRCS} ${lib_windows_SRCS}
                                       PROPERTIES LANGUAGE CXX)

set (CMAKE_MODULE_LINKER_FLAGS   "${CMAKE_MODULE_LINKER_FLAGS}\"/DEF:${CMAKE_CURRENT_SOURCE_DIR}/vs/libieeeC37_118.def\"") 

set (library_SRCS
	${lib_common_SRCS}
    ${lib_windows_SRCS}
)

ELSEIF(UNIX)
IF(APPLE)
set (library_SRCS
	${lib_common_SRCS}
    ${lib_bsd_SRCS}
)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
set (library_SRCS
	${lib_common_SRCS}
    ${lib_bsd_SRCS}
)
ELSE()
set (library_SRCS
	${lib_common_SRCS}
    ${lib_linux_SRCS}	
)
ENDIF(APPLE)
ENDIF(WIN32)

include (GenerateExportHeader)

add_library (ieeeC37_118-shared SHARED ${library_SRCS} )

set_target_properties(ieeeC37_118-shared PROPERTIES
           OUTPUT_NAME ieeeC37_118
#           SOVERSION "${LIB_VERSION_MAJOR}.${LIB_VERSION_MINOR}.${LIB_VERSION_PATCH}"
)


GENERATE_EXPORT_HEADER(ieeeC37_118-shared
			BASE_NAME ieeeC37_118-shared
			EXPORT_MACRO_NAME ieeeC37_118-shared_EXPORT
			EXPORT_FILE_NAME ieeeC37_118-shared_export.h
			STATIC_DEFINE ieeeC37_118-shared_BUILT_AS_STATIC
)

add_library (ieeeC37_118 STATIC ${library_SRCS})

IF(UNIX)
  IF (CONFIG_SYSTEM_HAS_CLOCK_GETTIME)
     target_link_libraries (ieeeC37_118
         -lpthread
         -lm
         -lrt
     )
  ELSE ()
     target_link_libraries (ieeeC37_118
         -lpthread
         -lm
     )
  ENDIF (CONFIG_SYSTEM_HAS_CLOCK_GETTIME)
ENDIF(UNIX)

if(MSVC)
		set_target_properties(ieeeC37_118-shared PROPERTIES
           	LINK_FLAGS "/DEF:\"${CMAKE_CURRENT_SOURCE_DIR}/vs/libieeeC37_118.def\""
       	)
endif()

install (TARGETS ieeeC37_118 ieeeC37_118-shared
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

