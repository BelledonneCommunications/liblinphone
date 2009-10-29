## Process this file with automake to produce Makefile.in
if BUILD_MACAU
#the media_api library is the only one we have to build here
lib_LTLIBRARIES=libmediaplugins.la

#definition of the sources of libmediaplugins
libmediaplugins_la_SOURCES=	auiosnd.c

# libmediaplugins  needs libmediastreamer
libmediaplugins_la_LIBADD=$(top_srcdir)/mediastreamer/libmediastreamer.la


endif

DEFS=@DEFS@ @SOUNDDEFS@ -DDEBUG " 

INCLUDES=-I$(top_srcdir)/mediastreamer 


