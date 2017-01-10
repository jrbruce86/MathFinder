/*
 * NumVerticallyStackedBlobsFeatureExtractorDescription.cpp
 *
 *  Created on: Dec 11, 2016
 *      Author: jake
 */

#include <StackedDesc.h>

#include <BlobFeatExtCat.h>

#include <string>

NumVerticallyStackedBlobsFeatureExtractorDescription
::NumVerticallyStackedBlobsFeatureExtractorDescription(
      BlobFeatureExtractorCategory* const category) {
  this->category = category;
}

std::string NumVerticallyStackedBlobsFeatureExtractorDescription
::getName() {
  return getName_();
}

std::string NumVerticallyStackedBlobsFeatureExtractorDescription
::getUniqueName() {
  return determineUniqueName();
}

BlobFeatureExtractorCategory* NumVerticallyStackedBlobsFeatureExtractorDescription
::getCategory() {
  return category;
}

std::string NumVerticallyStackedBlobsFeatureExtractorDescription
::getDescriptionText() {
  return "TODO Fill this in...";
}

std::string NumVerticallyStackedBlobsFeatureExtractorDescription::getName_() {
  return "NumVerticallyStackedBlobsFeatureExtractor";
}


