/*
 * OtherRecognitionFeatureExtractor.h
 *
 *  Created on: Dec 4, 2016
 *      Author: jake
 */

#ifndef OTHERRECOGNITIONFEATUREEXTRACTOR_H_
#define OTHERRECOGNITIONFEATUREEXTRACTOR_H_

#include <BlobFeatExt.h>
#include <OtherRecDesc.h>
#include <BlobDataGrid.h>
#include <DoubleFeature.h>
#include <BlobData.h>
#include <BlobFeatExtDesc.h>
#include <CharData.h>
#include <FeatExtFlagDesc.h>
#include <StopwordHelper.h>

#include <baseapi.h>

#include <vector>
#include <string>

class OtherRecognitionFeatureExtractor : public virtual BlobFeatureExtractor {

 public:

  OtherRecognitionFeatureExtractor(
      OtherRecognitionFeatureExtractorDescription* const description);

  void doTrainerInitialization();

  void doFinderInitialization();

  void doPreprocessing(BlobDataGrid* const blobDataGrid);

  std::vector<DoubleFeature*> extractFeatures(BlobData* const blob);

  BlobFeatureExtractorDescription* getFeatureExtractorDescription();

  std::vector<FeatureExtractorFlagDescription*> getEnabledFlagDescriptions();

  void enableVdarbFlag();
  void enableHeightFlag();
  void enableWidthHeightFlag();
  void enableIsOcrMathFlag();
  void enableIsItalicFlag();
  void enableConfidenceFlag();
  void enableIsOcrValidFlag();
  void enableIsOnValidOcrRowFlag();
  void enableIsOnBadPageFlag();
  void enableIsInOcrStopwordFlag();

 private:

  double findBaselineDist(TesseractCharData* tesseractCharData);

  bool vdarbFlagEnabled;
  bool heightFlagEnabled;
  bool widthHeightFlagEnabled;
  bool isOcrMathFlagEnabled;
  bool isItalicFlagEnabled;
  bool confidenceFlagEnabled;
  bool isOcrValidFlagEnabled;
  bool isOnValidOcrRowFlagEnabled;
  bool isOnBadPageFlagEnabled;
  bool isInOcrStopwordFlagEnabled;

  BlobDataGrid* blobDataGrid;

  OtherRecognitionFeatureExtractorDescription* description;
  std::vector<FeatureExtractorFlagDescription*> enabledFlagDescriptions;

  StopwordFileReader* stopwordHelper;

  double avg_blob_height;
  double avg_whr;

  bool bad_page;

  GenericVector<std::string> mathwords;

  double avg_confidence;   // average ocr confidence

  std::string dbgdir;
};


#endif /* OTHERRECOGNITIONFEATUREEXTRACTOR_H_ */
