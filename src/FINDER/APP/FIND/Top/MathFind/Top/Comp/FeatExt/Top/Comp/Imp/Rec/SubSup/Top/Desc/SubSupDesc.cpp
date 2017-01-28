/*
 * SubOrSuperscriptsFeatureExtractorDescription.cpp
 *
 *  Created on: Dec 11, 2016
 *      Author: jake
 */

#include <SubSupDesc.h>

#include <BlobFeatExtCat.h>
#include <FeatExtFlagDesc.h>

#include <string>
#include <vector>

SubOrSuperscriptsFeatureExtractorDescription
::SubOrSuperscriptsFeatureExtractorDescription(
    BlobFeatureExtractorCategory* const category) {
  this->category = category;

  this->hasSubscript = new HasSubscriptFlagDescription;
  this->isSubscript = new IsSubscriptFlagDescription;
  this->hasSuperscript = new HasSuperscriptFlagDescription;
  this->isSuperscript = new IsSuperscriptFlagDescription;
  this->flagDescriptions.push_back(hasSubscript);
  this->flagDescriptions.push_back(isSubscript);
  this->flagDescriptions.push_back(hasSuperscript);
  this->flagDescriptions.push_back(isSuperscript);
}

SubOrSuperscriptsFeatureExtractorDescription::
~SubOrSuperscriptsFeatureExtractorDescription() {
  std::vector<FeatureExtractorFlagDescription*> flags = getFlagDescriptions();
  for(int i = 0; i < flags.size(); ++i) {
    delete flags[i];
  }
}

std::string SubOrSuperscriptsFeatureExtractorDescription
::getName() {
  return "SubOrSuperscriptsFeature";
}

std::string SubOrSuperscriptsFeatureExtractorDescription
::getUniqueName() {
  return determineUniqueName();
}

BlobFeatureExtractorCategory* SubOrSuperscriptsFeatureExtractorDescription
::getCategory() {
  return category;
}

std::vector<FeatureExtractorFlagDescription*> SubOrSuperscriptsFeatureExtractorDescription
::getFlagDescriptions() {
  return flagDescriptions;
}

std::string SubOrSuperscriptsFeatureExtractorDescription
::getDescriptionText() {
  return std::string("Consists of up to four features describing sub/superscripts. ")
      + std::string("These include describing whether a blob is a super/subscript to a ")
      + std::string("neighboring blob or has a neighboring blob which is seen as a ")
      + std::string("sub/superscript to it.");
}

HasSubscriptFlagDescription* SubOrSuperscriptsFeatureExtractorDescription
::getHasSubscriptDescription() {
  return hasSubscript;
}
IsSubscriptFlagDescription* SubOrSuperscriptsFeatureExtractorDescription
::getIsSubscriptDescription() {
  return isSubscript;
}
HasSuperscriptFlagDescription* SubOrSuperscriptsFeatureExtractorDescription
::getHasSuperscriptDescription() {
  return hasSuperscript;
}
IsSuperscriptFlagDescription* SubOrSuperscriptsFeatureExtractorDescription
::getIsSuperscriptDescription() {
  return isSuperscript;
}

