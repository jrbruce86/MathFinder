/*
 * DoubleFeature.h
 *
 *  Created on: Nov 12, 2016
 *      Author: jake
 */

#ifndef DOUBLEFEATURE_H_
#define DOUBLEFEATURE_H_

#include <string>

#include <BlobFeatExtDesc.h>
#include <EmptyFlagDesc.h>

class DoubleFeature {

 public:

  DoubleFeature(BlobFeatureExtractorDescription* featureDescription,
      const double feature,
      FeatureExtractorFlagDescription* flagDescription=
          new EmptyFlagDescription());

  double getFeature();

  bool operator==(const DoubleFeature& othersample);

  BlobFeatureExtractorDescription* getFeatureExtractorDescription();

  FeatureExtractorFlagDescription* getFlagDescription();

 private:
  double feature;
  BlobFeatureExtractorDescription* featureExtractorDescription;
  FeatureExtractorFlagDescription* flagDescription;
};


#endif /* DOUBLEFEATURE_H_ */
