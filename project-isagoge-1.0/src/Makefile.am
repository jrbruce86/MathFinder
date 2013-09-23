bin_PROGRAMS = isagoge

SUBDIRS = Tesseract Groundtruth_Generation/select_random \
Groundtruth_Generation/select_training \
Groundtruth_Generation Evaluation Utils MEDS

isagoge_CPPFLAGS = -ITesseract/api \
-ITesseract/ccmain -ITesseract/ccstruct \
-ITesseract/ccutil -ITesseract/classify \
-ITesseract/config -ITesseract/contrib \
-ITesseract/cube -ITesseract/cutil \
-ITesseract/dict -ITesseract/doc \
-ITesseract/image -ITesseract/java \
-ITesseract/m4 -ITesseract/neural_networks \
-ITesseract/po -ITesseract/tessdata \
-ITesseract/testing -ITesseract/textord \
-ITesseract/training -ITesseract/viewer \
-ITesseract/wordrec -IEvaluation -IMEDS -IUtils

isagoge_LDADD = /usr/local/lib/liblept.so \
Tesseract/api/libtesseract.la \
Utils/libUtils.la \
Evaluation/libevaluation.la \
MEDS/libMEDS.la

isagoge_SOURCES = IsagogeMain.cpp
