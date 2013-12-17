/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:       MEDS.h
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:         Created Nov 20, 2013 2:28:16 AM
 * ------------------------------------------------------------------------
 * Description: Overrides Tesseract's EquationBaseDetect for Detector training
 *              purposes only
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

#ifndef TESSINTERFACE_H
#define TESSINTERFACE_H

#include <M_Utils.h>
#include <BlobInfoGrid.h>
#include <equationdetectbase.h>

namespace tesseract {

class Tesseract;
class ColPartition;
class ColPartitionGrid;
class ColPartitionSet;

class TessInterface : public EquationDetectBase {
 public:

  TessInterface();

  ~TessInterface();

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

  inline BlobInfoGrid* getGrid() {
    return blobinfogrid;
  }

  inline void setTessAPI(TessBaseAPI* api_) {
    api = api_;
  }

  // Clear all heap memory that is specific to just one image
  // so that memory is available to another one. This
  // includes the BlobInfoGrid. The TessBaseApi is owned
  // outside of this class (and is actually allocated on
  // the stack). The image is owned and destroyed outside of the class
  // as well. M_Utils is placed on the stack so is fine
  void reset();


 private:
  BlobInfoGrid* blobinfogrid; // blob grid on which I extract features, carry out
                              // binary classification, and segment regions of
                              // interest
  M_Utils mutils; // static class with assorted useful functions
  Tesseract* tess; // language-specific ocr engine
  PIX* img; // the binary image that is being operated on
  TessBaseAPI* api;
};


}

#endif
