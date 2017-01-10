/*
 * NumCompletelyNestedBlobsFeatureExtractorDescription.h
 *
 *  Created on: Dec 11, 2016
 *      Author: jake
 */

#ifndef NUMCOMPLETELYNESTEDBLOBSFEATUREEXTRACTORDESCRIPTION_H_
#define NUMCOMPLETELYNESTEDBLOBSFEATUREEXTRACTORDESCRIPTION_H_

#include <BlobFeatExtDesc.h>
#include <BlobFeatExtCat.h>

#include <string>

class NumCompletelyNestedBlobsFeatureExtractorDescription :
public virtual BlobFeatureExtractorDescription {

 public:

  NumCompletelyNestedBlobsFeatureExtractorDescription(
      BlobFeatureExtractorCategory* const category);

  std::string getName();

  std::string getUniqueName();

  BlobFeatureExtractorCategory* getCategory();

  std::string getDescriptionText();

 private:

  BlobFeatureExtractorCategory* category;
};


#endif /* NUMCOMPLETELYNESTEDBLOBSFEATUREEXTRACTORDESCRIPTION_H_ */
