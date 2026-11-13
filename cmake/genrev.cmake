# Copyright (C) 2011-2016 Project SkyFire <http://www.projectskyfire.org/
# Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

# Resolve the source location in both configure mode and `cmake -P` build mode.
get_filename_component(_revision_source_dir "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
find_program(_revision_git git)
set(_revision_from_git FALSE)
if(_revision_git)
  execute_process(
    COMMAND "${_revision_git}" rev-parse --verify HEAD
    WORKING_DIRECTORY "${_revision_source_dir}"
    OUTPUT_VARIABLE rev_hash_str
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE _revision_git_result
  )
  if(_revision_git_result EQUAL 0)
    set(_revision_from_git TRUE)
    execute_process(
      COMMAND "${_revision_git}" rev-list --count HEAD
      WORKING_DIRECTORY "${_revision_source_dir}"
      OUTPUT_VARIABLE rev_id_str
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
      COMMAND "${_revision_git}" show -s --format=%cs HEAD
      WORKING_DIRECTORY "${_revision_source_dir}"
      OUTPUT_VARIABLE rev_date
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
      COMMAND "${_revision_git}" symbolic-ref --short -q HEAD
      WORKING_DIRECTORY "${_revision_source_dir}"
      OUTPUT_VARIABLE rev_branch
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
    )
    if(NOT rev_branch)
      set(rev_branch "detached")
    endif()
    execute_process(
      COMMAND "${_revision_git}" diff --quiet --ignore-submodules HEAD --
      WORKING_DIRECTORY "${_revision_source_dir}"
      RESULT_VARIABLE _revision_dirty
      ERROR_QUIET
    )
    if(_revision_dirty EQUAL 1)
      string(APPEND rev_hash_str "+dirty")
    endif()
  endif()
endif()

# Retain support for historical Mercurial checkouts and exported archives.
if(NOT _revision_from_git)
execute_process(
  COMMAND hg id -n
  WORKING_DIRECTORY "${_revision_source_dir}"
  OUTPUT_VARIABLE rev_id_str
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

execute_process(
  COMMAND hg id -i
  WORKING_DIRECTORY "${_revision_source_dir}"
  OUTPUT_VARIABLE rev_hash_str
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

execute_process(
  COMMAND  hg tip --template "{date|shortdate}"
  WORKING_DIRECTORY "${_revision_source_dir}"
  OUTPUT_VARIABLE rev_date
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

if(EXISTS "${_revision_source_dir}/.hg_archival.txt")
  file(READ
    "${_revision_source_dir}/.hg_archival.txt" rev_hash_str
    LIMIT 10
    OFFSET 7
    NEWLINE_CONSUME
  )
  string(STRIP ${rev_hash_str} rev_hash_str)
  set(rev_id_str "Archive")
  set(rev_id "0")
  set(rev_hash ${rev_hash_str})
endif()
endif()

# Last minute check - ensure that we have a proper revision
# If everything above fails (means the user has erased the mercurial revisional control directory, or runs archive and erased their .hg_archival.txt)
if(NOT rev_id_str)
  message("")
  message(STATUS "WARNING - No revision-information found - have you been tampering with the sources?")

  # Ok, since we have no valid ways of finding/setting the revision, let's force some defaults
  set(rev_hash_str "Archive")
  set(rev_hash "0")
  set(rev_id_str "0")
  set(rev_id "0")
endif()

# Strip off excess strings (shows when the source is actually modified)
if(NOT rev_id_str MATCHES "Archive")
  string(REPLACE "+" "" rev_id ${rev_id_str})
endif()
string(REPLACE "+" "" rev_hash ${rev_hash_str})
if(rev_id MATCHES "^[0-9]+$")
  # Windows VERSIONINFO components are unsigned 16-bit integers.
  math(EXPR rev_id "${rev_id} % 65536")
endif()
if(rev_date MATCHES "^([0-9][0-9][0-9][0-9])")
  set(rev_year "${CMAKE_MATCH_1}")
endif()

# Its not set during initial run
if(NOT BUILDDIR)
  set(BUILDDIR ${CMAKE_BINARY_DIR})
endif()

# configure_file only replaces the output if its contents change. Checking only
# the revision count missed branch switches and changes to the commit hash.
  configure_file(
    "${_revision_source_dir}/revision.h.in.cmake"
    "${BUILDDIR}/revision.h"
    @ONLY
  )
