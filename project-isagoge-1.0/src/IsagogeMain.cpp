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

#include <allheaders.h>
#include <MEDS_Trainer.h>
#include <DocumentLayoutTest.h>
#include <MEDS.h>

#include <iostream>
#include <string>
using namespace std;

// Runs evaluation test on the given detector for the given dataset
// which should be within the "topdir" specified (if detector is null
// then uses Tesseract's default one. The testname is the name of
// the test to be run on the dataset and the name of directory
// wherein the test results will be held (this name should be indicative
// of which equation detector is being used)
void evaluateDataSet(EquationDetectBase* detector,
    string topdir, string dataset, string testname,
    string extension=(string)".png");

// For debugging the evaluator
void dbgColorCount(DocumentLayoutTester*);

int main() {
  string topdir = "../test_sets/";
  string dataset = "advcalc1_without_labels";
  string train_dir = "training/";
  string train_set = "SVM_AdvCalc1_15/";
  const string trainpath = topdir+train_dir+train_set;
  // Test Tesseract's default equation detector
 // evaluateDataSet(NULL, topdir, dataset, "tessdefault");


  // set this to false if only want to train the module
  // if it hasn't been trained yet.
  bool train_always = true;

  // Pick a detector/segmentor combo and train if necessary
  EquationDetectBase* tess_interface = new TessInterface();
  MEDS_Trainer<Detector1> trainer(train_always, trainpath, false);
  trainer.trainDetector();



  // test mine
  // initialize the equation detection/segmenation module
  //EquationDetectBase* mymeds = new MEDS<Detector1>;
  //EquationDetectBase* myMEDS = new MEDS;
  //evaluateDataSet(mymeds, topdir, dataset, "myMEDS");

  // test default
  //evaluateDataSet(NULL, topdir, dataset, "tessdefault");


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
  test.activateEquOutput();

  // run layout analysis on the images first (this includes
  // running the equation detection as well)
  // WARNING: All output .png images (and maybe .tifs?)
  // from tesseract get cut and pasted to the test
  // set's output directory!!
  test.runTessLayout(testname);

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

