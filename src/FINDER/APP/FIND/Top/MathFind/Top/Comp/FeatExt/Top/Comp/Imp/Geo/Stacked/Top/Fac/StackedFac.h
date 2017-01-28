/*
 * NumVerticallyStackedBlobsData.h
 *
 *  Created on: Dec 4, 2016
 *      Author: jake
 */

#ifndef NUMVERTICALLYSTACKEDBLOBSFEATUREEXTRACTORFACTORY_H_
#define NUMVERTICALLYSTACKEDBLOBSFEATUREEXTRACTORFACTORY_H_

#include <BlobFeatExtFac.h>
#include <BlobFeatExtCat.h>
#include <StackedFeatExt.h>
#include <FinderInfo.h>
#include <BlobFeatExtDesc.h>
#include <StackedDesc.h>

class NumVerticallyStackedBlobsFeatureExtractorFactory
: public virtual BlobFeatureExtractorFactory {

 public:

  NumVerticallyStackedBlobsFeatureExtractorFactory(
      BlobFeatureExtractorCategory* const category);

  ~NumVerticallyStackedBlobsFeatureExtractorFactory();

  NumVerticallyStackedBlobsFeatureExtractor* create(FinderInfo* const finderInfo);

  BlobFeatureExtractorDescription* getDescription();

 private:

  NumVerticallyStackedBlobsFeatureExtractorDescription*
    description;
};


#endif /* NUMVERTICALLYSTACKEDBLOBSDATA_H_ */
