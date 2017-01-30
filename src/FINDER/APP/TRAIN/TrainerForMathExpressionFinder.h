/*
 * TrainerForMathExpressionFinder.h
 *
 *  Created on: Dec 4, 2016
 *      Author: jake
 */

#ifndef TRAINERFORMATHEXPRESSIONFINDER_H_
#define TRAINERFORMATHEXPRESSIONFINDER_H_

#include <FinderInfo.h>
#include <FeatExt.h>
#include <Detector.h>
#include <Seg.h>
#include <Sample.h>

#include <vector>

class GeometryBasedExtractorCategory;
class RecognitionBasedExtractorCategory;

class TrainerForMathExpressionFinder {

 public:

  TrainerForMathExpressionFinder(FinderInfo* finderInfo,
      MathExpressionFeatureExtractor* mathExpressionFeatureExtractor,
      MathExpressionDetector* mathExpressionDetector,
      MathExpressionSegmentor* mathExpressionSegmentor);

  void runTraining();

  std::vector<std::vector<BLSample*> > getSamples();

  ~TrainerForMathExpressionFinder();

 private:

  void trainDetector();
  void trainSegmentor();

  FinderInfo* finderInfo; // owned by this class
  MathExpressionFeatureExtractor* featureExtractor; // owned by this class
  MathExpressionDetector* detector; // owned by this class
  MathExpressionSegmentor* segmentor; // owned by this class

  std::vector<std::vector<BLSample*> > samples;
};

class TrainerForMathExpressionFinderFactory {
 public:

  /**
   * Creates the trainer, allocating on the heap. Allocated memory owned by
   * caller
   */
  TrainerForMathExpressionFinder* create(FinderInfo* const finderInfo,
      std::vector<BlobFeatureExtractorFactory*> featExtFactories);

  /**
   * Creates the trainer, allocating on the heap. Allocated memory owned by
   * caller
   */
  TrainerForMathExpressionFinder* create(FinderInfo* const finderInfo,
      GeometryBasedExtractorCategory* const spatialCategory,
      RecognitionBasedExtractorCategory* const recognitionCategory);

  /**
   * Creates the trainer, allocating on the heap. Allocated memory owned by
   * caller
   */
  TrainerForMathExpressionFinder* create(FinderInfo* const finderInfo,
      MathExpressionFeatureExtractor* const featExt);
};


#endif /* TRAINERFORMATHEXPRESSIONFINDER_H_ */
