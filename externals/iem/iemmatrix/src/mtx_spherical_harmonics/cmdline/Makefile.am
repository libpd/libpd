#check_PROGRAMS = \
   # 

bin_PROGRAMS = \
	legendre_a_cmd \
	sharmonics_normalization_cmd \
	chebyshev12_cmd \
	sharmonics_cmd

#TESTS = $(check_PROGRAMS)

AM_CPPFLAGS = -I$(top_srcdir)

legendre_a_cmd_LDADD = ../libsharmonics.a

sharmonics_normalization_cmd_LDADD = ../libsharmonics.a

chebyshev12_cmd_LDADD = ../libsharmonics.a

sharmonics_cmd_LDADD = ../libsharmonics.a

legendre_a_cmd_SOURCES = legendre_a_cmd.c

sharmonics_normalization_cmd_SOURCES = sharmonics_normalization_cmd.c

chebyshev12_cmd_SOURCES = chebyshev12_cmd.c

sharmonics_cmd_SOURCES = sharmonics_cmd.c

