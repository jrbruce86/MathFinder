/*
 * BlobFeatureExtractor.h
 *
 *  Created on: Nov 9, 2016
 *      Author: jake
 */

#ifndef BLOBFEATUREEXTRACTOR_H_
#define BLOBFEATUREEXTRACTOR_H_

#include <BlobDataGrid.h>
#include <DoubleFeature.h>
#include <BlobData.h>
#include <BlobFeatExtDesc.h>

#include <vector>

class BlobFeatureExtractor {

 public:

  BlobFeatureExtractor();

  /**
   * Called once during initialization before using this extractor for training
   * purposes only.
   *
   */
  virtual void doTrainerInitialization();

  /**
   * Called once during initialization before using this extractor for finding
   * (detection and segmentation) purposes only.
   */
  virtual void doFinderInitialization();

  /**
   * Does any processing that requires looking at an entire page up front
   */
  virtual void doPreprocessing(BlobDataGrid* const blobDataGrid) = 0;

  /**
   * Extracts the features from the given blob while persisting any data
   * that may be needed later into the blob's variable data vector. The
   * feature extracted are formatted as a vector of double values normalized
   * for later use by a binary classifier in the detection stage. The persisted
   * data may be needed by the segmentation stage. Returns the extracted features
   * as a vector of doubles.
   */
  virtual std::vector<DoubleFeature*> extractFeatures(BlobData* const blob) = 0;

  /**
   * Gets the description of this feature extractor. The memory pointed at
   * is owned by implementations of this class.
   */
  virtual BlobFeatureExtractorDescription* getFeatureExtractorDescription() = 0;

  virtual std::vector<FeatureExtractorFlagDescription*> getEnabledFlagDescriptions();

  virtual ~BlobFeatureExtractor();

 protected:
  /**
   * Determines the index in each blob's variable data vector where this
   * feature extractor will place its data. This is done by just getting
   * the size of the first blob's vector (all of the vectors will have the
   * same amount of data in the same order). Each feature extractor holds
   * onto that index and uses it to look up its data. Once the data is retrieved
   * it is cast back into something the feature extractor finds useful
   * somehow.
   */
  int findOpenBlobDataIndex(BlobDataGrid* const blobDataGrid);

};


#endif /* BLOBFEATUREEXTRACTOR_H_ */
