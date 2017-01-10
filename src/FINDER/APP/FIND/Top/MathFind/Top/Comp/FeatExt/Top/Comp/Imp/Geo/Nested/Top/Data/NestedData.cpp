/*
 * NumCompletelyNestedBlobsData.cpp
 *
 *  Created on: Dec 4, 2016
 *      Author: jake
 */

#include <NestedData.h>

NumCompletelyNestedBlobsData::NumCompletelyNestedBlobsData() : nestedBlobsCount(0) {}

NumCompletelyNestedBlobsData* NumCompletelyNestedBlobsData::setNestedBlobsCount(const int nestedBlobsCount) {
  this->nestedBlobsCount = nestedBlobsCount;
  return this;
}
int NumCompletelyNestedBlobsData::getNestedBlobsCount() {
  return nestedBlobsCount;
}



