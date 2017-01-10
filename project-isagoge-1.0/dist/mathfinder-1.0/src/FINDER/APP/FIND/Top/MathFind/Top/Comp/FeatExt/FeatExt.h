/*
 * MathExpressionFeatureExtractor.h
 *
 *  Created on: Oct 30, 2016
 *      Author: jake
 */

#ifndef MATHEXPRESSIONFEATUREEXTRACTOR_H_
#define MATHEXPRESSIONFEATUREEXTRACTOR_H_

#include <BlobFeatExt.h>

#include <FinderInfo.h>

#include <BlobDataGrid.h>

#include <vector>

/**
 * Public API for math expression feature extraction
 */
class MathExpressionFeatureExtractor {
 public:

  /**
   * Constructor
   * Takes information about the finder this helps to create (directories, filenames, etc.)
   * Takes a list of blob feature extractors which are utilized. Both the pre-processing
   * and the individual blob feature extraction are invoked for all extractors provided
   * for each blob in the image. The results of the feature extractions are stored in the
   * blobs.
   */
  MathExpressionFeatureExtractor(FinderInfo* finderInfo,
      std::vector<BlobFeatureExtractor*> blobFeatureExtractors);

  /**
   * Initialization to be invoked when in Finder mode
   */
  void doFinderInitialization();

  /**
   * Initialization to be invoked when in Trainer mode
   * for an entire set of training images
   */
  void doTrainerInitialization();

  /**
   * Extracts features from the given grid of connected components (groups of connected
   * pixels) that will be used for mathematical expression detection and segmentation.
   * The provided grid is updated so that each entry contains its extracted features. The
   * extracted features are represented by a vector containing the features as double
   * values in the expected order depending on which feature extraction technique is
   * used for the subsequent detection and segmentation phases. Other values
   * and data structures may be required depending on the detection and segmentation
   * technique being utilized.
   */
  void extractFeatures(BlobDataGrid* const blobDataGrid);

  std::vector<BlobFeatureExtractor*> getBlobFeatureExtractors();

  ~MathExpressionFeatureExtractor(); // delete the dependencies

  FinderInfo* getFinderInfo();

 private:

  FinderInfo* finderInfo;

  std::vector<BlobFeatureExtractor*> blobFeatureExtractors;
};


#endif /* MATHEXPRESSIONFEATUREEXTRACTOR_H_ */
