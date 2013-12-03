lib_LTLIBRARIES = libGrid.la
SUBDIRS = ../Detection/FeatureExtractor/Sentence \
../Detection/FeatureExtractor/NGrams
libGrid_la_SOURCES = BlobInfoGrid.h BlobInfoGrid.cpp \
M_Utils.h M_Utils.cpp
libGrid_la_LIBADD = ../Tesseract/api/libtesseract.la \
/usr/local/lib/liblept.so ../Utils/libUtils.la \
../Detection/FeatureExtractor/Sentence/libSentence.la \
../Detection/FeatureExtractor/NGrams/libNGrams.la
libGrid_la_CPPFLAGS = -I../Tesseract/api \
-I../Tesseract/ccmain -I../Tesseract/ccstruct \
-I../Tesseract/ccutil -I../Tesseract/classify \
-I../Tesseract/config -I../Tesseract/contrib \
-I../Tesseract/cube -I../Tesseract/cutil \
-I../Tesseract/dict -I../Tesseract/doc \
-I../Tesseract/image -I../Tesseract/java \
-I../Tesseract/m4 -I../Tesseract/neural_networks \
-I../Tesseract/po -I../Tesseract/tessdata \
-I../Tesseract/testing -I../Tesseract/textord \
-I../Tesseract/training -I../Tesseract/viewer \
-I../Tesseract/wordrec -I../Tesseract -I../Utils \
-I../Detection/FeatureExtractor/NGrams \
-I../Detection/FeatureExtractor/Sentence
