/*
 * BlobMergeData.h
 *
 *  Created on: Dec 18, 2016
 *      Author: jake
 */

#ifndef BLOBMERGEDATA_H_
#define BLOBMERGEDATA_H_

#include <stddef.h>
#include <baseapi.h>

enum RESULT_TYPE {DISPLAYED, EMBEDDED, LABEL};


class BlobData;

struct Segmentation {

  Segmentation();

  ~Segmentation();

  TBOX* box;

  RESULT_TYPE res;
};

struct BlobMergeData {

  BlobMergeData();

  bool is_processed;
  inT32 processed_seg_area; // the segment area for when the blob was processed
                          // blob may need to be reprocessed if this area differs
                          // when the same blob is reached multiple times
  int seg_id; // unique identifier for each segment
  GenericVector<BlobData*> left;
  GenericVector<BlobData*> right;
  GenericVector<BlobData*> up;
  GenericVector<BlobData*> down;
  GenericVector<BlobData*> intersecting;
  TBOX* segment_box; // stored in the BlobInfoGrid class's "Segments" GenericVector
};





#endif /* BLOBMERGEDATA_H_ */
