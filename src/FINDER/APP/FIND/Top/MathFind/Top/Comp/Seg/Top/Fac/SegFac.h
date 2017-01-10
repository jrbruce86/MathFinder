/*
 * MathExpressionSegmentorFactory.h
 *
 *  Created on: Dec 7, 2016
 *      Author: jake
 */

#ifndef MATHEXPRESSIONSEGMENTORFACTORY_H_
#define MATHEXPRESSIONSEGMENTORFACTORY_H_

#include <Seg.h>
#include <FinderInfo.h>
#include <FeatExt.h>

#include <vector>
#include <string>

class MathExpressionSegmentorFactory {

 public:

  MathExpressionSegmentorFactory();

  MathExpressionSegmentor* createMathExpressionSegmentor(
      FinderInfo* const finderInfo,
      MathExpressionFeatureExtractor* const mathExpressionFeatureExtractor);


  std::vector<std::string> getSupportedSegmentorNames();

 private:

  // Supported segmentor names
  std::string heuristicSegmentorName;
  std::vector<std::string> supportedSegmentorNames; // as a list
};


#endif /* MATHEXPRESSIONSEGMENTORFACTORY_H_ */
