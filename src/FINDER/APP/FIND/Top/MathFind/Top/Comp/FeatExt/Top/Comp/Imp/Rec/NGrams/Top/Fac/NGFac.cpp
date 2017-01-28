/*
 * SenetenceNGramsFeatureExtractorFactory.cpp
 *
 *  Created on: Dec 5, 2016
 *      Author: jake
 */

#include <NGFac.h>

#include <RecCat.h>
#include <NGramsFE.h>
#include <FinderInfo.h>
#include <BlobFeatExtDesc.h>

#include <string>
#include <assert.h>

SentenceNGramsFeatureExtractorFactory::SentenceNGramsFeatureExtractorFactory(
    RecognitionBasedExtractorCategory* const category) {
  this->description = new SentenceNGramsFeatureExtractorDescription(category);
}

SentenceNGramsFeatureExtractorFactory::
~SentenceNGramsFeatureExtractorFactory() {
  delete description;
}

SentenceNGramsFeatureExtractor* SentenceNGramsFeatureExtractorFactory
::create(FinderInfo* const finderInfo) {

  SentenceNGramsFeatureExtractor* result =
      new SentenceNGramsFeatureExtractor(description, finderInfo);

  const std::string unigramFlagName = description->getUnigramFlag()->getName();
  const std::string bigramFlagName = description->getBigramFlag()->getName();
  const std::string trigramFlagName = description->getTrigramFlag()->getName();

  for(int i = 0; i < getSelectedFlags().size(); ++i) {
    const std::string selectedFlagName = getSelectedFlags()[i]->getName();
    if(selectedFlagName == unigramFlagName) {
      result->enableUnigramFlag();
    } else if(selectedFlagName == bigramFlagName) {
      result->enableBigramFlag();
    } else if(selectedFlagName == trigramFlagName) {
      result->enableTrigramFlag();
    } else {
      assert(false); // shouldn't get here
    }
  }

  return result;
}

BlobFeatureExtractorDescription* SentenceNGramsFeatureExtractorFactory
::getDescription() {
  return description;
}


