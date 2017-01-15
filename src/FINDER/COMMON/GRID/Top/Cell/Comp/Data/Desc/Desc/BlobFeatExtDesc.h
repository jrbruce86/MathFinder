/*
 * BlobFeatureExtractorDescription.h
 *
 *  Created on: Dec 11, 2016
 *      Author: jake
 */

#ifndef BLOBFEATUREEXTRACTORDESCRIPTION_H_
#define BLOBFEATUREEXTRACTORDESCRIPTION_H_

#include <BlobFeatExtCat.h>
#include <FeatExtFlagDesc.h>

#include <string>
#include <stddef.h>

class BlobFeatureExtractorDescription {

 public:

  /**
   * A short name to distinguish this feature extractor
   */
  virtual std::string getName() = 0;

  /**
   * A longer name used to uniquely identify this feature extractor
   */
  virtual std::string getUniqueName() = 0;

  /**
   * The category this feature extractor belongs to. For instance
   * could be a recognition based feature extractor which uses OCR
   * results or could be a geometric one just based on spatial
   * orientation on the page.
   */
  virtual BlobFeatureExtractorCategory* getCategory() = 0;

  /**
   * Optional list of flags that can be enabled or disabled for this
   * feature extractor in order to alter the behavior.
   */
  virtual std::vector<FeatureExtractorFlagDescription*> getFlagDescriptions();

  /**
   * A brief desription on this feature extractor to sum up what it
   * does. If the feature extractor includes flags, this should also
   * include that info.
   */
  virtual std::string getDescriptionText() = 0;

  /**
   * Destructor must be overridden.
   */
  virtual ~BlobFeatureExtractorDescription();

 protected:

  std::string determineUniqueName() {
    std::string uniqueName = "";
    BlobFeatureExtractorCategory* parentCategory = getCategory();
    while(parentCategory != NULL) {
      uniqueName.append(parentCategory->getName()).append(".");
      parentCategory = parentCategory->getParentCategory();
    }
    return uniqueName.append(getName());
  }
};


#endif /* BLOBFEATUREEXTRACTORDESCRIPTION_H_ */
