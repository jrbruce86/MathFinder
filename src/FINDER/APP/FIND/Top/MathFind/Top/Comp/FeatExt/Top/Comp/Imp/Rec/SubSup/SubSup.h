/*
 * SubOrSuperScriptsFeatureExtractor.h
 *
 *  Created on: Nov 13, 2016
 *      Author: jake
 */

#ifndef SUBORSUPERSCRIPTSFEATUREEXTRACTOR_H_
#define SUBORSUPERSCRIPTSFEATUREEXTRACTOR_H_

#include <BlobFeatExt.h>
#include <SubSupDesc.h>
#include <BlobDataGrid.h>
#include <DoubleFeature.h>
#include <BlobFeatExtDesc.h>
#include <FeatExtFlagDesc.h>
#include <BlobData.h>
#include <FinderInfo.h>

#include <vector>

enum SubSuperScript {SUB, SUPER};

class SubOrSuperscriptsFeatureExtractor
: public virtual BlobFeatureExtractor {

public:

  SubOrSuperscriptsFeatureExtractor(
      SubOrSuperscriptsFeatureExtractorDescription* const description,
      FinderInfo* const finderInfo);

  void doPreprocessing(BlobDataGrid* const blobDataGrid);

  std::vector<DoubleFeature*> extractFeatures(BlobData* const blob);

  BlobFeatureExtractorDescription* getFeatureExtractorDescription();

  std::vector<FeatureExtractorFlagDescription*> getEnabledFlagDescriptions();

  void enableHasSubFeature();
  void enableIsSubFeature();
  void enableHasSupFeature();
  void enableIsSupFeature();

 private:

  // Determines whether or not the blob in question has a super/subscript
  // and if it does have a super/subscript then its has_sup/has_sub features
  // are set and its sub/superscript blob's is_sup/is_sub feature is set also
  void setBlobSubSuperScript(BlobData* const blob, BlobDataGrid* const blobDataGrid,
      const SubSuperScript subsuper);

  int blobSubscriptDataKey;

  SubOrSuperscriptsFeatureExtractorDescription* description;
  std::vector<FeatureExtractorFlagDescription*> enabledFlagDescriptions;

  bool hasSubFeatureEnabled;
  bool isSubFeatureEnabled;
  bool hasSupFeatureEnabled;
  bool isSupFeatureEnabled;

  const float wordCertaintyThresh;
  const float wordCertaintyThreshHigh;

  // Debug methods/variables
  void dbgSubSuper(BlobData* blob, BlobData* neighbor, SubSuperScript subsuper);
  std::string subSupDir;
};

#endif /* SUBORSUPERSCRIPTS_H_ */
