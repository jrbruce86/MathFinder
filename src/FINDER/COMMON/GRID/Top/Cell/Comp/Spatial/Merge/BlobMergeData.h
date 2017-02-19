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

#include <Direction.h>

enum RESULT_TYPE {DISPLAYED, EMBEDDED, LABEL};


class BlobData;


struct Segmentation {

  Segmentation();

  ~Segmentation();

  TBOX* box; // This memory is managed by the BlobMergeData

  RESULT_TYPE res;
};

class BlobMergeData {

 public:

  BlobMergeData(Segmentation* const seedSeg, const int segId);

  ~BlobMergeData();

  bool hasNonEmptyBuffer();

  void clearBuffers(); // clear the merge buffers

  TBOX* getSegBox();

  int getSegId() const ;

  Segmentation* getSegmentation();

  // buffers
  GenericVector<BlobData*> left;
  GenericVector<BlobData*> right;
  GenericVector<BlobData*> up;
  GenericVector<BlobData*> down;
  GenericVector<BlobData*> intersecting;

 private:

  int seg_id; // unique identifier for each segment

  Segmentation* segmentation; // The grid's view of the segment
};





#endif /* BLOBMERGEDATA_H_ */
