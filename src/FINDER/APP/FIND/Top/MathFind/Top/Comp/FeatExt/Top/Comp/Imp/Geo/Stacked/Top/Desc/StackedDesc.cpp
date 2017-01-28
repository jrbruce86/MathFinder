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

NumVerticallyStackedBlobsFeatureExtractorDescription::
~NumVerticallyStackedBlobsFeatureExtractorDescription() {
  std::vector<FeatureExtractorFlagDescription*> flags = getFlagDescriptions();
  for(int i = 0; i < flags.size(); ++i) {
    delete flags[i];
  }
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


