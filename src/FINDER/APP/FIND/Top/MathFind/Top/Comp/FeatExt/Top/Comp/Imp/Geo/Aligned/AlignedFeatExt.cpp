/*
 * NumAlignedBlobsFeatureExtractor.cpp
 *
 *  Created on: Nov 11, 2016
 *      Author: jake
 */

#include <AlignedFeatExt.h>

#include <AlignedDesc.h>
#include <BlobDataGrid.h>
#include <BlobData.h>
#include <AlignedData.h>
#include <Direction.h>
#include <M_Utils.h>
#include <BlobFeatExtDesc.h>
#include <FeatExtFlagDesc.h>

#include <baseapi.h>

#include <assert.h>
#include <string>
#include <vector>

NumAlignedBlobsFeatureExtractor::NumAlignedBlobsFeatureExtractor(NumAlignedBlobsFeatureExtractorDescription* description)
: rightwardFeatureEnabled(false),
  downwardFeatureEnabled(false),
  upwardFeatureEnabled(false),
  blobDataKey(-1) {
  this->description = description;
}

void NumAlignedBlobsFeatureExtractor::doPreprocessing(BlobDataGrid* const blobDataGrid) {
  // Get the key that will be used for retrieving data associated with
  // this class for each blob.
  blobDataKey = findOpenBlobDataIndex(blobDataGrid);

  // Determine the adjacent covered neighbors in the rightward, downward,
  // and/or upward directions for each blob depending on which features are
  // enabled
  BlobDataGridSearch gridSearch(blobDataGrid);
  gridSearch.StartFullSearch();
  BlobData* blob = NULL;
  while((blob = gridSearch.NextFullSearch()) != NULL) {
    NumAlignedBlobsData* const data = new NumAlignedBlobsData(description);
    assert(blobDataKey == blob->appendNewVariableData(data));
    if(rightwardFeatureEnabled) {
      const int count = countCoveredBlobs(blob, blobDataGrid, BlobSpatial::RIGHT);
      data->setRhabcCount(count)
          ->setRhabcFeature(M_Utils::expNormalize(count));
    }
    if(upwardFeatureEnabled) {
      const int count = countCoveredBlobs(blob, blobDataGrid, BlobSpatial::UP);
      data->setUvabcCount(count)
          ->setUvabcFeature(M_Utils::expNormalize(count));
    }
    if(downwardFeatureEnabled) {
      const int count = countCoveredBlobs(blob, blobDataGrid, BlobSpatial::DOWN);
      data->setDvabcCount(count)
          ->setDvabcFeature(M_Utils::expNormalize(count));
    }
  }
}

std::vector<DoubleFeature*> NumAlignedBlobsFeatureExtractor::extractFeatures(BlobData* const blob) {

  NumAlignedBlobsData* const data = (NumAlignedBlobsData*)(blob->getVariableDataAt(blobDataKey));

#ifdef DBG_FEAT1
  double rhabc = (double)(data->getRhabcCount());
  double uvabc = (double)(data->getUvabcCount());
  double dvabc = (double)(data->getDvabcCount());
  if(rhabc > 1 && rightwardFeatureEnabled)
    cout << "Displayed blob has rhabc = " << rhabc << endl;
  if(uvabc > 1 && upwardFeatureEnabled)
    cout << "Displayed blob has uvabc = " << uvabc << endl;
  if(dvabc > 1 && downwardownwardFeatureEnabled)
    cout << "Displayed blob has dvabc = " << dvabc << endl;
  if(rhabc > 1 || uvabc > 1 || dvabc > 1)
    dbgDisplayBlob(blob);
#endif

  return data->getExtractedFeatures();
}


int NumAlignedBlobsFeatureExtractor::countCoveredBlobs(BlobData* const blob,
    BlobDataGrid* const blobDataGrid, BlobSpatial::Direction dir, bool seg_mode) {

  BlobDataGridSearch bigs(blobDataGrid);
  GenericVector<BlobData*> covered_blobs;
  GenericVector<BlobData*> noncovered_blobs;

  TBOX* segbox = NULL;

  TBOX* blob_box; // this could either be the blob itself or the blob's segmentation depending on
                  // whether seg_mode is turned off or on respectively.
  TBOX bbox = blob->getBoundingBox();
  if(seg_mode) {
    // !!!!!!Segmode should only be true when running the segmentor!!!!!

    NumAlignedBlobsData* data = (NumAlignedBlobsData*)(blob->getVariableDataAt(blobDataKey));
    TBOX* segbox = data->blobMergeInfo->segment_box;
    assert(segbox != NULL);
    // if in segmentation mode, then the blob may already have
    // covered blobs that were previously found, go ahead and add these to start with
    if(dir == BlobSpatial::UP)
      covered_blobs = data->uvabc_blobs;
    else if(dir == BlobSpatial::DOWN)
      covered_blobs = data->dvabc_blobs;
    else if(dir == BlobSpatial::LEFT)
      covered_blobs = data->lhabc_blobs;
    else if(dir == BlobSpatial::RIGHT)
      covered_blobs = data->rhabc_blobs;
    else
      assert(false);
    blob_box = segbox;
  }
  else
    blob_box = &bbox;
  assert(blob_box != NULL);

  int count = 0;
  bool indbg = false;
#ifdef DBG_COVER_FEATURE
  bool single_mode = true; // if this is true thne only debugging one blob and its neighbors
                            // otherwise then debug all of them
  inT16 dbgleft = -1;
  inT16 dbgtop = -1;
  if(dbgleft < 0 || dbgtop < 0) // if i'm not debugging anything in particular debug everything
    single_mode = false;

  inT16 neighborleft = -1;
  inT16 neighbortop = -1;

  TBOX blobbox = blob->bounding_box();
  if(blobbox.left() == dbgleft && blobbox.top() == dbgtop) {
    cout << "found it!\n";
    M_Utils m;
    m.dispBlobInfoRegion(blob, curimg);
    m.dispHlBlobInfoRegion(blob, curimg);
    m.waitForInput();
    indbg = true;
  }
#endif
  // Below code and comment currently in question.... Should I do this?? I'm thinking not. All blobs should have this feature I think. Can't rely on Tesseract in face of math, so no telling if it's right or wrong.
  // ----------------COMMENT AND CODE IN QUESTION START---------------------
  // if the blob in question belongs to a valid word then
  // I discard it immediately, unless it belongs to an "abnormal row"
  // in which case the feature is still found. see findAllRowCharacteristics() method
  // of the BlobInfoGrid for what factors that are used to determine this. This feature
  // really isn't too effective for normal rows
  // TODO: See about finding a way to put the below code back in
  // if(blob->validword && blob->onRowNormal() /*&& !seg_mode*/)
  //  return 0;
  // ----------------COMMENT AND CODE IN QUESTION END---------------------
  int range;
  if(dir == BlobSpatial::RIGHT || dir == BlobSpatial::LEFT)
    range = blob_box->height();
  else if(dir == BlobSpatial::UP || dir == BlobSpatial::DOWN) {
    range = blob_box->width();
  }
  else {
    std::cout << "ERROR: countCoveredBlobs only "
         << "supports upward, downward, leftward, and rightward searches\n";
    assert(false);
  }

  // do beam searches to look for covered blobs
  // TO consider: For increased efficiency do fewer searches (come up with reasonable empirical
  //       way of dividing the current blob so enough searches are done but it isn't excessive
  for(int i = 0; i < range; ++i) {
    BlobData* n = NULL; // the neighbor
    if(dir == BlobSpatial::RIGHT || dir == BlobSpatial::LEFT) {
      bigs.StartSideSearch((dir == BlobSpatial::RIGHT) ? blob_box->right() : blob_box->left(),
          blob_box->bottom()+i, blob_box->bottom()+i+1);
      n = bigs.NextSideSearch((dir == BlobSpatial::RIGHT) ? false : true); // this starts with the current blob
    }
    else {
      bigs.StartVerticalSearch(blob_box->left()+i, blob_box->left()+i+1,
          (dir == BlobSpatial::UP) ? blob_box->top() : blob_box->bottom());
      n = bigs.NextVerticalSearch((dir == BlobSpatial::UP) ? false : true);
    }
    if(n == NULL)
      continue;
    bool nothing = false;
    BlobData* previous_neighbor_ptr = NULL;
    while(n->getBoundingBox() == *blob_box
        || ((dir == BlobSpatial::RIGHT) ? (n->getBoundingBox().left() <= blob_box->right())
            : ((dir == BlobSpatial::LEFT) ? (n->getBoundingBox().right() >= blob_box->left())
              : ((dir == BlobSpatial::UP) ? n->getBoundingBox().bottom() <= blob_box->top()
                : n->getBoundingBox().top() >= blob_box->bottom())))
                  || noncovered_blobs.bool_binary_search(n)) {
      if(dir == BlobSpatial::RIGHT || dir == BlobSpatial::LEFT)
        n = bigs.NextSideSearch(dir == BlobSpatial::RIGHT ? false : true);
      else
        n = bigs.NextVerticalSearch(dir == BlobSpatial::UP ? false : true);
      if(n == NULL) {
        nothing = true;
        break;
      }
    }
    if(nothing)
      continue;
#ifdef DBG_COVER_FEATURE
    bool was_added = false;
    if(indbg) {
      cout << "displaying element found " << ((dir == RIGHT) ? "to the right of "
          : (dir == UP) ? "above " : "below ") << "the blob of interest\n";
      M_Utils::dispHlBlobInfoRegion(n, curimg);
      M_Utils::dispBlobInfoRegion(n, curimg);
      cout << "at i: " << i << endl;
      cout << "point: " <<  ((dir == RIGHT) ? (blob->bottom() + i) :
          (blob->left() + i)) << endl;
    }
#endif
    // if the neighbor is covered and isn't already on the list
    // then add it to the list (in ascending order)
    bool dbgdontcare = false;
    if(!covered_blobs.bool_binary_search(n) && n != blob) {
      // Determine whether or not the neighbor is covered by the current bounding box
      // If in segmentation mode, then determine whether or not the neighbor is covered by the current blob's segmentation box
      if(isNeighborCovered(n->getBoundingBox(), !seg_mode ? blob->getBoundingBox() : *segbox, dir, seg_mode)) {
#ifdef DBG_COVER_FEATURE
        if(indbg)
          was_added = true;
#endif
        covered_blobs.push_back(n);
        covered_blobs.sort();
        ++count;
      }
    }
    if(!noncovered_blobs.bool_binary_search(n) && n != blob) {
      // if it's already been found to not be covered by the blob
      // then put it on this list so it won't get tested again
      noncovered_blobs.push_back(n);
      noncovered_blobs.sort();
    }
#ifdef DBG_COVER_FEATURE
    if(indbg) {
      if(was_added) {
        cout << "the displayed element is being added to the covered list\n";
        M_Utils::waitForInput();
      }
      else {
        cout << "the displayed element was not added to the covered list\n";
        if(!dbgdontcare)
          M_Utils::waitForInput();
      }
    }
#endif
  }
#ifdef DBG_COVER_FEATURE
  if(count > 1 && !single_mode) {
    cout << "the highlighted blob is the one being evaluated and has " << count
         << " covered blobs " << "in the " << ((dir == RIGHT) ? " rightward "
             : ((dir == UP) ? " upward " : " downward ")) << "direction\n";
    M_Utils::dispHlBlobInfoRegion(blob, curimg);
    M_Utils::dispBlobInfoRegion(blob, curimg);
    M_Utils::waitForInput();
#ifdef DBG_COVER_FEATURE_ALOT
    for(int j = 0; j < covered_blobs.length(); ++j) {
      cout << "the highlighted blob is covered by the blob previously shown\n";
      M_Utils::dispHlBlobInfoRegion(covered_blobs[j], curimg);
      M_Utils::dispBlobInfoRegion(covered_blobs[j], curimg);
      M_Utils::waitForInput();
    }
#endif
  }
#endif
  NumAlignedBlobsData* const data = (NumAlignedBlobsData*)(blob->getVariableDataAt(blobDataKey));
  if(dir == BlobSpatial::UP)
    data->uvabc_blobs = covered_blobs;
  else if(dir == BlobSpatial::DOWN)
    data->dvabc_blobs = covered_blobs;
  else if(dir == BlobSpatial::RIGHT)
    data->rhabc_blobs = covered_blobs;
  else if (dir == BlobSpatial::LEFT)
    data->lhabc_blobs = covered_blobs;
  else
    assert(false);
  return count;
}

/**
 * Determines whether or not a neighbor bounding box is "covered" by the current
 * bounding box based on distance vs size relationships between the two. A neighbor
 * bounding box is "covered" by the current bounding box in a given direction when
 * it resides entirely within the boundary of the current bounding box, is close
 * enough to the current object in the desired direction, is not too small in
 * relation to the current object, and has a size greater than or equal to half
 * of the its distance from the current object (where size is the greater of width
 * or height). If seg_mode is true then the current blob's bounding box is assumed
 * to correspond with that blob's segmentation results and some of the paramters are
 * slightly tweaked.
 */
bool NumAlignedBlobsFeatureExtractor::isNeighborCovered(const TBOX& neighbor,
    const TBOX& blob_box, const BlobSpatial::Direction& dir, const bool seg_mode) {
  // Below code and comment currently in question.... Should I do this?? I'm thinking not. All blobs should have this feature I think. Can't rely on Tesseract in face of math, so no telling if it's right or wrong.
  // ----------------COMMENT AND CODE IN QUESTION START---------------------
  //if(neighbor->onRowNormal())
  //  return false;
  // ----------------COMMENT AND CODE IN QUESTION END---------------------

  inT16 neighbor_dist;
  inT16 blob_upper;
  inT16 blob_lower;
  inT16 neighbor_center;
  double dist_thresh_param = (double)2;
  if(seg_mode)
    dist_thresh_param = (double)4;
  double dist_threshold;
  double area_thresh;
  double dist_to_size_thresh = (double)2;
  if(dir == BlobSpatial::RIGHT || dir == BlobSpatial::LEFT) {
    blob_lower = blob_box.bottom();
    blob_upper = blob_box.top();
    dist_threshold = (double)blob_box.height() / dist_thresh_param;
    neighbor_dist = (dir == BlobSpatial::RIGHT) ? neighbor.left() - blob_box.right()
        : blob_box.left() - neighbor.right();
    area_thresh = (double)(blob_box.height()) / (double)32;
    neighbor_center = M_Utils::centery(neighbor);
  }
  else if(dir == BlobSpatial::UP || dir == BlobSpatial::DOWN) {
    blob_lower = blob_box.left();
    blob_upper = blob_box.right();
    dist_threshold = (double)(blob_box.width()) / dist_thresh_param; // TODO: tweak for seg_mode!!!
    if(dir == BlobSpatial::UP)
      neighbor_dist = neighbor.bottom() - blob_box.top();
    else
      neighbor_dist = blob_box.bottom() - neighbor.top();
    area_thresh = (double)(blob_box.width()) / (double)32;
    neighbor_center = M_Utils::centerx(neighbor);
  }
  else {
    cout << "ERROR: isNeighborCovered only supports RIGHT, UP, and DOWN directions\n";
    assert(false);
  }
  // is the neighbor covered?
  if(neighbor_center >= blob_lower && neighbor_center <= blob_upper) {
    // is the neighbor adjacent?
    if(neighbor_dist <= dist_threshold) {
      const double area = (double)(neighbor.area());
      if(area > area_thresh) {
        double blobsize = (neighbor.height() >= neighbor.width())
            ? (double)(neighbor.height()) : (double)(neighbor.width());
        double dist_to_size_ratio = (double)neighbor_dist /
            blobsize;
        if(dist_to_size_ratio < dist_to_size_thresh) {
          return true;
        }
#ifdef DBG_COVER_FEATURE
        else {
          if(indbg)
            cout << "The neighbor is not covered because it's distance from the "
                 << "blob of interest is too great in relation to it's size.\n";
        }
#endif
      }
#ifdef DBG_COVER_FEATURE
      else {
      if(indbg)
        cout << "The neighbor is not covered because it is too small\n";
      }
#endif
    }
#ifdef DBG_COVER_FEATURE
    else {
      if(indbg) {
        cout << "The neighbor is not covered because its distance is above the threshold of "
           << "half of the blob's width\n";
        dbgdontcare = true;
      }
    }
#endif
  }
#ifdef DBG_COVER_FEATURE
  else {
    if(indbg)
      cout << "The neighbor is not covered because its center is outside the boundary\n";
  }
#endif
  return false;
}

void NumAlignedBlobsFeatureExtractor::enableRightwardFeature() {
  enabledFlagDescriptions.push_back(description->getRightwardFlagDescription());
  rightwardFeatureEnabled = true;
}
void NumAlignedBlobsFeatureExtractor::enableDownwardFeature() {
  enabledFlagDescriptions.push_back(description->getDownwardFlagDescription());
  downwardFeatureEnabled = true;
}

void NumAlignedBlobsFeatureExtractor::enableUpwardFeature() {
  enabledFlagDescriptions.push_back(description->getUpwardFlagDescription());
  upwardFeatureEnabled = true;
}

BlobFeatureExtractorDescription* NumAlignedBlobsFeatureExtractor::getFeatureExtractorDescription() {
  return description;
}

std::vector<FeatureExtractorFlagDescription*> NumAlignedBlobsFeatureExtractor::getEnabledFlagDescriptions() {
  return enabledFlagDescriptions;
}

void NumAlignedBlobsFeatureExtractor::doSegmentationInit(BlobDataGrid* blobDataGrid) {
  if(blobDataKey < 0) {
    blobDataKey = findOpenBlobDataIndex(blobDataGrid);
  }
}

NumAlignedBlobsData* NumAlignedBlobsFeatureExtractor::getBlobFeatureData(BlobData* const blobData) {
  if(blobDataKey < 0) {
    return NULL;
  }
  return (NumAlignedBlobsData*)(blobData->getVariableDataAt(blobDataKey));
}

