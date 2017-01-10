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
  this->blobFeatureExtractorDescription =
      new NumVerticallyStackedBlobsFeatureExtractorDescription(
          category);
}

NumVerticallyStackedBlobsFeatureExtractor*
NumVerticallyStackedBlobsFeatureExtractorFactory
::create(FinderInfo* const finderInfo) {
  return new NumVerticallyStackedBlobsFeatureExtractor(blobFeatureExtractorDescription);
}

BlobFeatureExtractorDescription* NumVerticallyStackedBlobsFeatureExtractorFactory::getDescription() {
  return blobFeatureExtractorDescription;
}

