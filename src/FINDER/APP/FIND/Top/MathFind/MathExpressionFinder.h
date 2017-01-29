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
      MathExpressionFeatureExtractor* const featureExtractor,
      MathExpressionDetector* const detector,
      MathExpressionSegmentor* const segmentor,
      FinderInfo* const finderInfo);

  /**
   * Detects the math expressions in one or more images provided, returning the
   * results as a vector where the result vector indexes correspond with the
   * image ones (i.e., the result at the first index is for the image at that
   * same index in its array). "Detecting" the math expressions involves just
   * running the detector algorithm and returning those results (without running
   * segmentation). Prints error message returns empty vector if something went
   * wrong.
   */
  std::vector<MathExpressionFinderResults*> detectMathExpressions(
      Pixa* const images,
      std::vector<std::string> imageNames);

  /**
   * Finds the math expressions in one or more images provided, returning the
   * results as a vector where the result vector indexes correspond with the
   * image ones (i.e., the result at the first index is for the image at that
   * same index in its array). "Finding" the math expressions involves running
   * the detector algorithm followed by the segmentation one. Prints error
   * message and returns empty vector if something went wrong.
   */
  std::vector<MathExpressionFinderResults*> findMathExpressions(
      Pixa* const images,
      std::vector<std::string> imageNames);

  MathExpressionFeatureExtractor* getFeatureExtractor();

  ~MathExpressionFinder();

 private:

  // Modes to run in (either just detect, or both detect and segment)
  std::vector<MathExpressionFinderResults*> getResultsInRunMode(
      RunMode runMode,
      Pixa* const images,
      std::vector<std::string> imageNames);

  MathExpressionFeatureExtractor* mathExpressionFeatureExtractor;
  MathExpressionDetector* mathExpressionDetector;
  MathExpressionSegmentor* mathExpressionSegmentor;
  FinderInfo* finderInfo;

  // internal variables/flags
  bool init;
};



#endif
