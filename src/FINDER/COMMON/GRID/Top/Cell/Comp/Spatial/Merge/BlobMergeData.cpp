/*
 * BlobMergeData.cpp
 *
 *  Created on: Dec 18, 2016
 *      Author: jake
 */

#include <BlobMergeData.h>

#include <Direction.h>

#include <baseapi.h>

Segmentation::Segmentation() : box(NULL), res(RESULT_TYPE()) {}

Segmentation::~Segmentation() {
}

BlobMergeData::BlobMergeData(Segmentation* const seedSeg, const int segId) {
  this->segmentation = seedSeg;
  this->seg_id = segId;
}

BlobMergeData::~BlobMergeData() {
  delete segmentation->box;
  delete this->segmentation;
  segmentation = NULL;
  clearBuffers();
}

bool BlobMergeData::hasNonEmptyBuffer() {
  return !(up.empty() && down.empty() && left.empty()
      && right.empty() && intersecting.empty());
}

void BlobMergeData::clearBuffers() {
  up.clear();
  down.clear();
  right.clear();
  left.clear();
  intersecting.clear();
}

TBOX* BlobMergeData::getSegBox() {
  return segmentation->box;
}

int BlobMergeData::getSegId() const {
  return seg_id;
}

Segmentation* BlobMergeData::getSegmentation() {
  return segmentation;
}

