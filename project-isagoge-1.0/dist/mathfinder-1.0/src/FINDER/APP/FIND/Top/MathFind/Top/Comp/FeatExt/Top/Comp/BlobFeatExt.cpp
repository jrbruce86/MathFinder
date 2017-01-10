/*
 * BlobFeatureExtractor.cpp
 *
 *  Created on: Nov 11, 2016
 *      Author: jake
 */

#include <BlobFeatExt.h>

#include <BlobDataGrid.h>
#include <BlobData.h>
#include <FeatExtFlagDesc.h>

#include <vector>

BlobFeatureExtractor::BlobFeatureExtractor() {}


/**
 * Determines the index in each blob's variable data vector where this
 * feature extractor will place its data. This is done by just getting
 * the size of the first blob's vector (all of the vectors will have the
 * same amount of data in the same order). Each feature extractor holds
 * onto that index and uses it to look up its data. Once the data is retrieved
 * it is cast back into something the feature extractor can find useful
 * somehow.
 */
int BlobFeatureExtractor::findOpenBlobDataIndex(BlobDataGrid* const blobDataGrid) {
  BlobDataGridSearch gridSearch(blobDataGrid);
  gridSearch.StartFullSearch();
  BlobData* blob = gridSearch.NextFullSearch();
  return blob->getVariableDataLength();
}

/**
 * Called once during initialization before using this extractor for training
 * purposes only.
 */
void BlobFeatureExtractor::doTrainerInitialization() {}

/**
 * Called once during initialization before using this extractor for finding
 * purposes only.
 */
void BlobFeatureExtractor::doFinderInitialization() {}

std::vector<FeatureExtractorFlagDescription*> BlobFeatureExtractor::getEnabledFlagDescriptions() {
  return std::vector<FeatureExtractorFlagDescription*>();
}

BlobFeatureExtractor::~BlobFeatureExtractor() {}

