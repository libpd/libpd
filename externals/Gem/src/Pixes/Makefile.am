#####################################################################
# Gem/Pixes: objectclasses for image manipulation
#####################################################################

AUTOMAKE_OPTIONS = foreign
AM_CPPFLAGS = -I$(top_srcdir)/src
include ../check-sources.mk

noinst_LTLIBRARIES = libPixes.la

libPixes_la_CXXFLAGS =
libPixes_la_LIBADD   =
libPixes_la_LDFLAGS  =

# RTE flags
libPixes_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
libPixes_la_LIBADD   += @GEM_RTE_LIBS@
libPixes_la_LDFLAGS  += @GEM_ARCH_LDFLAGS@

libPixes_la_SOURCES=  \
    FreeFrame.h \
    frei0r.h \
    pix_2grey.cpp \
    pix_2grey.h \
    pix_a_2grey.cpp \
    pix_a_2grey.h \
    pix_add.cpp \
    pix_add.h \
    pix_aging.cpp \
    pix_aging.h \
    pix_alpha.cpp \
    pix_alpha.h \
    pix_background.cpp \
    pix_background.h \
    pix_backlight.cpp \
    pix_backlight.h \
    pix_biquad.cpp \
    pix_biquad.h \
    pix_bitmask.cpp \
    pix_bitmask.h \
    pix_blob.cpp \
    pix_blob.h \
    pix_blur.cpp \
    pix_blur.h \
    pix_buf.cpp \
    pix_buffer.cpp \
    pix_buffer.h \
    pix_buffer_read.cpp \
    pix_buffer_read.h \
    pix_buffer_write.cpp \
    pix_buffer_write.h \
    pix_buf.h \
    pix_chroma_key.cpp \
    pix_chroma_key.h \
    pix_clearblock.cpp \
    pix_clearblock.h \
    pix_coloralpha.cpp \
    pix_coloralpha.h \
    pix_color.cpp \
    pix_color.h \
    pix_colormatrix.cpp \
    pix_colormatrix.h \
    pix_colorreduce.cpp \
    pix_colorreduce.h \
    pix_compare.cpp \
    pix_compare.h \
    pix_composite.cpp \
    pix_composite.h \
    pix_contrast.cpp \
    pix_contrast.h \
    pix_convert.cpp \
    pix_convert.h \
    pix_convolve.cpp \
    pix_convolve.h \
    pix_coordinate.cpp \
    pix_coordinate.h \
    pix_crop.cpp \
    pix_crop.h \
    pix_cubemap.cpp \
    pix_cubemap.h \
    pix_curve.cpp \
    pix_curve.h \
    pix_data.cpp \
    pix_data.h \
    pix_deinterlace.cpp \
    pix_deinterlace.h \
    pix_delay.cpp \
    pix_delay.h \
    pix_diff.cpp \
    pix_diff.h \
    pix_dot.cpp \
    pix_dot.h \
    pix_draw.cpp \
    pix_draw.h \
    pix_dump.cpp \
    pix_dump.h \
    pix_duotone.cpp \
    pix_duotone.h \
    pix_emboss.cpp \
    pix_emboss.h \
    pix_film.cpp \
    pix_film.h \
    pix_flip.cpp \
    pix_flip.h \
    pix_freeframe.cpp \
    pix_freeframe.h \
    pix_frei0r.cpp \
    pix_frei0r.h \
    pix_gain.cpp \
    pix_gain.h \
    pix_grey.cpp \
    pix_grey.h \
    pix_halftone.cpp \
    pix_halftone.h \
    pix_histo.cpp \
    pix_histo.h \
    pix_hit.cpp \
    pix_hit.h \
    pix_hsv2rgb.cpp \
    pix_hsv2rgb.h \
    pix_image.cpp \
    pix_image.h \
    pix_imageInPlace.cpp \
    pix_imageInPlace.h \
    pix_info.cpp \
    pix_info.h \
    pix_invert.cpp \
    pix_invert.h \
    pix_kaleidoscope.cpp \
    pix_kaleidoscope.h \
    pix_levels.cpp \
    pix_levels.h \
    pix_lumaoffset.cpp \
    pix_lumaoffset.h \
    pix_mask.cpp \
    pix_mask.h \
    pix_mean_color.cpp \
    pix_mean_color.h \
    pix_metaimage.cpp \
    pix_metaimage.h \
    pix_mix.cpp \
    pix_mix.h \
    pix_motionblur.cpp \
    pix_motionblur.h \
    pix_movement2.cpp \
    pix_movement2.h \
    pix_movement.cpp \
    pix_movement.h \
    pix_movie.cpp \
    pix_movie.h \
    pix_multiblob.cpp \
    pix_multiblob.h \
    pix_multiimage.cpp \
    pix_multiimage.h \
    pix_multiply.cpp \
    pix_multiply.h \
    pix_multitexture.cpp \
    pix_multitexture.h \
    pix_normalize.cpp \
    pix_normalize.h \
    pix_offset.cpp \
    pix_offset.h \
    pix_pix2sig.cpp \
    pix_pix2sig.h \
    pix_posterize.cpp \
    pix_posterize.h \
    pix_puzzle.cpp \
    pix_puzzle.h \
    pix_rds.cpp \
    pix_rds.h \
    pix_record.cpp \
    pix_record.h \
    pix_rectangle.cpp \
    pix_rectangle.h \
    pix_refraction.cpp \
    pix_refraction.h \
    pix_resize.cpp \
    pix_resize.h \
    pix_rgb2hsv.cpp \
    pix_rgb2hsv.h \
    pix_rgba.cpp \
    pix_rgba.h \
    pix_roll.cpp \
    pix_roll.h \
    pix_rtx.cpp \
    pix_rtx.h \
    pix_scanline.cpp \
    pix_scanline.h \
    pix_set.cpp \
    pix_set.h \
    pix_share.h \
    pix_share_read.cpp \
    pix_share_read.h \
    pix_share_write.cpp \
    pix_share_write.h \
    pix_sig2pix.cpp \
    pix_sig2pix.h \
    pix_snap2tex.cpp \
    pix_snap2tex.h \
    pix_snap.cpp \
    pix_snap.h \
    pix_subtract.cpp \
    pix_subtract.h \
    pix_takealpha.cpp \
    pix_takealpha.h \
    pix_test.cpp \
    pix_test.h \
    pix_texture.cpp \
    pix_texture.h \
    pix_threshold_bernsen.cpp \
    pix_threshold_bernsen.h \
    pix_threshold.cpp \
    pix_threshold.h \
    pix_tIIR.cpp \
    pix_tIIRf.cpp \
    pix_tIIRf.h \
    pix_tIIR.h \
    pix_video.cpp \
    pix_video.h \
    pix_vpaint.cpp \
    pix_vpaint.h \
    pix_write.cpp \
    pix_write.h \
    pix_writer.cpp \
    pix_writer.h \
    pix_yuv.cpp \
    pix_yuv.h \
    pix_zoom.cpp \
    pix_zoom.h



# deprecated stuff
EXTRA_DIST = \
    pix_filmOS.cpp \
    pix_filmOS.h \
    pix_filmDarwin.cpp \
    pix_filmDarwin.h \
    pix_movieOS.cpp \
    pix_movieOS.h \
    pix_movieDarwin.cpp \
    pix_movieDarwin.h \
    pix_movieDS.cpp \
    pix_movieDS.h \
    pix_videoDarwin.cpp \
    pix_videoDarwin.h \
    pix_videoOS.cpp \
    pix_videoOS.h \
    pix_recordQT.cpp \
    pix_recordQT.h

CHECKUNUSED_IGNORE=$(EXTRA_DIST)

