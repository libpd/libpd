#####################################################################
# Gem/Particles: objectlcasses for particle systems
#####################################################################

AUTOMAKE_OPTIONS = foreign
AM_CPPFLAGS = -I$(top_srcdir)/src
include ../check-sources.mk

noinst_LTLIBRARIES = libParticles.la

libParticles_la_CXXFLAGS =
libParticles_la_LIBADD   =
libParticles_la_LDFLAGS  =

# RTE flags
libParticles_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
libParticles_la_LIBADD   += @GEM_RTE_LIBS@
libParticles_la_LDFLAGS  += @GEM_ARCH_LDFLAGS@


libParticles_la_SOURCES=  \
    papi.h \
    part_color.cpp \
    part_color.h \
    part_damp.cpp \
    part_damp.h \
    part_draw.cpp \
    part_draw.h \
    part_follow.cpp \
    part_follow.h \
    part_gravity.cpp \
    part_gravity.h \
    part_head.cpp \
    part_head.h \
    part_info.cpp \
    part_info.h \
    part_killold.cpp \
    part_killold.h \
    part_killslow.cpp \
    part_killslow.h \
    partlib_actionapi.cpp \
    partlib_actions.cpp \
    partlib_base.cpp \
    partlib_base.h \
    partlib_general.h \
    partlib_opengl.cpp \
    partlib_system.cpp \
    partlib_vector.h \
    part_orbitpoint.cpp \
    part_orbitpoint.h \
    part_render.cpp \
    part_render.h \
    part_sink.cpp \
    part_sink.h \
    part_size.cpp \
    part_size.h \
    part_source.cpp \
    part_source.h \
    part_targetcolor.cpp \
    part_targetcolor.h \
    part_targetsize.cpp \
    part_targetsize.h \
    part_velcone.cpp \
    part_velcone.h \
    part_velocity.cpp \
    part_velocity.h \
    part_velsphere.cpp \
    part_velsphere.h \
    part_vertex.cpp \
    part_vertex.h

