/*
 * OtherRecognitionFeatureExtractorFactory.h
 *
 *  Created on: Dec 11, 2016
 *      Author: jake
 */

#ifndef OTHERRECOGNITIONFEATUREEXTRACTORFACTORY_H_
#define OTHERRECOGNITIONFEATUREEXTRACTORFACTORY_H_

#include <BlobFeatExtCat.h>
#include <RecCat.h>
#include <OtherRec.h>

class OtherRecognitionFeatureExtractorFactory
: public virtual BlobFeatureExtractorFactory {

 public:

  OtherRecognitionFeatureExtractorFactory(
      RecognitionBasedExtractorCategory* const category);

  OtherRecognitionFeatureExtractor* create(FinderInfo* const finderInfo);

  /**
   * An object which describes the feature extractor being created
   * by this factory.
   */
  BlobFeatureExtractorDescription* getDescription();

 private:

  OtherRecognitionFeatureExtractorDescription* description;
};


#endif /* OTHERRECOGNITIONFEATUREEXTRACTORFACTORY_H_ */
