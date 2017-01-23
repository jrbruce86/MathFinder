/*
 * MathExpressionFeatureExtractor.cpp
 *
 *  Created on: Nov 9, 2016
 *      Author: jake
 */

#include <FeatExt.h>

#include <BlobDataGrid.h>
#include <Utils.h>
#include <M_Utils.h>

#define DBG_FEAT_EXT
#define DBG_AFTER_EXTRACTION
//#define DBG_FEAT_EXT_WAIT
//#define DBG_FEATURE_ORDERING

MathExpressionFeatureExtractor::MathExpressionFeatureExtractor(
    FinderInfo* finderInfo,
    std::vector<BlobFeatureExtractor*> blobFeatureExtractors) {
  this->finderInfo = finderInfo;
  this->blobFeatureExtractors = blobFeatureExtractors;
}

void MathExpressionFeatureExtractor::doFinderInitialization() {
  for(int i = 0; i < blobFeatureExtractors.size(); ++i) {
    blobFeatureExtractors[i]->doFinderInitialization();
  }
}

void MathExpressionFeatureExtractor::doTrainerInitialization() {
  for(int i = 0; i < blobFeatureExtractors.size(); ++i) {
#ifdef DBG_FEAT_EXT
    std::cout << "Running trainer intialization on extractor " << blobFeatureExtractors[i]->getFeatureExtractorDescription()->getName() << std::endl;
#endif
    blobFeatureExtractors[i]->doTrainerInitialization();
#ifdef DBG_FEAT_EXT
    std::cout << "Done running trainer initialization on extractor " << blobFeatureExtractors[i]->getFeatureExtractorDescription()->getName() << std::endl;
#ifdef DBG_FEAT_EXT_WAIT
    Utils::waitForInput();
#endif
#endif
  }
}

void MathExpressionFeatureExtractor::extractFeatures(BlobDataGrid* const blobDataGrid) {

  // For each feature extractor, first do any necessary preprocessing
  for(int i = 0; i < blobFeatureExtractors.size(); ++i) {
#ifdef DBG_FEAT_EXT
    std::cout << "Running preprocessing for the " << blobFeatureExtractors[i]->getFeatureExtractorDescription()->getName() << " extractor.\n";
#endif
    blobFeatureExtractors[i]->doPreprocessing(blobDataGrid);
#ifdef DBG_FEAT_EXT
    std::cout << "Done running preprocessing for the " << blobFeatureExtractors[i]->getFeatureExtractorDescription()->getName() << " extractor.\n";
#ifdef DBG_FEAT_EXT_WAIT
    Utils::waitForInput();
#endif
#endif
  }

  // Now, for each blob on the grid, run all of the blob feature extraction logic
  BlobDataGridSearch search(blobDataGrid);
  search.StartFullSearch();
  BlobData* blob = NULL;
  while((blob = search.NextFullSearch()) != NULL) {
    for(int i = 0; i < blobFeatureExtractors.size(); ++i) {
      // Do the feature extraction
      std::vector<DoubleFeature*> unorderedBlobFeatures = blobFeatureExtractors[i]->extractFeatures(blob);
      assert(unorderedBlobFeatures.size() > 0);

      // If it's just one feature then go ahead and append it to the blob's extracted feature list
      if(unorderedBlobFeatures.size() == 1) {
        blob->appendExtractedFeatures(unorderedBlobFeatures);
        continue;
      }

      // If there are multiple features here then they were extracted based
      // on multiple flags within the same extractor. If so, need to make sure
      // these flag-based extracted features are added to the blob in the same
      // order they were specified in based on this Finder's info.
      std::vector<DoubleFeature*> orderedFlagFeatures;
      std::vector<FeatureExtractorFlagDescription*> orderedFlagDescriptions = blobFeatureExtractors[i]->getEnabledFlagDescriptions();
      for(int j = 0; j < orderedFlagDescriptions.size(); ++j) {
        bool found = false;
        for(int k = 0; k < unorderedBlobFeatures.size(); ++k) {
          if(unorderedBlobFeatures[k]->getFlagDescription()->getName() == orderedFlagDescriptions[j]->getName()) {
            orderedFlagFeatures.push_back(unorderedBlobFeatures[k]);
            found = true;
            break;
          }
        }
        assert(found); // sanity
      }
      assert(orderedFlagFeatures.size() > 1); // sanity
      blob->appendExtractedFeatures(orderedFlagFeatures);
    }
#ifdef DBG_FEATURE_ORDERING
      dbgShowFeatureOrdering(blob);
#endif
  }
}

std::vector<BlobFeatureExtractor*> MathExpressionFeatureExtractor::getBlobFeatureExtractors() {
  return blobFeatureExtractors;
}

FinderInfo* MathExpressionFeatureExtractor::getFinderInfo() {
  return finderInfo;
}


MathExpressionFeatureExtractor::~MathExpressionFeatureExtractor() {
  for(int i = 0; i < blobFeatureExtractors.size(); ++i) {
    delete blobFeatureExtractors[i];
  }
  blobFeatureExtractors.clear();
}

void MathExpressionFeatureExtractor::dbgShowFeatureOrdering(
    BlobData* const blobData) {
  std::cout << "Finished adding features for the displayed blob. Here are the features (format -> [featurName]_[featureFlag]):\n";
  for(int j = 0; j < blobData->getExtractedFeatures().size(); ++j) {
    std::cout << blobData->getExtractedFeatures()[j]->getFeatureExtractorDescription()->getName()
        << "_" << blobData->getExtractedFeatures()[j]->getFlagDescription()->getName()
        << ": " << blobData->getExtractedFeatures()[j]->getFeature() << std::endl;
  }
  M_Utils::dbgDisplayBlob(blobData);
}
