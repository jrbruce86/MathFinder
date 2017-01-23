lib_LTLIBRARIES = libCOMMON.la

libCOMMON_la_SOURCES = GRID/BlobDataGrid.h \
UTIL/Lept_Utils.h \
UTIL/M_Utils.h \
UTIL/TessParamManager.h \
UTIL/Utils.h \
GRID/Top/Cell/BlobData.h \
GRID/Top/Fac/BlobDataGridFactory.h \
GRID/Top/Cell/Comp/Data/BlobFeatExtData.h \
GRID/Top/Cell/Comp/Spatial/Direction.h \
GRID/Top/Cell/Comp/Data/DoubleFeat/DoubleFeature.h \
GRID/Top/Cell/Comp/Data/Fac/BlobFeatExtFac.h \
GRID/Top/Cell/Comp/Data/Stopword/StopwordHelper.h \
GRID/Top/Cell/Comp/RecData/Block/BlockData.h \
GRID/Top/Cell/Comp/RecData/Block/OldDebugMethods.h \
GRID/Top/Cell/Comp/RecData/Char/CharData.h \
GRID/Top/Cell/Comp/RecData/Row/RowData.h \
GRID/Top/Cell/Comp/RecData/Word/WordData.h \
GRID/Top/Cell/Comp/Spatial/Merge/BlobMergeData.h \
GRID/Top/Cell/Comp/Data/Desc/Cat/BlobFeatExtCat.h \
GRID/Top/Cell/Comp/Data/Desc/Desc/BlobFeatExtDesc.h \
GRID/Top/Cell/Comp/Data/Desc/Flag/FeatExtFlagDesc.h \
GRID/Top/Cell/Comp/Data/Desc/Flag/Empty/EmptyFlagDesc.h \
GRID/Top/Cell/Comp/Data/NGram/NGProfile/NGram.h \
GRID/Top/Cell/Comp/Data/NGram/NGProfile/NGramRanker.h \
GRID/Top/Cell/Comp/RecData/Block/Sentence/SentenceData.h \
GRID/BlobDataGrid.cpp \
UTIL/Lept_Utils.cpp \
UTIL/M_Utils.cpp \
UTIL/TessParamManager.cpp \
UTIL/Utils.cpp \
GRID/Top/Cell/BlobData.cpp \
GRID/Top/Fac/BlobDataGridFactory.cpp \
GRID/Top/Cell/Comp/Data/BlobFeatExtData.cpp \
GRID/Top/Cell/Comp/Data/DoubleFeat/DoubleFeature.cpp \
GRID/Top/Cell/Comp/Data/Fac/BlobFeatExtFac.cpp \
GRID/Top/Cell/Comp/Data/Stopword/StopwordHelper.cpp \
GRID/Top/Cell/Comp/RecData/Block/BlockData.cpp \
GRID/Top/Cell/Comp/RecData/Char/CharData.cpp \
GRID/Top/Cell/Comp/RecData/Row/RowData.cpp \
GRID/Top/Cell/Comp/RecData/Word/WordData.cpp \
GRID/Top/Cell/Comp/Spatial/Merge/BlobMergeData.cpp \
GRID/Top/Cell/Comp/Data/Desc/Cat/BlobFeatExtCat.cpp \
GRID/Top/Cell/Comp/Data/Desc/Desc/BlobFeatExtDesc.cpp \
GRID/Top/Cell/Comp/Data/Desc/Flag/FeatExtFlagDesc.cpp \
GRID/Top/Cell/Comp/Data/Desc/Flag/Empty/EmptyFlagDesc.cpp \
GRID/Top/Cell/Comp/Data/NGram/NGProfile/NGramRanker.cpp \
GRID/Top/Cell/Comp/RecData/Block/Sentence/SentenceData.cpp

tesspath=../../THIRDPARTY/Tesseract

libCOMMON_la_CPPFLAGS = -IGRID \
-IGRID/Top/Cell/Comp/Spatial/Merge \
-IGRID/Top/Cell \
-IGRID/Top/Cell/Comp/Data \
-IGRID/Top/Cell/Comp/Data/DoubleFeat \
-IGRID/Top/Cell/Comp/Data/Desc/Desc \
-IGRID/Top/Cell/Comp/RecData/Char \
-IGRID/Top/Cell/Comp/RecData/Row \
-IUTIL \
-IGRID/Top/Fac \
-IGRID/Top/Cell/Comp/RecData/Block \
-IGRID/Top/Cell/Comp/RecData/Word \
-IGRID/Top/Cell/Comp/Data/Fac \
-IGRID/Top/Cell/Comp/Data/Stopword \
-IGRID/Top/Cell/Comp/Data/Desc/Cat \
-IGRID/Top/Cell/Comp/Data/Desc/Flag \
-IGRID/Top/Cell/Comp/Data/Desc/Flag/Empty \
-IGRID/Top/Cell/Comp/Data/NGram/NGProfile \
-IGRID/Top/Cell/Comp/RecData/Block/Sentence \
-I/usr/local/include/leptonica \
-I$(tesspath)/api \
-I$(tesspath)/ccmain -I$(tesspath)/ccstruct \
-I$(tesspath)/ccutil -I$(tesspath)/classify \
-I$(tesspath)/config -I$(tesspath)/contrib \
-I$(tesspath)/cube -I$(tesspath)/cutil \
-I$(tesspath)/dict -I$(tesspath)/doc \
-I$(tesspath)/image -I$(tesspath)/java \
-I$(tesspath)/m4 -I$(tesspath)/neural_networks \
-I$(tesspath)/po -I$(tesspath)/tessdata \
-I$(tesspath)/testing -I$(tesspath)/textord \
-I$(tesspath)/training -I$(tesspath)/viewer \
-I$(tesspath)/wordrec -I$(tesspath)

libCOMMON_la_LIBADD = /usr/local/lib/liblept.so \
$(tesspath)/api/libtesseract.la

