/*
 * NumCompletelyNestedBlobsFeatureExtractor.cpp
 *
 *  Created on: Nov 13, 2016
 *      Author: jake
 */

#ifndef NUMCOMPLETELYNESTEDBLOBSFEATUREEXTRACTOR_CPP_
#define NUMCOMPLETELYNESTEDBLOBSFEATUREEXTRACTOR_CPP_

#include <NestedFeatExt.h>

#include <NestedDesc.h>
#include <BlobDataGrid.h>
#include <BlobData.h>
#include <NestedData.h>
#include <DoubleFeature.h>
#include <M_Utils.h>

#include <baseapi.h>

#include <vector>
#include <stddef.h>
#include <assert.h>

//#define DBG_NESTED_FEATURE
#define DBG_FEAT2

NumCompletelyNestedBlobsFeatureExtractor
::NumCompletelyNestedBlobsFeatureExtractor(
    NumCompletelyNestedBlobsFeatureExtractorDescription* const description) {
  this->description = description;
}

void NumCompletelyNestedBlobsFeatureExtractor::doPreprocessing(BlobDataGrid* const blobDataGrid) {
  blobDataKey = findOpenBlobDataIndex(blobDataGrid);

  // Go ahead and extract this feature for each blob and store results in its data
  BlobDataGridSearch gridSearch(blobDataGrid);
  gridSearch.StartFullSearch();
  BlobData* blob = NULL;
  while((blob = gridSearch.NextFullSearch()) != NULL) {

    // Create this feature's data for this blob
    NumCompletelyNestedBlobsData* const data = new NumCompletelyNestedBlobsData();

    // Add the data to the blob's variable data array
    assert(blobDataKey == blob->appendNewVariableData(data));

    // Extract the feature put it in this feature's data (also adding any other necessary
    // info to the feature's data).
    data->appendExtractedFeature(
        new DoubleFeature(description,
            M_Utils::expNormalize(
                countNestedBlobs(blob,
                    blobDataGrid))));
  }
}

std::vector<DoubleFeature*> NumCompletelyNestedBlobsFeatureExtractor::extractFeatures(BlobData* const blobData) {
  // Already did the extraction during preprocessing, so just return result
  return blobData->getVariableDataAt(blobDataKey)->getExtractedFeatures();
}

int NumCompletelyNestedBlobsFeatureExtractor::countNestedBlobs(BlobData* const blob,
    BlobDataGrid* const blobDataGrid) {
  int nested = 0;

  // ----------------COMMENT AND/OR CODE IN QUESTION START---------------------
//  if(blob->validword || blob->onRowNormal()) {
//    return 0;
//  }
  // ----------------COMMENT AND/OR CODE IN QUESTION END----------------------
  NumCompletelyNestedBlobsData* data = (NumCompletelyNestedBlobsData*)blob->getVariableDataAt(blobDataKey);
  BlobDataGridSearch bigs(blobDataGrid);
  bigs.StartRectSearch(blob->getBoundingBox());
  BlobData* nestblob = NULL;
  GenericVector<BlobData*> nestedbloblist;
  while((nestblob = bigs.NextRectSearch()) != NULL) {
    if(nestblob == blob ||
        (nestblob->getBoundingBox() == blob->getBoundingBox()) ||
        nestedbloblist.bool_binary_search(nestblob))
      continue;
    else {
      // make sure the nestblob is entirely contained within
      // the blob
      TBOX nestbox = nestblob->getBoundingBox();
      TBOX blobbox = blob->getBoundingBox();
      if(!blobbox.contains(nestbox))
        continue;
      // make sure it passes an area threshold
      double area_thresh = (double)1/(double)64;
      if(nestbox.area() < ((double)(blobbox.area())*area_thresh))
        continue;
      nestedbloblist.push_back(nestblob);
      nestedbloblist.sort(); // sort to avoid duplicates
      ++nested;
    }
  }
#ifdef DBG_NESTED_FEATURE
  //int left=1458, top=1983, right=1759, bottom=1899;
  //TBOX tb(left, bottom, right, top);
  //if(blob->bounding_box() == tb) {
    if(nested > 0) {
      cout << "displaying blob which has " << nested << " nested element(s)\n";
      cout << "blob has area " << blob->bounding_box().area() << endl;
      M_Utils::dbgDisplayBlob(blob);
      for(int i = 0; i < nestedbloblist.length(); ++i) {
        cout << "displaying nested element # " << i << endl;
        cout << "nested element has area of " << nestedbloblist[i]->bounding_box().area() << endl;
        cout << "nested element coordinates:\n";
        nestedbloblist[i]->bounding_box().print();
        M_Utils::dispBlobDataRegion(nestedbloblist[i], blobDataGrid->getBinaryImage());
        M_Utils::waitForInput();
      }
    }
  //}
#endif

#ifdef DBG_FEAT2
  if(nested > 0) {
    cout << "Displayed blob has " << nested << " nested blobs\n";
    M_Utils::dbgDisplayBlob(blob);
  }
#endif

  data->setNestedBlobsCount(nested);

  return nested;
}

#endif /* NUMCOMPLETELYNESTEDBLOBSFEATUREEXTRACTOR_CPP_ */

BlobFeatureExtractorDescription* NumCompletelyNestedBlobsFeatureExtractor::getFeatureExtractorDescription() {
  return description;
}
