# CPM Package Lock
# This file should be committed to version control

# CPMLicenses.cmake
CPMDeclarePackage(CPMLicenses.cmake
  VERSION 0.0.4
  GITHUB_REPOSITORY TheLartians/CPMLicenses.cmake
)
# Common CMake additions for Amiga
CPMDeclarePackage(CMakeAmigaCommon
  GIT_TAG 1.0.7
  GITHUB_REPOSITORY AmigaPorts/cmake-amiga-common-library
)
# Emu68 i2c.library
CPMDeclarePackage(i2c.library
  GIT_TAG 7efe7da922f9021b6398f4bdaaf5c6b61e4b31e0
  GITHUB_REPOSITORY michalsc/i2c.library
  EXCLUDE_FROM_ALL YES
)
