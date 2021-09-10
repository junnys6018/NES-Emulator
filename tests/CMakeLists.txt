file(GLOB_RECURSE SOURCES src/*)
# Include timer from application project
set(TIMER ../application/src/Util/Timer.c ../application/src/Util/Timer.h)

add_executable(tests ${SOURCES} ${TIMER})

target_include_directories(tests PUBLIC
    src
    ${CMAKE_SOURCE_DIR}/core/src
    ${CMAKE_SOURCE_DIR}/application/src/Util
)
target_link_libraries(tests PUBLIC 
    core
)

# set working dir when debugging on visual studio
set_property(TARGET tests PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/")