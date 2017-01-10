/*
 * SubOrSuperscriptsFeatureExtractorFactory.cpp
 *
 *  Created on: Dec 4, 2016
 *      Author: jake
 */

#include <SubSupFac.h>

#include <RecCat.h>
#include <SubSupDesc.h>
#include <SubSup.h>
#include <FinderInfo.h>
#include <FeatExtFlagDesc.h>


#include <string>
#include <assert.h>

SubOrSuperscriptsFeatureExtractorFactory
::SubOrSuperscriptsFeatureExtractorFactory(
    RecognitionBasedExtractorCategory* const category) {
  this->description = new SubOrSuperscriptsFeatureExtractorDescription(category);
}


SubOrSuperscriptsFeatureExtractor*
SubOrSuperscriptsFeatureExtractorFactory::create(FinderInfo* const finderInfo) {

  SubOrSuperscriptsFeatureExtractor* result = new SubOrSuperscriptsFeatureExtractor(description);

  const std::string hasSubFlagName = description->getHasSubscriptDescription()->getName();
  const std::string isSubFlagName = description->getIsSubscriptDescription()->getName();
  const std::string hasSupFlagName = description->getHasSuperscriptDescription()->getName();
  const std::string isSupFlagName = description->getIsSuperscriptDescription()->getName();

  for(int i = 0; i < getSelectedFlags().size(); ++i) {
    FeatureExtractorFlagDescription* curFlag = getSelectedFlags()[i];
    if(curFlag->getName() == hasSubFlagName) {
      result->enableHasSubFeature();
    } else if(curFlag->getName() == isSubFlagName) {
      result->enableIsSubFeature();
    } else if(curFlag->getName() == hasSupFlagName) {
      result->enableHasSupFeature();
    } else if(curFlag->getName() == isSupFlagName) {
      result->enableIsSupFeature();
    } else {
      assert(false); // Shouldn't get here
    }
  }

  return result;
}

BlobFeatureExtractorDescription* SubOrSuperscriptsFeatureExtractorFactory
::getDescription() {
  return description;
}

