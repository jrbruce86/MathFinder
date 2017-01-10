/*
 * RecognitionBasedExtractorCategory.h
 *
 *  Created on: Dec 11, 2016
 *      Author: jake
 */

#ifndef RECOGNITIONBASEDEXTRACTORCATEGORY_H_
#define RECOGNITIONBASEDEXTRACTORCATEGORY_H_

#include <BlobFeatExtCat.h>
#include <BlobFeatExtDesc.h>
#include <StopwordHelper.h>
#include <BlobFeatExtFac.h>

#include <string>
#include <vector>

class RecognitionBasedExtractorCategory : public BlobFeatureExtractorCategory {

 public:

  RecognitionBasedExtractorCategory();

  ~RecognitionBasedExtractorCategory();

   std::string getName();

   std::string getDescription();

   BlobFeatureExtractorCategory* getParentCategory();

   std::vector<BlobFeatureExtractorCategory*> getChildCategories();

   std::vector<BlobFeatureExtractorFactory*> getFeatureExtractorFactories();

   StopwordFileReader* getStopwordHelper();

 private:

   std::vector<BlobFeatureExtractorFactory*> featureExtractorFactories;
   StopwordFileReader* stopwordHelper;
};


#endif /* RECOGNITIONBASEDEXTRACTORCATEGORY_H_ */
