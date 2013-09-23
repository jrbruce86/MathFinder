lib_LTLIBRARIES = libevaluation.la
SUBDIRS = ../Utils
libevaluation_la_SOURCES = DocumentLayoutTest.h \
DocumentLayoutTest.cpp \
BipartiteGraph.h \
BipartiteGraph.cpp
libevaluation_la_LIBADD = /usr/local/lib/liblept.so \
../Tesseract/api/libtesseract.la \
../Utils/libUtils.la
libevaluation_la_CPPFLAGS = -I../Tesseract/api \
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
-I../Tesseract/wordrec -I../Utils -I../MEDS
