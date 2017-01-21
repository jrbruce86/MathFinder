/*
 * NumVerticallyStackedBlobsFeatureExtractor.h
 *
 *  Created on: Nov 13, 2016
 *      Author: jake
 */

#ifndef NUMVERTICALLYSTACKEDBLOBSFEATUREEXTRACTOR_H_
#define NUMVERTICALLYSTACKEDBLOBSFEATUREEXTRACTOR_H_

#include <BlobFeatExt.h>
#include <StackedDesc.h>
#include <BlobDataGrid.h>
#include <DoubleFeature.h>
#include <BlobData.h>
#include <BlobFeatExtDesc.h>
#include <StackedData.h>
#include <Direction.h>
#include <FinderInfo.h>

#include <baseapi.h>

#include <vector>
#include <stddef.h>
#include <string>

/**
 * Counts the number of "vertically stacked" neighbors (including the blob itself)
 * at the given blob's position. Neighbor is vertically stacked if it is within a
 * vertical distance <= half the current blob's height.
 * (count of stacked characters at character position (coscacp))
 */
class NumVerticallyStackedBlobsFeatureExtractor
: public virtual BlobFeatureExtractor {
 public:

  NumVerticallyStackedBlobsFeatureExtractor(
      NumVerticallyStackedBlobsFeatureExtractorDescription* const description,
      FinderInfo* const finderInfo);

  void doPreprocessing(BlobDataGrid* const blobDataGrid);

  std::vector<DoubleFeature*> extractFeatures(BlobData* const blob);

  BlobFeatureExtractorDescription* getFeatureExtractorDescription();

  /**
   * Returns true if the neighbor blob is at a distance <= half the curblob's
   * height/width if dir is vertical/horizontal respectively. if dimblob is
   * non-null then uses the same distance as with curblob but instead used
   * dimblob's dimensions for the thresholding.
   */
  static bool isAdjacent(BlobData* const neighbor, BlobData* const curblob, const BlobSpatial::Direction dir,
      const bool seg_mode, TBOX* const dimblob=NULL);

  void doSegmentationInit(BlobDataGrid* blobDataGrid);

  NumVerticallyStackedBlobsData* getBlobFeatureData(BlobData* const blobData);

 private:

  /**
   * Count "stacked" neighbors either above or below depending on direction provided
   */
  int countStacked(BlobData* const blobData, BlobDataGrid* const blobDataGrid, const BlobSpatial::Direction dir);

  int blobDataKey;

  NumVerticallyStackedBlobsFeatureExtractorDescription* description;

  std::string stackedDirPath;

  const float wordConfidenceThresh;
};


#endif /* NUMVERTICALLYSTACKEDBLOBSFEATUREEXTRACTOR_H_ */
