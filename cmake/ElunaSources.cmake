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

# The stamp also covers this file, so a change to how the copy is made (for
# example a fix to the patch step) remakes copies made by an older version.
file(SHA256 "${CMAKE_CURRENT_LIST_FILE}" ELUNA_SCRIPT_HASH)
set(ELUNA_STAMP_TEXT "script ${ELUNA_SCRIPT_HASH}\n")
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

find_program(ELUNA_GIT_EXECUTABLE NAMES git)
find_program(ELUNA_PATCH_EXECUTABLE NAMES patch)
if(ELUNA_PATCHES AND NOT ELUNA_GIT_EXECUTABLE AND NOT ELUNA_PATCH_EXECUTABLE)
  message(FATAL_ERROR "Eluna: neither git nor patch was found to apply dep/eluna-patches")
endif()

# Runs the patch tool on the copy. mode: apply, or verify (the patch must be
# present: it reverses cleanly).
#
# `git apply` run inside a git work tree takes the paths of the patch relative
# to that repository's root; with the build directory inside the core's
# checkout (cmake -B build) it skips every file and still exits 0. Git is
# therefore kept from looking above the copy (GIT_CEILING_DIRECTORIES) and
# GIT_DIR / GIT_WORK_TREE are cleared, so it works as a plain patch tool on
# the copy, and a "Skipped patch" line counts as a failure as well.
function(eluna_patch_tool mode patch result_var output_var)
  get_filename_component(copy_parent "${ELUNA_SOURCE_DIR}" DIRECTORY)
  if(ELUNA_GIT_EXECUTABLE)
    set(git_args apply -v --whitespace=nowarn -p1)
    if(mode STREQUAL "verify")
      list(APPEND git_args --check --reverse)
    endif()
    execute_process(
      COMMAND "${CMAKE_COMMAND}" -E env --unset=GIT_DIR --unset=GIT_WORK_TREE
              "GIT_CEILING_DIRECTORIES=${copy_parent}"
              "${ELUNA_GIT_EXECUTABLE}" ${git_args} "${patch}"
      WORKING_DIRECTORY "${ELUNA_SOURCE_DIR}"
      RESULT_VARIABLE result
      OUTPUT_VARIABLE output
      ERROR_VARIABLE output)
  else()
    # --fuzz=0: a hunk lands only where its context matches exactly (not
    # somewhere nearby after a submodule bump). Verify uses --force, not
    # --batch: with --batch a patch that is NOT applied gives "Unreversed patch
    # detected!  Ignoring -R." and exits 0; --force never asks and assumes the
    # patch is not reversed, so reversing an absent patch fails its hunks.
    set(patch_args -p1 --fuzz=0 -i "${patch}")
    if(mode STREQUAL "verify")
      list(APPEND patch_args -R --dry-run --force)
    else()
      list(APPEND patch_args --forward --batch)
    endif()
    execute_process(
      COMMAND "${ELUNA_PATCH_EXECUTABLE}" ${patch_args}
      WORKING_DIRECTORY "${ELUNA_SOURCE_DIR}"
      RESULT_VARIABLE result
      OUTPUT_VARIABLE output
      ERROR_VARIABLE output)
  endif()
  # Any of these means the patch was not (or not wholly) applied or present,
  # whatever the exit code.
  if(result EQUAL 0 AND output MATCHES "Skipped patch|[Uu]nreversed patch|Ignoring -R|[Rr]eversed \\(or previously applied\\)|FAILED|[Ff]uzz [1-9]")
    set(result 1)
  endif()
  # git -v names every file it checks: no "Checking patch" means it did nothing.
  if(result EQUAL 0 AND ELUNA_GIT_EXECUTABLE AND NOT output MATCHES "Checking patch")
    set(result 1)
  endif()
  set(${result_var} "${result}" PARENT_SCOPE)
  set(${output_var} "${output}" PARENT_SCOPE)
endfunction()

# Sets out_var to the empty string when every patch is present in the copy,
# else to the name (and tool output) of the first one that is not. Two checks:
# the patch reverses cleanly, and its marker (each patch adds a
# "#define ELUNA_TW_PATCH_<n> 1" line, which src/game/ElunaScriptBridge.cpp
# also requires at compile time) is in the copy's headers.
function(eluna_verify_copy out_var)
  set(missing "")
  if(NOT EXISTS "${ELUNA_SOURCE_DIR}/LuaEngine.h")
    set(missing "no copy")
  endif()
  if(NOT missing)
    file(GLOB copy_headers "${ELUNA_SOURCE_DIR}/*.h")
    set(copy_text "")
    foreach(header IN LISTS copy_headers)
      file(READ "${header}" header_text)
      string(APPEND copy_text "${header_text}")
    endforeach()
    foreach(patch IN LISTS ELUNA_PATCHES)
      get_filename_component(patch_name "${patch}" NAME)
      file(STRINGS "${patch}" marker_lines REGEX "^\\+#define ELUNA_TW_PATCH_[0-9]+ ")
      if(NOT marker_lines)
        message(FATAL_ERROR "Eluna: ${patch_name} adds no \"#define ELUNA_TW_PATCH_<n> 1\" marker")
      endif()
      list(GET marker_lines 0 marker_line)
      string(REGEX REPLACE "^\\+" "" marker "${marker_line}")
      string(FIND "${copy_text}" "${marker}" marker_position)
      if(marker_position EQUAL -1)
        set(missing "${patch_name} (marker not in the copy)")
        break()
      endif()
      eluna_patch_tool(verify "${patch}" result output)
      if(NOT result EQUAL 0)
        set(missing "${patch_name}:\n${output}")
        break()
      endif()
    endforeach()
  endif()
  set(${out_var} "${missing}" PARENT_SCOPE)
endfunction()

set(ELUNA_STAMP_FILE "${ELUNA_SOURCE_DIR}/.eluna-patch-stamp")
set(ELUNA_OLD_STAMP "")
if(EXISTS "${ELUNA_STAMP_FILE}")
  file(READ "${ELUNA_STAMP_FILE}" ELUNA_OLD_STAMP)
endif()

set(ELUNA_REMAKE_COPY FALSE)
if(NOT ELUNA_OLD_STAMP STREQUAL ELUNA_STAMP)
  set(ELUNA_REMAKE_COPY TRUE)
else()
  # Same inputs: still check that every patch is in the copy.
  eluna_verify_copy(ELUNA_MISSING_PATCH)
  if(ELUNA_MISSING_PATCH)
    message(STATUS "Eluna sources         : copy lacks ${ELUNA_MISSING_PATCH}; remaking it")
    set(ELUNA_REMAKE_COPY TRUE)
  endif()
endif()

if(ELUNA_REMAKE_COPY)
  list(LENGTH ELUNA_PATCHES patch_count)
  message(STATUS "Eluna sources         : dep/Eluna + ${patch_count} patches -> ${ELUNA_SOURCE_DIR}")
  file(REMOVE_RECURSE "${ELUNA_SOURCE_DIR}")
  file(MAKE_DIRECTORY "${ELUNA_SOURCE_DIR}")
  foreach(relative_path IN LISTS ELUNA_PRISTINE_FILES)
    get_filename_component(relative_dir "${relative_path}" DIRECTORY)
    file(COPY "${ELUNA_PRISTINE_DIR}/${relative_path}" DESTINATION "${ELUNA_SOURCE_DIR}/${relative_dir}")
  endforeach()

  foreach(patch IN LISTS ELUNA_PATCHES)
    get_filename_component(patch_name "${patch}" NAME)
    eluna_patch_tool(apply "${patch}" patch_result patch_output)
    if(NOT patch_result EQUAL 0)
      file(REMOVE_RECURSE "${ELUNA_SOURCE_DIR}")
      message(FATAL_ERROR "Eluna: ${patch_name} does not apply to dep/Eluna:\n${patch_output}")
    endif()
    message(STATUS "Eluna patch           : ${patch_name}")
  endforeach()

  eluna_verify_copy(ELUNA_MISSING_PATCH)
  if(ELUNA_MISSING_PATCH)
    file(REMOVE_RECURSE "${ELUNA_SOURCE_DIR}")
    message(FATAL_ERROR "Eluna: a patch is not in the copy after applying it: ${ELUNA_MISSING_PATCH}")
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
