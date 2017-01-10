/*
 * NumAlignedBlobsData.h
 *
 *  Created on: Dec 4, 2016
 *      Author: jake
 */

#ifndef NUMALIGNEDBLOBSDATA_H_
#define NUMALIGNEDBLOBSDATA_H_

#include <BlobFeatExtData.h>
#include <AlignedDesc.h>
#include <BlobMergeData.h>

#include <baseapi.h>

#include <string>

class NumAlignedBlobsData : public BlobFeatureExtractionData {

 public:

  NumAlignedBlobsData(NumAlignedBlobsFeatureExtractorDescription* const description);

  NumAlignedBlobsData* setRhabcFeature(const double feature);
  NumAlignedBlobsData* setRhabcCount(const int rhabcCount);
  int getRhabcCount();

  NumAlignedBlobsData* setUvabcFeature(const double feature);
  NumAlignedBlobsData* setUvabcCount(const int uvabcCount);
  int getUvabcCount();

  NumAlignedBlobsData* setDvabcFeature(const double feature);
  NumAlignedBlobsData* setDvabcCount(const int dvabcCount);
  int getDvabcCount();


  GenericVector<BlobData*> rhabc_blobs;
  GenericVector<BlobData*> lhabc_blobs;
  GenericVector<BlobData*> uvabc_blobs;
  GenericVector<BlobData*> dvabc_blobs;

  BlobMergeData* blobMergeInfo; // Data which may be useful during segmentation. If so, the
                                // segmentation step needs access to this data.

 private:
  /**
   * Features
   */
  // righward horizontally adjacent blobs covered
  int rhabcCount;
  const std::string rhabcFeatureName;

  // leftward horizontally adjacent blobs covered
  int lhabcCount;
  const std::string lhabcFeatureName;

  // upward vertically adjacent blobs covered
  int uvabcCount;
  const std::string uvabcFeatureName;

  // downward vertically adjacent blobs covered
  int dvabcFeatureIndex;
  int dvabcCount;
  const std::string dvabcFeatureName;

  NumAlignedBlobsFeatureExtractorDescription* description;
};


#endif /* NUMALIGNEDBLOBSDATA_H_ */
