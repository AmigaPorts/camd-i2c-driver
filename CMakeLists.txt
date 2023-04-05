cmake_minimum_required(VERSION 3.14.0)

function(get_verstring OUTPUT)
	find_package(Git)
	string(TIMESTAMP PROJECT_DATE "(%d.%m.%Y)")
	execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE GIT_HASH OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND ${GIT_EXECUTABLE} status --porcelain WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE GIT_PORCELAIN OUTPUT_STRIP_TRAILING_WHITESPACE)
	if (NOT GIT_PORCELAIN STREQUAL "")
		set(GIT_PORCELAIN ",dirty")
	endif()

	set(${OUTPUT} "$VER: ${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_DATE} git:${GIT_HASH}${GIT_PORCELAIN}" PARENT_SCOPE)
endfunction(get_verstring)

project(camd-i2c VERSION 1.0.0)
get_verstring(VERSTRING)

string(REPLACE "$VER: ${PROJECT_NAME} " "" VERDATE "${VERSTRING}")

add_link_options(-ffreestanding -nostdlib -nostartfiles -Wl,-e,__start)
add_compile_options(-Os -m68040 -fbbb=- -fomit-frame-pointer)
add_compile_definitions(PRIVATE VERSION_STRING="${VERSTRING}" VERSION_NAME_DATE="${VERDATE}" VERSION="${VERSION}" REVISION="${REVISION}" __NOLIBBASE__=1)

add_library(camd-i2c INTERFACE)
target_include_directories(camd-i2c INTERFACE include)

add_executable(camd-i2c.driver
	src/main.c
	src/camd-i2c.c
)

target_include_directories(camd-i2c.driver PRIVATE src)
target_link_libraries(camd-i2c.driver camd-i2c amiga)
target_compile_definitions(camd-i2c.driver PUBLIC
	LIBRARY_NAME="camd-i2c.driver"
	LIBRARY_VERSION=${PROJECT_VERSION_MAJOR}
	LIBRARY_REVISION=${PROJECT_VERSION_MINOR}
	LIBRARY_PRIORITY=-120
)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/camd-i2c.driver DESTINATION ./midi/)
