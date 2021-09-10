file(GLOB_RECURSE SOURCES src/*)

add_library(core STATIC ${SOURCES})

source_group(" " REGULAR_EXPRESSION "src/.*")
source_group(Cheatcodes REGULAR_EXPRESSION "src/Cheatcodes/.*")
source_group(Mappers REGULAR_EXPRESSION "src/Mappers/.*")

target_include_directories(core PUBLIC 
    src
)