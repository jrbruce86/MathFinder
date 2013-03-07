/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:		EquationDetectorSVM.h
 * Written by:	Jake Bruce, Copyright (C) 2013
 * History: 		Created Feb 28, 2013 9:08:15 PM
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

#ifndef EQUATIONDETECTORSVM_H_
#define EQUATIONDETECTORSVM_H_

#include <cstddef>
#include <equationdetectbase.h>
using namespace tesseract;

class EquationDetectorSVM : public EquationDetectBase {
public:

  EquationDetectorSVM();

  // Iterate over the blobs inside to_block, and set the blobs that we want to
  // process to BSTT_NONE. (By default, they should be BSTT_SKIP). The function
  // returns 0 upon success.
  virtual int LabelSpecialText(TO_BLOCK* to_block);

  // Interface to find possible equation partition grid from part_grid. This
  // should be called after IdentifySpecialText function.
  virtual int FindEquationParts(ColPartitionGrid* part_grid,
                                ColPartitionSet** best_columns);

};

#endif // EQUATION_DETECT_SVMH__
