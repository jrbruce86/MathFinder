/*
 * RecognitionBasedExtractorCategory.cpp
 *
 *  Created on: Dec 11, 2016
 *      Author: jake
 */

#include <RecCat.h>
#include <BlobFeatExtFac.h>
#include <StopwordHelper.h>
#include <SubSupFac.h>
#include <NGFac.h>
#include <OtherRecFac.h>
#include <BlobFeatExtDesc.h>

#include <vector>
#include <stddef.h>
#include <iostream>

RecognitionBasedExtractorCategory::RecognitionBasedExtractorCategory() {
  this->stopwordHelper = new StopwordFileReader();
  this->featureExtractorFactories.push_back(new SubOrSuperscriptsFeatureExtractorFactory(this));
  this->featureExtractorFactories.push_back(new SentenceNGramsFeatureExtractorFactory(this));
  this->featureExtractorFactories.push_back(new OtherRecognitionFeatureExtractorFactory(this));
}

std::string RecognitionBasedExtractorCategory::getName() {
  return "recognition-based features";
}

std::string RecognitionBasedExtractorCategory::getDescription() {
  return std::string("Extracts features based upon the results of the Tesseract ")
      + std::string("OCR engine.");
}

BlobFeatureExtractorCategory* RecognitionBasedExtractorCategory
::getParentCategory() {
  return NULL;
}

std::vector<BlobFeatureExtractorCategory*> RecognitionBasedExtractorCategory
::getChildCategories() {
  return std::vector<BlobFeatureExtractorCategory*>();
}

std::vector<BlobFeatureExtractorFactory*> RecognitionBasedExtractorCategory
::getFeatureExtractorFactories() {
  return featureExtractorFactories;
}

StopwordFileReader* RecognitionBasedExtractorCategory::getStopwordHelper() {
  return stopwordHelper;
}

RecognitionBasedExtractorCategory::~RecognitionBasedExtractorCategory() {
  std::cout << "Recognition category desctructor invoked... Is the base class destructor invoked too??\n";
  for(int i = 0; i < featureExtractorFactories.size(); ++i) {
    delete featureExtractorFactories[i];
  }
  delete stopwordHelper;
}


