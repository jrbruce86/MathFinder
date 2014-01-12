lib_LTLIBRARIES = libSegmentation.la
libSegmentation_la_SOURCES = Segmentation.h ISegmentor.h Segmentor1.h Segmentor1.cpp
libSegmentation_la_LIBADD = ../Utils/libUtils.la \
../Tesseract/api/libtesseract.la ../Grid/libGrid.la \
../Detection/FeatureExtractor/Implementations/F_Ext1/libF_Ext1.la
libSegmentation_la_CPPFLAGS = -I../Utils -I../Grid \
-I../Tesseract/api \
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
-I../Tesseract/wordrec -I../Tesseract \
-I../Detection/FeatureExtractor/NGrams \
-I../Detection/FeatureExtractor/NGrams/NGProfileGen \
-I../Detection/FeatureExtractor/Sentence \
-I../Detection/FeatureExtractor/Implementations/F_Ext1
