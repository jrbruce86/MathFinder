/*
 * GeometryBasedExtractorCategory.cpp
 *
 *  Created on: Dec 11, 2016
 *      Author: jake
 */

#include <GeometryCat.h>

#include <AlignedFac.h>
#include <NestedFac.h>
#include <StackedFac.h>
#include <BlobFeatExtCat.h>
#include <BlobFeatExtFac.h>

#include <string>
#include <vector>
#include <stddef.h>
#include <assert.h>

GeometryBasedExtractorCategory::GeometryBasedExtractorCategory() {
  this->featureExtractorFactories.push_back(new NumAlignedBlobsFeatureExtractorFactory(this));
  this->featureExtractorFactories.push_back(new NumCompletelyNestedBlobsFeatureExtractorFactory(this));
  this->featureExtractorFactories.push_back(new NumVerticallyStackedBlobsFeatureExtractorFactory(this));
}


std::string GeometryBasedExtractorCategory
::getName() {
  return "spatial-based features";
}

std::string GeometryBasedExtractorCategory
::getDescription() {
  return std::string("Extracts features based on spatial relationships ")
      + std::string("between connected components (blobs).");
}

BlobFeatureExtractorCategory* GeometryBasedExtractorCategory
::getParentCategory() {
  return NULL;
}

std::vector<BlobFeatureExtractorCategory*> GeometryBasedExtractorCategory
::getChildCategories() {
  return std::vector<BlobFeatureExtractorCategory*>();
}

std::vector<BlobFeatureExtractorFactory*> GeometryBasedExtractorCategory
::getFeatureExtractorFactories() {
  return featureExtractorFactories;
}


