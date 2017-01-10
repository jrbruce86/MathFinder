/*
 * SubOrSuperscriptsFeatureExtractorFactory.h
 *
 *  Created on: Dec 4, 2016
 *      Author: jake
 */

#ifndef SUBORSUPERSCRIPTSFEATUREEXTRACTORFACTORY_H_
#define SUBORSUPERSCRIPTSFEATUREEXTRACTORFACTORY_H_

#include <BlobFeatExtFac.h>
#include <SubSupDesc.h>
#include <RecCat.h>
#include <SubSup.h>
#include <FinderInfo.h>
#include <BlobFeatExtDesc.h>

class SubOrSuperscriptsFeatureExtractorFactory : public virtual BlobFeatureExtractorFactory {

 public:

  SubOrSuperscriptsFeatureExtractorFactory(
      RecognitionBasedExtractorCategory* const category);

  SubOrSuperscriptsFeatureExtractor* create(FinderInfo* const finderInfo);

  BlobFeatureExtractorDescription* getDescription();

 private:

  SubOrSuperscriptsFeatureExtractorDescription* description;
};



#endif /* SUBORSUPERSCRIPTSFEATUREEXTRACTORFACTORY_H_ */
