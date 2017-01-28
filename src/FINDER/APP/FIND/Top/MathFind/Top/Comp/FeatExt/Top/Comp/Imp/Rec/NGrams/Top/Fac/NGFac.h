/*
 * SentenceNGramsFeatureExtractorFactory.h
 *
 *  Created on: Dec 5, 2016
 *      Author: jake
 */

#ifndef SENTENCENGRAMSFEATUREEXTRACTORFACTORY_H_
#define SENTENCENGRAMSFEATUREEXTRACTORFACTORY_H_

#include <BlobFeatExtFac.h>
#include <RecCat.h>
#include <NGramsFE.h>
#include <FinderInfo.h>
#include <NGDesc.h>


class SentenceNGramsFeatureExtractorFactory
: public virtual BlobFeatureExtractorFactory {

 public:

  SentenceNGramsFeatureExtractorFactory(RecognitionBasedExtractorCategory* const category);

  ~SentenceNGramsFeatureExtractorFactory();

  SentenceNGramsFeatureExtractor* create(FinderInfo* const finderInfo);

  BlobFeatureExtractorDescription* getDescription();

 private:

  SentenceNGramsFeatureExtractorDescription* description;
};


#endif /* SENTENCENGRAMSFEATUREEXTRACTORFACTORY_H_ */
