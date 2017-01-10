/*
 * BlobFeatureExtractionData.h
 *
 *  Created on: Nov 12, 2016
 *      Author: jake
 */

#ifndef BLOBFEATUREEXTRACTIONDATA_H_
#define BLOBFEATUREEXTRACTIONDATA_H_

#include <vector>
#include <DoubleFeature.h>

/**
 * Base class overridden by any feature extractor for
 * storing its extracted features and optionally other data into
 * a blob. The extracted data for a blob is stored within the
 * blob's grid entry as an object overriding this type
 */
class BlobFeatureExtractionData {
 public:

  BlobFeatureExtractionData();

  std::vector<DoubleFeature*> getExtractedFeatures();

  ~BlobFeatureExtractionData();

  void appendExtractedFeature(DoubleFeature* const feature);

 private:

  std::vector<DoubleFeature*> extractedFeatures;
};


#endif /* BLOBFEATUREEXTRACTIONDATA_H_ */
