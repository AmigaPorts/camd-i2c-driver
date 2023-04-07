cmake_minimum_required(VERSION 3.14.0)
project(camd-i2c VERSION 1.0.0)
get_verstring(VERSTRING)
string(REPLACE "$VER: ${PROJECT_NAME} " "" VERDATE "${VERSTRING}")

add_link_options(-ffreestanding -nostdlib -nostartfiles -Wl,-e,__start)
add_compile_options(-Os -m68040 -fomit-frame-pointer)
add_compile_definitions(PRIVATE VERSION_STRING="${VERSTRING}" VERSION_NAME_DATE="${VERDATE}" VERSION="${VERSION}" REVISION="${REVISION}" __NOLIBBASE__=1)

add_library(camd-i2c INTERFACE)
target_include_directories(camd-i2c INTERFACE include)

add_executable(camd-i2c.driver
	src/main.c
	src/camd-i2c.c
)

target_include_directories(camd-i2c.driver PRIVATE src)
target_link_libraries(camd-i2c.driver camd-i2c)
target_compile_definitions(camd-i2c.driver PUBLIC
	LIBRARY_NAME="camd-i2c.driver"
	LIBRARY_VERSION=${PROJECT_VERSION_MAJOR}
	LIBRARY_REVISION=${PROJECT_VERSION_MINOR}
	LIBRARY_PRIORITY=-120
)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/camd-i2c.driver DESTINATION ./camd-i2c/Devs/midi/)