/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   MEDS_Trainer.h
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Oct 27, 2013 1:15:36 AM
 * ------------------------------------------------------------------------
 * Description: High level class for initiating Tesseract to run the MEDS
 *              module in training mode for several training pages,
 *              converting their grid information into
 *              feature vectors (based upon which feature extractor is
 *              in use), aggregating the vectors into a single vector,
 *              and then sending that vector into the Detector/TrainerPredictor
 *              so it may be used for training purposes.
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
#ifndef MEDS_TRAINER_H
#define MEDS_TRAINER_H

#include <Detection.h>
#include <MEDS.h>
#include <Sample.h>
#include <iostream>
#include <string>
using namespace std;

#define DBG_MEDS_TRAINER // comment this out to turn off debugging

// This class carry out any training needed for both the detector and segmenter
template <typename TrainerPredictorType>
class MEDS_Trainer {
 public:
  // The first three arguments are in regards to the detector:
  // 1. train_detector - If true then train the detector even if
  //                     it has already been trained. Otherwise
  //                     only train it if necessary
  // 2. detector_path  - This is the path to both the detector's
  //                     training dataset and to the detector
  //                     itself (i.e. the result of training). These
  //                     two are subdirectories in this path.
  // 3. groundtruth file name - This file is expected to be located
  //                            in the detector_path along with the
  //                            predictor/ and training_set/ subdirectories.
  //                            It holds all the labels used in training.
  // 4. ext (optional) - The extension expected for all images
  //                     (png by default)
  MEDS_Trainer(bool always_train_, const string& detector_path,
      const string& ext_=".png") : detector_segmentor(NULL) {
    top_path = Basic_Utils::checkTrailingSlash(detector_path);
    groundtruth_path = top_path + (string)"*.dat";
    groundtruth_path = Basic_Utils::exec((string)"ls "
        + groundtruth_path + (string)" | tail -n 1 | tr -d '\n'");
    if(groundtruth_path.empty()) {
      cout << "ERROR: A *.dat (groundtruth) file is expected in "
           << top_path << " but was not found.\n";
      exit(EXIT_FAILURE);
    }
    predictor_path = detector_path + (string)"predictor/";
    training_set_path = detector_path + (string)"training_set/";
    always_train = always_train_;
    ext = ext_;
    api.Init("/usr/local/share/", "eng");
     char* page_seg_mode = (char*)"tessedit_pageseg_mode";
     if (!api.SetVariable(page_seg_mode, "3")) {
       cout << "ERROR: Could not set tesseract's api corectly during training!\n";
       exit(EXIT_FAILURE);
     }
     // make sure that we're in the right document layout analysis mode
     int psm = 0;
     api.GetIntVariable(page_seg_mode, &psm);
     assert(psm == tesseract::PSM_AUTO);
     // turn on equation detection
     if (!api.SetVariable("textord_equation_detect", "true")) {
       cout << "Could not turn Tesseract's equation detection during training!\n";
       exit(EXIT_FAILURE);
     }
  }

  inline void setDetectorSegmentor(tesseract::MEDS* detseg) {
    detector_segmentor = detseg;
  }


  // Here is where training of the detection component
  // is carried out. Compile-time polymorphism is used so that
  // different types of detection components can be easily
  // interchanged. If the train_detector argument is true, then
  // training will be carried out even if the type of classifier
  // being used has already been trained using the chosen
  // classifier/extractor/trainer/dataset combination. If it is false, then
  // training will only be carried out if the classifier hasn't been
  // trained on the chosen classifier/extractor/trainer/dataset combination
  void trainDetector() {
    if(detector_segmentor == NULL) {
      cout << "ERROR: trainDetector() called with a NULL MEDS module."
           << " setDetectorSegmentor needs to be called before training.\n";
      exit(EXIT_FAILURE);
    }
    // make sure the directories are there!
    if(!Basic_Utils::existsDirectory(predictor_path))
      Basic_Utils::exec((string)"mkdir " + predictor_path);
    if(!Basic_Utils::existsDirectory(training_set_path)) {
      cout << "ERROR: " << training_set_path << " is expected "
           << "to exist and contain the images that can be used "
           << "to train the detector if necessary. The directory does not exist!\n";
      exit(EXIT_FAILURE);
    }
    if(Basic_Utils::fileCount((string)training_set_path) == 0) {
      cout << "ERROR: " << training_set_path << " is expected "
           << "to contain training images but is empty!\n";
      exit(EXIT_FAILURE);
    }
    // assumes all n files in the training_ are images
    // named 1.png, 2.png, 3.png, .... n.png
    // do training only if necessary (i.e. if always_train is turned on
    // or it's not turned on and there is no trained predictor available
    // in the predictor_path)
    if(always_train || (!always_train &&
        Basic_Utils::fileCount(predictor_path) == 0)) {
      detector_segmentor->setTrainingMode(true); // tell MEDS module we're in training mode
      // tell the detector where it can find the groundtruth.dat file
      // so it can determine the label of each sample
      detector.initTrainingPaths(groundtruth_path, training_set_path, ext);

      // set the tesseract api's equation detector to the one being used
      api.setEquationDetector((tesseract::EquationDetectBase*)detector_segmentor);
      // count the number of training images in the training_set_path
      int img_num = Basic_Utils::fileCount(training_set_path);
      // api that will be used for recognition
      // i'm putting it on the stack here, in hopes that
      // this may help avoid memory allocation conflicts
      tesseract::TessBaseAPI newapi;
      detector_segmentor->setTessAPI(newapi);
      // assumes all n files in the training dir are images
      // named 1.png, 2.png, 3.png, .... n.png
      // iterate the images in the dataset, getting the features
      // from each and appending them to the samples vector

      // initialize any feature extraction parameters which need to do
      // precomputations on the entire training set prior to any feature
      // extraction. this may or may not be applicable depending on the
      // feature extraction implementation being used by the detector
      detector.initFeatExtFull(api);

      for(int i = 1; i <= img_num; i++) {
        string img_name = Basic_Utils::intToString(i) + ext;
        string img_filepath = training_set_path + img_name;
        Pix* curimg = Basic_Utils::leptReadImg(img_filepath);
        api.SetImage(curimg); // SetImage SHOULD deallocate everything from the last page
        // including my MEDS module, the BlobInfoGrid, etc!!!!
        api.AnalyseLayout(); // Run Tesseract's layout analysis
        tesseract::BlobInfoGrid* grid = detector_segmentor->getGrid();
#ifdef DBG_MEDS_TRAINER
        // debug: make sure it worked!!!! and we got the grid out of it...
        string winname = "BlobInfoGrid for Image " + Basic_Utils::intToString(i);
        ScrollView* gridviewer = grid->MakeWindow(100, 100, winname.c_str());
        grid->DisplayBoxes(gridviewer);
        Basic_Utils::waitForInput();
#endif
        // now to get the features from the grid and append them to the
        // samples vector.
        vector<BLSample*> img_samples = detector.getAllSamples(grid, i);
#ifdef DBG_MEDS_TRAINER
        // to debug I'll color all the blobs that are labeled as math
        // red and all the other ones as blue
        Pix* colorimg = Basic_Utils::leptReadImg(img_filepath);
        colorimg = pixConvertTo32(colorimg);
        Lept_Utils lu;
        for(int j = 0; j < img_samples.size(); j++) {
          BLSample* sample = img_samples[j];
          GroundTruthEntry* entry = sample->entry;
          if(sample->label)
            lu.fillBoxForeground(colorimg, sample->blobbox, LayoutEval::RED);
          else
            lu.fillBoxForeground(colorimg, sample->blobbox, LayoutEval::BLUE);
        }
        pixDisplay(colorimg, 100, 100);
        string dbgname = "dbg_training_im" + Basic_Utils::intToString(i) + ".png";
        pixWrite(dbgname.c_str(), colorimg, IFF_PNG);
        M_Utils mutils;
        mutils.waitForInput();
        pixDestroy(&colorimg);
        delete gridviewer;
        gridviewer = NULL;
#endif
        // now append the samples found for the current image to the
        // the vector which holds all of them. For now I have this organized
        // as a vector for each image
        samples.push_back(img_samples);
        pixDestroy(&curimg); // destroy finished image
        // clear the memory used by the current MEDS module
        detector_segmentor->reset();
      }
      // now initialize the training with the samples
      detector.initTraining(samples, predictor_path);
     // detector.train_();
    }
  }

  // Its much harder to do supervised training on this part. Much of the computations
  // at this stage will either be purely heuristic in nature or unsupervised for now.
  // This is kept as a place holder for the case where supervised learning may be
  // applicable in future work.
  void trainSegmentor() {

  }

 private:
  // Tesseract framework
  tesseract::TessBaseAPI api;

  // This is the MEDS module that is used within the Tesseract framework
  tesseract::MEDS* detector_segmentor; // overrides Tesseract's EquationDetectBase class

  // here different trainer_predictors in Detection/Detection.h can be chosen from
  // and experimented with through compile-time polymorphism
  TrainerPredictorType detector; // see Detection.h for details on this class

  vector<vector<BLSample*> > samples;

  // some paths and such
  string top_path; // this is the root of the detector directory
  string predictor_path;  // top_path/predictor
  string training_set_path; // top_path/training_set
  string groundtruth_path; // top_path/[groundtruthname].dat
  string ext; // image extension (i.e. png, jpg, etc.)
  bool always_train; // if false only train if no trained predictor
                     // is available in top_path/predictor
};


#endif

