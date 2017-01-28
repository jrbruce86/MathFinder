/*
 * NumAlignedBlobsFeatureExtractorFactory.h
 *
 *  Created on: Dec 4, 2016
 *      Author: jake
 */

#ifndef NUMALIGNEDBLOBSFEATUREEXTRACTORFACTORY_H_
#define NUMALIGNEDBLOBSFEATUREEXTRACTORFACTORY_H_

#include <BlobFeatExtFac.h>
#include <BlobFeatExtCat.h>
#include <AlignedFeatExt.h>
#include <AlignedDesc.h>
#include <FinderInfo.h>

class NumAlignedBlobsFeatureExtractorFactory : public BlobFeatureExtractorFactory {

 public:

  NumAlignedBlobsFeatureExtractorFactory(
      BlobFeatureExtractorCategory* const category);

  ~NumAlignedBlobsFeatureExtractorFactory();

  NumAlignedBlobsFeatureExtractor* create(FinderInfo* const finderInfo);

  NumAlignedBlobsFeatureExtractorDescription* getDescription();

 private:

  NumAlignedBlobsFeatureExtractorDescription* description;
};

#endif /* NUMALIGNEDBLOBSFEATUREEXTRACTORFACTORY_H_ */
