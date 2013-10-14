bin_PROGRAMS = isagoge

SUBDIRS = Tesseract Groundtruth_Generation/select_random \
Groundtruth_Generation/select_training \
Groundtruth_Generation Evaluation Utils dlib-18.4 MEDS

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
-ITesseract/wordrec -ITesseract/ -IEvaluation/ \
-IMEDS/ -IUtils/ -Idlib-18.4

isagoge_LDADD = /usr/local/lib/liblept.so \
MEDS/libMEDS.la /usr/lib64/libX11.so \
/usr/lib64/libX11-xcb.so \
Tesseract/api/libtesseract.la \
Utils/libUtils.la \
Evaluation/libevaluation.la

isagoge_SOURCES = IsagogeMain.cpp
