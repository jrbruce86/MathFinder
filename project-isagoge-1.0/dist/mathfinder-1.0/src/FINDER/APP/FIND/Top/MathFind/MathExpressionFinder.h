/**************************************************************************
 * File name:   MathExpressionFinder.h
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Oct 4, 2013 9:08:15 PM
 *              Updated Oct 30, 2016
 ***************************************************************************/

#ifndef MATHEXPRESSIONFINDER_H_
#define MATHEXPRESSIONFINDER_H_

#include <FeatExt.h>
#include <Detector.h>
#include <Seg.h>
#include <MFinderResults.h>

#include <CharData.h>

#include <BlobDataGridFactory.h>
#include <BlobDataGrid.h>

#include <M_Utils.h>

#include <vector>
#include <string>
#include <assert.h>

class MathExpressionFinder {
 public:

  MathExpressionFinder(
      MathExpressionFeatureExtractor* featureExtractor,
      MathExpressionDetector* detector,
      MathExpressionSegmentor* segmentor);


  std::vector<MathExpressionFinderResults*> findMathExpressions(Pixa* const images,
      std::vector<std::string> imageNames);

  MathExpressionFeatureExtractor* getFeatureExtractor();

  ~MathExpressionFinder();

 private:
  MathExpressionFeatureExtractor* mathExpressionFeatureExtractor;
  MathExpressionDetector* mathExpressionDetector;
  MathExpressionSegmentor* mathExpressionSegmentor;

  // internal variables/flags
  bool init;
};



#endif
