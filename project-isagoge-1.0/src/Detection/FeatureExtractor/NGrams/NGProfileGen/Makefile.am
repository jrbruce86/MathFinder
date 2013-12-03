lib_LTLIBRARIES = libNGProfileGen.la
SUBDIRS = ../../../../Grid ../../../TessInterface ../../../Sample
libNGProfileGen_la_SOURCES = NGProfileGen.h \
NGProfileGen.cpp
libNGProfileGen_la_LIBADD = ../../../../Grid/libGrid.la \
../../../TessInterface/libTessInterface.la ../libNGrams.la \
../../../Sample/libGTParser.la
libNGProfileGen_la_CPPFLAGS = -I../../../../Grid \
-I../../../TessInterface \
-I../../../../Tesseract/api \
-I../../../../Tesseract/ccmain -I../../../../Tesseract/ccstruct \
-I../../../../Tesseract/ccutil -I../../../../Tesseract/classify \
-I../../../../Tesseract/config -I../../../../Tesseract/contrib \
-I../../../../Tesseract/cube -I../../../../Tesseract/cutil \
-I../../../../Tesseract/dict -I../../../../Tesseract/doc \
-I../../../../Tesseract/image -I../../../../Tesseract/java \
-I../../../../Tesseract/m4 -I../../../../Tesseract/neural_networks \
-I../../../../Tesseract/po -I../../../../Tesseract/tessdata \
-I../../../../Tesseract/testing -I../../../../Tesseract/textord \
-I../../../../Tesseract/training -I../../../../Tesseract/viewer \
-I../../../../Tesseract/wordrec -I../../../../Tesseract \
-I../../../../Utils -I../../../Sample -I../../Sentence -I../
