lib_LTLIBRARIES = libDetect.la
SUBDIRS = BinaryClassifier FeatureExtractor Trainer
libDetect_la_SOURCES = Detection.h
libDetect_la_CPPFLAGS = -IBinaryClassifier -IFeatureExtractor -ITrainer
libDetect_la_LIBADD = BinaryClassifier/libBinaryClassifier.la \
FeatureExtractor/libFeatureExtractor.la Trainer/libTrainer.la
