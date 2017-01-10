/*
 * SentenceNGramsFeatureExtractorDescription.h
 *
 *  Created on: Dec 29, 2016
 *      Author: jake
 */

#ifndef SENTENCENGRAMSFEATUREEXTRACTORDESCRIPTION_H_
#define SENTENCENGRAMSFEATUREEXTRACTORDESCRIPTION_H_

#include <BlobFeatExtDesc.h>
#include <BlobFeatExtCat.h>
#include <RecCat.h>
#include <NGFlagDesc.h>
#include <FeatExtFlagDesc.h>

#include <string>
#include <vector>

class SentenceNGramsFeatureExtractorDescription :
public virtual BlobFeatureExtractorDescription {

 public:

  SentenceNGramsFeatureExtractorDescription(
      RecognitionBasedExtractorCategory* category);

  ~SentenceNGramsFeatureExtractorDescription();

  std::string getName();

  std::string getUniqueName();

  RecognitionBasedExtractorCategory* getCategory();

  std::vector<FeatureExtractorFlagDescription*> getFlagDescriptions();

  std::string getDescriptionText();

  UnigramFlagDescription* getUnigramFlag();
  BigramFlagDescription* getBigramFlag();
  TrigramFlagDescription* getTrigramFlag();

 private:

  RecognitionBasedExtractorCategory* category;

  UnigramFlagDescription* unigramFlag;
  BigramFlagDescription* bigramFlag;
  TrigramFlagDescription* trigramFlag;

  std::vector<FeatureExtractorFlagDescription*> flagDescriptions;
};

#endif /* SENTENCENGRAMSFEATUREEXTRACTORDESCRIPTION_H_ */
