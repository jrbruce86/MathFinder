/*
 * NumVerticallyStackedBlobsFeatureExtractorDescription.h
 *
 *  Created on: Dec 11, 2016
 *      Author: jake
 */

#ifndef NUMVERTICALLYSTACKEDBLOBSFEATUREEXTRACTORDESCRIPTION_H_
#define NUMVERTICALLYSTACKEDBLOBSFEATUREEXTRACTORDESCRIPTION_H_

#include <BlobFeatExtDesc.h>
#include <BlobFeatExtCat.h>

#include <string>

class NumVerticallyStackedBlobsFeatureExtractorDescription :
public virtual BlobFeatureExtractorDescription {

 public:

  NumVerticallyStackedBlobsFeatureExtractorDescription(
      BlobFeatureExtractorCategory* const category);

  std::string getName();

  std::string getUniqueName();

  BlobFeatureExtractorCategory* getCategory();

  std::string getDescriptionText();

  static std::string getName_();

private:

 BlobFeatureExtractorCategory* category;

};


#endif /* NUMVERTICALLYSTACKEDBLOBSFEATUREEXTRACTORDESCRIPTION_H_ */
