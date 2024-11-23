cmake_minimum_required (VERSION 3.25)
project (libpd C)

include(CMakeDependentOption)
include(CheckLibraryExists)
include(CheckIncludeFile)

option(PD_UTILS  "Compile utilities" ON)
option(PD_EXTRA  "Compile extras" ON)
option(PD_MULTI  "Compile with multiple instance support" OFF)
option(PD_LOCALE "Set the LC_NUMERIC number format to the default C locale" ON)
option(PD_DEFINE_EXTERN "Define EXTERN to a custom string for special linking purposes" "")
option(LIBPD_STATIC "Build static library" ON)
option(LIBPD_SHARED "Build shared library" ON)
cmake_dependent_option(PD_BUILD_C_EXAMPLES "Builds C API example programs" OFF "LIBPD_SHARED" OFF)

set(CMAKE_C_STANDARD 11)

if (MSVC)
    set(PTHREADS_LIB CACHE FILEPATH "Path to pthreads library binary file (ex. C:/src/vcpkg/packages/pthreads_x64-windows/lib/pthreadVC3.lib)")
    set(PTHREADS_INCLUDE_DIR CACHE PATH "Path to folder with pthreads library header files (ex. C:/src/vcpkg/packages/pthreads_x64-windows/include)")
    # We need pthreads.
    # Please provide the path to the pthreads library include path and
    # the path to the pthread library by specifying the following arguments
    # on the CMake command-line:
    # -DPTHREADS_LIB:PATH=<path to library, either MSVC (for example pthreadVC3.lib) or GNUC version>
    # -DPTHREADS_INCLUDE_DIR:PATH=<path to the pthread header files>
    if (NOT PTHREADS_LIB OR NOT PTHREADS_INCLUDE_DIR)
        message(FATAL_ERROR "Please provide a path to the pthreads library and its headers.")
    endif()
    # Prefer pthread implementation on platforms where multiple are available.
    set(CMAKE_THREAD_PREFER_PTHREAD ON)
endif()

if(WIN32)
    # Use Windows APIs compatible with most versions
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DWINVER=0x502 -DWIN32 -D_WIN32")
endif()
if (MSVC)
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DHAVE_STRUCT_TIMESPEC")
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_compile_options(/wd4091 /wd4996)

    # vcruntime_c11_stdatomic.h(36,24): error C2061: syntax error: identifier 'atomic_bool'
    add_compile_options(/experimental:c11atomics)
    if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
        # Select appropriate long int type on 64-bit Windows
        add_compile_definitions("PD_LONGINTTYPE=long long")
    endif()
else()
    set(CMAKE_C_FLAGS         "${CMAKE_C_FLAGS} -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -ffast-math -funroll-loops -fomit-frame-pointer -O3")
    set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG} -g -O0")
    if(NOT APPLE AND NOT WIN32)
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-Bsymbolic")
    endif()
    if(APPLE)
        set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -D_DARWIN_C_SOURCE -D_DARWIN_UNLIMITED_SELECT -DFD_SETSIZE=10240")
    endif()
endif()

if(NOT PROJECT_IS_TOP_LEVEL)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_MACOSX_RPATH ON)

# Look for dependencies
check_include_file(alloca.h HAVE_ALLOCA_H)
check_include_file(endian.h HAVE_ENDIAN_H)
check_include_file(machine/endian.h HAVE_MACHINE_ENDIAN_H)
check_include_file(unistd.h HAVE_UNISTD_H)

if(APPLE OR LINUX)
  check_include_file(dlfcn.h HAVE_LIBDL)
endif()

if(NOT MSVC)
  find_package(Threads REQUIRED)
endif()

if(NOT MSVC AND NOT APPLE)
  check_library_exists(m pow "" HAVE_LIBM)
  if(HAVE_LIBM)
    set(M_LIBRARIES m)
  endif()
endif()

# Build
foreach (_BUILD_TYPE RELEASE DEBUG)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${_BUILD_TYPE} ${CMAKE_CURRENT_BINARY_DIR}/libs)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${_BUILD_TYPE} ${CMAKE_CURRENT_BINARY_DIR}/libs)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_BUILD_TYPE} ${CMAKE_CURRENT_BINARY_DIR}/libs)
endforeach()

set(PD_SOURCES
    pure-data/src/d_arithmetic.c
    pure-data/src/d_array.c
    pure-data/src/d_ctl.c
    pure-data/src/d_dac.c
    pure-data/src/d_delay.c
    pure-data/src/d_fft.c
    pure-data/src/d_fft_fftsg.c
    pure-data/src/d_filter.c
    pure-data/src/d_global.c
    pure-data/src/d_math.c
    pure-data/src/d_misc.c
    pure-data/src/d_osc.c
    pure-data/src/d_resample.c
    pure-data/src/d_soundfile.c
    pure-data/src/d_soundfile_aiff.c
    pure-data/src/d_soundfile_caf.c
    pure-data/src/d_soundfile_next.c
    pure-data/src/d_soundfile_wave.c
    pure-data/src/d_ugen.c
    pure-data/src/g_all_guis.c
    pure-data/src/g_all_guis.h
    pure-data/src/g_array.c
    pure-data/src/g_bang.c
    pure-data/src/g_canvas.c
    pure-data/src/g_canvas.h
    pure-data/src/g_clone.c
    pure-data/src/g_editor.c
    pure-data/src/g_editor_extras.c
    pure-data/src/g_graph.c
    pure-data/src/g_guiconnect.c
    pure-data/src/g_io.c
    pure-data/src/g_mycanvas.c
    pure-data/src/g_numbox.c
    pure-data/src/g_radio.c
    pure-data/src/g_readwrite.c
    pure-data/src/g_rtext.c
    pure-data/src/g_scalar.c
    pure-data/src/g_slider.c
    pure-data/src/g_template.c
    pure-data/src/g_text.c
    pure-data/src/g_toggle.c
    pure-data/src/g_traversal.c
    pure-data/src/g_undo.c
    pure-data/src/g_vumeter.c
    pure-data/src/m_atom.c
    pure-data/src/m_binbuf.c
    pure-data/src/m_class.c
    pure-data/src/m_conf.c
    pure-data/src/m_glob.c
    pure-data/src/m_imp.h
    pure-data/src/m_memory.c
    pure-data/src/m_obj.c
    pure-data/src/m_pd.c
    pure-data/src/m_pd.h
    pure-data/src/m_sched.c
    pure-data/src/s_audio.c
    pure-data/src/s_audio_dummy.c
    pure-data/src/s_inter.c
    pure-data/src/s_inter_gui.c
    pure-data/src/s_loader.c
    pure-data/src/s_main.c
    pure-data/src/s_net.c
    pure-data/src/s_path.c
    pure-data/src/s_print.c
    pure-data/src/s_stuff.h
    pure-data/src/s_utf8.c
    pure-data/src/s_utf8.h
    pure-data/src/x_acoustics.c
    pure-data/src/x_arithmetic.c
    pure-data/src/x_array.c
    pure-data/src/x_connective.c
    pure-data/src/x_file.c
    pure-data/src/x_gui.c
    pure-data/src/x_interface.c
    pure-data/src/x_list.c
    pure-data/src/x_midi.c
    pure-data/src/x_misc.c
    pure-data/src/x_net.c
    pure-data/src/x_scalar.c
    pure-data/src/x_text.c
    pure-data/src/x_time.c
    pure-data/src/x_vexp.c
    pure-data/src/x_vexp.h
    pure-data/src/x_vexp_fun.c
    pure-data/src/x_vexp_if.c
)

set(PD_EXTRA_SOURCES
    pure-data/extra/bob~/bob~.c
    pure-data/extra/bonk~/bonk~.c
    pure-data/extra/choice/choice.c
    pure-data/extra/fiddle~/fiddle~.c
    pure-data/extra/loop~/loop~.c
    pure-data/extra/lrshift~/lrshift~.c
    pure-data/extra/pd~/pdsched.c
    pure-data/extra/pd~/pd~.c
    pure-data/extra/pique/pique.c
    pure-data/extra/sigmund~/sigmund~.c
    pure-data/extra/stdout/stdout.c
)

set(LIBPD_SOURCES
    libpd_wrapper/s_libpdmidi.c
    libpd_wrapper/x_libpdreceive.c
    libpd_wrapper/x_libpdreceive.h
    libpd_wrapper/z_hooks.c
    libpd_wrapper/z_hooks.h
    libpd_wrapper/z_libpd.c
)
set(LIBPD_UTILS_SOURCES
    libpd_wrapper/util/z_ringbuffer.c
    libpd_wrapper/util/z_ringbuffer.h
    libpd_wrapper/util/z_print_util.c
    libpd_wrapper/util/z_print_util.h
    libpd_wrapper/util/z_queued.c
    libpd_wrapper/util/z_queued.h
)

source_group(pd         FILES ${PD_SOURCES})
source_group(pdextra    FILES ${PD_EXTRA_SOURCES})
source_group(libpd      FILES ${LIBPD_SOURCES})
source_group(libpdutils FILES ${LIBPD_UTILS_SOURCES})

# set the output library name for libpd depending on the settings
set(LIBPD_OUTPUT_NAME     pd)
if (WIN32)
    set(LIBPD_OUTPUT_NAME libpd)
endif()
if (PD_MULTI)
    set(LIBPD_OUTPUT_NAME ${LIBPD_OUTPUT_NAME}-multi)
endif()

# create final list of source files
set(SOURCE_FILES ${PD_SOURCES} ${LIBPD_SOURCES})
if(PD_UTILS)
    list(APPEND SOURCE_FILES ${LIBPD_UTILS_SOURCES})
endif()
if(PD_EXTRA)
    list(APPEND SOURCE_FILES ${PD_EXTRA_SOURCES})
endif()

add_library(libpd_private_settings INTERFACE)
add_library(libpd_public_settings INTERFACE)
if(LIBPD_STATIC)
  add_library(libpd_static STATIC ${SOURCE_FILES})
  target_link_libraries(libpd_static
      PRIVATE libpd_private_settings
      PUBLIC libpd_public_settings
  )

  if (WIN32)
      set_target_properties(libpd_static PROPERTIES OUTPUT_NAME ${LIBPD_OUTPUT_NAME}-static)
  else()
      set_target_properties(libpd_static PROPERTIES OUTPUT_NAME ${LIBPD_OUTPUT_NAME})
  endif()
  set_target_properties(libpd_static PROPERTIES
    UNITY_BUILD OFF
  )
endif()

if(LIBPD_SHARED)
  add_library(libpd        SHARED ${SOURCE_FILES})
  target_link_libraries(libpd
      PRIVATE libpd_private_settings
      PUBLIC libpd_public_settings
  )
  set_target_properties(libpd PROPERTIES
    OUTPUT_NAME ${LIBPD_OUTPUT_NAME}
    UNITY_BUILD OFF
  )

  target_link_libraries(libpd
    PUBLIC
      $<$<BOOL:${MSVC}>:Ws2_32>
      $<$<BOOL:${MINGW}>:-Wl,--export-all-symbols ws2_32 kernel32 -static-libgcc>
      $<IF:$<BOOL:${MSVC}>,${PTHREADS_LIB},Threads::Threads>

      ${M_LIBRARIES}
      ${CMAKE_DL_LIBS}
  )
endif()


target_compile_definitions(libpd_private_settings
  INTERFACE
    PD_INTERNAL=1
)

target_compile_definitions(libpd_public_settings
  INTERFACE
    PD=1
    USEAPI_DUMMY=1
   "$<$<BOOL:${PD_DEFINE_EXTERN}>:EXTERN=${PD_DEFINE_EXTERN}>"
    $<$<BOOL:${APPLE}>:_DARWIN_C_SOURCE>
    $<$<BOOL:${HAVE_ALLOCA_H}>:HAVE_ALLOCA_H=1>
    $<$<BOOL:${HAVE_ENDIAN_H}>:HAVE_ENDIAN_H=1>
    $<$<BOOL:${HAVE_LIBDL}>:HAVE_LIBDL=1>
    $<$<BOOL:${HAVE_MACHINE_ENDIAN_H}>:HAVE_MACHINE_ENDIAN_H=1>
    $<$<BOOL:${HAVE_UNISTD_H}>:HAVE_UNISTD_H=1>
    $<$<BOOL:${PD_EXTRA}>:LIBPD_EXTRA=1>
    $<$<BOOL:${PD_MULTI}>:PDINSTANCE=1 PDTHREADS=1>
    $<$<NOT:$<BOOL:${PD_LOCALE}>>:LIBPD_NO_NUMERIC=1>
)

target_include_directories(libpd_private_settings
  INTERFACE
    $<$<BOOL:${MSVC}>:${PTHREADS_INCLUDE_DIR}>
)

target_include_directories(libpd_public_settings
  INTERFACE
    libpd_wrapper
    pure-data/src
)

if(PD_BUILD_C_EXAMPLES)
    macro(ADD_EXAMPLE name)
        add_executable(${name} samples/c/${name}/${name}.c)
        target_include_directories(${name} PRIVATE 
          $<$<BOOL:${MSVC}>:${PTHREADS_INCLUDE_DIR}>
        )
        target_link_libraries(${name} PUBLIC libpd)
        if(DEFINED CMAKE_DEBUG_POSTFIX)
            set_target_properties(${name} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
        endif()
    endmacro()

    add_example(pdtest)
    if(PD_MULTI)
        add_example(pdtest_multi)
        if(NOT MSVC)
            # uses gettimeofday()
            add_example(pdtest_gui)
        endif()
    endif()
    add_example(pdtest_thread)
endif()
