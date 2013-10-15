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

#include "M_Utils.h"
#include "BlobInfoGrid.h"
#include "equationdetectbase.h"

namespace tesseract {

class Tesseract;
class ColPartition;
class ColPartitionGrid;
class ColPartitionSet;

class MEDS : public EquationDetectBase {
public:

  MEDS();

  // Iterate over the blobs inside to_block, and set the blobs that we want to
  // process to BSTT_NONE. (By default, they should be BSTT_SKIP). The function
  // returns 0 upon success.
  int LabelSpecialText(TO_BLOCK* to_block);

  // Interface to find possible equation partition grid from part_grid. This
  // should be called after IdentifySpecialText function.
  int FindEquationParts(ColPartitionGrid* part_grid,
                                ColPartitionSet** best_columns);

  // Reset the lang_tesseract_ pointer. This function should be called before we
  // do any detector work.
  void SetLangTesseract(Tesseract* lang_tesseract);

private:
  BlobInfoGrid* blobinfogrid; // blob grid on which I extract features, carry out
                              // binary classification, and segment regions of
                              // interest
  M_Utils mutils; // static class with assorted useful functions
  Tesseract* tess; // language-specific ocr engine
  PIX* img; // the binary image that is being operated on
};


}

#endif
