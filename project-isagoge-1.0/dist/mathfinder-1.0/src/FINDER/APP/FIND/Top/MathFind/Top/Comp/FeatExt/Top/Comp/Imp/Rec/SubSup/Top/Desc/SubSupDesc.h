/*
 * SubOrSuperscriptsFeatureExtractorDescription.h
 *
 *  Created on: Dec 11, 2016
 *      Author: jake
 */

#ifndef SUBORSUPERSCRIPTSFEATUREEXTRACTORDESCRIPTION_H_
#define SUBORSUPERSCRIPTSFEATUREEXTRACTORDESCRIPTION_H_

#include <BlobFeatExtDesc.h>
#include <BlobFeatExtCat.h>
#include <SubSupFlagDesc.h>

#include <string>
#include <vector>

class SubOrSuperscriptsFeatureExtractorDescription
: public virtual BlobFeatureExtractorDescription {

 public:

  SubOrSuperscriptsFeatureExtractorDescription(
      BlobFeatureExtractorCategory* const category);

  std::string getName();

  std::string getUniqueName();

  BlobFeatureExtractorCategory* getCategory();

  std::vector<FeatureExtractorFlagDescription*> getFlagDescriptions();

  std::string getDescriptionText();

  HasSubscriptFlagDescription* getHasSubscriptDescription();
  IsSubscriptFlagDescription* getIsSubscriptDescription();
  HasSuperscriptFlagDescription* getHasSuperscriptDescription();
  IsSuperscriptFlagDescription* getIsSuperscriptDescription();

 private:

  BlobFeatureExtractorCategory* category;

  HasSubscriptFlagDescription* hasSubscript;
  IsSubscriptFlagDescription* isSubscript;
  HasSuperscriptFlagDescription* hasSuperscript;
  IsSuperscriptFlagDescription* isSuperscript;
  std::vector<FeatureExtractorFlagDescription*> flagDescriptions;
};


#endif /* SUBORSUPERSCRIPTSFEATUREEXTRACTORDESCRIPTION_H_ */
