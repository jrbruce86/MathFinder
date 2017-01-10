/*
 * BlobMergeData.cpp
 *
 *  Created on: Dec 18, 2016
 *      Author: jake
 */

#include <BlobMergeData.h>

BlobMergeData::BlobMergeData()
: is_processed(false), processed_seg_area(-1),
  seg_id(-1), segment_box(NULL) {}

Segmentation::Segmentation() : box(NULL), res(RESULT_TYPE()) {}

Segmentation::~Segmentation() {
  if(box != NULL) {
    delete box;
    box = NULL;
  }
}
