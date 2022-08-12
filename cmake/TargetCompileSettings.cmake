
# CMakeshift 4.0
# TargetCompileSettings.cmake
# Author: Moritz Beutel


# 1. Adjust CMake's default behavior
# ==================================

# Fall back to linking Release builds of library targets when building with MinSizeRel or RelWithDebInfo
if(NOT DEFINED CMAKE_MAP_IMPORTED_CONFIG_MINSIZEREL)
    set(CMAKE_MAP_IMPORTED_CONFIG_MINSIZEREL "MinSizeRel;Release")
endif()
if(NOT DEFINED CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO)
    set(CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO "RelWithDebInfo;Release")
endif()

# Sanity check: no in-source builds
if(NOT COMMAND cmakeshift_target_compile_settings)
    if(NOT CMAKESHIFT_PERMIT_IN_SOURCE_BUILD)
        get_filename_component(_CMAKESHIFT_SOURCE_DIR "${CMAKE_SOURCE_DIR}" REALPATH)
        get_filename_component(_CMAKESHIFT_BINARY_DIR "${CMAKE_BINARY_DIR}" REALPATH)
        if("${_CMAKESHIFT_SOURCE_DIR}" STREQUAL "${_CMAKESHIFT_BINARY_DIR}")
            message(WARNING "cmakeshift_target_compile_settings() sanity check: the project source directory is identical to the build directory. \
    This practice is discouraged. \
    Delete all build artifacts in the source directory (CMakeCache.txt, CMakeFiles/, cmake_install.cmake) and configure the project again with a different build directory. \
    If you need to build in-source, you can suppress this warning by setting CMAKESHIFT_PERMIT_IN_SOURCE_BUILD=ON.")
        endif()
    endif()
endif()

# Sanity check: if all of `CMAKE_*_FLAGS_*` are defined but empty, CMake had failed to find the compiler in the first run.
# This is dangerous because, if a subsequent run has found the compiler, the default compiler flags will now silently be missing. 
# The recommended way out is to purge the build directory and to rebuild.
if(NOT COMMAND cmakeshift_target_compile_settings)
    if(NOT CMAKESHIFT_PERMIT_EMPTY_FLAGS)
        foreach(LANG IN ITEMS C CXX CUDA)
            if(DEFINED CMAKE_${LANG}_COMPILER)
                set(_CMAKESHIFT_SANITYCHECK_PASS FALSE)
                foreach(CFG IN ITEMS DEBUG MINSIZEREL RELEASE RELWITHDEBINFO)
                    if(NOT DEFINED CMAKE_${LANG}_FLAGS_${CFG} OR NOT "${CMAKE_${LANG}_FLAGS_${CFG}}" STREQUAL "")
                        set(_CMAKESHIFT_SANITYCHECK_PASS TRUE)
                        break()
                    endif()
                endforeach()
                if(NOT _CMAKESHIFT_SANITYCHECK_PASS)
                    message(WARNING "cmakeshift_target_compile_settings() sanity check: The default compile flags for ${LANG} are empty. \
    This usually happens if CMake fails to find a particular compiler during configuration. \
    Please purge the build directory, then make sure the compilers are available and configure the project again. \
    If you need to build with empty default compile flags, you can disable this sanity check by setting CMAKESHIFT_PERMIT_EMPTY_FLAGS=ON.")
                endif()
            endif()
        endforeach()
    endif()
endif()

# MSVC workaround: CMake's default options for the Debug build configuration for MSVC on Windows include the "/RTC1" flag which
# enables a number of runtime checks:
# https://docs.microsoft.com/en-us/cpp/build/reference/rtc-run-time-error-checks
# This flag currently isn't compatible with AddressSanitizer, cf.
# https://developercommunity.visualstudio.com/content/problem/1144217/runtime-checks-and-address-sanitizer-are-disabled.html
# To avoid the warning, we implicitly disable runtime checks by excising the flag from the `CMAKE_CXX_FLAGS_<config>`
# variable if ASan is enabled for the given target.
# Also, ASan is currently not compatible with the "/INCREMENTAL" linker flag, which is set by default in
# `CMAKE_EXE_LINKER_FLAGS[_<config>]`, `CMAKE_SHARED_LINKER_FLAGS[_<config>]`, and `CMAKE_MODULE_LINKER_FLAGS[_<config>]` for
# some configurations. We therefore force "/INCREMENTAL:NO" for all configurations if ASan is used.
if(NOT DEFINED _cmakeshift_compile_options)
    set(_cmakeshift_compile_options "")
    set(_cmakeshift_link_options "")

    set(_cmakeshift_sanitize_expr "$<NOT:$<IN_LIST:address,$<GENEX_EVAL:$<TARGET_PROPERTY:CMAKESHIFT_SANITIZE>>>>")
    set(_cmakeshift_passthrough_C "")
    set(_cmakeshift_passthrough_CXX "")
    set(_cmakeshift_passthrough_CUDA "-Xcompiler=")

    foreach(_cmakeshift_lang IN ITEMS "C" "CXX" "CUDA")
        if(CMAKE_${_cmakeshift_lang}_FLAGS MATCHES "(/RTC[1csu])")
            list(APPEND _cmakeshift_compile_options "$<$<AND:$<COMPILE_LANGUAGE:${_cmakeshift_lang}>,${_cmakeshift_sanitize_expr}>:${_cmakeshift_passthrough_${_cmakeshift_lang}}${CMAKE_MATCH_1}>")
            string(REGEX REPLACE "/RTC[1csu]" " " CMAKE_${_cmakeshift_lang}_FLAGS "${CMAKE_${_cmakeshift_lang}_FLAGS}")
        endif()
        foreach(_cmakeshift_config IN ITEMS "DEBUG" "RELEASE" "RELWITHDEBINFO" "MINSIZEREL")
            if(CMAKE_${_cmakeshift_lang}_FLAGS_${_cmakeshift_config} MATCHES "(/RTC[1csu])")
                list(APPEND _cmakeshift_compile_options "$<$<AND:$<CONFIG:${_cmakeshift_config}>,$<COMPILE_LANGUAGE:${_cmakeshift_lang}>,${_cmakeshift_sanitize_expr}>:${_cmakeshift_passthrough_${_cmakeshift_lang}}${CMAKE_MATCH_1}>")
                string(REGEX REPLACE "/RTC[1csu]" " " CMAKE_${_cmakeshift_lang}_FLAGS_${_cmakeshift_config} "${CMAKE_${_cmakeshift_lang}_FLAGS_${_cmakeshift_config}}")
            endif()
        endforeach()
    endforeach()
    unset(_cmakeshift_lang)

    set(_cmakeshift_cond "1")
    if(CMAKE_EXE_LINKER_FLAGS MATCHES "(/INCREMENTAL$|/INCREMENTAL )")
        set(_cmakeshift_cond "${_cmakeshift_cond},$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>")
        string(REGEX REPLACE "(/INCREMENTAL$|/INCREMENTAL )" " " CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
    endif()
    if(CMAKE_SHARED_LINKER_FLAGS MATCHES "(/INCREMENTAL$|/INCREMENTAL )")
        set(_cmakeshift_cond "${_cmakeshift_cond},$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>")
        string(REGEX REPLACE "(/INCREMENTAL$|/INCREMENTAL )" " " CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
    endif()
    if(CMAKE_MODULE_LINKER_FLAGS MATCHES "(/INCREMENTAL$|/INCREMENTAL )")
        set(_cmakeshift_cond "${_cmakeshift_cond},$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>")
        string(REGEX REPLACE "(/INCREMENTAL$|/INCREMENTAL )" " " CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS}")
    endif()
    foreach(_cmakeshift_config IN ITEMS "DEBUG" "RELEASE" "RELWITHDEBINFO" "MINSIZEREL")
        set(_cmakeshift_subcond "1")
        if(CMAKE_EXE_LINKER_FLAGS_${_cmakeshift_config} MATCHES "(/INCREMENTAL$|/INCREMENTAL )")
            set(_cmakeshift_subcond "${_cmakeshift_subcond},$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>")
            string(REGEX REPLACE "(/INCREMENTAL$|/INCREMENTAL )" " " CMAKE_EXE_LINKER_FLAGS_${_cmakeshift_config} "${CMAKE_EXE_LINKER_FLAGS_${_cmakeshift_config}}")
        endif()
        if(CMAKE_SHARED_LINKER_FLAGS_${_cmakeshift_config} MATCHES "(/INCREMENTAL$|/INCREMENTAL )")
            set(_cmakeshift_subcond "${_cmakeshift_subcond},$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>")
            string(REGEX REPLACE "(/INCREMENTAL$|/INCREMENTAL )" " " CMAKE_SHARED_LINKER_FLAGS_${_cmakeshift_config} "${CMAKE_SHARED_LINKER_FLAGS_${_cmakeshift_config}}")
        endif()
        if(CMAKE_MODULE_LINKER_FLAGS_${_cmakeshift_config} MATCHES "(/INCREMENTAL$|/INCREMENTAL )")
            set(_cmakeshift_subcond "${_cmakeshift_subcond},$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>")
            string(REGEX REPLACE "(/INCREMENTAL$|/INCREMENTAL )" " " CMAKE_MODULE_LINKER_FLAGS_${_cmakeshift_config} "${CMAKE_MODULE_LINKER_FLAGS_${_cmakeshift_config}}")
        endif()
        if(NOT _cmakeshift_subcond STREQUAL "1")
            set(_cmakeshift_cond "${_cmakeshift_cond},$<AND:$<CONFIG:${_cmakeshift_config}>,$<OR:${_cmakeshift_subcond}>>")
        endif()
    endforeach()
    if(NOT _cmakeshift_cond STREQUAL "1")
        list(APPEND _cmakeshift_link_options "$<$<OR:${_cmakeshift_cond}>:$<IF:${_cmakeshift_sanitize_expr},/INCREMENTAL,/INCREMENTAL:NO>>")
    endif()
    unset(_cmakeshift_config)
    unset(_cmakeshift_cond)
    unset(_cmakeshift_subcond)
    unset(_cmakeshift_sanitize_expr)
endif()


if(COMMAND cmakeshift_target_compile_settings)
    return()
endif()


# 2. Define compile settings as target properties
# ===============================================

define_property(TARGET
    PROPERTY CMAKESHIFT_WARNING_LEVEL INHERITED
    BRIEF_DOCS "compiler warning level (default, high)"
    FULL_DOCS "compiler warning level"
)

define_property(TARGET
    PROPERTY CMAKESHIFT_FP_MODEL INHERITED
    BRIEF_DOCS "floating-point model (default, strict, consistent, precise, fast, fastest)"
    FULL_DOCS
        "Supported values:"
        "default    - compiler default setting (equivalent to 'precise' for most compilers, 'fast' for ICC)"
        "strict     - Value safety, no contractions (e.g. fused multiply–add), precise FP exceptions"
        "consistent - Value safety, no contractions (e.g. fused multiply–add)"
        "precise    - Value safety"
        "fast       - Permit optimizations affecting value safety"
        "fastest    - Permit aggressive optimizations affecting value safety"
)

define_property(TARGET
    PROPERTY CMAKESHIFT_DEBUG_DEVICE_CODE INHERITED
    BRIEF_DOCS "generate debuggable device code (ON, OFF)"
    FULL_DOCS "(setting this option suppresses most device-code optimizations and should only be used in debug builds)"
)

define_property(TARGET
    PROPERTY CMAKESHIFT_SOURCE_FILE_ENCODING INHERITED
    BRIEF_DOCS "text encoding of source files (default, UTF-8)"
    FULL_DOCS "Some compilers default to a system-specific text encoding. Use this setting to force compilers to consider \
source files as UTF-8-encoded. This is crucial when using Unicode characters in string literals."
)

define_property(TARGET
    PROPERTY CMAKESHIFT_SANITIZE INHERITED
    BRIEF_DOCS "which runtime sanitizers to enable (address, undefined)"
    FULL_DOCS "Use this setting to enable runtime sanitizers (AddressSanitizer, UndefinedBehaviorSanitizer). \
Note that, for contiuous testing, sanitizers should be configured for the entire project (including dependencies) using a \
toolchain file."
)


# 3. Define `cmakeshift_target_compile_settings()`
# ================================================

set(CMAKESHIFT_TRACE_OUTPUT OFF CACHE BOOL "Enable trace output for CMakeshift routines")
mark_as_advanced(CMAKESHIFT_TRACE_OUTPUT)

function(CMAKESHIFT_TARGET_COMPILE_SETTINGS)

    # Define generator expressions for language/compiler identification

    ## Filter unnecessary generator expressions by checking if the compiler in question is involved at all
    set(ANY_MSVC FALSE)
    set(ANY_GNU FALSE)
    set(ANY_CLANG FALSE)
    set(ANY_NVIDIA FALSE)
    set(ANY_INTEL FALSE)

    set(C_EXPR "$<COMPILE_LANGUAGE:C>")
    set(HOST_EXPR "$<OR:$<COMPILE_LANGUAGE:CXX>,$<AND:$<COMPILE_LANGUAGE:CUDA>,$<CUDA_COMPILER_ID:NVIDIA>>>")
    
    ## MSVC or clang-cl
    #set(MSVC_C_EXPR "$<OR:$<COMPILE_LANG_AND_ID:C,MSVC>,$<AND:$<COMPILE_LANG_AND_ID:C,Clang>,$<STREQUAL:x${CMAKE_C_SIMULATE_ID},xMSVC>>>")
    #set(MSVC_CXX_EXPR "$<OR:$<COMPILE_LANG_AND_ID:CXX,MSVC>,$<AND:$<COMPILE_LANG_AND_ID:CXX,Clang>,$<STREQUAL:x${CMAKE_CXX_SIMULATE_ID},xMSVC>>>")
    #set(MSVC_EXPR "$<OR:${MSVC_C_EXPR},${MSVC_CXX_EXPR}>")
    set(MSVC_C_EXPR "$<OR:$<C_COMPILER_ID:MSVC>,$<AND:$<C_COMPILER_ID:Clang>,$<STREQUAL:x${CMAKE_C_SIMULATE_ID},xMSVC>>>")
    set(MSVC_CXX_EXPR "$<OR:$<CXX_COMPILER_ID:MSVC>,$<AND:$<CXX_COMPILER_ID:Clang>,$<STREQUAL:x${CMAKE_CXX_SIMULATE_ID},xMSVC>>>")
    set(MSVC_EXPR "$<OR:$<AND:$<COMPILE_LANGUAGE:C>,${MSVC_C_EXPR}>,$<AND:$<COMPILE_LANGUAGE:CXX>,${MSVC_CXX_EXPR}>>")
    set(MSVC_CXX_HOST_EXPR "$<AND:${HOST_EXPR},${MSVC_CXX_EXPR}>")
    set(MSVC_HOST_EXPR "$<OR:$<AND:$<COMPILE_LANGUAGE:C>,${MSVC_C_EXPR}>,$<AND:${HOST_EXPR},${MSVC_CXX_EXPR}>>")
    if(MSVC)
        set(ANY_MSVC TRUE)
    endif()

    ## GCC
    #set(GNU_C_EXPR "$<COMPILE_LANG_AND_ID:C,GNU>")
    #set(GNU_CXX_EXPR "$<COMPILE_LANG_AND_ID:CXX,GNU>")
    #set(GNU_EXPR "$<OR:${GNU_C_EXPR},${GNU_CXX_EXPR}>")
    set(GNU_C_EXPR "$<C_COMPILER_ID:GNU>")
    set(GNU_CXX_EXPR "$<CXX_COMPILER_ID:GNU>")
    set(GNU_EXPR "$<OR:$<AND:$<COMPILE_LANGUAGE:C>,${GNU_C_EXPR}>,$<AND:$<COMPILE_LANGUAGE:CXX>,${GNU_CXX_EXPR}>>")
    set(GNU_HOST_EXPR "$<OR:$<AND:$<COMPILE_LANGUAGE:C>,${GNU_C_EXPR}>,$<AND:${HOST_EXPR},${GNU_CXX_EXPR}>>")
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(ANY_GNU TRUE)
    endif()

    ## Clang (other than clang-cl)
    set(CLANG_C_EXPR "$<AND:$<NOT:$<STREQUAL:x$<FILTER:$<C_COMPILER_ID>,INCLUDE,Clang>,x>>,$<NOT:$<STREQUAL:x${CMAKE_C_SIMULATE_ID},xMSVC>>>")
    set(CLANG_CXX_EXPR "$<AND:$<NOT:$<STREQUAL:x$<FILTER:$<CXX_COMPILER_ID>,INCLUDE,Clang>,x>>,$<NOT:$<STREQUAL:x${CMAKE_CXX_SIMULATE_ID},xMSVC>>>")
    set(CLANG_CUDA_EXPR "$<NOT:$<STREQUAL:x$<FILTER:$<CUDA_COMPILER_ID>,INCLUDE,Clang>,x>>")
    #set(CLANG_EXPR "$<OR:${CLANG_C_EXPR},${CLANG_CXX_EXPR},${CLANG_CUDA_EXPR}>")
    set(CLANG_EXPR "$<OR:$<AND:$<COMPILE_LANGUAGE:C>,${CLANG_C_EXPR}>,$<AND:$<COMPILE_LANGUAGE:CXX>,${CLANG_CXX_EXPR}>,$<AND:$<COMPILE_LANGUAGE:CUDA>,${CLANG_CUDA_EXPR}>>")
    set(CLANG_HOST_EXPR "$<OR:$<AND:$<COMPILE_LANGUAGE:C>,${CLANG_C_EXPR}>,$<AND:${HOST_EXPR},${CLANG_CXX_EXPR}>>")
    if(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CUDA_COMPILER_ID MATCHES "Clang")
        set(ANY_CLANG TRUE)
    endif()

    ## Any Clang (also clang-cl)
    set(ANYCLANG_C_EXPR "$<NOT:$<STREQUAL:x$<FILTER:$<C_COMPILER_ID>,INCLUDE,Clang>,x>>")
    set(ANYCLANG_CXX_EXPR "$<NOT:$<STREQUAL:x$<FILTER:$<CXX_COMPILER_ID>,INCLUDE,Clang>,x>>")
    set(ANYCLANG_CUDA_EXPR "$<NOT:$<STREQUAL:x$<FILTER:$<CUDA_COMPILER_ID>,INCLUDE,Clang>,x>>")
    set(ANYCLANG_EXPR "$<OR:${ANYCLANG_C_EXPR},${ANYCLANG_CXX_EXPR},${ANYCLANG_CUDA_EXPR}>")
    set(ANYCLANG_EXPR "$<OR:$<AND:$<COMPILE_LANGUAGE:C>,${ANYCLANG_C_EXPR}>,$<AND:$<COMPILE_LANGUAGE:CXX>,${ANYCLANG_CXX_EXPR}>,$<AND:$<COMPILE_LANGUAGE:CUDA>,${ANYCLANG_CUDA_EXPR}>>")
    set(ANYCLANG_HOST_EXPR "$<OR:$<AND:$<COMPILE_LANGUAGE:C>,${ANYCLANG_C_EXPR}>,$<AND:${HOST_EXPR},${ANYCLANG_CXX_EXPR}>>")

    ## NVCC
    set(NVIDIA_EXPR "$<COMPILE_LANG_AND_ID:CUDA,NVIDIA>")
    #set(NVIDIA_GNU_EXPR "$<AND:${NVIDIA_EXPR},$<STREQUAL:x$<CXX_COMPILER_ID>,xGNU>>")
    #set(NVIDIA_MSVC_EXPR "$<AND:${NVIDIA_EXPR},$<OR:$<STREQUAL:x$<CXX_COMPILER_ID>,xMSVC>,$<AND:$<NOT:$<STREQUAL:x$<FILTER:$<CXX_COMPILER_ID>,INCLUDE,Clang>,x>>,$<STREQUAL:x${CMAKE_CXX_SIMULATE_ID},xMSVC>>>>")
    #set(NVIDIA_CLANG_EXPR "$<AND:${NVIDIA_EXPR},$<NOT:$<STREQUAL:x$<FILTER:$<CXX_COMPILER_ID>,INCLUDE,Clang>,x>>,$<NOT:$<STREQUAL:x${CMAKE_CXX_SIMULATE_ID},xMSVC>>>")
    #set(NVIDIA_ANYCLANG_EXPR "$<AND:${NVIDIA_EXPR},$<NOT:$<STREQUAL:x$<FILTER:$<CXX_COMPILER_ID>,INCLUDE,Clang>,x>>>")
    if(CMAKE_CUDA_COMPILER_ID STREQUAL "NVIDIA")
        set(ANY_NVIDIA TRUE)
    endif()

    ## ICC
    set(INTEL_C_EXPR "$<C_COMPILER_ID:Intel>")
    set(INTEL_CXX_EXPR "$<CXX_COMPILER_ID:Intel>")
    set(INTEL_EXPR "$<OR:$<AND:$<COMPILE_LANGUAGE:C>,${INTEL_C_EXPR}>,$<AND:$<COMPILE_LANGUAGE:CXX>,${INTEL_CXX_EXPR}>>")
    #set(INTEL_C_EXPR "$<COMPILE_LANG_AND_ID:C,Intel>")
    #set(INTEL_CXX_EXPR "$<COMPILE_LANG_AND_ID:CXX,Intel>")
    #set(INTEL_EXPR "$<OR:${INTEL_C_EXPR},${INTEL_CXX_EXPR}>")
    if(WIN32)
        set(INTEL_OPT "/")
        set(INTEL_FP_MODEL_OPT "/fp:")
    else()
        set(INTEL_OPT "-")
        set(INTEL_FP_MODEL_OPT "-fp-model ")
    endif()
    if(CMAKE_C_COMPILER_ID STREQUAL "Intel" OR CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
        set(ANY_INTEL TRUE)
    endif()

    # Detect enabled languages and set flags.
    set(HAVE_C FALSE)
    set(HAVE_CXX FALSE)
    set(HAVE_CUDA FALSE)
    get_property(_ENABLED_LANGUAGES GLOBAL PROPERTY ENABLED_LANGUAGES)
    if(C IN_LIST _ENABLED_LANGUAGES)
        set(HAVE_C TRUE)
    endif()
    if(CXX IN_LIST _ENABLED_LANGUAGES)
        set(HAVE_CXX TRUE)
    endif()
    if(CUDA IN_LIST _ENABLED_LANGUAGES)
        set(HAVE_CUDA TRUE)
    endif()

    # Set variable PASSTHROUGH to support the NVCC compiler driver.
    set(PASSTHROUGH "")
    if(HAVE_CUDA)
        if(CMAKE_CUDA_COMPILER_ID MATCHES "NVIDIA")
            set(PASSTHROUGH "$<$<COMPILE_LANGUAGE:CUDA>:-Xcompiler=>")
        endif()
    endif()

    if(CMAKESHIFT_TRACE_OUTPUT)
        message("[cmakeshift_target_compile_settings()] ANY_MSVC: ${ANY_MSVC}")
        message("[cmakeshift_target_compile_settings()] ANY_GNU: ${ANY_GNU}")
        message("[cmakeshift_target_compile_settings()] ANY_CLANG: ${ANY_CLANG}")
        message("[cmakeshift_target_compile_settings()] ANY_NVIDIA: ${ANY_NVIDIA}")
        message("[cmakeshift_target_compile_settings()] ANY_INTEL: ${ANY_INTEL}")
    endif()

    function(CMAKESHIFT_TARGET_COMPILE_SETTINGS_IMPL_ TARGET)
        # Set all available settings with generator expression:

        ## Warning level
        set(HIGH_WARNING_LEVEL_EXPR "$<STREQUAL:x$<GENEX_EVAL:$<TARGET_PROPERTY:CMAKESHIFT_WARNING_LEVEL>>,xhigh>")
        if(ANY_MSVC)
            # highest warning level
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${HIGH_WARNING_LEVEL_EXPR},${MSVC_HOST_EXPR}>:${PASSTHROUGH}/W4>")
            # enumerator 'identifier' in a switch of enum 'enumeration' is not handled
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${HIGH_WARNING_LEVEL_EXPR},${MSVC_HOST_EXPR}>:${PASSTHROUGH}/w44062>")
            # 'identifier': conversion from 'type1' to 'type2', possible loss of data
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${HIGH_WARNING_LEVEL_EXPR},${MSVC_HOST_EXPR}>:${PASSTHROUGH}/w44242>")
            # 'operator': conversion from 'type1' to 'type2', possible loss of data
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${HIGH_WARNING_LEVEL_EXPR},${MSVC_HOST_EXPR}>:${PASSTHROUGH}/w44254>")
            # 'class': class has virtual functions, but destructor is not virtual
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${HIGH_WARNING_LEVEL_EXPR},${MSVC_HOST_EXPR}>:${PASSTHROUGH}/w44265>")
        endif()
        if(ANY_GNU)
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${HIGH_WARNING_LEVEL_EXPR},${GNU_HOST_EXPR}>:${PASSTHROUGH}-Wall>")
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${HIGH_WARNING_LEVEL_EXPR},${GNU_HOST_EXPR}>:${PASSTHROUGH}-Wextra>")
            # The code generated by NVCC isn't "-pedantic"-clean. Specifically, it may generate preprocessor line statements as "#123",
            # which makes GCC warn that the "style of line directive is a GCC extension". This was observed in conjunction with the
            # "--generate-line-info" flag. We thus avoid "-pedantic" for all CUDA source files for the sake of consistency.
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${HIGH_WARNING_LEVEL_EXPR},${GNU_EXPR}>:-pedantic>")
        endif()
        if(ANY_CLANG)
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${HIGH_WARNING_LEVEL_EXPR},${CLANG_HOST_EXPR}>:${PASSTHROUGH}-Wall>")
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${HIGH_WARNING_LEVEL_EXPR},${CLANG_HOST_EXPR}>:${PASSTHROUGH}-Wextra>")
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${HIGH_WARNING_LEVEL_EXPR},${CLANG_HOST_EXPR}>:${PASSTHROUGH}-pedantic>")
        endif()
        if(ANY_INTEL)
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${HIGH_WARNING_LEVEL_EXPR},${INTEL_EXPR}>:${INTEL_OPT}Wall>")
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${HIGH_WARNING_LEVEL_EXPR},${INTEL_EXPR}>:${INTEL_OPT}Wextra>")
        endif()

        ## Floating-point model
        set(FP_MODEL_EXPR "$<GENEX_EVAL:$<TARGET_PROPERTY:CMAKESHIFT_FP_MODEL>>")
        if(ANY_MSVC)
            target_compile_options(${TARGET} PRIVATE "$<$<AND:$<IN_LIST:X${FP_MODEL_EXPR},Xstrict;Xconsistent>,${MSVC_HOST_EXPR}>:${PASSTHROUGH}/fp:strict>")
            target_compile_options(${TARGET} PRIVATE "$<$<AND:$<STREQUAL:X${FP_MODEL_EXPR},Xprecise>,${MSVC_HOST_EXPR}>:${PASSTHROUGH}/fp:precise>")
            target_compile_options(${TARGET} PRIVATE "$<$<AND:$<IN_LIST:X${FP_MODEL_EXPR},Xfast;Xfastest>,${MSVC_HOST_EXPR}>:${PASSTHROUGH}/fp:fast>")
        endif()
        if(ANY_GNU)
            target_compile_options(${TARGET} PRIVATE "$<$<AND:$<IN_LIST:X${FP_MODEL_EXPR},Xstrict;Xconsistent>,${GNU_HOST_EXPR}>:${PASSTHROUGH}-ffp-contract=off>")
            target_compile_options(${TARGET} PRIVATE "$<$<AND:$<STREQUAL:X${FP_MODEL_EXPR},Xfast>,${GNU_HOST_EXPR}>:${PASSTHROUGH}-funsafe-math-optimizations>")
            target_compile_options(${TARGET} PRIVATE "$<$<AND:$<STREQUAL:X${FP_MODEL_EXPR},Xfastest>,${GNU_HOST_EXPR}>:${PASSTHROUGH}-ffast-math>")
        endif()
        if(ANY_CLANG)
            target_compile_options(${TARGET} PRIVATE "$<$<AND:$<IN_LIST:X${FP_MODEL_EXPR},Xfast;Xfastest>,${CLANG_HOST_EXPR}>:${PASSTHROUGH}-ffast-math>")
        endif()
        if(ANY_NVCC)
            target_compile_options(${TARGET} PRIVATE "$<$<AND:$<IN_LIST:X${FP_MODEL_EXPR},Xstrict;Xconsistent>,${NVIDIA_EXPR}>:--fmad=false>")  # do not fuse multiplications and additions
            target_compile_options(${TARGET} PRIVATE "$<$<AND:$<STREQUAL:X${FP_MODEL_EXPR},Xfast>,${NVIDIA_EXPR}>:--ftz=true>")
            target_compile_options(${TARGET} PRIVATE "$<$<AND:$<STREQUAL:X${FP_MODEL_EXPR},Xfast>,${NVIDIA_EXPR}>:--prec-div=false>")
            target_compile_options(${TARGET} PRIVATE "$<$<AND:$<STREQUAL:X${FP_MODEL_EXPR},Xfast>,${NVIDIA_EXPR}>:--prec-sqrt=false>")
            target_compile_options(${TARGET} PRIVATE "$<$<AND:$<IN_LIST:X${FP_MODEL_EXPR},Xfast;Xfastest>,${NVIDIA_EXPR}>:--use_fast_math>")  # implies everything in "fast" above
        endif()
        if(ANY_INTEL)
            target_compile_options(${TARGET} PRIVATE "$<$<AND:$<IN_LIST:X${FP_MODEL_EXPR},Xstrict;Xconsistent;Xprecise;Xfast>,${INTEL_EXPR}>,${INTEL_FP_MODEL_OPT}${FP_MODEL_EXPR}>")
            target_compile_options(${TARGET} PRIVATE "$<$<AND:$<STREQUAL:X${FP_MODEL_EXPR},Xfastest>,${INTEL_EXPR}>:${INTEL_FP_MODEL_OPT}fast=2>")
        endif()

        ## Device code debugging
        if(ANY_NVIDIA)
            set(DEBUG_DEVICE_CODE_EXPR "$<BOOL:$<GENEX_EVAL:$<TARGET_PROPERTY:CMAKESHIFT_DEBUG_DEVICE_CODE>>>")
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${DEBUG_DEVICE_CODE_EXPR},${NVIDIA_EXPR},$<CONFIG:Debug>>:--device-debug>")
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${DEBUG_DEVICE_CODE_EXPR},${NVIDIA_EXPR},$<CONFIG:RelWithDebInfo>>:--generate-line-info>")
        endif()

        ## Source file encoding
        if(ANY_MSVC)
            set(SOURCE_FILE_ENCODING_EXPR "$<GENEX_EVAL:$<STREQUAL:x$<TARGET_PROPERTY:CMAKESHIFT_SOURCE_FILE_ENCODING>,xUTF-8>>")
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${SOURCE_FILE_ENCODING_EXPR},${MSVC_HOST_EXPR}>:${PASSTHROUGH}/utf-8>")
        endif()

        ## Sanitizers
        set(SANITIZERS_EXPR "$<GENEX_EVAL:$<TARGET_PROPERTY:CMAKESHIFT_SANITIZE>>")
        set(ASAN_EXPR "$<IN_LIST:address,${SANITIZERS_EXPR}>")
        set(UBSAN_EXPR "$<IN_LIST:undefined,${SANITIZERS_EXPR}>")
        if(ANY_MSVC)
            ### ASan
            # MSVC has had support for Address Sanitizer for both x86 and x64 targets since VS 16.7:
            # https://devblogs.microsoft.com/cppblog/asan-for-windows-x64-and-debug-build-support/
            # Unfortunately it required tedious manual linking of certain runtime libraries. VS 2019 16.9 finally overcame
            # these obstacles, and all it takes now to enable ASan is to pass a single flag. Therefore we won't bother trying
            # to make ASan work for earlier versions.
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${ASAN_EXPR},${MSVC_HOST_EXPR}>:-fsanitize=address>")
            ### UBSan
            # MSVC currently doesn't implement UBSan, so setting this option will result in a compile error, which is intentional.
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${UBSAN_EXPR},${MSVC_HOST_EXPR}>:-fsanitize=undefined>")
        endif()
        if(ANY_GNU)
            ### ASan
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${ASAN_EXPR},${GNU_HOST_EXPR}>:-fsanitize=address>")
            target_link_options(${TARGET} PRIVATE "$<$<AND:${ASAN_EXPR},${GNU_HOST_EXPR}>:-fsanitize=address>")
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${ASAN_EXPR},${GNU_HOST_EXPR}>:-fno-omit-frame-pointer>")
            ### UBSan
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${UBSAN_EXPR},${GNU_HOST_EXPR}>:-fsanitize=undefined>")
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${UBSAN_EXPR},${GNU_HOST_EXPR}>:-fsanitize-undefined-trap-on-error>")
        endif()
        if(ANY_CLANG)
            ### ASan
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${ASAN_EXPR},${CLANG_HOST_EXPR}>:-fsanitize=address>")
            target_link_options(${TARGET} PRIVATE "$<$<AND:${ASAN_EXPR},${CLANG_HOST_EXPR}>:-fsanitize=address>")
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${ASAN_EXPR},${ANYCLANG_HOST_EXPR}>:-fno-omit-frame-pointer>")
            ### UBSan
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${UBSAN_EXPR},${ANYCLANG_HOST_EXPR}>:-fsanitize=undefined>")
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${ASAN_EXPR},${ANYCLANG_HOST_EXPR}>:-fsanitize-trap=undefined>")
            target_compile_options(${TARGET} PRIVATE "$<$<AND:${ASAN_EXPR},${ANYCLANG_HOST_EXPR}>:-fsanitize=implicit-integer-arithmetic-value-change>")
        endif()
    endfunction()

    # Parse arguments
    set(options
    )
    set(oneValueArgs
        WARNING_LEVEL
        FP_MODEL
        DEBUG_DEVICE_CODE
        SOURCE_FILE_ENCODING
    )
    set(multiValueArgs
        TARGETS
        SANITIZE
    )
    cmake_parse_arguments(PARSE_ARGV 0 "SCOPE" "${options}" "${oneValueArgs}" "${multiValueArgs}")
    if(SCOPE_UNPARSED_ARGUMENTS)
        list(LENGTH SCOPE_UNPARSED_ARGUMENTS _NUM_UNPARSED_ARGUMENTS)
        if(_NUM_UNPARSED_ARGUMENTS GREATER_EQUAL 1)
            if(SCOPE_TARGETS)
                message(SEND_ERROR "cmakeshift_target_compile_settings(): Specify either a single target as first parameter or multiple targets with the TARGETS argument, but not both")
            else()
                list(GET SCOPE_UNPARSED_ARGUMENTS 0 SCOPE_TARGETS)
                list(REMOVE_AT SCOPE_UNPARSED_ARGUMENTS 0)
            endif()
            if(_NUM_UNPARSED_ARGUMENTS GREATER 1)
                list(JOIN SCOPE_UNPARSED_ARGUMENTS "\", \"" SCOPE_UNPARSED_ARGUMENTS_STR)
                list(JOIN "${oneValueArgs};${multiValueArgs}" "\", \"" POSSIBLE_ARGUMENTS_STR)
                message(SEND_ERROR "cmakeshift_target_compile_settings(): Invalid argument keyword(s) \"${SCOPE_UNPARSED_ARGUMENTS_STR}\"; expected one of \"${POSSIBLE_ARGUMENTS_STR}\"")
            endif()
            if(SCOPE_KEYWORDS_MISSING_VALUES)
                list(JOIN SCOPE_KEYWORDS_MISSING_VALUES "\", \"" SCOPE_KEYWORDS_MISSING_VALUES_STR)
                message(SEND_ERROR "cmakeshift_target_compile_settings(): argument keyword(s) \"${SCOPE_KEYWORDS_MISSING_VALUES_STR}\" missing values")
            endif()
        endif()
    endif()
    if(NOT SCOPE_TARGETS)
        message(SEND_ERROR "cmakeshift_target_compile_settings(): No target given; specify either a single target as first parameter or multiple targets with the TARGETS argument")
    endif()

    # Set target properties
    if(SCOPE_WARNING_LEVEL)
        set_target_properties(${SCOPE_TARGETS} PROPERTIES CMAKESHIFT_WARNING_LEVEL "${SCOPE_WARNING_LEVEL}")
    elseif(DEFINED CMAKESHIFT_WARNING_LEVEL)
        set_target_properties(${SCOPE_TARGETS} PROPERTIES CMAKESHIFT_WARNING_LEVEL "${CMAKESHIFT_WARNING_LEVEL}")
    endif()
    if(SCOPE_FP_MODEL)
        set_target_properties(${SCOPE_TARGETS} PROPERTIES CMAKESHIFT_FP_MODEL "${SCOPE_FP_MODEL}")
    elseif(DEFINED CMAKESHIFT_FP_MODEL)
        set_target_properties(${SCOPE_TARGETS} PROPERTIES CMAKESHIFT_FP_MODEL "${CMAKESHIFT_FP_MODEL}")
    endif()
    if(SCOPE_DEBUG_DEVICE_CODE)
        set_target_properties(${SCOPE_TARGETS} PROPERTIES CMAKESHIFT_DEBUG_DEVICE_CODE "${SCOPE_DEBUG_DEVICE_CODE}")
    elseif(DEFINED CMAKESHIFT_DEBUG_DEVICE_CODE)
        set_target_properties(${SCOPE_TARGETS} PROPERTIES CMAKESHIFT_DEBUG_DEVICE_CODE "${CMAKESHIFT_DEBUG_DEVICE_CODE}")
    endif()
    if(SCOPE_SOURCE_FILE_ENCODING)
        set_target_properties(${SCOPE_TARGETS} PROPERTIES CMAKESHIFT_SOURCE_FILE_ENCODING "${SCOPE_SOURCE_FILE_ENCODING}")
    elseif(DEFINED CMAKESHIFT_SOURCE_FILE_ENCODING)
        set_target_properties(${SCOPE_TARGETS} PROPERTIES CMAKESHIFT_SOURCE_FILE_ENCODING "${CMAKESHIFT_SOURCE_FILE_ENCODING}")
    endif()
    if(SCOPE_SANITIZE)
        set_target_properties(${SCOPE_TARGETS} PROPERTIES CMAKESHIFT_SANITIZE "${SCOPE_SANITIZE}")
    elseif(DEFINED CMAKESHIFT_SANITIZE)
        set_target_properties(${SCOPE_TARGETS} PROPERTIES CMAKESHIFT_SANITIZE "${CMAKESHIFT_SANITIZE}")
    endif()

    foreach(TARGET IN LISTS SCOPE_TARGETS)
        get_target_property(_TYPE ${TARGET} TYPE)
        if (${_TYPE} STREQUAL "INTERFACE_LIBRARY")
            message(SEND_ERROR "cmakeshift_target_compile_settings(): Cannot set compile settings on INTERFACE targets")
            return()
        endif()
    endforeach()

    if(NOT _cmakeshift_compile_options STREQUAL "")
        foreach(TARGET IN LISTS SCOPE_TARGETS)
            target_compile_options(${TARGET} PRIVATE ${_cmakeshift_compile_options})
        endforeach()
    endif()
    if(NOT _cmakeshift_link_options STREQUAL "")
        foreach(TARGET IN LISTS SCOPE_TARGETS)
            target_link_options(${TARGET} PRIVATE ${_cmakeshift_link_options})
        endforeach()
    endif()

    foreach(TARGET IN LISTS SCOPE_TARGETS)
        CMAKESHIFT_TARGET_COMPILE_SETTINGS_IMPL_(${TARGET})
    endforeach()

    SET(CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG} PARENT_SCOPE)
    SET(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG} PARENT_SCOPE)
    foreach(_target_kind IN ITEMS "EXE" "SHARED" "MODULE")
        foreach(_cfg_suffix IN ITEMS "" "_DEBUG" "_RELEASE" "_RELWITHDEBINFO" "_MINSIZEREL")
            SET(CMAKE_${_target_kind}_LINKER_FLAGS${_cfg_suffix} ${CMAKE_${_target_kind}_LINKER_FLAGS${_cfg_suffix}} PARENT_SCOPE)
        endforeach()
    endforeach()
endfunction()
