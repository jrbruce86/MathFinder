/*
 * GeometryBasedExtractorCategory.h
 *
 *  Created on: Dec 11, 2016
 *      Author: jake
 */

#ifndef GEOMETRYBASEDEXTRACTORCATEGORY_H_
#define GEOMETRYBASEDEXTRACTORCATEGORY_H_

#include <BlobFeatExtCat.h>

class GeometryBasedExtractorCategory : public virtual BlobFeatureExtractorCategory {

public:

  GeometryBasedExtractorCategory();

  ~GeometryBasedExtractorCategory();

  std::string getName();

  std::string getDescription();

  BlobFeatureExtractorCategory* getParentCategory();

  std::vector<BlobFeatureExtractorCategory*> getChildCategories();

  std::vector<BlobFeatureExtractorFactory*> getFeatureExtractorFactories();

 private:

  std::vector<BlobFeatureExtractorFactory*> featureExtractorFactories;
};


#endif /* GEOMETRYBASEDEXTRACTORCATEGORY_H_ */
