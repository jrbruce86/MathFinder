bin_PROGRAMS = isagoge

SUBDIRS = Tesseract

isagoge_CPPFLAGS = -ITesseract/api \
-ITesseract/ccmain -ITesseract/ccstruct \
-ITesseract/ccutil -ITesseract/classify \
-ITesseract/config -ITesseract/contrib \
-ITesseract/cube -ITesseract/cutil \
-ITesseract/dict\ -ITesseract/doc \
-ITesseract/image -ITesseract/java \
-ITesseract/m4 -ITesseract/neural_networks \
-ITesseract/po -ITesseract/tessdata \
-ITesseract/testing -ITesseract/textord \
-ITesseract/training -ITesseract/viewer \
-ITesseract/wordrec

isagoge_LDADD = /usr/local/lib/liblept.so \
Tesseract/api/.libs/libtesseract.so

isagoge_SOURCES = DocumentLayoutTest.cpp DocumentLayoutTest.h \
	EquationDetectorSVM.cpp EquationDetectorSVM.h IsagogeMain.cpp \
	TesseractDocumentation.h



