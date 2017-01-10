/*
 * MathExpressionSegmentorFactory.cpp
 *
 *  Created on: Dec 7, 2016
 *      Author: jake
 */

#include <SegFac.h>

#include <Seg.h>
#include <FinderInfo.h>
#include <FeatExt.h>
#include <HeuristicMerge.h>

#include <string>
#include <iostream>
#include <vector>

MathExpressionSegmentorFactory::MathExpressionSegmentorFactory()
: heuristicSegmentorName("heuristicMerge") {
  supportedSegmentorNames.push_back(heuristicSegmentorName);
}

MathExpressionSegmentor* MathExpressionSegmentorFactory::
createMathExpressionSegmentor(FinderInfo* const finderInfo,
    MathExpressionFeatureExtractor* const mathExpressionFeatureExtractor) {

  const std::string segmentorName = finderInfo->getSegmentorName();
  if(segmentorName == heuristicSegmentorName) {
    return new HeuristicMerge(mathExpressionFeatureExtractor);
  }

  std::cout << "Error: Could not find the segmentor named " << finderInfo->getDetectorName() << "\n";
  return NULL;
}

std::vector<std::string> MathExpressionSegmentorFactory::getSupportedSegmentorNames() {
  return supportedSegmentorNames;
}

