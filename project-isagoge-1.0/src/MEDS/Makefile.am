lib_LTLIBRARIES = libMEDS.la
libMEDS_la_SOURCES = MEDS.h
SUBDIRS = ../Detection
libMEDS_la_LIBADD = /usr/local/lib/liblept.so \
../Tesseract/api/libtesseract.la ../Grid/libGrid.la \
../Utils/libUtils.la ../dlib-18.4/dlib/libdlib.la \
../Detection/libDetection.la
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
-I../Tesseract/wordrec -I../Tesseract -I../Grid \
-I../dlib-18.4 -I../Utils \
-I../Detection -I../Detection/Trainer \
-I../Detection/FeatureExtractor \
-I../Detection/FeatureExtractor/Implementations \
-I../Detection/FeatureExtractor/Implementations/F_Ext1 \
-I../Detection/FeatureExtractor/NGrams \
-I../Detection/Trainer -I../Detection/Sample \
-I../Detection/BinaryClassifier \
-I../Detection/TessInterface


##TODO: Make a special library/include directory for detection predictors and include it here.. training is separate and actually requires the MEDS library so separating out the prediction and training functionality helps to avoid circular dependencies!!
