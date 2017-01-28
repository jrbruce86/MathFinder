/*
 * NumCompletelyNestedBlobsFeatureExtractorDescription.cpp
 *
 *  Created on: Dec 11, 2016
 *      Author: jake
 */

#include <NestedDesc.h>

#include <BlobFeatExtCat.h>

#include <string>

NumCompletelyNestedBlobsFeatureExtractorDescription
::NumCompletelyNestedBlobsFeatureExtractorDescription(
      BlobFeatureExtractorCategory* const category) {
  this->category = category;
}

NumCompletelyNestedBlobsFeatureExtractorDescription::
~NumCompletelyNestedBlobsFeatureExtractorDescription() {
  std::vector<FeatureExtractorFlagDescription*> flags = getFlagDescriptions();
  for(int i = 0; i < flags.size(); ++i) {
    delete flags[i];
  }
}

std::string NumCompletelyNestedBlobsFeatureExtractorDescription
::getName() {
  return "NumCompletelyNestedBlobsFeatureExtractor";
}

std::string NumCompletelyNestedBlobsFeatureExtractorDescription
::getUniqueName() {
  return determineUniqueName();
}

BlobFeatureExtractorCategory* NumCompletelyNestedBlobsFeatureExtractorDescription
::getCategory() {
  return category;
}

std::string NumCompletelyNestedBlobsFeatureExtractorDescription
::getDescriptionText() {
  return "TODO Fill in later.";
}

