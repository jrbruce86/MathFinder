/*
 * NumAlignedBlobsFeatureExtractor.h
 *
 *  Created on: Nov 11, 2016
 *      Author: jake
 */

#ifndef NUMALIGNEDBLOBSFEATUREEXTRACTOR_H_
#define NUMALIGNEDBLOBSFEATUREEXTRACTOR_H_

#include <BlobFeatExt.h>
#include <AlignedDesc.h>
#include <BlobDataGrid.h>
#include <DoubleFeature.h>
#include <BlobData.h>
#include <BlobFeatExtDesc.h>
#include <FeatExtFlagDesc.h>
#include <Direction.h>
#include <AlignedData.h>

#include <baseapi.h>

#include <vector>

class NumAlignedBlobsFeatureExtractor : public virtual BlobFeatureExtractor {
 public:

  NumAlignedBlobsFeatureExtractor(NumAlignedBlobsFeatureExtractorDescription* description);

  void doPreprocessing(BlobDataGrid* const blobDataGrid);

  std::vector<DoubleFeature*> extractFeatures(BlobData* const blob);

  BlobFeatureExtractorDescription* getFeatureExtractorDescription();

  std::vector<FeatureExtractorFlagDescription*> getEnabledFlagDescriptions();


  void enableRightwardFeature();
  void enableDownwardFeature();
  void enableUpwardFeature();

  int countCoveredBlobs(BlobData* const blob, BlobDataGrid* const blobDataGrid, BlobSpatial::Direction dir, bool segmode=false);

  /**
   * Specialized initialization for when this feature extractor
   * is used for segmentation purposes
   */
  void doSegmentationInit(BlobDataGrid* blobDataGrid);

  /**
   * Gets the data associated with the given blob that is
   * relevant to this feature extractor
   */
  NumAlignedBlobsData* getBlobFeatureData(BlobData* blobData);

 private:


  /**
   * Determines whether or not a neighbor bounding box is "covered" by the current
   * bounding box based on distance vs size relationships between the two. A neighbor
   * bounding box is "covered" by the current bounding box in a given direction when
   * it resides entirely within the boundary of the current bounding box, is close
   * enough to the current object in the desired direction, is not too small in
   * relation to the current object, and has a size greater than or equal to half
   * of the its distance from the current object (where size is the greater of width
   * or height). If seg_mode is true then the current blob's bounding box is assumed
   * to correspond with that blob's segmentation results and some of the paramters are
   * slightly tweaked.
   */
  bool isNeighborCovered(const TBOX& neighbor, const TBOX& blob, const BlobSpatial::Direction& dir,
      const bool seg_mode);

  int blobDataKey;

  NumAlignedBlobsFeatureExtractorDescription* description;
  std::vector<FeatureExtractorFlagDescription*> enabledFlagDescriptions;
  bool rightwardFeatureEnabled;
  bool downwardFeatureEnabled;
  bool upwardFeatureEnabled;

  const float highCertaintyThresh;

  // dbg
  Pix* rightwardIm; // colors blobs with one or more rightward adjacent neighbors red
  bool indbg;
  bool dbgdontcare;
};

#endif /* NUMALIGNEDBLOBSFEATUREEXTRACTOR_H_ */
