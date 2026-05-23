# add lunasvg
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
add_subdirectory(lunasvg EXCLUDE_FROM_ALL)

# add borealis
add_subdirectory(borealis/library)

# add libnxtc
if (PLATFORM_SWITCH)
    add_library(nxtc
        libnxtc/source/nxtc.c
        libnxtc/source/nxtc_log.c
        libnxtc/source/nxtc_utils.c
    )
    target_include_directories(nxtc PUBLIC libnxtc/include)
    target_link_libraries(nxtc PUBLIC z)
    target_compile_options(nxtc PRIVATE -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -ftls-model=local-exec)
endif()
