file(GLOB_RECURSE SOURCES src/*)


if (EMSCRIPTEN)
    add_executable(emu ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/emscripten/api.c)
    target_include_directories(emu PUBLIC 
        src
    )

    set(EXPORTED_JSON ${CMAKE_CURRENT_SOURCE_DIR}/emscripten/exported.json)

    set_target_properties(emu PROPERTIES
        LINK_FLAGS "-s EXPORT_ES6 -s MODULARIZE -s WASM_BIGINT -s WASM=1 -s EXPORTED_RUNTIME_METHODS=\"['cwrap']\" -s EXPORTED_FUNCTIONS=\"@${EXPORTED_JSON}\""
        LINK_DEPENDS "${EXPORTED_JSON}"
    )
else ()
    add_library(core STATIC ${SOURCES})

    source_group(" " REGULAR_EXPRESSION "src/.*")
    source_group(Cheatcodes REGULAR_EXPRESSION "src/Cheatcodes/.*")
    source_group(Mappers REGULAR_EXPRESSION "src/Mappers/.*")
    target_include_directories(core PUBLIC 
        src
    )
endif ()
