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
#include <Utils.h>

#include <baseapi.h>

#include <assert.h>
#include <string>
#include <vector>

//#define DBG_COVER_FEATURE
//#define DBG_COVER_FEATURE_ALOT
//#define DBG_FEATURE
//#define DBG_DRAW_RIGHTWARD

NumAlignedBlobsFeatureExtractor::NumAlignedBlobsFeatureExtractor(NumAlignedBlobsFeatureExtractorDescription* description)
: rightwardFeatureEnabled(false),
  downwardFeatureEnabled(false),
  upwardFeatureEnabled(false),
  blobDataKey(-1), indbg(false),
  dbgdontcare(false), highCertaintyThresh(Utils::getCertaintyThresh() / 2) {
  this->description = description;
}

void NumAlignedBlobsFeatureExtractor::doPreprocessing(BlobDataGrid* const blobDataGrid) {
  // Get the key that will be used for retrieving data associated with
  // this class for each blob.
  blobDataKey = findOpenBlobDataIndex(blobDataGrid);

#ifdef DBG_DRAW_RIGHTWARD
  rightwardIm = pixCopy(NULL, blobDataGrid->getBinaryImage());
  rightwardIm = pixConvertTo32(rightwardIm);
#endif

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

#ifdef DBG_DRAW_RIGHTWARD
  pixDisplay(rightwardIm, 100, 100);
  std::cout << "Showing the blobs that have at least one rightward adjacent neighbor (feature) in red.\n";
  Utils::waitForInput();
  pixDestroy(&rightwardIm);
#endif
}

std::vector<DoubleFeature*> NumAlignedBlobsFeatureExtractor::extractFeatures(BlobData* const blob) {

  NumAlignedBlobsData* const data = (NumAlignedBlobsData*)(blob->getVariableDataAt(blobDataKey));

#ifdef DBG_FEATURE
  double rhabc = (double)(data->getRhabcCount());
  double uvabc = (double)(data->getUvabcCount());
  double dvabc = (double)(data->getDvabcCount());
  if(rhabc > 0 && rightwardFeatureEnabled)
    std::cout << "Displayed blob has rhabc = " << rhabc << std::endl;
  if(uvabc > 0 && upwardFeatureEnabled)
    std::cout << "Displayed blob has uvabc = " << uvabc << std::endl;
  if(dvabc > 0 && downwardFeatureEnabled)
    std::cout << "Displayed blob has dvabc = " << dvabc << std::endl;
  if(rhabc > 0 || uvabc > 0 || dvabc > 0)
    M_Utils::dbgDisplayBlob(blob);
#endif

  return data->getExtractedFeatures();
}


int NumAlignedBlobsFeatureExtractor::countCoveredBlobs(BlobData* const blob,
    BlobDataGrid* const blobDataGrid, BlobSpatial::Direction dir, bool seg_mode) {

  BlobDataGridSearch bigs(blobDataGrid);
  GenericVector<BlobData*> covered_blobs;
  GenericVector<BlobData*> noncovered_blobs;

  TBOX* segbox = NULL;

  TBOX* blob_box = NULL; // this could either be the blob itself or the blob's segmentation depending on
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
  indbg = false;
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
    std::cout << "found it!\n";
    M_Utils::dispBlobDataRegion(blob, blobDataGrid->getBinaryImage());
    M_Utils::dispHlBlobDataRegion(blob, blobDataGrid->getBinaryImage());
    M_Utils::waitForInput();
    indbg = true;
  }
#endif
  // Below code and comment currently in question.... Should I do this?? I'm thinking not. All blobs should have this feature I think. Can't rely on Tesseract in face of math, so no telling if it's right or wrong.
  // ----------------COMMENT AND CODE IN QUESTION START---------------------
  // Some filters. First off I don't filter anything if I'm using this at the
  // heuristic merge stage (the seg_mode flag would be set to true). So with
  // that out of the way, I filter if the blob belongs to a word matching
  // an entry in Tesseract's dictionary and the word was recognized with a
  // high confidence. I also filter if the blob resides on a row that was
  // found to have a high likelihood of being part of a paragraph (i.e.,
  // 'normal' row of text).
  if(!seg_mode &&
      ((blob->belongsToRecognizedWord() &&
          (blob->getWordRecognitionConfidence() > Utils::getCertaintyThresh()))
       || blob->belongsToRecognizedNormalRow()
       || blob->getWordRecognitionConfidence() > highCertaintyThresh)) {
    return 0;
  }
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
      std::cout << "displaying element found " << ((dir == BlobSpatial::RIGHT) ? "to the right of "
          : (dir == BlobSpatial::UP) ? "above " : "below ") << "the blob of interest\n";
      M_Utils::dispHlBlobDataRegion(n, blobDataGrid->getBinaryImage());
      M_Utils::dispBlobDataRegion(n, blobDataGrid->getBinaryImage());
      std::cout << "at i: " << i << std::endl;
      std::cout << "point: " <<  ((dir == BlobSpatial::RIGHT) ? (blob->bottom() + i) :
          (blob->left() + i)) << std::endl;
    }
#endif
    // if the neighbor is covered and isn't already on the list
    // then add it to the list (in ascending order)
    dbgdontcare = false;
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
        std::cout << "the displayed element is being added to the covered list\n";
        M_Utils::waitForInput();
      }
      else {
        std::cout << "the displayed element was not added to the covered list\n";
        if(!dbgdontcare)
          M_Utils::waitForInput();
      }
    }
#endif
  }
#ifdef DBG_COVER_FEATURE
  if(count > 1 && !single_mode) {
    std::cout << "the highlighted blob is the one being evaluated and has " << count
         << " covered blobs " << "in the " << ((dir == BlobSpatial::RIGHT) ? " rightward "
             : ((dir == BlobSpatial::UP) ? " upward " : " downward ")) << "direction\n";
    M_Utils::dispHlBlobDataRegion(blob, blobDataGrid->getBinaryImage());
    M_Utils::dispBlobDataRegion(blob, blobDataGrid->getBinaryImage());
    M_Utils::waitForInput();
#ifdef DBG_COVER_FEATURE_ALOT
    for(int j = 0; j < covered_blobs.length(); ++j) {
      cout << "the highlighted blob is covered by the blob previously shown\n";
      M_Utils::dispHlBlobDataRegion(covered_blobs[j], blobDataGrid->getBinaryImage());
      M_Utils::dispBlobDataRegion(covered_blobs[j], blobDataGrid->getBinaryImage());
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
  else if(dir == BlobSpatial::RIGHT) {
    data->rhabc_blobs = covered_blobs;
#ifdef DBG_DRAW_RIGHTWARD
    if(covered_blobs.size() > 0) {
      M_Utils::drawHlBlobDataRegion(blob, rightwardIm, LayoutEval::RED);
    }
#endif
  } else if (dir == BlobSpatial::LEFT)
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
    std::cout << "ERROR: isNeighborCovered only supports RIGHT, UP, and DOWN directions\n";
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
            std::cout << "The neighbor is not covered because it's distance from the "
                 << "blob of interest is too great in relation to it's size.\n";
        }
#endif
      }
#ifdef DBG_COVER_FEATURE
      else {
      if(indbg)
        std::cout << "The neighbor is not covered because it is too small\n";
      }
#endif
    }
#ifdef DBG_COVER_FEATURE
    else {
      if(indbg) {
        std::cout << "The neighbor is not covered because its distance is above the threshold of "
           << "half of the blob's width\n";
        dbgdontcare = true;
      }
    }
#endif
  }
#ifdef DBG_COVER_FEATURE
  else {
    if(indbg)
      std::cout << "The neighbor is not covered because its center is outside the boundary\n";
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

