/*
 * NumCompletelyNestedBlobsFeatureExtractorFactory.h
 *
 *  Created on: Dec 4, 2016
 *      Author: jake
 */

#ifndef NUMCOMPLETELYNESTEDBLOBSFEATUREEXTRACTORFACTORY_H_
#define NUMCOMPLETELYNESTEDBLOBSFEATUREEXTRACTORFACTORY_H_

#include <BlobFeatExtFac.h>
#include <BlobFeatExtCat.h>
#include <NestedFeatExt.h>
#include <FinderInfo.h>
#include <BlobFeatExtDesc.h>

/**
 * Factory
 */
class NumCompletelyNestedBlobsFeatureExtractorFactory : public virtual BlobFeatureExtractorFactory {
 public:

  NumCompletelyNestedBlobsFeatureExtractorFactory(
      BlobFeatureExtractorCategory* const category);

  ~NumCompletelyNestedBlobsFeatureExtractorFactory();

  NumCompletelyNestedBlobsFeatureExtractor* create(FinderInfo* const finderInfo);

  BlobFeatureExtractorDescription* getDescription();

 private:

  NumCompletelyNestedBlobsFeatureExtractorDescription*
    description;
};


#endif /* NUMCOMPLETELYNESTEDBLOBSFEATUREEXTRACTORFACTORY_H_ */
