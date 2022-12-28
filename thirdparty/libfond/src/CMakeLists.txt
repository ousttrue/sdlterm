set(TARGET_NAME fond)
add_library(${TARGET_NAME} SHARED fond.c fond_common.c fond_buffer.c fond_windows.c)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
