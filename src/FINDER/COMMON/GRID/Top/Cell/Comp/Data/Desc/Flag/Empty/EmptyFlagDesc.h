/*
 * EmptyFlagDesc.h
 *
 *  Created on: Jan 22, 2017
 *      Author: jake
 */

#ifndef EMPTYFLAGDESC_H_
#define EMPTYFLAGDESC_H_

#include <FeatExtFlagDesc.h>

#include <string>

/**
 * A default flag description for features which have no flags
 * Features only have flags if their extractor can do multiple features,
 * each of which can be enabled or disabled (for instance the n-gram feature
 * extractor can extract unigrams, bigrams, and trigrams or any combination of
 * those three like just unigrams and bigrams, just trigrams, etc.).
 */
class EmptyFlagDescription
: public virtual FeatureExtractorFlagDescription {

 public:

  EmptyFlagDescription();

  std::string getName();

  std::string getDescriptionText();
};


#endif /* EMPTYFLAGDESC_H_ */
