lib_LTLIBRARIES = libMEDS.la
SUBDIRS = Detection
libMEDS_la_SOURCES = MEDS.h \
MEDS.cpp BlobInfoGrid.h BlobInfoGrid.cpp \
M_Utils.h M_Utils.cpp
libMEDS_la_LIBADD = /usr/local/lib/liblept.so \
../Tesseract/api/libtesseract.la \
../Utils/libUtils.la ../dlib-18.4/dlib/libdlib.la
libMEDS_la_CPPFLAGS = -I../Tesseract/api \
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
-I../Tesseract/wordrec -I../Tesseract -I../LibUtils \
-I../dlib-18.4
