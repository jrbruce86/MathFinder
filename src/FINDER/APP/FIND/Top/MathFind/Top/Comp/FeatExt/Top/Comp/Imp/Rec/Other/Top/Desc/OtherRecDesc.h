/*
 * OtherRecognitionFeatureExtractorDescription.h
 *
 *  Created on: Dec 11, 2016
 *      Author: jake
 */

#ifndef OTHERRECOGNITIONFEATUREEXTRACTORDESCRIPTION_H_
#define OTHERRECOGNITIONFEATUREEXTRACTORDESCRIPTION_H_

#include <BlobFeatExtDesc.h>
#include <BlobFeatExtCat.h>
#include <RecCat.h>
#include <FeatExtFlagDesc.h>
#include <OtherRecFlagDesc.h>

#include <string>
#include <vector>

class OtherRecognitionFeatureExtractorDescription :
public virtual BlobFeatureExtractorDescription {

 public:

  OtherRecognitionFeatureExtractorDescription(
      RecognitionBasedExtractorCategory* const category);

  ~OtherRecognitionFeatureExtractorDescription();

  std::string getName();

  static std::string getName_();

  std::string getUniqueName();

  RecognitionBasedExtractorCategory* getCategory();

  std::vector<FeatureExtractorFlagDescription*> getFlagDescriptions();

  std::string getDescriptionText();

  VerticalDistanceAboveRowBaselineFlagDescription* getVdarbFlag();
  HeightToAverageHeightRatioFlagDescription* getHeightFlag();
  WidthHeightRatioToAverageRatioFlagDescription* getWidthHeightFlag();
  IsPartOfRecognizedMathSymbolFlagDescription* getIsOcrMathFlag();
  IsItalicFlagDescription* getIsItalicFlag();
  ConfidenceToAverageRatioFlagDescription* getConfidenceFlag();
  IsValidAccordingToTesseractFlagDescription* getIsOcrValidFlag();
  BelongsToRowWithRecognizedTextFlagDescription* getIsOnValidOcrRowFlag();
  BelongsToBadPageFlagDescription* getIsOnBadPageFlag();
  BelongsToStopWordFlagDescription* getIsInOcrStopwordFlag();


 private:

  RecognitionBasedExtractorCategory* category;

  VerticalDistanceAboveRowBaselineFlagDescription* vdarbFlag;
  HeightToAverageHeightRatioFlagDescription* heightFlag;
  WidthHeightRatioToAverageRatioFlagDescription* widthHeightFlag;
  IsPartOfRecognizedMathSymbolFlagDescription* isOcrMathFlag;
  IsItalicFlagDescription* isItalicFlag;
  ConfidenceToAverageRatioFlagDescription* confidenceFlag;
  IsValidAccordingToTesseractFlagDescription* isOcrValidFlag;
  BelongsToRowWithRecognizedTextFlagDescription* isOnValidOcrRowFlag;
  BelongsToBadPageFlagDescription* isOnBadPageFlag;
  BelongsToStopWordFlagDescription* isInOcrStopwordFlag;
  std::vector<FeatureExtractorFlagDescription*> flagDescriptions;
};



#endif /* OTHERRECOGNITIONFEATUREEXTRACTORDESCRIPTION_H_ */
