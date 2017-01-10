/*
 * BlobFeatureExtractorFactory.cpp
 *
 *  Created on: Dec 15, 2016
 *      Author: jake
 */
#include <BlobFeatExtFac.h>

BlobFeatureExtractorFactory::BlobFeatureExtractorFactory() {}

std::ostream& operator<<(std::ostream& stream, BlobFeatureExtractorFactory& fact) {
  stream << fact.getDescription()->getName();
  if(!fact.getSelectedFlags().empty()) {
    stream << " with the following flags enabled:\n\n";
  }
  for(int i = 0; i < fact.getSelectedFlags().size(); ++i) {
    stream << "Flag " << i << ": " << *(fact.getSelectedFlags()[i]) << "\n";
  }
  return stream;
}

std::vector<FeatureExtractorFlagDescription*>& BlobFeatureExtractorFactory
::getSelectedFlags() {
  return selectedFlags;
}

BlobFeatureExtractorFactory::~BlobFeatureExtractorFactory() {
  delete getDescription();
  getSelectedFlags().clear();
}

