lib_LTLIBRARIES = libMEDS_Trainer.la
libMEDS_Trainer_la_SOURCES = MEDS_Trainer.h
libMEDS_Trainer_la_LIBADD = /usr/local/lib/liblept.so \
../Tesseract/api/libtesseract.la \
../Utils/libUtils.la \
../Detection/libDetection.la \
../Grid/libGrid.la \
../MEDS/libMEDS.la \
../Segmentation/libSegmentation.la
libMEDS_Trainer_la_CPPFLAGS = -I../Tesseract/api \
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
-I../Detection -I../Detection/Trainer \
-I../Detection/FeatureExtractor -I../Detection/BinaryClassifier \
-I../dlib-18.4 -I../Grid -I../MEDS
