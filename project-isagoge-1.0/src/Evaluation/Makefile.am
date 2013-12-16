lib_LTLIBRARIES = libevaluation.la
libevaluation_la_SOURCES = DocumentLayoutTest.h \
BipartiteGraph.h \
BipartiteGraph.cpp
libevaluation_la_LIBADD = /usr/local/lib/liblept.so \
../Tesseract/api/libtesseract.la \
../Utils/libUtils.la ../MEDS/libMEDS.la
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
-I../Tesseract/wordrec -I../Utils -I../Tesseract \
-I../MEDS -I../Grid -I../Detection/FeatureExtractor/NGrams \
-I../Detection/FeatureExtractor/Sentence -I../Detection \
-I../Detection/Sample -I../Detection/Trainer \
-I../Detection/BinaryClassifier -I../dlib-18.4 \
-I../Detection/FeatureExtractor \
-I../Detection/FeatureExtractor/Implementations \
-I../Detection/FeatureExtractor/Implementations/F_Ext1 \
-I../Detection/FeatureExtractor/NGrams/NGProfileGen \
-I../Segmentation
