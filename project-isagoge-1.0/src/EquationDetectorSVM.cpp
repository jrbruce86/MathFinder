/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:		EquationDetectorSVM.cpp
 * Written by:	Jake Bruce, Copyright (C) 2013
 * History: 		Created Feb 28, 2013 9:11:16 PM
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

#include "EquationDetectorSVM.h"

EquationDetectorSVM::EquationDetectorSVM() {

}

int EquationDetectorSVM::FindEquationParts(ColPartitionGrid* part_grid,
    ColPartitionSet** best_columns) {
  // precompute some thresholds



  // run through each blob to determine its features

  // run classifier

  // display result for debugging and evaluation

  return 0;
}

// I kept this as the default implementation provided with Tesseract
int EquationDetectorSVM::LabelSpecialText(TO_BLOCK* to_block) {
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





