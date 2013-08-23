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
#include <string>
using namespace std;

void dbgColorCount(DocumentLayoutTester*);

int main() {
  string topdir = "../test_sets/";
  DocumentLayoutTester google_test;

  google_test.setFileStructure(topdir, "smallertestset1_nolabels", ".png");
  //google_test.colorGroundTruthBlobs();
  google_test.activateNonScrollView();
  //google_test.activateBoolParam("textord_tabfind_show_partitions");
  //google_test.activateBoolParam("textord_tabfind_show_blocks");
  //google_test.activateBoolParam("textord_debug_images");
  //google_test.activateAllParams();
  //google_test.activateIntParam("textord_tabfind_show_images");
  // deactivate dumping table images (requires input file to have
  // specific name "test1.tif")
  google_test.deActivateBoolParam("textord_dump_table_images");
  //google_test.deActivateBoolParam("textord_debug_images");


  string default_test = "default_test";

  // run layout analysis on google's test images first:
  google_test.runTessLayout(default_test);


  google_test.evalTessLayout(default_test, true);

  //dbgColorCount(&google_test);
  /*
  DocumentLayoutTester scan_test;
  scan_test.setFileStructure(topdir, "scanned_text/", ".png");
  scan_test.activateNonScrollView();

  // run layout analysis on my scanned test images second:
  scan_test.runTessLayout();*/

  /*
   // ResultIterator* page = api.GetIterator();
   page->Begin(); // move to the beginning of the page
   Pix* im = page->GetBinaryImage(RIL_PARA);
   pixDisplay(im, 100, 100);
   */
  return 0;
}

void dbgColorCount(DocumentLayoutTester* dlt) {
  cout << "True Positive Pixels: " \
       << dlt->dbgColorCount((string)"Eval_DBG.png", LayoutEval::RED) \
       << endl;
  cout << "False Positive Pixels: " \
       << dlt->dbgColorCount((string)"Eval_DBG.png", LayoutEval::BLUE) \
       << endl;
  cout << "False Negative Pixels: " \
       << dlt->dbgColorCount((string)"Eval_DBG.png", LayoutEval::GREEN) \
       << endl;
}

