/*
 * NumVerticallyStackedBlobsData.h
 *
 *  Created on: Dec 4, 2016
 *      Author: jake
 */

#ifndef NUMVERTICALLYSTACKEDBLOBSDATA_H_
#define NUMVERTICALLYSTACKEDBLOBSDATA_H_

#include <BlobFeatExtData.h>
#include <BlobData.h>

#include <baseapi.h>

class NumVerticallyStackedBlobsData : public BlobFeatureExtractionData {

 public:

  NumVerticallyStackedBlobsData();

  GenericVector<BlobData*>& getStackedBlobs();

  void setHasBeenProcessed(const bool hasBeenProcessed_);

  void setStackedBlobsCount(const int stackedBlobsCount_);
  int getStackedBlobsCount();

  bool hasBeenProcessed();

 private:

  int stackedBlobsCount_;
  bool hasBeenProcessed_;
  GenericVector<BlobData*> stackedBlobs;
};


#endif /* NUMVERTICALLYSTACKEDBLOBSDATA_H_ */
