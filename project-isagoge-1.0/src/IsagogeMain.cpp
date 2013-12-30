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
#include <MEDS_Types.h>

#include <iostream>
#include <string>
using namespace std;

/**************************************************************************
 * Uncomment out the MEDS module to be tested. Make sure that one and only
 * one of them are uncommented!
 *************************************************************************/
#define MEDS1TEST
//#define MEDS2TEST

#ifdef MEDS1TEST
typedef MEDS1 MEDSType;
typedef MEDS1Detector DetectorType;
#define MEDSNUM 1;
#endif

#ifdef MEDS2TEST
typedef MEDS2 MEDSType;
typedef MEDS2Detector DetectorType;
#define MEDSNUM 2;
#endif

// Runs evaluation test on the given detector for the given dataset
// which should be within the "topdir" specified (if detector is null
// then uses Tesseract's default one. The testname is the name of
// the test to be run on the dataset and the name of directory
// wherein the test results will be held (this name should be indicative
// of which equation detector is being used). if the bool argument, type_eval
// is turned off then only math/non-math is evaluated, if it is turned on then
// displayed, embedded, and labels are evaluated seperately.
void evaluateDataSets(EquationDetectBase*& detector,
    string topdir, vector<string> datasets, string testname, bool type_eval=true,
    string extension=(string)".png");

int main() {
  string topdir = "../test_sets/";
  string dataset = "test";
  string train_dir = "training/";
  string train_set = "SVM_AdvCalc1_15/";
  const string trainpath = topdir+train_dir+train_set;

  // set this to false if only want to train the module
  // if it hasn't been trained yet.
  bool train_always = true;
  // if true then extract features during training even
  // if they've already been written to a file
  bool new_samples = true;

  // specify how the tesseract api should always be initialized
  // i.e. what language and the path to the training files necessary
  vector<string> api_init_params;
  api_init_params.push_back((string)"/usr/local/share/"); // tesseract training file path
  api_init_params.push_back((string)"eng"); // tesseract language

  // Pick a detector/segmentor combo and train if necessary
  MEDS_Trainer<DetectorType> trainer(train_always, trainpath, new_samples, api_init_params);
  trainer.trainDetector();
  cout << "Finished training the detector!\n";

  // Instantiate a MEDS module which uses the detector that was
  // initialized by the trainer
  EquationDetectBase* meds = new MEDSType(); // MEDS is owned by the evaluator
  DetectorType* detector = trainer.getDetector();
  ((MEDSType*)meds)->setDetector(detector); // MEDS owns the detector
  cout << "Finished initializing MEDS class\n";
  // Test it
  vector<string> datasets;
  for(int i = 0; i < 4; ++i) {
    string dataset_ = dataset + intToString(i+1);
    datasets.push_back(dataset_);
  }
  int testnum_ = MEDSNUM;
  string testnum = intToString(testnum_);
  cout << "About to run evaluation.\n";
  evaluateDataSets(meds, topdir, datasets, "myMEDS" + testnum, false);
  cout << "outside evaluateDataSets\n";
  // TODO: Evaluate default and compare the results!

  return 0;
}

void evaluateDataSets(EquationDetectBase*& meds, string topdir,
    vector<string> datasets, string testname, bool type_eval,
    string extension) {
  bool meds_given = false;
  if(meds)
    meds_given = true;
  DocumentLayoutTester<MEDSType> test(meds);
  cout << "Finished constructing evaluator class.\n";
  if(!type_eval)
    test.turnOffTypeEval();
  test.activateEquOutput();
  for(int i = 0; i < datasets.size(); ++i) {
    string dataset = datasets[i];
    test.setFileStructure(topdir, dataset, extension);
    test.runTessLayout(testname);
    test.evalTessLayout(testname, false);
  }
}

