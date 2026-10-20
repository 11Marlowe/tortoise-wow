# Prepares the Eluna sources for an Eluna build (BUILD_ELUNA=ON only).
#
# The engine is the dep/Eluna submodule, kept pristine at its pinned commit.
# The fixes this core needs in it are the patch files in dep/eluna-patches,
# applied in name order to a copy in the build tree. The game library compiles
# that copy: ELUNA_SOURCE_DIR is set for the callers.
#
# The copy is remade only when the submodule's files or a patch change (a
# content stamp), so an unchanged tree does not recompile Eluna.

set(ELUNA_PRISTINE_DIR "${CMAKE_SOURCE_DIR}/dep/Eluna")
set(ELUNA_PATCH_DIR "${CMAKE_SOURCE_DIR}/dep/eluna-patches")
set(ELUNA_SOURCE_DIR "${CMAKE_BINARY_DIR}/dep/eluna-src")

if(NOT EXISTS "${ELUNA_PRISTINE_DIR}/LuaEngine.h")
  message(FATAL_ERROR
    "Eluna submodule is missing. Run: git submodule update --init --recursive dep/Eluna")
endif()

file(GLOB_RECURSE ELUNA_PRISTINE_FILES LIST_DIRECTORIES false
  RELATIVE "${ELUNA_PRISTINE_DIR}" "${ELUNA_PRISTINE_DIR}/*")
list(FILTER ELUNA_PRISTINE_FILES EXCLUDE REGEX "^\\.git")
list(SORT ELUNA_PRISTINE_FILES)
file(GLOB ELUNA_PATCHES CONFIGURE_DEPENDS "${ELUNA_PATCH_DIR}/*.patch")
list(SORT ELUNA_PATCHES)

set(ELUNA_STAMP_TEXT "")
foreach(relative_path IN LISTS ELUNA_PRISTINE_FILES)
  file(SHA256 "${ELUNA_PRISTINE_DIR}/${relative_path}" file_hash)
  string(APPEND ELUNA_STAMP_TEXT "${relative_path} ${file_hash}\n")
endforeach()
foreach(patch IN LISTS ELUNA_PATCHES)
  file(SHA256 "${patch}" file_hash)
  get_filename_component(patch_name "${patch}" NAME)
  string(APPEND ELUNA_STAMP_TEXT "patch ${patch_name} ${file_hash}\n")
endforeach()
string(SHA256 ELUNA_STAMP "${ELUNA_STAMP_TEXT}")

set(ELUNA_STAMP_FILE "${ELUNA_SOURCE_DIR}/.eluna-patch-stamp")
set(ELUNA_OLD_STAMP "")
if(EXISTS "${ELUNA_STAMP_FILE}")
  file(READ "${ELUNA_STAMP_FILE}" ELUNA_OLD_STAMP)
endif()

if(NOT ELUNA_OLD_STAMP STREQUAL ELUNA_STAMP)
  list(LENGTH ELUNA_PATCHES patch_count)
  message(STATUS "Eluna sources         : dep/Eluna + ${patch_count} patches -> ${ELUNA_SOURCE_DIR}")
  file(REMOVE_RECURSE "${ELUNA_SOURCE_DIR}")
  file(MAKE_DIRECTORY "${ELUNA_SOURCE_DIR}")
  foreach(relative_path IN LISTS ELUNA_PRISTINE_FILES)
    get_filename_component(relative_dir "${relative_path}" DIRECTORY)
    file(COPY "${ELUNA_PRISTINE_DIR}/${relative_path}" DESTINATION "${ELUNA_SOURCE_DIR}/${relative_dir}")
  endforeach()

  if(ELUNA_PATCHES)
    find_program(ELUNA_GIT_EXECUTABLE NAMES git)
    find_program(ELUNA_PATCH_EXECUTABLE NAMES patch)
    foreach(patch IN LISTS ELUNA_PATCHES)
      get_filename_component(patch_name "${patch}" NAME)
      if(ELUNA_GIT_EXECUTABLE)
        # Outside a repository `git apply` works as a plain patch tool.
        execute_process(
          COMMAND "${ELUNA_GIT_EXECUTABLE}" apply --whitespace=nowarn -p1 "${patch}"
          WORKING_DIRECTORY "${ELUNA_SOURCE_DIR}"
          RESULT_VARIABLE patch_result
          OUTPUT_VARIABLE patch_output
          ERROR_VARIABLE patch_output)
      elseif(ELUNA_PATCH_EXECUTABLE)
        execute_process(
          COMMAND "${ELUNA_PATCH_EXECUTABLE}" -p1 --forward --batch -i "${patch}"
          WORKING_DIRECTORY "${ELUNA_SOURCE_DIR}"
          RESULT_VARIABLE patch_result
          OUTPUT_VARIABLE patch_output
          ERROR_VARIABLE patch_output)
      else()
        message(FATAL_ERROR "Eluna: neither git nor patch was found to apply ${patch_name}")
      endif()
      if(NOT patch_result EQUAL 0)
        file(REMOVE_RECURSE "${ELUNA_SOURCE_DIR}")
        message(FATAL_ERROR "Eluna: ${patch_name} does not apply to dep/Eluna:\n${patch_output}")
      endif()
      message(STATUS "Eluna patch           : ${patch_name}")
    endforeach()
  endif()

  file(WRITE "${ELUNA_STAMP_FILE}" "${ELUNA_STAMP}")
endif()

# A changed patch or submodule file re-runs CMake, which remakes the copy.
foreach(patch IN LISTS ELUNA_PATCHES)
  set_property(DIRECTORY "${CMAKE_SOURCE_DIR}" APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${patch}")
endforeach()
foreach(relative_path IN LISTS ELUNA_PRISTINE_FILES)
  set_property(DIRECTORY "${CMAKE_SOURCE_DIR}" APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${ELUNA_PRISTINE_DIR}/${relative_path}")
endforeach()
