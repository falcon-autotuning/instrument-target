cmake_minimum_required(VERSION 3.20)

project(
  instrument-target
  VERSION 1.0.6
  DESCRIPTION "A instrument target implemenation in C99 for use by Cpp, Go, and Lua"
  HOMEPAGE_URL "https://github.com/falcon-autotuning/instrument-target"
  LANGUAGES C
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)
option(BUILD_LUA "Build Lua bindings" OFF)
option(BUILD_TESTS "Build tests" OFF)
option(USE_CCACHE "Enable ccache or sccache if available" ON)

# ============================================================
# Windows Definitions
# ============================================================

if(WIN32)
  add_compile_definitions(NOMINMAX)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

if(USE_CCACHE)
  find_program(SCCACHE_PROGRAM sccache)
  if(SCCACHE_PROGRAM)
    message(STATUS "Using sccache: ${SCCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER ${SCCACHE_PROGRAM})
  else()
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
      message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
      set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    endif()
  endif()
endif()

add_library(instrument-target SHARED
  src/instrument-target.c
)

include(GenerateExportHeader)
set(GENERATED_INCLUDE_DIR
    ${CMAKE_CURRENT_BINARY_DIR}/include/instrument-target)
file(MAKE_DIRECTORY ${GENERATED_INCLUDE_DIR})
generate_export_header(instrument-target
  EXPORT_FILE_NAME
    ${GENERATED_INCLUDE_DIR}/instrument-target-export.h
)
target_include_directories(instrument-target
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# ============================================================
# Lua Bindings
# ============================================================

if(BUILD_LUA)
  find_package(Lua REQUIRED)

  # Normalize Lua target across environments
  if(NOT TARGET Lua::Lua)
    add_library(Lua::Lua INTERFACE IMPORTED)
    set_target_properties(Lua::Lua PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${LUA_INCLUDE_DIR}"
      INTERFACE_LINK_LIBRARIES "${LUA_LIBRARIES}"
    )
  endif()

  add_library(instrument-target-lua SHARED
    src/instrument-target-lua.c
  )

  target_link_libraries(instrument-target-lua
    PRIVATE
      instrument-target
      Lua::Lua
  )

  target_include_directories(instrument-target-lua
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )

  target_compile_definitions(instrument-target-lua
    PRIVATE BUILD_LUA_BINDINGS
  )
endif()

set_target_properties(instrument-target PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
)

if(BUILD_LUA)
  set_target_properties(instrument-target-lua PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
  )
endif()

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# ============================================================
# Install
# ============================================================

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(FILES
  ${GENERATED_INCLUDE_DIR}/instrument-target-export.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/instrument-target
)
if(BUILD_LUA)
  set(EXPORTED_TARGETS instrument-target instrument-target-lua)
else()
  set(EXPORTED_TARGETS instrument-target)
endif()

install(TARGETS ${EXPORTED_TARGETS}
  EXPORT instrument-target-targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(EXPORT instrument-target-targets
  FILE instrument-target-targets.cmake
  NAMESPACE instrument-target::
  DESTINATION ${CMAKE_INSTALL_DATADIR}/instrument-target
)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/instrument-target-config-version.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)
configure_package_config_file(
  cmake/instrument-target-config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/instrument-target-config.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/instrument-target
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/instrument-target-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/instrument-target-config-version.cmake"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/instrument-target
)

configure_file(
  cmake/instrument-target.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/instrument-target.pc
  @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/instrument-target.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

if(CMAKE_GENERATOR STREQUAL "Ninja" AND WIN32)
  file(TOUCH "${CMAKE_BINARY_DIR}/build.ninja")
endif()
