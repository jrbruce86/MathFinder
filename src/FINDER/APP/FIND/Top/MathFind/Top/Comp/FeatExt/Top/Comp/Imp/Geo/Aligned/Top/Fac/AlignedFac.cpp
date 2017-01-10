/*
 * NumAlignedBlobsFeatureExtractorFactory.cpp
 *
 *  Created on: Dec 4, 2016
 *      Author: jake
 */

#include <AlignedFac.h>

#include <BlobFeatExtCat.h>
#include <AlignedDesc.h>
#include <AlignedFeatExt.h>
#include <FinderInfo.h>

#include <stddef.h>
#include <string>
#include <assert.h>

NumAlignedBlobsFeatureExtractorFactory
::NumAlignedBlobsFeatureExtractorFactory(
    BlobFeatureExtractorCategory* const category) {
  this->description =
      new NumAlignedBlobsFeatureExtractorDescription(category);
}

NumAlignedBlobsFeatureExtractor* NumAlignedBlobsFeatureExtractorFactory
::create(FinderInfo* const finderInfo) {

  const std::string rightwardFlagName = description->getRightwardFlagDescription()->getName();
  const std::string upwardFlagName = description->getUpwardFlagDescription()->getName();
  const std::string downwardFlagName = description->getDownwardFlagDescription()->getName();

  NumAlignedBlobsFeatureExtractor* result =
      new NumAlignedBlobsFeatureExtractor(description);

  for(int i = 0; i < getSelectedFlags().size(); ++i) {
    const std::string selectedFlagName = getSelectedFlags()[i]->getName();
    if(selectedFlagName == rightwardFlagName) {
      result->enableRightwardFeature();
    }
    else if(selectedFlagName == upwardFlagName) {
      result->enableUpwardFeature();
    }
    else if(selectedFlagName == downwardFlagName) {
      result->enableDownwardFeature();
    } else {
      assert(false); // Shouldn't get here
    }
  }

  return result;
}

NumAlignedBlobsFeatureExtractorDescription*
NumAlignedBlobsFeatureExtractorFactory::getDescription() {
  return description;
}


