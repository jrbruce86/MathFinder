lib_LTLIBRARIES = libFeatureExtractor.la
SUBDIRS = ../../Grid ../../MEDS ../Sample
libFeatureExtractor_la_SOURCES = IFeatureExtractor.h F_Ext1.h F_Ext1.cpp
libFeatureExtractor_la_LIBADD = ../../Grid/libGrid.la \
../../MEDS/libMEDS.la ../Sample/libGTParser.la
libFeatureExtractor_la_CPPFLAGS = -I../../Grid -I../../MEDS \
-I../../Tesseract/api \
-I../../Tesseract/ccmain -I../../Tesseract/ccstruct \
-I../../Tesseract/ccutil -I../../Tesseract/classify \
-I../../Tesseract/config -I../../Tesseract/contrib \
-I../../Tesseract/cube -I../../Tesseract/cutil \
-I../../Tesseract/dict -I../../Tesseract/doc \
-I../../Tesseract/image -I../../Tesseract/java \
-I../../Tesseract/m4 -I../../Tesseract/neural_networks \
-I../../Tesseract/po -I../../Tesseract/tessdata \
-I../../Tesseract/testing -I../../Tesseract/textord \
-I../../Tesseract/training -I../../Tesseract/viewer \
-I../../Tesseract/wordrec -I../../Tesseract -I../../Utils \
-I../Sample
