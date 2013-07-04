/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:		isagoge_main.cpp
 * Written by:	Jake Bruce, Copyright (C) 2013
 * History: 		Created Feb 28, 2013 9:34:34 PM 
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

#include "allheaders.h"
#include "DocumentLayoutTest.h"

#include <iostream>
using namespace std;

int main() {
  string topdir = "../test_sets/";
  DocumentLayoutTester google_test;
  google_test.setFileStructure(topdir, "single_image_1/", ".png");
  google_test.activateNonScrollView();
  //google_test.activateAllBoolParams();

  // run layout analysis on google's test images first:
  google_test.runTessLayout();

  /*
  DocumentLayoutTester scan_test;
  scan_test.setFileStructure(topdir, "scanned_text/", ".png");
  scan_test.activateNonScrollView();

  // run layout analysis on my scanned test images second:
  scan_test.runTessLayout();
*/
  /*
   // ResultIterator* page = api.GetIterator();
   page->Begin(); // move to the beginning of the page
   Pix* im = page->GetBinaryImage(RIL_PARA);
   pixDisplay(im, 100, 100);
   */
  return 0;
}



