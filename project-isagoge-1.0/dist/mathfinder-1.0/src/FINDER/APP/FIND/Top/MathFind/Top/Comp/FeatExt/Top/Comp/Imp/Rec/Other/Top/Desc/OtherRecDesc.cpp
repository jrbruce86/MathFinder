/*
 * OtherRecognitionFeatureExtractorDescription.cpp
 *
 *  Created on: Dec 11, 2016
 *      Author: jake
 */
#include <OtherRecDesc.h>

#include <BlobFeatExtCat.h>
#include <OtherRecFlagDesc.h>
#include <BlobFeatExtCat.h>
#include <RecCat.h>
#include <FeatExtFlagDesc.h>

#include <string>
#include <vector>

OtherRecognitionFeatureExtractorDescription
::OtherRecognitionFeatureExtractorDescription(
    RecognitionBasedExtractorCategory* const category) {
  this->category = category;

  this->vdarbFlag = new VerticalDistanceAboveRowBaselineFlagDescription;
  this->heightFlag = new HeightToAverageHeightRatioFlagDescription;
  this->widthHeightFlag = new WidthHeightRatioToAverageRatioFlagDescription;
  this->isOcrMathFlag = new IsPartOfRecognizedMathSymbolFlagDescription;
  this->isItalicFlag = new IsItalicFlagDescription;
  this->confidenceFlag = new ConfidenceToAverageRatioFlagDescription;
  this->isOcrValidFlag = new IsValidAccordingToTesseractFlagDescription;
  this->isOnValidOcrRowFlag = new BelongsToRowWithRecognizedTextFlagDescription;
  this->isOnBadPageFlag = new BelongsToBadPageFlagDescription;
  this->isInOcrStopwordFlag = new BelongsToStopWordFlagDescription;

  this->flagDescriptions.push_back(vdarbFlag);
  this->flagDescriptions.push_back(heightFlag);
  this->flagDescriptions.push_back(widthHeightFlag);
  this->flagDescriptions.push_back(isOcrMathFlag);
  this->flagDescriptions.push_back(isItalicFlag);
  this->flagDescriptions.push_back(confidenceFlag);
  this->flagDescriptions.push_back(isOcrValidFlag);
  this->flagDescriptions.push_back(isOnValidOcrRowFlag);
  this->flagDescriptions.push_back(isOnBadPageFlag);
  this->flagDescriptions.push_back(isInOcrStopwordFlag);
}

std::string OtherRecognitionFeatureExtractorDescription
::getName() {
  return "OtherRecognitionFeatureExtractor";
}

std::string OtherRecognitionFeatureExtractorDescription
::getUniqueName() {
  return determineUniqueName();
}

RecognitionBasedExtractorCategory* OtherRecognitionFeatureExtractorDescription
::getCategory() {
  return category;
}

std::vector<FeatureExtractorFlagDescription*> OtherRecognitionFeatureExtractorDescription
::getFlagDescriptions() {
  return flagDescriptions;
}

std::string OtherRecognitionFeatureExtractorDescription
::getDescriptionText() {
  return "Contains various feature extractors that measure small bits of OCR results in each connected component.";
}

VerticalDistanceAboveRowBaselineFlagDescription*
OtherRecognitionFeatureExtractorDescription::getVdarbFlag() {
  return vdarbFlag;
}
HeightToAverageHeightRatioFlagDescription*
OtherRecognitionFeatureExtractorDescription::getHeightFlag() {
  return heightFlag;
}
WidthHeightRatioToAverageRatioFlagDescription*
OtherRecognitionFeatureExtractorDescription::getWidthHeightFlag() {
  return widthHeightFlag;
}
IsPartOfRecognizedMathSymbolFlagDescription*
OtherRecognitionFeatureExtractorDescription::getIsOcrMathFlag() {
  return isOcrMathFlag;
}
IsItalicFlagDescription*
OtherRecognitionFeatureExtractorDescription::getIsItalicFlag() {
  return isItalicFlag;
}
ConfidenceToAverageRatioFlagDescription*
OtherRecognitionFeatureExtractorDescription::getConfidenceFlag() {
  return confidenceFlag;
}
IsValidAccordingToTesseractFlagDescription*
OtherRecognitionFeatureExtractorDescription::getIsOcrValidFlag() {
  return isOcrValidFlag;
}
BelongsToRowWithRecognizedTextFlagDescription*
OtherRecognitionFeatureExtractorDescription::getIsOnValidOcrRowFlag() {
  return isOnValidOcrRowFlag;
}
BelongsToBadPageFlagDescription*
OtherRecognitionFeatureExtractorDescription::getIsOnBadPageFlag() {
  return isOnBadPageFlag;
}
BelongsToStopWordFlagDescription*
OtherRecognitionFeatureExtractorDescription::getIsInOcrStopwordFlag() {
  return isInOcrStopwordFlag;
}
