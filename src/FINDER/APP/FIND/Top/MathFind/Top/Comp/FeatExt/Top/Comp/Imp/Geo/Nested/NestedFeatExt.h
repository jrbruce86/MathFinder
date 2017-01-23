/*
 * NumCompletelyNestedBlobsFeatureExtractor.h
 *
 *  Created on: Nov 13, 2016
 *      Author: jake
 */

#ifndef NUMCOMPLETELYNESTEDBLOBSFEATUREEXTRACTOR_H_
#define NUMCOMPLETELYNESTEDBLOBSFEATUREEXTRACTOR_H_

#include <BlobFeatExt.h>
#include <NestedDesc.h>
#include <BlobDataGrid.h>
#include <BlobData.h>
#include <BlobFeatExtDesc.h>
#include <DoubleFeature.h>
#include <FinderInfo.h>

#include <vector>

class NumCompletelyNestedBlobsFeatureExtractor
: public virtual BlobFeatureExtractor {

 public:

  NumCompletelyNestedBlobsFeatureExtractor(
      NumCompletelyNestedBlobsFeatureExtractorDescription* const description,
      FinderInfo* const finderInfo);

  void doPreprocessing(BlobDataGrid* const blobDataGrid);

  virtual std::vector<DoubleFeature*> extractFeatures(BlobData* const blob);

  BlobFeatureExtractorDescription* getFeatureExtractorDescription();

 private:

  int countNestedBlobs(BlobData* const blob, BlobDataGrid* const blobDataGrid);

  int blobDataKey;

  NumCompletelyNestedBlobsFeatureExtractorDescription* description;

  const float highCertaintyThresh;

  // dbg
  std::string nestedDir;
};

#endif /* NUMCOMPLETELYNESTEDBLOBSFEATUREEXTRACTOR_H_ */
