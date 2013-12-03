lib_LTLIBRARIES = libFeatureExtractor.la
SUBDIRS = ../../Grid Implementations
libFeatureExtractor_la_SOURCES = IFeatureExtractor.h
libFeatureExtractor_la_LIBADD = ../../Grid/libGrid.la \
Implementations/libFE_Implementations.la
libFeatureExtractor_la_CPPFLAGS = -I../../Grid -IImplementations \
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
-I../../Tesseract/wordrec -I../../Tesseract
