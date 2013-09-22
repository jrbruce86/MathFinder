/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:		isagoge_main.cpp
 * Written by:	Jake Bruce, Copyright (C) 2013
 * History: 		Created Feb 28, 2013 9:34:34 PM 
 * ------------------------------------------------------------------------
 * Description: Main file from which evaluations of various equation
 *              detectors on various datasets and experiments are run.
 *              Primarily uses DocumentLayoutTester class functions to
 *              perform tests. See DocumentLayoutTest.h for more info.
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

#include "allheaders.h"
#include "DocumentLayoutTest.h"

#include <iostream>
#include <string>
using namespace std;

// Runs evaluation test on the given detector for the given dataset
// which should be within the "topdir" specified (if detector is null
// then uses Tesseract's default one. The testname is the name of
// the test to be run on the dataset and the name of directory
// wherein the test results will be held (this name should be indicative
// of which equation detector is being used)
void evaluateDataSet(EquationDetectBase* detector, \
    string topdir, string dataset, string testname, \
    string extension=(string)".png");

// For debugging the evaluator
void dbgColorCount(DocumentLayoutTester*);

int main() {
  string topdir = "../test_sets/";
  string dataset = "advcalc1_without_labels";

  // Test Tesseract's default equation detector
  evaluateDataSet(NULL, topdir, dataset, "tessdefault");

  /*
  // Test my equation detector
  EquationDetectBase* mydetector = new EquationDetectorSVM();
  evaluateDataSet(mydetector, topdir, dataset, "my_detector");
  */

  // TODO: Compare the results!

  return 0;
}

void evaluateDataSet(EquationDetectBase* detector, \
    string topdir, string dataset, string testname, \
    string extension) {
  // constructor sets the equation detector
  DocumentLayoutTester test(detector);
  // create the file structure
  test.setFileStructure(topdir, dataset, extension);
  //test.colorGroundTruthBlobs();
  test.activateNonScrollView();
  //test.activateBoolParam("textord_tabfind_show_partitions");
  //test.activateBoolParam("textord_tabfind_show_blocks");
  //test.activateBoolParam("textord_debug_images");
  //test.activateAllParams();
  //test.activateIntParam("textord_tabfind_show_images");
  // deactivate dumping table images (requires input file to have
  // specific name "test1.tif")
  test.deActivateBoolParam("textord_dump_table_images");
  //test.deActivateBoolParam("textord_debug_images");

  // run layout analysis on the images first (this includes
  // running the equation detection as well)
  //test.runTessLayout(testname);


  test.evalTessLayout(testname, true);

  // TODO: Modify DocumentLayoutTester's destructor to avoid
  //       memory leaks!!!
  //dbgColorCount(&test);
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

