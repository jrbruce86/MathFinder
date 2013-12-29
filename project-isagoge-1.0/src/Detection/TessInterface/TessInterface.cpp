/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:       MEDS.cpp
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:         Created Oct 4, 2013 9:11:16 PM
 * Description: TODO
 *
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

#include "TessInterface.h"
#include "tesseractclass.h"
#include "bbgrid.h"

//#define SHOW_GRID

namespace tesseract {

TessInterface::TessInterface() : tess(NULL), blobinfogrid(NULL), img(NULL),
    api(NULL) {}

TessInterface::~TessInterface() {
  reset();
  // TODO: Any remaining cleanup as necessary
}

void TessInterface::reset() {
  if(blobinfogrid != NULL) {
    delete blobinfogrid; // tesseract api owned by and thus deleted with the grid
    blobinfogrid = NULL;
  }
  // TODO: Delete any other heap allocated datastructures
  //       specific to a single image
}

int TessInterface::FindEquationParts(ColPartitionGrid* part_grid,
    ColPartitionSet** best_columns) {
  // I'll extract features from my own custom grid which holds both
  // information that can be gleaned from language recognition as
  // well as everything which couldn't (will hold all of the blobs
  // and if they were recognized then holds the word and sentence
  // it belongs to as well)
#ifdef SHOW_GRID
  static int dbg_img_index = 1;
#endif
  blobinfogrid = new BlobInfoGrid(part_grid->gridsize(), part_grid->bleft(),
      part_grid->tright());
  blobinfogrid->setTessAPI(api);
  blobinfogrid->prepare(part_grid, best_columns, tess);
#ifdef SHOW_GRID
    string winname = "BlobInfoGrid for Image " + Basic_Utils::intToString(dbg_img_index);
    ScrollView* gridviewer = blobinfogrid->MakeWindow(100, 100, winname.c_str());
    blobinfogrid->DisplayBoxes(gridviewer);
    M_Utils::waitForInput();
    ++dbg_img_index;
#endif
  return 0;
}

// I'll keep this as the default implementation provided with Tesseract (for now)
int TessInterface::LabelSpecialText(TO_BLOCK* to_block) {
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

// this gets called when the equation detector is being set
void TessInterface::SetLangTesseract(Tesseract* lang_tesseract) {
  tess = lang_tesseract;
}



} // end namespace tesseract

