cmake_minimum_required(VERSION 3.14.0)
project(camd-i2c VERSION 1.0.0)
set(DRIVER_NAME "${PROJECT_NAME}.driver")
get_verstring(VERSTRING)
string(REPLACE "$VER: ${PROJECT_NAME} " "" VERDATE "${VERSTRING}")

if(NOT TARGET i2c)
	CPMGetPackage(i2c.library)
endif()

add_library(${PROJECT_NAME} INTERFACE)

add_executable(${DRIVER_NAME}
	src/main.c
	src/camd-i2c.c
)

target_include_directories(${PROJECT_NAME} INTERFACE include)
target_link_options(${DRIVER_NAME}  PRIVATE -ffreestanding -nostdlib -nostartfiles -Wl,-e,__start)
target_compile_options(${DRIVER_NAME}  PRIVATE -Os -m68040 -fomit-frame-pointer)
target_compile_definitions(${DRIVER_NAME}  PRIVATE VERSION_STRING="${VERSTRING}" VERSION_NAME_DATE="${VERDATE}" VERSION="${VERSION}" REVISION="${REVISION}" __NOLIBBASE__=1)
target_link_libraries(${DRIVER_NAME}  PUBLIC ${PROJECT_NAME} i2c)
target_compile_definitions(${DRIVER_NAME}  PUBLIC
	LIBRARY_NAME="${DRIVER_NAME} "
	LIBRARY_VERSION=${PROJECT_VERSION_MAJOR}
	LIBRARY_REVISION=${PROJECT_VERSION_MINOR}
	LIBRARY_PRIORITY=-120
)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${DRIVER_NAME} DESTINATION ${PROJECT_NAME}/Devs/midi/)