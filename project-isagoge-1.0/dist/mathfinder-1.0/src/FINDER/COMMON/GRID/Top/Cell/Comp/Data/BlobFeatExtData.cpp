/*
 * BlobFeatureExtractionData.cpp
 *
 *  Created on: Nov 16, 2016
 *      Author: jake
 */

#include <BlobFeatExtData.h>

BlobFeatureExtractionData::BlobFeatureExtractionData() {}

std::vector<DoubleFeature*> BlobFeatureExtractionData::getExtractedFeatures() {
  return extractedFeatures;
}

BlobFeatureExtractionData::~BlobFeatureExtractionData() {
  for(int i = 0; i < extractedFeatures.size(); ++i) {
    delete extractedFeatures[i];
  }
}

void BlobFeatureExtractionData::appendExtractedFeature(DoubleFeature* const feature) {
  extractedFeatures.push_back(feature);
}


