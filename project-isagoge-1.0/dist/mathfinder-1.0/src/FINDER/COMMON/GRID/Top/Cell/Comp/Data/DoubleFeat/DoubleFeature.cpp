/*
 * DoubleFeature.cpp
 *
 *  Created on: Nov 12, 2016
 *      Author: jake
 */

#include <DoubleFeature.h>

#include <string>

DoubleFeature::DoubleFeature(
    BlobFeatureExtractorDescription* featureExtractorDescription,
    const double feature,
    FeatureExtractorFlagDescription* flagDescription) {
  this->featureExtractorDescription = featureExtractorDescription;
  this->feature = feature;
  this->flagDescription = flagDescription;
}

double DoubleFeature::getFeature() {
  return feature;
}

BlobFeatureExtractorDescription* DoubleFeature
::getFeatureExtractorDescription() {
  return featureExtractorDescription;
}

FeatureExtractorFlagDescription* DoubleFeature
::getFlagDescription() {
  return flagDescription;
}

bool DoubleFeature::operator==(const DoubleFeature& other) {
  return feature == other.feature
      && featureExtractorDescription->getUniqueName() == other.featureExtractorDescription->getUniqueName()
      && flagDescription->getName() == other.flagDescription->getName();
}

