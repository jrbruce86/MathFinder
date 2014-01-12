/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   Segmentor1.h
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Oct 27, 2013 1:29:14 AM
 * ------------------------------------------------------------------------
 * Description: First segmentor implementation
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
#ifndef SEGMENTOR1_H
#define SEGMENTOR1_H

#include <BlobInfoGrid.h>
#include <F_Ext1.h>

class Segmentor1 : F_Ext1 {
 public:
  Segmentor1();

  BlobInfoGrid* runSegmentation(BlobInfoGrid* grid);

  inline void setDbgImg(PIX* im) {
    dbgim = im;
  }

 protected:
  void decideAndMerge(BLOBINFO* blob, const int& seg_id);
  void mergeDecision(BLOBINFO* blob, Direction dir);
  void checkIntersecting(BLOBINFO* blob);
  void mergeOperation(BLOBINFO* merge_from, BLOBINFO* to_merge,
      Direction merge_dir);

  bool isOperator(BLOBINFO* blob);
  bool wasAlreadyMerged(BLOBINFO* neighbot, BLOBINFO* blob);
};

#endif
