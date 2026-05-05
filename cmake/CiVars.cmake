if (NOT DEFINED GIT_HASH)
  execute_process(
      COMMAND git rev-parse --short HEAD
      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
      OUTPUT_VARIABLE GIT_HASH
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  # Optional: Add "-dirty" if there are uncommitted changes
  execute_process(
      COMMAND git status --porcelain
      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
      OUTPUT_VARIABLE GIT_STATUS
  )
  if(NOT "${GIT_STATUS}" STREQUAL "")
    set(GIT_HASH "${GIT_HASH}-dirty")
  endif()
  message(STATUS "Got git hash ${GIT_HASH}")
endif()
