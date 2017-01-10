/**************************************************************************
 * File name:   HeuristicMerge.h
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Oct 27, 2013 1:29:14 AM
 *              Updated Oct 30, 2016
 ***************************************************************************/
#ifndef HEURISTICMERGE_H
#define HEURISTICMERGE_H

#include <Seg.h>
#include <FeatExt.h>
#include <MFinderResults.h>
#include <BlobDataGrid.h>
#include <BlobData.h>
#include <Direction.h>
#include <AlignedFeatExt.h>
#include <StackedFeatExt.h>

#include <allheaders.h>

#include <vector>
#include <string>

class HeuristicMerge : public virtual MathExpressionSegmentor {
 public:
  HeuristicMerge(MathExpressionFeatureExtractor* const featureExtractor);

  MathExpressionFinderResults* runSegmentation(BlobDataGrid* grid);

  ~HeuristicMerge();

 protected:
  void setDbgImg(Pix* im);
  void decideAndMerge(BlobData* blob, const int& seg_id);
  void mergeDecision(BlobData* blob, BlobSpatial::Direction dir);
  void checkIntersecting(BlobData* blob);
  void mergeOperation(BlobData* merge_from, BlobData* to_merge,
      BlobSpatial::Direction merge_dir);

  bool isOperator(BlobData* blob);
  bool wasAlreadyMerged(BlobData* neighbor, BlobData* blob);

 private:

  /**
   * Gets a list of all of the blobs having the same parent as the given blob
   * (includes the blob itself in the list)
   */
  std::vector<BlobData*> getBlobsWithSameParent(BlobData* const blobData);

  /**
   * Gets the feature extractor with the given name if exists. If not then creates it.
   * Caller has to cast back to correct type.
   */
  BlobFeatureExtractor* getFeatureExtractor(const std::string& name);

  /**
   * Gets a visual display of the results of segmentation.
   * The pix memory is allocated on the heap and owned by the caller
   */
  Pix* getVisualResultsDisplay();

  MathExpressionFeatureExtractor* featureExtractor;

  Pix* dbgim;

  NumAlignedBlobsFeatureExtractor* numAlignedBlobsFeatureExtractor;
  NumVerticallyStackedBlobsFeatureExtractor* numVerticallyStackedFeatureExtractor;
  BlobDataGrid* blobDataGrid;
};

#endif
