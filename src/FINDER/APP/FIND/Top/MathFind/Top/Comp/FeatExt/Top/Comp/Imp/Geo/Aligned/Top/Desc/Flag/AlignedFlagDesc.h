/*
 * NumAlignedBlobsFeatureExtractorFlagDescriptions.h
 *
 *  Created on: Dec 11, 2016
 *      Author: jake
 */

#ifndef NUMALIGNEDBLOBSFEATUREEXTRACTORFLAGDESCRIPTIONS_H_
#define NUMALIGNEDBLOBSFEATUREEXTRACTORFLAGDESCRIPTIONS_H_

#include <FeatExtFlagDesc.h>

#include <string>

class NumAlignedBlobsRightwardFeatureFlagDescription
: public virtual FeatureExtractorFlagDescription {

 public:

  std::string getName();

  std::string getDescriptionText();
};

class NumAlignedBlobsDownwardFeatureFlagDescription
: public virtual FeatureExtractorFlagDescription {

public:

 std::string getName();

 std::string getDescriptionText();
};

class NumAlignedBlobsUpwardFeatureFlagDescription
: public virtual FeatureExtractorFlagDescription {

public:

 std::string getName();

 std::string getDescriptionText();
};


#endif /* NUMALIGNEDBLOBSFEATUREEXTRACTORFLAGDESCRIPTIONS_H_ */
