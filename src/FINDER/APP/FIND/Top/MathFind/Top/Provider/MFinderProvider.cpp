/*
 * MathExpressionFeatureExtractorFactoryProvider.cpp
 *
 *  Created on: Dec 3, 2016
 *      Author: jake
 */

#include <MFinderProvider.h>

#include <MathExpressionFinder.h>
#include <FinderInfo.h>
#include <GeometryCat.h>
#include <RecCat.h>
#include <BlobFeatExtFac.h>
#include <BlobDataGridFactory.h>
#include <FeatExtFac.h>
#include <DetFac.h>
#include <SegFac.h>
#include <InfoFileParser.h>

#include <vector>
#include <assert.h>
#include <string>

MathExpressionFinder* MathExpressionFinderProvider
::createMathExpressionFinder(GeometryBasedExtractorCategory* const spatialCategory,
    RecognitionBasedExtractorCategory* const recognitionCategory,
    FinderInfo* const finderInfo) {

  MathExpressionFeatureExtractor* const mathExpressionFeatureExtractor =
      MathExpressionFeatureExtractorFactory().createMathExpressionFeatureExtractor(
          finderInfo,
          spatialCategory,
          recognitionCategory);

  return new MathExpressionFinder(
      mathExpressionFeatureExtractor,
      MathExpressionDetectorFactory().createMathExpressionDetector(finderInfo),
      MathExpressionSegmentorFactory().createMathExpressionSegmentor(finderInfo, mathExpressionFeatureExtractor),
      finderInfo);
}

