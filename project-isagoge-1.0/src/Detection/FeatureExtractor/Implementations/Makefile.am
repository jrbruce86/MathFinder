lib_LTLIBRARIES = libFE_Implementations.la
SUBDIRS = F_Ext1 F_Ext2 F_Ext3 F_Ext4
libFE_Implementations_la_SOURCES = FE_Implementations.h
libFE_Implementations_la_LIBADD = F_Ext1/libF_Ext1.la F_Ext2/libF_Ext2.la \
F_Ext3/libF_Ext3.la F_Ext4/libF_Ext4.la
libFE_Implementations_la_CPPFLAGS = -IF_Ext1 -IF_Ext2 -IF_Ext3 -IF_Ext4
