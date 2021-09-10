
# ---------- Add Third Party Library ----------

# glad
add_subdirectory(vendor/glad)

# Move library to its own folder
set_property(TARGET glad PROPERTY FOLDER "vendor")

# ---------- Main Application ----------

file(GLOB_RECURSE SOURCES src/*)
set(CJSON vendor/cJSON/cJSON.c)
add_executable(application ${SOURCES} ${CJSON})

source_group(" " REGULAR_EXPRESSION "src/.*")
source_group(Vendor FILES "vendor/cJSON/cJSON.c")
source_group(Models REGULAR_EXPRESSION "src/Models/.*")
source_group(OpenGL REGULAR_EXPRESSION "src/OpenGL/.*")
source_group(Util REGULAR_EXPRESSION "src/Util/.*")
source_group(Views REGULAR_EXPRESSION "src/Views/.*")

target_include_directories(application PUBLIC
    src
    vendor/SDL2-2.0.12/include
    vendor/stb
    vendor/cJSON
    vendor/glad/include
    ${CMAKE_SOURCE_DIR}/core/src
)

target_link_libraries(application PUBLIC 
    SDL2
    SDL2main
    core
    glad
)

if (WIN32)
    target_link_directories(application PUBLIC vendor/SDL2-2.0.12/lib/x64)
    target_sources(application PRIVATE res/NES-Emulator.rc)

    # see https://stackoverflow.com/questions/1620006/post-build-step-only-for-release-build/46439715
    add_custom_command(TARGET application POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E $<$<CONFIG:Debug>:echo> copy $<TARGET_FILE:application> ${CMAKE_CURRENT_SOURCE_DIR}/NES-Emulator.exe)
else()
    add_custom_command(TARGET application POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E $<$<CONFIG:Debug>:echo> cp $<TARGET_FILE:application> ${CMAKE_CURRENT_SOURCE_DIR}/NES-Emulator)
endif()

# set working dir when debugging on visual studio
set_property(TARGET application PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/")