/*
 * BlobFeatureExtractorFactory.h
 *
 *  Created on: Dec 11, 2016
 *      Author: jake
 */

#ifndef BLOBFEATUREEXTRACTORFACTORY_H_
#define BLOBFEATUREEXTRACTORFACTORY_H_

#include <vector>
#include <BlobFeatExtDesc.h>

class BlobFeatureExtractor;
class FinderInfo;

class BlobFeatureExtractorFactory {

 public:

  BlobFeatureExtractorFactory();

  /**
   * Creates the blob feature extractor associated with an
   * implementation of this factory.
   */
  virtual BlobFeatureExtractor* create(FinderInfo* const finderInfo) = 0;

  /**
   * An object which describes the feature extractor being created
   * by this factory.
   */
  virtual BlobFeatureExtractorDescription* getDescription() {return NULL;};

  virtual ~BlobFeatureExtractorFactory();

  /**
   * Returns a reference to the flags associated with this factory.
   * These flags specify optional parameters that alter the behavior
   * of the feature extractor being created. It's entirely up to the
   * factory "create" method implementation as far as what is done with
   * these flags. It's up to the calling code of this class as far as
   * which flags are selected (the returned reference allows calling
   * code to modify the contents of the list directly through this method).
   */
  std::vector<FeatureExtractorFlagDescription*>& getSelectedFlags();

  friend std::ostream& operator<<(std::ostream& stream, BlobFeatureExtractorFactory& fact);

 private:

  std::vector<FeatureExtractorFlagDescription*> selectedFlags;
};


#endif /* BLOBFEATUREEXTRACTORFACTORY_H_ */
