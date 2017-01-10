/*
 * BlobFeatureExtractorCategory.h
 *
 *  Created on: Dec 11, 2016
 *      Author: jake
 */

#ifndef BLOBFEATUREEXTRACTORCATEGORY_H_
#define BLOBFEATUREEXTRACTORCATEGORY_H_

#include <vector>
#include <string>
#include <iostream>

class BlobFeatureExtractorFactory;

/**
 * This is basically the container class for all of both the descriptions
 * and the factories. Any class that needs a reference to the feature extractor
 * descriptions should only be in use while this class is in scope. If this
 * class gets deleted while feature extractors are still in use, then whenever
 * the extractors try to access their description, they'll be accessing deallocated memory.
 */
class BlobFeatureExtractorCategory {

 public:

  /**
   * Gets the name of this category
   */
  virtual std::string getName();

  /**
   * Gets a brief description of this category
   */
  virtual std::string getDescription();

  /**
   * Gets the parent of this category if it is not the root,
   * otherwise returns null.
   */
  virtual BlobFeatureExtractorCategory* getParentCategory();

  /**
   * Gets one or more children categories if this category isn't a
   * leaf. Otherwise returns null.
   */
  virtual std::vector<BlobFeatureExtractorCategory*> getChildCategories();

  /**
   * Gets one or more feature extractor factories if this category
   * is a leaf. Otherwise returns null. The factories contain the descriptions.
   * This class manages the memory for both the factories and the descriptions and
   * should thus be kept in scope wherever the descriptions are needed. Factories are
   * cheap memory-wise so keeping them in memory isn't a concern.
   */
  virtual std::vector<BlobFeatureExtractorFactory*> getFeatureExtractorFactories();

  /**
   * Looks up and returns the feature extractor factory with the given name, enabling the given flags for it
   * if the factory exists in this category. If not, returns NULL.
   */
  BlobFeatureExtractorFactory* getFactoryFromNameAndFlags(std::string name, std::vector<std::string> flags);

  virtual ~BlobFeatureExtractorCategory();
};


#endif /* BLOBFEATUREEXTRACTORCATEGORY_H_ */
