cmake_minimum_required(VERSION 3.14.0)
project(CAMD-I2C VERSION 1.0.0)

# Additional CMake modules
include(cmake/common/setup_amiga_common.cmake)

# Set up information for Aminet
add_project_info(
	"i2c driver for camd.library"
	"Marlon Beijer"
	"mus/midi"
	"m68k-amigaos"
)

add_subdirectory(camd-i2c.driver)
add_aminet_readme_generator(camd-i2c.driver "camd-i2c.readme")
