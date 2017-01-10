/*
 * SentenceNGramsFeatureExtractorFlagDescriptions.h
 *
 *  Created on: Dec 30, 2016
 *      Author: jake
 */

#ifndef SENTENCENGRAMSFEATUREEXTRACTORFLAGDESCRIPTIONS_H_
#define SENTENCENGRAMSFEATUREEXTRACTORFLAGDESCRIPTIONS_H_

#include <FeatExtFlagDesc.h>

#include <string>

class UnigramFlagDescription
: public virtual FeatureExtractorFlagDescription {

 public:

  std::string getName();

  std::string getDescriptionText();
};

class BigramFlagDescription
: public virtual FeatureExtractorFlagDescription {

 public:

  std::string getName();

  std::string getDescriptionText();
};

class TrigramFlagDescription
: public virtual FeatureExtractorFlagDescription {

 public:

  std::string getName();

  std::string getDescriptionText();
};

#endif /* SENTENCENGRAMSFEATUREEXTRACTORFLAGDESCRIPTIONS_H_ */
