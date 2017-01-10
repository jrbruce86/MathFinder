/*
 * MathExpressionFeatureExtractorFactory.h
 *
 *  Created on: Nov 9, 2016
 *      Author: jake
 */

#ifndef MATHEXPRESSIONFEATUREEXTRACTORFACTORY_H_
#define MATHEXPRESSIONFEATUREEXTRACTORFACTORY_H_

#include <FeatExt.h>
#include <FinderInfo.h>
#include <BlobFeatExtFac.h>
#include <BlobFeatExt.h>

#include <string>
#include <vector>

/**
 * Factory for creating the math expression feature extractor. This consists of
 * one or more "blob feature extractors". The "blob feature extractors" each extract
 * information about an individual blob on the image. The "blob feature extractors"
 * to be created within this math expression extractor are specified by the list
 * of strings provided to the factory creation method.
 */
class MathExpressionFeatureExtractorFactory {
 public:

  MathExpressionFeatureExtractor* createMathExpressionFeatureExtractor(
      FinderInfo* const finderInfo,
      std::vector<BlobFeatureExtractorFactory*> featureFactories);

 private:

  /**
   * Returns true if the blob feature extractor with the given name was already added to
   * the given list.
   */
  bool isNameAlreadyOnList(const std::string& name,
      std::vector<BlobFeatureExtractor*>* const list);
};

#endif /* MATHEXPRESSIONFEATUREEXTRACTORFACTORY_H_ */
