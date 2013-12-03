lib_LTLIBRARIES = libDetection.la
SUBDIRS = Sample BinaryClassifier FeatureExtractor Trainer
libDetection_la_SOURCES = Detection.h Detector.h
libDetection_la_CPPFLAGS = -ISample -IBinaryClassifier -IFeatureExtractor \
-IFeatureExtractor/Implementations -ITrainer -I../Utils 
libDetection_la_LIBADD = Sample/libGTParser.la \
BinaryClassifier/libBinaryClassifier.la \
FeatureExtractor/libFeatureExtractor.la Trainer/libTrainer.la

