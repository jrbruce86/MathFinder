/*
 * NumVerticallyStackedBlobsFeatureExtractorFactory.cpp
 *
 *  Created on: Dec 4, 2016
 *      Author: jake
 */

#include <StackedFac.h>

NumVerticallyStackedBlobsFeatureExtractorFactory
::NumVerticallyStackedBlobsFeatureExtractorFactory(
    BlobFeatureExtractorCategory* const category) {
  this->description =
      new NumVerticallyStackedBlobsFeatureExtractorDescription(
          category);
}

NumVerticallyStackedBlobsFeatureExtractorFactory::
~NumVerticallyStackedBlobsFeatureExtractorFactory() {
  delete description;
}

NumVerticallyStackedBlobsFeatureExtractor*
NumVerticallyStackedBlobsFeatureExtractorFactory
::create(FinderInfo* const finderInfo) {
  return new NumVerticallyStackedBlobsFeatureExtractor(description, finderInfo);
}

BlobFeatureExtractorDescription* NumVerticallyStackedBlobsFeatureExtractorFactory::getDescription() {
  return description;
}

