cmake_minimum_required (VERSION 2.8.11)
project (libpd C)

option(BUILD_SHARED_LIBS "Build libpd as a shared library instead of static" ON)
option(pd_utils "Compile utilities" ON)
option(pd_extra "Compile extras" ON)
option(pd_multi "Compile with multiple instance support" ON)
option(pd_locale "Set the LC_NUMERIC number format to the default C locale" ON)
option(pd_build_c_examples "Builds C API example programs" OFF)
option(COVERALLS "Build with coveralls" OFF)

if(WIN32)
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DWINVER=0x502 -DWIN32 -D_WIN32")
endif()
if (MSVC)
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DHAVE_STRUCT_TIMESPEC")
    add_definitions("/D_CRT_SECURE_NO_WARNINGS /wd4091 /wd4996")
    if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
        set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DPD_LONGINTTYPE=\"long long\"")
    endif()
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -ffast-math -funroll-loops -fomit-frame-pointer -O3")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -g -O0")
endif()
add_definitions(-DPD=1 -DUSEAPI_DUMMY=1)

if(COVERALLS)
    message(STATUS "Build with coveralls")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

if(NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.7)
endif()
if(NOT DEFINED CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "i386;x86_64")
endif()
set(CMAKE_MACOSX_RPATH ON)

#---------------------------------------#
#        The Pure Data Sources          #
#---------------------------------------#
set(PDSOURCES
    ${PROJECT_SOURCE_DIR}/pure-data/src/d_arithmetic.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/d_array.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/d_ctl.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/d_dac.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/d_delay.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/d_fft.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/d_fft_fftsg.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/d_filter.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/d_global.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/d_math.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/d_misc.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/d_osc.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/d_resample.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/d_soundfile.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/d_ugen.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/g_all_guis.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/g_all_guis.h
    ${PROJECT_SOURCE_DIR}/pure-data/src/g_array.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/g_bang.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/g_canvas.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/g_canvas.h
    ${PROJECT_SOURCE_DIR}/pure-data/src/g_clone.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/g_editor.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/g_graph.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/g_guiconnect.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/g_hdial.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/g_hslider.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/g_io.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/g_mycanvas.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/g_numbox.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/g_readwrite.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/g_rtext.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/g_scalar.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/g_template.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/g_text.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/g_toggle.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/g_traversal.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/g_vdial.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/g_vslider.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/g_vumeter.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/m_atom.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/m_binbuf.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/m_class.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/m_conf.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/m_glob.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/m_imp.h
    ${PROJECT_SOURCE_DIR}/pure-data/src/m_memory.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/m_obj.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/m_pd.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/m_pd.h
    ${PROJECT_SOURCE_DIR}/pure-data/src/m_sched.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/s_audio.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/s_audio_dummy.c
    #${PROJECT_SOURCE_DIR}/pure-data/src/s_file.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/s_inter.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/s_loader.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/s_main.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/s_path.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/s_print.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/s_stuff.h
    ${PROJECT_SOURCE_DIR}/pure-data/src/s_utf8.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/s_utf8.h
    ${PROJECT_SOURCE_DIR}/pure-data/src/x_acoustics.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/x_arithmetic.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/x_array.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/x_connective.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/x_gui.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/x_interface.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/x_list.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/x_midi.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/x_misc.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/x_net.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/x_scalar.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/x_text.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/x_time.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/x_vexp.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/x_vexp.h
    ${PROJECT_SOURCE_DIR}/pure-data/src/x_vexp_fun.c
    ${PROJECT_SOURCE_DIR}/pure-data/src/x_vexp_if.c
)

#---------------------------------------#
#        The Pure Data Extra Sources    #
#---------------------------------------#
set(PDEXTRASOURCES
    ${PROJECT_SOURCE_DIR}/pure-data/extra/bob~/bob~.c
    ${PROJECT_SOURCE_DIR}/pure-data/extra/bonk~/bonk~.c
    ${PROJECT_SOURCE_DIR}/pure-data/extra/choice/choice.c
    ${PROJECT_SOURCE_DIR}/pure-data/extra/fiddle~/fiddle~.c
    ${PROJECT_SOURCE_DIR}/pure-data/extra/loop~/loop~.c
    ${PROJECT_SOURCE_DIR}/pure-data/extra/lrshift~/lrshift~.c
    ${PROJECT_SOURCE_DIR}/pure-data/extra/pique/pique.c
    ${PROJECT_SOURCE_DIR}/pure-data/extra/sigmund~/sigmund~.c
    ${PROJECT_SOURCE_DIR}/pure-data/extra/stdout/stdout.c
)

#---------------------------------------#
#         The libpd Sources             #
#---------------------------------------#

set(LIBPDSOURCES
    ${PROJECT_SOURCE_DIR}/libpd_wrapper/z_libpd.c
    ${PROJECT_SOURCE_DIR}/libpd_wrapper/z_libpd.h
    ${PROJECT_SOURCE_DIR}/libpd_wrapper/s_libpdmidi.c
    ${PROJECT_SOURCE_DIR}/libpd_wrapper/x_libpdreceive.c
    ${PROJECT_SOURCE_DIR}/libpd_wrapper/x_libpdreceive.h
    ${PROJECT_SOURCE_DIR}/libpd_wrapper/z_hooks.c
    ${PROJECT_SOURCE_DIR}/libpd_wrapper/z_hooks.h
)
set(LIBPDUTILSSOURCES
    ${PROJECT_SOURCE_DIR}/libpd_wrapper/util/ringbuffer.c
    ${PROJECT_SOURCE_DIR}/libpd_wrapper/util/ringbuffer.h
    ${PROJECT_SOURCE_DIR}/libpd_wrapper/util/z_print_util.c
    ${PROJECT_SOURCE_DIR}/libpd_wrapper/util/z_print_util.h
    ${PROJECT_SOURCE_DIR}/libpd_wrapper/util/z_queued.c
    ${PROJECT_SOURCE_DIR}/libpd_wrapper/util/z_queued.h
)

source_group(pd FILES ${PDSOURCES})
source_group(pdextra FILES ${PDEXTRASOURCES})
source_group(libpd FILES ${LIBPDSOURCES})
source_group(libpdutils FILES ${LIBPDUTILSSOURCES})

include_directories(${PROJECT_SOURCE_DIR}/libpd_wrapper)
include_directories(${PROJECT_SOURCE_DIR}/pure-data/src)

if(WIN32)
    include_directories(${PROJECT_SOURCE_DIR}/winpthreads/include)
    link_directories(${PROJECT_SOURCE_DIR}/winpthreads/lib)
endif()

find_package(Threads REQUIRED)

set(SOURCE_FILES ${PDSOURCES} ${LIBPDSOURCES})
if(pd_utils)
    list(APPEND SOURCE_FILES ${LIBPDUTILSSOURCES})
endif()
if(pd_extra)
    list(APPEND SOURCE_FILES ${PDEXTRASOURCES})
endif()

add_library(libpd SHARED ${SOURCE_FILES})

if(NOT MSVC)
    target_compile_definitions(libpd PRIVATE HAVE_UNISTD_H=1)
endif()

if(pd_extra)
    target_compile_definitions(libpd PRIVATE LIBPD_EXTRA=1)
endif()

if(pd_multi)
    target_compile_definitions(libpd PRIVATE PDINSTANCE=1 PDTHREADS=1)
endif()

if(NOT pd_locale)
    target_compile_definitions(libpd PRIVATE LIBPD_NO_NUMERIC=1)
endif()
target_compile_definitions(libpd PRIVATE PD_INTERNAL)

if(MSVC)
    target_link_libraries(libpd PUBLIC Ws2_32)
    target_include_directories(libpd PUBLIC ${PTHREADS_INCLUDE_DIR})
    target_link_libraries(libpd PUBLIC ${CMAKE_THREAD_LIBS_INIT})
endif()
set_target_properties(libpd PROPERTIES OUTPUT_NAME libpd)

if(NOT MSVC AND NOT APPLE)
    find_library(MATH_LIB m)
    target_link_libraries(libpd PUBLIC ${MATH_LIB})
    target_link_libraries(libpd PUBLIC ${CMAKE_DL_LIBS})
endif()

if(pd_build_c_examples)
    macro(ADD_EXAMPLE name)
        add_executable(${name} samples/c/${name}/${name}.c)
        target_link_libraries(${name} PUBLIC libpd)
    endmacro(ADD_EXAMPLE)

    ADD_EXAMPLE(pdtest)
    if(NOT MSVC)
        # uses gettimeofday()
        ADD_EXAMPLE(pdtest_gui)
    endif()
    if(pd_multi)
        ADD_EXAMPLE(pdtest_multi)
    endif()
    ADD_EXAMPLE(pdtest_thread)
endif()
