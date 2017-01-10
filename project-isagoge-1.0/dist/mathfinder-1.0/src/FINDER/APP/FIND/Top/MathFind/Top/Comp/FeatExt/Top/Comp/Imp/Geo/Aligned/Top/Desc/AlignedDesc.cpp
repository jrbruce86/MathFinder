/*
 * NumAlignedBlobsFeatureExtractorDescription.cpp
 *
 *  Created on: Dec 11, 2016
 *      Author: jake
 */

#include <AlignedDesc.h>

#include <BlobFeatExtCat.h>
#include <AlignedDesc.h>
#include <FeatExtFlagDesc.h>

#include <vector>
#include <string>

NumAlignedBlobsFeatureExtractorDescription::NumAlignedBlobsFeatureExtractorDescription(
    BlobFeatureExtractorCategory* const category) {
  this->category = category;
  this->rightwardFlagDescription = new NumAlignedBlobsRightwardFeatureFlagDescription();
  this->downwardFlagDescription = new NumAlignedBlobsDownwardFeatureFlagDescription();
  this->upwardFlagDescription = new NumAlignedBlobsUpwardFeatureFlagDescription();
  this->flagDescriptions.push_back(rightwardFlagDescription);
  this->flagDescriptions.push_back(downwardFlagDescription);
  this->flagDescriptions.push_back(upwardFlagDescription);
}

std::string NumAlignedBlobsFeatureExtractorDescription
::getName() {
  return getName_();
}

std::string NumAlignedBlobsFeatureExtractorDescription
::getName_() {
  return "NumAlignedBlobsFeatureExtractor";
}

std::string NumAlignedBlobsFeatureExtractorDescription
::getUniqueName() {
  return determineUniqueName();
}

BlobFeatureExtractorCategory* NumAlignedBlobsFeatureExtractorDescription
::getCategory() {
  return category;
}

std::vector<FeatureExtractorFlagDescription*> NumAlignedBlobsFeatureExtractorDescription
::getFlagDescriptions() {
  return flagDescriptions;
}

std::string NumAlignedBlobsFeatureExtractorDescription
::getDescriptionText() {
  std::string flagDescriptionString;
  for(int i = 0; i < flagDescriptions.size(); ++i) {
    flagDescriptionString.append(
        flagDescriptions[i]->getName() + ": "
        + flagDescriptions[i]->getDescriptionText() + "\n");
  }
  return std::string("This feature extractor will count the number of neighboring blobs that are ")
      + std::string("vertically and/or horizontally aligned to the current one. ")
      + std::string("To be aligned horizontally the neighboring blob must be within a horizontal distance of ")
      + std::string("half of the current blob's height and within the vertical bounds of the current ")
      + std::string("blob's bounding box. Similar is true for vertical alignment but with the ")
      + std::string("directions reversed. The following flags can be enabled or disabled to alter behavior:\n")
      + flagDescriptionString;
}

NumAlignedBlobsRightwardFeatureFlagDescription* NumAlignedBlobsFeatureExtractorDescription::getRightwardFlagDescription() {
  return rightwardFlagDescription;
}
NumAlignedBlobsDownwardFeatureFlagDescription* NumAlignedBlobsFeatureExtractorDescription::getDownwardFlagDescription() {
  return downwardFlagDescription;
}
NumAlignedBlobsUpwardFeatureFlagDescription* NumAlignedBlobsFeatureExtractorDescription::getUpwardFlagDescription() {
  return upwardFlagDescription;
}


