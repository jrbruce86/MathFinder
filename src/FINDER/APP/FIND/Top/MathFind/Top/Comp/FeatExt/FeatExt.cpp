/*
 * MathExpressionFeatureExtractor.cpp
 *
 *  Created on: Nov 9, 2016
 *      Author: jake
 */

#include <FeatExt.h>

#include <BlobDataGrid.h>

#define DBG_FEAT_EXT

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
  }
}

void MathExpressionFeatureExtractor::extractFeatures(BlobDataGrid* const blobDataGrid) {

  // For each feature extractor, first do any necessary preprocessing
  for(int i = 0; i < blobFeatureExtractors.size(); ++i) {
    std::cout << "Running preprocessing for the " << blobFeatureExtractors[i]->getFeatureExtractorDescription()->getName() << " extractor.\n";
    blobFeatureExtractors[i]->doPreprocessing(blobDataGrid);
    std::cout << "Done running preprocessing for the " << blobFeatureExtractors[i]->getFeatureExtractorDescription()->getName() << " extractor.\n";
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
      blob->appendExtractedFeatures(orderedFlagFeatures);
    }
  }

  #ifdef DBG_AFTER_EXTRACTION
      // TODO: Preserve logic that used to exist here somehow
      feat_ext.dbgAfterExtraction();
  #endif
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
