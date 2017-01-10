/*
 * OtherRecognitionFeatureExtractorFactory.cpp
 *
 *  Created on: Dec 11, 2016
 *      Author: jake
 */

#include <OtherRecFac.h>

#include <RecCat.h>
#include <OtherRec.h>
#include <FinderInfo.h>
#include <BlobFeatExtDesc.h>

#include <stddef.h>
#include <string>
#include <assert.h>

OtherRecognitionFeatureExtractorFactory
::OtherRecognitionFeatureExtractorFactory(
    RecognitionBasedExtractorCategory* const category) {
  this->description =
      new OtherRecognitionFeatureExtractorDescription(category);
}

OtherRecognitionFeatureExtractor* OtherRecognitionFeatureExtractorFactory
::create(FinderInfo* const finderInfo) {

  OtherRecognitionFeatureExtractor* result = new OtherRecognitionFeatureExtractor(description);

  const std::string vDarbFlagName = description->getVdarbFlag()->getName();
  const std::string heightFlagName = description->getHeightFlag()->getName();
  const std::string widthHeightFlagName = description->getWidthHeightFlag()->getName();
  const std::string isOcrMathFlagName = description->getIsOcrMathFlag()->getName();
  const std::string isItalicFlagName = description->getIsItalicFlag()->getName();
  const std::string confidenceFlagName = description->getConfidenceFlag()->getName();
  const std::string isOcrValidFlagName = description->getIsOcrValidFlag()->getName();
  const std::string isOnValidOcrRowFlagName = description->getIsOnValidOcrRowFlag()->getName();
  const std::string isOnBadPageFlagName = description->getIsOnBadPageFlag()->getName();
  const std::string isInOcrStopwordFlagName = description->getIsInOcrStopwordFlag()->getName();

  for(int i = 0; i < getSelectedFlags().size(); ++i) {
    const std::string selectedFlagName = getSelectedFlags()[i]->getName();
    if(selectedFlagName == vDarbFlagName) {
      result->enableVdarbFlag();
    } else if(selectedFlagName == heightFlagName) {
      result->enableHeightFlag();
    } else if(selectedFlagName == widthHeightFlagName) {
      result->enableWidthHeightFlag();
    } else if(selectedFlagName == isOcrMathFlagName) {
      result->enableIsOcrMathFlag();
    } else if(selectedFlagName == isItalicFlagName) {
      result->enableIsItalicFlag();
    } else if(selectedFlagName == confidenceFlagName) {
      result->enableConfidenceFlag();
    } else if(selectedFlagName == isOcrValidFlagName) {
      result->enableIsOcrValidFlag();
    } else if(selectedFlagName == isOnValidOcrRowFlagName) {
      result->enableIsOnValidOcrRowFlag();
    } else if(selectedFlagName == isOnBadPageFlagName) {
      result->enableIsOnBadPageFlag();
    } else if(selectedFlagName == isInOcrStopwordFlagName) {
      result->enableIsInOcrStopwordFlag();
    } else {
      assert(false); // shouldn't get here
    }
  }

  return result;
}

/**
 * An object which describes the feature extractor being created
 * by this factory.
 */
BlobFeatureExtractorDescription* OtherRecognitionFeatureExtractorFactory::getDescription() {
  return description;
}


