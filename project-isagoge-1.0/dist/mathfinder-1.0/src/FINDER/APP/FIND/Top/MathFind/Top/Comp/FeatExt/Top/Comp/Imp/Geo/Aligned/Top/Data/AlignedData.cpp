/*
 * NumAlignedBlobsData.cpp
 *
 *  Created on: Dec 4, 2016
 *      Author: jake
 */

#include <AlignedData.h>

#include <DoubleFeature.h>

NumAlignedBlobsData::NumAlignedBlobsData(NumAlignedBlobsFeatureExtractorDescription* const description)
: blobMergeInfo(NULL) {
  this->description = description;
}

NumAlignedBlobsData* NumAlignedBlobsData::setRhabcFeature(const double feature) {
  appendExtractedFeature(new DoubleFeature(description, feature, description->getRightwardFlagDescription()));
  return this;
}
NumAlignedBlobsData* NumAlignedBlobsData::setRhabcCount(const int rhabcCount) {
  this->rhabcCount = rhabcCount;
  return this;
}
int NumAlignedBlobsData::getRhabcCount() {
  return rhabcCount;
}

NumAlignedBlobsData* NumAlignedBlobsData::setUvabcFeature(const double feature) {
  appendExtractedFeature(new DoubleFeature(description, feature, description->getUpwardFlagDescription()));
  return this;
}
NumAlignedBlobsData* NumAlignedBlobsData::setUvabcCount(const int uvabcCount) {
  this->uvabcCount = uvabcCount;
  return this;
}
int NumAlignedBlobsData::getUvabcCount() {
  return uvabcCount;
}

NumAlignedBlobsData* NumAlignedBlobsData::setDvabcFeature(const double feature) {
  appendExtractedFeature(new DoubleFeature(description, feature, description->getDownwardFlagDescription()));
  return this;
}
NumAlignedBlobsData* NumAlignedBlobsData::setDvabcCount(const int dvabcCount) {
  this->dvabcCount = dvabcCount;
  return this;
}
int NumAlignedBlobsData::getDvabcCount() {
  return dvabcCount;
}

