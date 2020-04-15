
set(c37_118_Receiver_SRCS
   c37_118_Receiver.c
)

IF(WIN32)
set_source_files_properties(${c37_118_Receiver_SRCS}
                                       PROPERTIES LANGUAGE CXX)
ENDIF(WIN32)

add_executable(c37_118_Receiver
  ${c37_118_Receiver_SRCS}
)

target_link_libraries(c37_118_Receiver 
ieeeC37_118-shared
)
