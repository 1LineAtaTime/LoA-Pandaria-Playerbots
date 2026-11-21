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

# Revision data from git, as AzerothCore's genrev.cmake does (this file used to ask Mercurial, so a git checkout
# always got "Archive" and an empty date and branch). The version string is the nearest release tag plus the
# commits since it and the commit hash, e.g. "v1.1.1-43-g98872246" ("+" when the tree has local changes).

if(NOT GIT_EXECUTABLE)
  set(GIT_EXECUTABLE "git")
endif()

execute_process(
  COMMAND "${GIT_EXECUTABLE}" describe --tags --long --always --dirty=+ --abbrev=8 --match "v*"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE rev_hash_str
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

execute_process(
  COMMAND "${GIT_EXECUTABLE}" rev-list --count HEAD
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE rev_id_str
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

execute_process(
  COMMAND "${GIT_EXECUTABLE}" show -s --format=%ci
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE rev_date
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

execute_process(
  COMMAND "${GIT_EXECUTABLE}" rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE rev_branch
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

# Last minute check - ensure that we have a proper revision (no git, or a source archive without .git)
if(NOT rev_hash_str)
  message("")
  message(STATUS "WARNING - No revision information found (no git repository?) - using \"Archive\"")
  set(rev_hash_str "Archive")
  set(rev_id_str "0")
  set(rev_date "1970-01-01 00:00:00 +0000")
  set(rev_branch "Archived")
endif()

if(NOT rev_id_str MATCHES "^[0-9]+$")
  set(rev_id_str "0")
endif()
set(rev_id ${rev_id_str})
set(rev_hash ${rev_hash_str})
string(SUBSTRING "${rev_date}" 0 4 rev_year)

# Its not set during initial run
if(NOT BUILDDIR)
  set(BUILDDIR ${CMAKE_BINARY_DIR})
endif()

# Create the actual revision.h file from the above params (only when the revision changed)
if(NOT "${rev_hash_cached}" STREQUAL "${rev_hash_str} ${rev_date} ${rev_branch}")
  configure_file(
    "${CMAKE_SOURCE_DIR}/revision.h.in.cmake"
    "${BUILDDIR}/revision.h"
    @ONLY
  )
  set(rev_hash_cached "${rev_hash_str} ${rev_date} ${rev_branch}" CACHE INTERNAL "Cached revision")
endif()
