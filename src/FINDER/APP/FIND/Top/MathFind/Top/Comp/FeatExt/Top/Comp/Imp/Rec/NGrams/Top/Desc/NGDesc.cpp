/*
 * SentenceNGramsFeatureExtractorDescription.cpp
 *
 *  Created on: Dec 29, 2016
 *      Author: jake
 */

#include <NGDesc.h>

#include <BlobFeatExtCat.h>
#include <FeatExtFlagDesc.h>
#include <NGFlagDesc.h>
#include <RecCat.h>

#include <vector>
#include <string>

SentenceNGramsFeatureExtractorDescription
::SentenceNGramsFeatureExtractorDescription(
    RecognitionBasedExtractorCategory* const category) {
  this->category = category;

  this->unigramFlag = new UnigramFlagDescription;
  this->bigramFlag = new BigramFlagDescription;
  this->trigramFlag = new TrigramFlagDescription;

  flagDescriptions.push_back(unigramFlag);
  flagDescriptions.push_back(bigramFlag);
  flagDescriptions.push_back(trigramFlag);
}

SentenceNGramsFeatureExtractorDescription
::~SentenceNGramsFeatureExtractorDescription() {
  for(int i = 0; i < flagDescriptions.size(); ++i) {
    delete flagDescriptions[i];
  }
}

std::string SentenceNGramsFeatureExtractorDescription::getName() {
  return "SentenceNGramFeatureExtractor";
}

std::string SentenceNGramsFeatureExtractorDescription::getUniqueName() {
  return determineUniqueName();
}

RecognitionBasedExtractorCategory* SentenceNGramsFeatureExtractorDescription
::getCategory() {
  return category;
}

std::vector<FeatureExtractorFlagDescription*> SentenceNGramsFeatureExtractorDescription
::getFlagDescriptions() {
  return flagDescriptions;
}

std::string SentenceNGramsFeatureExtractorDescription::getDescriptionText() {
  return "TODO: Fill in later";
}

UnigramFlagDescription* SentenceNGramsFeatureExtractorDescription
::getUnigramFlag() {
  return unigramFlag;
}

BigramFlagDescription* SentenceNGramsFeatureExtractorDescription
::getBigramFlag() {
  return bigramFlag;
}

TrigramFlagDescription* SentenceNGramsFeatureExtractorDescription
::getTrigramFlag() {
  return trigramFlag;
}


