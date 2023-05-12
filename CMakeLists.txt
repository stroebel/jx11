cmake_minimum_required(VERSION 3.15)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13 CACHE STRING "Minimum MacOS target")
set(CMAKE_XCODE_GENERATE_SCHEME OFF)


project(JX11 VERSION 0.0.1 LANGUAGES C CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS YES)
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Show all module sources in IDE projects" ON)

add_subdirectory(JUCE)

set(FORMATS AU VST3 AUv3 Standalone)

juce_add_plugin(
  "${PROJECT_NAME}"
  COPY_PLUGIN_AFTER_BUILD TRUE
  PLUGIN_MANUFACTURER_CODE Manu
  PLUGIN_CODE JX11
  FORMATS VST3 "${FORMATS}"
  PRODUCT_NAME "${PROJECT_NAME}"
  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  VST3_CATEGORIES Instrument Synth
  AU_MAIN_TYPE kAudioUnitType_MusicDevice)

# Global compiler/linker settings {{{
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND UNIX AND NOT APPLE AND NOT SURGE_SKIP_PIE_CHANGE)
  message(STATUS "Setting -no-pie on EXE flags; use SURGE_SKIP_PIE_CHANGE=TRUE to avoid" )
  set(CMAKE_EXE_LINKER_FLAGS "-no-pie")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  if (${SURGE_SANITIZE})
    message(STATUS "Sanitizer is on" )
  endif()
  # Any Clang or any GCC
  add_compile_options(
    -Wno-multichar
    # Targeting Windows with GCC/Clang is experimental
    $<$<NOT:$<BOOL:${WIN32}>>:-Werror>

    # PE/COFF doesn't support visibility
    $<$<NOT:$<BOOL:${WIN32}>>:-fvisibility=hidden>
    # Inlines visibility is only relevant with C++
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<COMPILE_LANGUAGE:CXX>>:-fvisibility-inlines-hidden>

    # BP note: If you want to turn on llvm/gcc sanitize undo this and the link options below
    $<$<BOOL:${SURGE_SANITIZE}>:-fsanitize=address>
    $<$<BOOL:${SURGE_SANITIZE}>:-fsanitize=undefined>
  )

  add_link_options(
    $<$<BOOL:${SURGE_SANITIZE}>:-fsanitize=address>
    $<$<BOOL:${SURGE_SANITIZE}>:-fsanitize=undefined>
  )

  # Enable SSE2 on x86-32 only. It's implied on x86-64 and N/A elsewhere.
  if(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    include(CheckCXXSourceCompiles)
    check_cxx_source_compiles("#ifndef __i386__
    #error
    #endif
    int main() {}" SURGE_ARCH_I386)
    if(SURGE_ARCH_I386)
      add_compile_options(-msse2 -mfpmath=sse)
    endif()
  endif()

  # Add AVX support
  include(CheckCXXSourceCompiles)
  check_cxx_source_compiles("
#if defined(__x86_64__) || defined(__SSE2__) || defined(_M_AMD64) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
  #ifndef __AVX__
  #error
  #endif
#endif
    int main() {}" COMPILER_HAS_AVX_OR_IS_ARM)
  if(NOT COMPILER_HAS_AVX_OR_IS_ARM)
    message(STATUS "Holding off on AVX support. See #4479 for the strategy")
    # add_compile_options("-mavx")
  endif()

  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Any Clang
    add_compile_options(
      -Wno-unused-command-line-argument
      -Wno-deprecated-declarations
      -Werror=inconsistent-missing-override
      -Werror=logical-op-parentheses
      -Werror=dynamic-class-memaccess
      -Werror=undefined-bool-conversion
      -Werror=bitwise-op-parentheses
      -Werror=pointer-bool-conversion
    )
    if(CMAKE_CXX_COMPILER_ID MATCHES "^AppleClang$")
      # Apple Clang only
      add_compile_options(
        -faligned-allocation
        -fasm-blocks
      )
    endif()
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "^GNU$")
    # GCC only
    add_compile_options(
      -Wformat-truncation=0 # squelch warning about snprintf truncating strings (see PR #3977)
      -Wno-free-nonheap-object # https://github.com/surge-synthesizer/surge/issues/4251
      -Wno-return-local-addr # squelch sqlite3 error: function may return address of local variable
    )
  endif()
endif()

file(GLOB_RECURSE SourceFiles CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
target_sources("${PROJECT_NAME}" PRIVATE ${SourceFiles})

set_target_properties("${PROJECT_NAME}" PROPERTIES FOLDER "")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX "" FILES ${SourceFiles})

foreach(target ${FORMATS} "All")
  if(TARGET ${PROJECT_NAME}_${target})
    set_target_properties(${PROJECT_NAME}_${target} PROPERTIES
      FOLDER "Targets"
      XCODE_SCHEME_EXECUTABLE "~/JUCE/extras/AudioPluginHost/Builds/MacOSX/build/Debug/AudioPluginHost.app"
      XCODE_GENERATE_SCHEME ON)
  endif()
endforeach()

target_compile_features("${PROJECT_NAME}" PRIVATE cxx_std_20)
target_compile_definitions("${PROJECT_NAME}"
  PUBLIC
  JUCE_USE_CURL=0
  JUCE_WEB_BROWSER=0
  JUCE_USE_CAMERA=disabled
  JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries("${PROJECT_NAME}"
  PUBLIC
  juce::juce_audio_utils
  juce::juce_audio_processors
  juce::juce_dsp
  juce::juce_recommended_config_flags
  juce::juce_recommended_lto_flags
  juce::juce_recommended_warning_flags)
