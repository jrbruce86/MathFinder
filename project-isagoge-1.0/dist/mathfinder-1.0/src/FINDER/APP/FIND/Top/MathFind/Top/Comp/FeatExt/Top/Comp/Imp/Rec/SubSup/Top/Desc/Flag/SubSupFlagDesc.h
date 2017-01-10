/*
 * SubOrSuperscriptsFeatureExtractorFlagDescription.h
 *
 *  Created on: Dec 11, 2016
 *      Author: jake
 */

#ifndef SUBORSUPERSCRIPTSFEATUREEXTRACTORFLAGDESCRIPTION_H_
#define SUBORSUPERSCRIPTSFEATUREEXTRACTORFLAGDESCRIPTION_H_

#include <FeatExtFlagDesc.h>

#include <string>

class HasSubscriptFlagDescription
: public virtual FeatureExtractorFlagDescription {

 public:

  std::string getName();

  std::string getDescriptionText();
};

class IsSubscriptFlagDescription
: public virtual FeatureExtractorFlagDescription {

 public:

  std::string getName();

  std::string getDescriptionText();
};

class HasSuperscriptFlagDescription
: public virtual FeatureExtractorFlagDescription {

 public:

  std::string getName();

  std::string getDescriptionText();
};

class IsSuperscriptFlagDescription
: public virtual FeatureExtractorFlagDescription {

 public:

  std::string getName();

  std::string getDescriptionText();
};



#endif /* SUBORSUPERSCRIPTSFEATUREEXTRACTORFLAGDESCRIPTION_H_ */
