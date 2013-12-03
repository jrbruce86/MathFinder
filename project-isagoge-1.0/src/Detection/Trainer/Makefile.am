lib_LTLIBRARIES = libTrainer.la
libTrainer_la_SOURCES = ITrainer.h CrossValidationSVM.h
libTrainer_la_CPPFLAGS = -I../BinaryClassifier \
-I../../Grid -I../../Tesseract/api \
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
-I../../Tesseract/wordrec -I../../Tesseract \
-I../../Utils \
-I../../dlib-18.4 -I../Sample
libTrainer_la_LIBADD = ../BinaryClassifier/libBinaryClassifier.la \
../../Grid/libGrid.la ../../Utils/libUtils.la \
../../dlib-18.4/dlib/libdlib.la
