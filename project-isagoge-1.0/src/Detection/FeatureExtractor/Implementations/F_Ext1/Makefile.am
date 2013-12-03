lib_LTLIBRARIES = libF_Ext1.la
SUBDIRS = ../../../../Grid ../../Sentence ../../NGrams \
../../NGrams/NGProfileGen ../../../Sample
libF_Ext1_la_SOURCES = F_Ext1.h F_Ext1.cpp
libF_Ext1_la_LIBADD = ../../../../Grid/libGrid.la \
../../../Sample/libGTParser.la \
../../Sentence/libSentence.la \
../../NGrams/libNGrams.la \
../../NGrams/NGProfileGen/libNGProfileGen.la
libF_Ext1_la_CPPFLAGS = -I../../../../Grid \
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
-I../../../../Utils -I../../../Sample -I../../Sentence \
-I../../NGrams -I../../../TessInterface \
-I../../NGrams/NGProfileGen
