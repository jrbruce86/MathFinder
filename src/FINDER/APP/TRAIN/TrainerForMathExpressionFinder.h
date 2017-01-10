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


#endif /* TRAINERFORMATHEXPRESSIONFINDER_H_ */
