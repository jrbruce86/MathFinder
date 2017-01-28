/*
 * BlobFeatureExtractorCategory.cpp
 *
 *  Created on: Dec 22, 2016
 *      Author: jake
 */

#include <BlobFeatExtCat.h>

#include <BlobFeatExtFac.h>

#include <stddef.h>

#include <assert.h>


BlobFeatureExtractorFactory* BlobFeatureExtractorCategory
::getFactoryFromNameAndFlags(std::string name, std::vector<std::string> selectedFlagNames) {
  for(int i = 0; i < getFeatureExtractorFactories().size(); ++i) {
    BlobFeatureExtractorFactory* const factory = getFeatureExtractorFactories()[i];
    if(factory->getDescription()->getUniqueName() != name) {
      continue; // keep looking
    }
    // Found match!! Now enable the flags that need to be enabled and return result
    std::vector<FeatureExtractorFlagDescription*> allFlags = factory->getDescription()->getFlagDescriptions();
    if(allFlags.size() > 0 && selectedFlagNames.size() == 0) {
      std::cout << "ERROR: the feature extractor, " << name << " requires at least one flag to be enabled in order to properly train it. Retry training with a flag enabled. Exiting.\n";
      assert(false); // shouldn't get here
    }
    for(int j = 0; j < selectedFlagNames.size(); ++j) {
      // find the flag description matching the name... (I know... O(n^2) but don't have time to make this better...)
      FeatureExtractorFlagDescription* matchingFlag = NULL;
      for(int k = 0; k < allFlags.size(); ++k) {
        if(selectedFlagNames[j] == allFlags[k]->getName()) {
          matchingFlag = allFlags[k];
          break;
        }
      }
      if(matchingFlag == NULL) {
        std::cout << "ERROR: Unknown feature flag, " << selectedFlagNames[j] << " was read in...\n";
        assert(false); // unrecoverable error
      }
      factory->getSelectedFlags().push_back(matchingFlag);
    }
    return factory;
  }
  return NULL;
}

BlobFeatureExtractorCategory::~BlobFeatureExtractorCategory() {}

std::string BlobFeatureExtractorCategory::getName() {
  return "";
}

/**
 * Gets a brief description of this category
 */
std::string BlobFeatureExtractorCategory::getDescription() {
  return "";
}

/**
 * Gets the parent of this category if it is not the root,
 * otherwise returns null.
 */
BlobFeatureExtractorCategory* BlobFeatureExtractorCategory::getParentCategory() {
  return NULL;
}

/**
 * Gets one or more children categories if this category isn't a
 * leaf. Otherwise returns null.
 */
std::vector<BlobFeatureExtractorCategory*> BlobFeatureExtractorCategory::getChildCategories() {
  return std::vector<BlobFeatureExtractorCategory*>();
}

/**
 * Gets one or more feature extractor factories if this category
 * is a leaf. Otherwise returns null. The factories contain the descriptions.
 * This class manages the memory for both the factories and the descriptions and
 * should thus be kept in scope wherever the descriptions are needed. Factories are
 * cheap memory-wise so keeping them in memory isn't a concern.
 */
std::vector<BlobFeatureExtractorFactory*> BlobFeatureExtractorCategory::getFeatureExtractorFactories() {
  return std::vector<BlobFeatureExtractorFactory*>();
}


