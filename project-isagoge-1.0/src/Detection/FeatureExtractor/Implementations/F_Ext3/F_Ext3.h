/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   F_Ext3.h
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Jan 1, 2014 1:27:34 PM
 * ------------------------------------------------------------------------
 * Description: Implements the FeatureExtractor interface. Derived from
 *              F_Ext1. See F_Ext3.cpp for more details.
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

#ifndef F_EXT3_H
#define F_EXT3_H

#include <F_Ext1.h>

class F_Ext3 : public F_Ext1 {
 public:

  void initFeatExtSinglePage();

  std::vector<double> extractFeatures(tesseract::BLOBINFO* blob);

  inline string getFeatExtName() {
    return (string)"F_Ext3";
  }

  int numFeatures();
};


#endif