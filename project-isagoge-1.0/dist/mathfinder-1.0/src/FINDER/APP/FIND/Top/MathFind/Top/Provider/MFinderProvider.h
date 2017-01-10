/*
 * MathExpressionFeatureExtractorFactoryProvider.h
 *
 *  Created on: Dec 3, 2016
 *      Author: jake
 */

#ifndef MATHEXPRESSIONFEATUREEXTRACTORFACTORYPROVIDER_H_
#define MATHEXPRESSIONFEATUREEXTRACTORFACTORYPROVIDER_H_

#include <MathExpressionFinder.h>
#include <GeometryCat.h>
#include <RecCat.h>
#include <FinderInfo.h>

#include <string>
#include <vector>

/**
 * Class created to hide all of the nasty factory creation logic.
 * Basically, a factory for the factory is embedded in here, but the public
 * api simply offers the end result. Destroying this object will only destroy
 * the factories, it is up to the calling code when the finder is destroyed.
 */
class MathExpressionFinderProvider {

 public:

  /**
   * Creates the math expression finder. The created finder is owned
   * by the calling code. Parsed arguments from the command line are
   * passed in to dictate how the expression finder is created.
   */
  MathExpressionFinder* createMathExpressionFinder(GeometryBasedExtractorCategory* const spatialCategory,
      RecognitionBasedExtractorCategory* const recognitionCategory,
      FinderInfo* const finderInfo);

 private:

  std::string stripFeatureFlags(const std::string& uniqueFeatureName);
  std::vector<std::string> getFeatureFlags(const std::string& uniqueFeatureName);

};


#endif /* MATHEXPRESSIONFEATUREEXTRACTORFACTORYPROVIDER_H_ */
