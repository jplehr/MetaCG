
# LLVM related stuff (XXX Do we actually need LLVM?)
find_package(LLVM REQUIRED CONFIG)

# Clang related stuff
find_package(Clang REQUIRED CONFIG)

function(add_clang target)
# clang specified as system lib to suppress all warnings from clang headers
  target_include_directories(${target} SYSTEM PUBLIC
    ${CLANG_INCLUDE_DIRS}
  )

  target_link_libraries(${target} PUBLIC
    clangTooling
  )
endfunction()

# TODO Add GoogleTest


# include directories
function(add_collector_include target)
  target_include_directories(${target}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/include>
  )
endfunction()

function(add_collector_lib target)
  target_link_libraries(${target}
    collectorlib
  )
endfunction()

# Compile flags
function(default_compile_options target)
  cmake_parse_arguments(ARG "" "" "PRIVATE_FLAGS;PUBLIC_FLAGS" ${ARGN})

  target_compile_options(${target} PRIVATE
    -Wall -Wextra -pedantic -Wunreachable-code -Wwrite-strings
    -Wpointer-arith -Wcast-align -Wcast-qual
    -fno-rtti
  )

  #-Werror not possible because of warnings caused by clang

  if(ARG_PRIVATE_FLAGS)
    target_compile_options(${target} PRIVATE
      "${ARG_PRIVATE_FLAGS}"
    )
  endif()

  if(ARG_PUBLIC_FLAGS)
    target_compile_options(${target} PUBLIC
      "${ARG_PUBLIC_FLAGS}"
    )
  endif()
endfunction()


# Clang tidy
find_program(CLANG_TIDY
  NAMES clang-tidy clang-tidy-8 clang-tidy-7 clang-tidy-6.0
)


function(register_to_clang_tidy target)
  set_target_properties(${target}
    PROPERTIES
      CXX_CLANG_TIDY ${CLANG_TIDY}
  )
endfunction()
