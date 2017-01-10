/*
 * MathExpressionFeatureExtractorFactory.cpp
 *
 *  Created on: Nov 9, 2016
 *      Author: jake
 */

#include <FeatExtFac.h>

#include <FeatExt.h>

#include <BlobFeatExt.h>

#include <string>
#include <vector>


MathExpressionFeatureExtractor* MathExpressionFeatureExtractorFactory
::createMathExpressionFeatureExtractor(FinderInfo* const finderInfo,
    std::vector<BlobFeatureExtractorFactory*> featureFactories) {

  std::vector<BlobFeatureExtractor*> featureExtractors;
  for(int i = 0; i < featureFactories.size(); ++i) {
    featureExtractors.push_back(featureFactories[i]->create(finderInfo));
  }

  return new MathExpressionFeatureExtractor(finderInfo, featureExtractors);
}

