/*
 * NumAlignedBlobsFeatureExtractorDescription.h
 *
 *  Created on: Dec 11, 2016
 *      Author: jake
 */

#ifndef NUMALIGNEDBLOBSFEATUREEXTRACTORDESCRIPTION_H_
#define NUMALIGNEDBLOBSFEATUREEXTRACTORDESCRIPTION_H_

#include <BlobFeatExtDesc.h>
#include <BlobFeatExtCat.h>
#include <FeatExtFlagDesc.h>
#include <AlignedFlagDesc.h>

#include <string>
#include <vector>

class NumAlignedBlobsFeatureExtractorDescription :
public virtual BlobFeatureExtractorDescription {

 public:

  NumAlignedBlobsFeatureExtractorDescription(
      BlobFeatureExtractorCategory* const category);

  std::string getName();

  static std::string getName_();

  std::string getUniqueName();

  BlobFeatureExtractorCategory* getCategory();

  std::vector<FeatureExtractorFlagDescription*> getFlagDescriptions();

  std::string getDescriptionText();

  NumAlignedBlobsRightwardFeatureFlagDescription* getRightwardFlagDescription();
  NumAlignedBlobsDownwardFeatureFlagDescription* getDownwardFlagDescription();
  NumAlignedBlobsUpwardFeatureFlagDescription* getUpwardFlagDescription();

 private:

  BlobFeatureExtractorCategory* category;

  NumAlignedBlobsRightwardFeatureFlagDescription* rightwardFlagDescription;
  NumAlignedBlobsDownwardFeatureFlagDescription* downwardFlagDescription;
  NumAlignedBlobsUpwardFeatureFlagDescription* upwardFlagDescription;
  std::vector<FeatureExtractorFlagDescription*> flagDescriptions;
};


#endif /* NUMALIGNEDBLOBSFEATUREEXTRACTORDESCRIPTION_H_ */
