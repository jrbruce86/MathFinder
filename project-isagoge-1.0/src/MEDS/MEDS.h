/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:		MEDS.h
 * Written by:	Jake Bruce, Copyright (C) 2013
 * History: 		Created Oct 4, 2013 9:08:15 PM
 * ------------------------------------------------------------------------
 * Description: This equation detector overrides Tesseract's default one which
 *              was implemented in the 2011 release. TODO: update this
 *              description.
 * ------------------------------------------------------------------------
 * This file is part of Project Isagoge.
 * 
 * Project Isagoge is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Project Isagoge is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Project Isagoge.  If not, see <http://www.gnu.org/licenses/>.  
 ***************************************************************************/

#ifndef MEDS_H_
#define MEDS_H_

//#include <M_Utils.h>
//#include <Basic_Utils.h>

#include <BlobInfoGrid.h>
#include <equationdetectbase.h>

#include <Detection.h>

namespace tesseract {

class Tesseract;
class ColPartition;
class ColPartitionGrid;
class ColPartitionSet;

template <typename DetectorType>
class MEDS : public EquationDetectBase {
 public:

  MEDS() : tess(NULL), blobinfogrid(NULL), img(NULL),
  newapi(NULL) {}

  ~MEDS() {
    reset();
    // TODO: Any remaining cleanup as necessary
  }

  // Iterate over the blobs inside to_block, and set the blobs that we want to
  // process to BSTT_NONE. (By default, they should be BSTT_SKIP). The function
  // returns 0 upon success.
  int LabelSpecialText(TO_BLOCK* to_block) {
    if (to_block == NULL) {
      tprintf("Warning: input to_block is NULL!\n");
      return -1;
    }
    GenericVector<BLOBNBOX_LIST*> blob_lists;
    blob_lists.push_back(&(to_block->blobs));
    blob_lists.push_back(&(to_block->large_blobs));
    for (int i = 0; i < blob_lists.size(); ++i) {
      BLOBNBOX_IT bbox_it(blob_lists[i]);
      for (bbox_it.mark_cycle_pt (); !bbox_it.cycled_list();
           bbox_it.forward()) {
        bbox_it.data()->set_special_text_type(BSTT_NONE);
      }
    }
    return 0;
  }

  // Interface to find possible equation partition grid from part_grid. This
  // should be called after IdentifySpecialText function.
  int FindEquationParts(ColPartitionGrid* part_grid,
                                ColPartitionSet** best_columns) {
    // TODO: This will only work in non-trainmode. A separate class which also
     //       overrides EquationDetectBase will be used for training. This one
     //       requires use of templates which complicates things for training.

     // I'll extract features from my own custom grid which holds both
     // information that can be gleaned from language recognition as
     // well as everything which couldn't (will hold all of the blobs
     // and if they were recognized then holds the word and sentence
     // it belongs to as well)
     blobinfogrid = new BlobInfoGrid(part_grid->gridsize(), part_grid->bleft(),
         part_grid->tright());
     TessBaseAPI a;
     if(newapi == NULL)
       newapi = &a; // can allocate this one on the stack if necessary
                    // (uninitialized it isn't expensive at all)
     blobinfogrid->setTessAPI(newapi);
     blobinfogrid->prepare(part_grid, best_columns, tess);
     // Once the blobinfo grid has been established, it becomes possible to then run
     // each individual blobinfo element through feature detection and classification.
     // After the feature detection/classification step, merging will be carried out
     // in order to ensure proper segmentation.

     // Go on and do predictions now if not in training mode
     detector.initFeatExtSinglePage();
     BLOBINFO* blob;
     BlobInfoGridSearch bigs(blobinfogrid);
     bigs.StartFullSearch();
     while((blob = bigs.NextFullSearch()) != NULL) {
       blob->predicted_math = detector.predict(blob);
     }

     // For now what I'll do is print out the images to be evaluated here and make
     // sure that they don't get overwritten by Tesseract (i.e. turn off any parameter
     // that will print anything later on with the same name)
     // TODO: Incorporate results into Tesseract!!!
     dbgPrintDetectionResults();


     exit(EXIT_SUCCESS);


     // ^^^ For creating the math segmentations I will be using my own custom data structure
     //     called the MathSegment, so I'll have a list of these, then I'll convert those into
     //     ColPartitions later. The MathSegment structure is designed to facilitate merging of
     //     detected mathematical blobs into proper segments for subsequent mathematical
     //     recognition. Each MathSegment initially consists of one BLOBINFO object which was
     //     deemed as mathati, the
     //     segments are then iteratively merged with their nearest neighbors on the blobinfogrid
     //     based on some heuristics. This procedure is carried out for each MathSegment.








     // Once the final results have been obtained, the issue becomes that of converting
     // all of my custom data structures back into structures that will be useful for
     // Tesseract. This will involve simply taking the initial ColPartsGrid and x

     // going to want to call setowner on the blobnbox to change the owner to the new
      //  colpartition... set_owner.... the inline expressions need to be placed in their
      // own separate ColPartition which will have the polyblocktype of INLINE_EXPRESSION
      // and removed from the ColPartition to which it was previously assumed to belong.

     // The InsertPartAfterAbsorb() method from Joe Liu's work is what I
     // will use as a starting point for this purpose.. The first step will be to convert
     // the MathSegmentation list entries into new ColPartitions. This will involve
     // moving all of the BLOBNBOXES inside the mathsegment to the associated ColPartition
     // the ClaimBoxes() method should be useful here, you need to remember to make the
     // original partition disown the boxes first by using DisownBoxes() otherwise an
     // exception will be raised. There may also be a lot of settings for the
     // ColPartition to which the box originally belonged that should be kept the same
     // in the new ColPartition.. This I'll play by ear
     // then I should be able to insert these
     // partitions back into the grid using Joe Liu's technique or something similar.





   /*
     BLOBNBOX* bb = bbgridsearch.NextFullSearch();
     BOX* bbbox = getBlobBoxImCoords(bb);
     cout << "original blob coords in image space:\n";
     dispBoxCoords(bbbox);
     dispRegion(bbbox);
     TBOX cbox = bb->cblob()->bounding_box();
     cout << "original blob coords in cblob grid space:\n";
     cout << "(l,t,r,b): (" << cbox.left() << ", " << cbox.top() \
          << ", " << cbox.right() << ", " << cbox.bottom() << ")\n";
   */


     //waitForInput();


     return 0;
  }

  // Reset the lang_tesseract_ pointer. This function should be called before we
  // do any detector work.
  void SetLangTesseract(Tesseract* lang_tesseract) {
    tess = lang_tesseract;
  }

  inline BlobInfoGrid* getGrid() {
    return blobinfogrid;
  }

  inline void setTessAPI(TessBaseAPI& api) {
    newapi = &api;
  }

  // Clear all heap memory that is specific to just one image
  // so that memory is available to another one. This
  // includes the BlobInfoGrid. The TessBaseApi is owned
  // outside of this class (and is actually allocated on
  // the stack). The image is owned and destroyed outside of the class
  // as well. M_Utils is placed on the stack so is fine
  void reset() {
    if(blobinfogrid != NULL) {
      delete blobinfogrid;
      blobinfogrid = NULL;
    }
    // TODO: Delete any other heap allocated datastructures
    //       specific to a single image
  }

  void dbgPrintDetectionResults() {

  }

  // The final results after segmentation is carried out
  void dbgPrintSegmentationResults() {

  }


 private:
  BlobInfoGrid* blobinfogrid; // blob grid on which I extract features, carry out
                              // binary classification, and segment regions of
                              // interest
  M_Utils mutils; // static class with assorted useful functions
  Tesseract* tess; // language-specific ocr engine
  PIX* img; // the binary image that is being operated on
  TessBaseAPI* newapi;

  DetectorType detector;
};

}

#endif
