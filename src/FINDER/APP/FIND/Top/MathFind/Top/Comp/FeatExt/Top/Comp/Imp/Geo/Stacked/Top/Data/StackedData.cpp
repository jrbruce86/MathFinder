/*
 * NumVerticallyStackedBlobsData.cpp
 *
 *  Created on: Dec 4, 2016
 *      Author: jake
 */

#include <StackedData.h>

#include <BlobData.h>

#include <baseapi.h>

NumVerticallyStackedBlobsData::NumVerticallyStackedBlobsData()
: hasBeenProcessed_(false) {}

GenericVector<BlobData*>& NumVerticallyStackedBlobsData::getStackedBlobs() {
  return stackedBlobs;
}
void NumVerticallyStackedBlobsData::setHasBeenProcessed(const bool hasBeenProcessed_) {
  this->hasBeenProcessed_ = hasBeenProcessed_;
}
bool NumVerticallyStackedBlobsData::hasBeenProcessed() {
  return hasBeenProcessed_;
}
void NumVerticallyStackedBlobsData::setStackedBlobsCount(const int stackedBlobsCount_) {
  this->stackedBlobsCount_ = stackedBlobsCount_;
}
int NumVerticallyStackedBlobsData::getStackedBlobsCount() {
  return stackedBlobsCount_;
}



