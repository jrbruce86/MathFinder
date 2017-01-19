/*
 * NumVerticallyStackedBlobs.cpp
 *
 *  Created on: Nov 13, 2016
 *      Author: jake
 */

#include <StackedFeatExt.h>

#include <StackedDesc.h>
#include <BlobDataGrid.h>
#include <BlobData.h>
#include <StackedData.h>
#include <Direction.h>
#include <DoubleFeature.h>
#include <M_Utils.h>

#include <allheaders.h>

#include <stddef.h>
#include <assert.h>

#define DBG_SHOW_STACKED_FEATURE
#define DBG_STACKED_FEATURE_ALOT
#define DBG_DISPLAY

NumVerticallyStackedBlobsFeatureExtractor
::NumVerticallyStackedBlobsFeatureExtractor(
    NumVerticallyStackedBlobsFeatureExtractorDescription* const description) {
  this->description = description;
}

void NumVerticallyStackedBlobsFeatureExtractor::doPreprocessing(BlobDataGrid* const blobDataGrid) {
  blobDataKey = findOpenBlobDataIndex(blobDataGrid);

  // Go ahead and extract the feature for each blob in the grid
  BlobDataGridSearch gridSearch(blobDataGrid);
  gridSearch.StartFullSearch();
  BlobData* blob = NULL;
  while((blob = gridSearch.NextFullSearch()) != NULL) {

    // Create data entry for this feature extractor to add to the blob's variable data array
    NumVerticallyStackedBlobsData* const data = new NumVerticallyStackedBlobsData();

    // Add the data to the blob's variable data array
    assert(blobDataKey == blob->appendNewVariableData(data));


    if(data->hasBeenProcessed()) // this is probably no longer relevant but keeping anyway for now
      continue;

    const int stacked_count =
        countStacked(blob, blobDataGrid, BlobSpatial::UP)
        + countStacked(blob, blobDataGrid, BlobSpatial::DOWN);

    data->setHasBeenProcessed(true); // probably not relevant but keeping for now

    data->setStackedBlobsCount(stacked_count);

    data->appendExtractedFeature(
        new DoubleFeature(
            description,
            M_Utils::expNormalize(
                (double)stacked_count)));
  }
#ifdef DBG_SHOW_STACKED_FEATURE
  gridSearch.StartFullSearch();
  Pix* dbgim2 = pixCopy(NULL, blobDataGrid->getBinaryImage());
  dbgim2 = pixConvertTo32(dbgim2);
  while((blob = gridSearch.NextFullSearch()) != NULL) {
    NumVerticallyStackedBlobsData* const curBlobData = (NumVerticallyStackedBlobsData*)(blob->getVariableDataAt(blobDataKey));
    if(curBlobData->getStackedBlobsCount() == 0)
      continue;
    if(curBlobData->getStackedBlobsCount() < 0) {
      cout << "ERROR: stacked character count < 0 >:-[\n";
      assert(false);
    }
    LayoutEval::Color color;
    if(curBlobData->getStackedBlobsCount() == 1)
      color = LayoutEval::RED;
    if(curBlobData->getStackedBlobsCount() == 2)
      color = LayoutEval::GREEN;
    if(curBlobData->getStackedBlobsCount() > 2)
      color = LayoutEval::BLUE;
    M_Utils::drawHlBlobDataRegion(blob, dbgim2, color);
  }
  pixWrite(("debug/stacked_blobs" + blobDataGrid->getImageName()).c_str(), dbgim2, IFF_PNG);
#ifdef DBG_DISPLAY
  pixDisplay(dbgim2, 100, 100);
  M_Utils::waitForInput();
#endif
  pixDestroy(&dbgim2);

#endif

}

std::vector<DoubleFeature*> NumVerticallyStackedBlobsFeatureExtractor::extractFeatures(BlobData* const blobData) {
  // Already did the extraction during preprocessing, so just return result
  return blobData->getVariableDataAt(blobDataKey)->getExtractedFeatures();
}

int NumVerticallyStackedBlobsFeatureExtractor::countStacked(BlobData* const blob,
    BlobDataGrid* const blobDataGrid, const BlobSpatial::Direction dir) {
  int count = 0;
  // if the blob belongs to a valid word then the feature is zero
#ifdef DBG_STACKED_FEATURE_ALOT
  int left=1773, top=1243, right=1810, bottom=1210;
  TBOX box(left, bottom, right, top);
#endif
  // ----------------COMMENT AND/OR CODE IN QUESTION START---------------------
  //if(blob->validword || blob->onRowNormal()) {
//#ifdef DBG_STACKED_FEATURE_ALOT
//      if(blob->bounding_box() == box) {
//        cout << "showing a blob which is part of a valid word or 'normal' row and thus can't have stacked elements\n";
//        dbgDisplayBlob(blob);
//      }
//#endif
    //return 0;
  //}
  // ----------------COMMENT AND/OR CODE IN QUESTION END-----------------------

  // go to first element above or below depending on the direction
  BlobDataGridSearch vsearch(blobDataGrid);
  vsearch.StartVerticalSearch(blob->getBoundingBox().left(), blob->getBoundingBox().right(),
      (dir == BlobSpatial::UP) ? blob->getBoundingBox().top() : blob->getBoundingBox().bottom());
  BlobData* const central_blob = blob;

  // Look up this feature's data entry for the current blob
  NumVerticallyStackedBlobsData* const data = (NumVerticallyStackedBlobsData*)blob->getVariableDataAt(blobDataKey);

  GenericVector<BlobData*>& stacked_blobs = data->getStackedBlobs();
  BlobData* stacked_blob = vsearch.NextVerticalSearch((dir == BlobSpatial::UP) ? false : true);
  BlobData* prev_stacked_blob = central_blob;
  while(true) {
    while(stacked_blob->getBoundingBox() == prev_stacked_blob->getBoundingBox()
        || ((dir == BlobSpatial::UP) ? (stacked_blob->getBoundingBox().bottom() < prev_stacked_blob->getBoundingBox().top())
            : (stacked_blob->getBoundingBox().top() > prev_stacked_blob->getBoundingBox().bottom()))
        || stacked_blob->getBoundingBox().left() >= prev_stacked_blob->getBoundingBox().right()
        || stacked_blob->getBoundingBox().right() <= prev_stacked_blob->getBoundingBox().left()
        || stacked_blobs.binary_search(stacked_blob)) {
      stacked_blob = vsearch.NextVerticalSearch((dir == BlobSpatial::UP) ? false : true);
      if(stacked_blob == NULL)
        break;
    }
    if(stacked_blob == NULL) {
#ifdef DBG_STACKED_FEATURE_ALOT
      if(blob->bounding_box() == box) {
        cout << "showing a blob which has no more adjacent blobs and " << count << " stacked elements\n";
        M_Utils::dbgDisplayBlob(blob);
      }
#endif
      break;
    }
    // found the element above/below the prev_stacked_blob.
    // is it adjacent based on the prev_stacked blob's location and central blob's height?

    // ----------------COMMENT AND/OR CODE IN QUESTION START---------------------
    //if(stacked_blob->validword || stacked_blob->onRowNormal()) { // if its part of a valid word then it's discarded here
//#ifdef DBG_STACKED_FEATURE_ALOT
      //if(blob->bounding_box() == box) {
        //cout << "showing a blob whose stacked element belongs to a valid word or normal row and thus can't be stacked\n";
        //dbgDisplayBlob(blob);
        //cout << "showing the candidate which can't be stacked\n";
        //dbgDisplayBlob(stacked_blob);
      //}
//#endif
      //return count;
    //}
    // ----------------COMMENT AND/OR CODE IN QUESTION END-----------------------

    TBOX central_bb = central_blob->getBoundingBox();
    if(isAdjacent(stacked_blob, prev_stacked_blob, dir, false, &central_bb)) {
#ifdef DBG_STACKED_FEATURE_ALOT
      int dbgall = false;
      if(blob->bounding_box() == box || dbgall) {
        cout << "showing the blob being measured to have " << count+1 << " stacked items\n";
        M_Utils::dbgDisplayBlob(blob);
        cout << "showing the stacked blob\n";
        cout << "here is the stacked blob's bottom and top y coords: "
             << stacked_blob->bottom() << ", " << stacked_blob->top() << endl;
        cout << "here is the bottom and top y coords of the blob being measured: "
             << central_blob->bottom() << ", " << central_blob->top() << endl;
        cout << "here is the bottom and top y coords of the current blob above/below the stacked blob: "
             << prev_stacked_blob->bottom() << ", " << prev_stacked_blob->top() << endl;
        M_Utils::dbgDisplayBlob(stacked_blob);
      }
#endif
      ++count;
      prev_stacked_blob = stacked_blob;
      assert(prev_stacked_blob->getBoundingBox() == stacked_blob->getBoundingBox());
      stacked_blobs.push_back(stacked_blob);
      stacked_blobs.sort();
    }
    else {
#ifdef DBG_STACKED_FEATURE_ALOT
      if(blob->bounding_box() == box) {
        cout << "showing a blob which has no (or no more) stacked features\n";
        M_Utils::dbgDisplayBlob(blob);
        cout << "showing the candidate which was not adjacent\n";
        M_Utils::dbgDisplayBlob(stacked_blob);
      }
#endif
      break;
    }
  }
  return count;
}

bool NumVerticallyStackedBlobsFeatureExtractor::isAdjacent(
    BlobData* const neighbor, BlobData* const curblob,
    const BlobSpatial::Direction dir, const bool seg_mode, TBOX* dimblob) {
  TBOX cb_boundbox = curblob->getBoundingBox();
  if(dimblob == NULL)
    dimblob = &cb_boundbox;
  double cutoff = (double)dimblob->area() / (double)16;
  if((double)(neighbor->getBoundingBox().area()) < cutoff)
    return false;
  bool vert = (dir == BlobSpatial::UP || dir == BlobSpatial::DOWN) ? true : false;
  bool ascending = (vert ? ((dir == BlobSpatial::UP) ? true : false)
      : ((dir == BlobSpatial::RIGHT) ? true : false));
  double dist_thresh = (double)(vert ? dimblob->height() : dimblob->width());
  double thresh_param = 2;
//  if(seg_mode)
//    thresh_param = 4;
  dist_thresh /= thresh_param;
  double distop1, distop2; // dist = distop1 - distop2
  if(vert) {
    distop1 = ascending ? (double)neighbor->getBoundingBox().bottom() : (double)curblob->getBoundingBox().bottom();
    distop2 = ascending ? (double)curblob->getBoundingBox().top() : (double)neighbor->getBoundingBox().top();
  }
  else {
    distop1 = ascending ? (double)neighbor->getBoundingBox().left() : (double)curblob->getBoundingBox().left();
    distop2 = ascending ? (double)curblob->getBoundingBox().right() : (double)neighbor->getBoundingBox().right();
  }
  assert(distop1 >= distop2);
  double dist = distop1 - distop2;
  if(dist <= dist_thresh)
    return true;
  return false;
}

void NumVerticallyStackedBlobsFeatureExtractor::doSegmentationInit(BlobDataGrid* blobDataGrid) {
  if(blobDataKey < 0) {
    blobDataKey = findOpenBlobDataIndex(blobDataGrid);
  }
}

NumVerticallyStackedBlobsData* NumVerticallyStackedBlobsFeatureExtractor::getBlobFeatureData(BlobData* const blobData) {
  if(blobDataKey < 0) {
    return NULL;
  }
  return (NumVerticallyStackedBlobsData*)(blobData->getVariableDataAt(blobDataKey));
}

BlobFeatureExtractorDescription* NumVerticallyStackedBlobsFeatureExtractor::getFeatureExtractorDescription() {
  return description;
}

