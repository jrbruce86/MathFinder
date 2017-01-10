/*
 * NumCompletelyNestedBlobsData.h
 *
 *  Created on: Dec 4, 2016
 *      Author: jake
 */

#ifndef NUMCOMPLETELYNESTEDBLOBSDATA_H_
#define NUMCOMPLETELYNESTEDBLOBSDATA_H_

#include <BlobFeatExtData.h>

class NumCompletelyNestedBlobsData : public BlobFeatureExtractionData {

 public:

  NumCompletelyNestedBlobsData();

  /**
   * Sets the number of nested blobs
   */
  NumCompletelyNestedBlobsData* setNestedBlobsCount(const int nestedCount);

  /**
   * Gets the number of nested blobs
   */
  int getNestedBlobsCount();

 private:

  int nestedBlobsCount;
};

#endif /* NUMCOMPLETELYNESTEDBLOBSDATA_H_ */
