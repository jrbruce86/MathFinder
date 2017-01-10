/*
 * BlobFeatureExtractorDescription.cpp
 *
 *  Created on: Dec 22, 2016
 *      Author: jake
 */

#include <BlobFeatExtDesc.h>

std::vector<FeatureExtractorFlagDescription*> BlobFeatureExtractorDescription
::getFlagDescriptions() {
  return std::vector<FeatureExtractorFlagDescription*>();
}

BlobFeatureExtractorDescription::~BlobFeatureExtractorDescription() {
  std::vector<FeatureExtractorFlagDescription*> flags = getFlagDescriptions();
  for(int i = 0; i < flags.size(); ++i) {
    delete flags[i];
  }
}

