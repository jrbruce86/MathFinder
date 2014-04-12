/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   TrainerPredictor.h
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Oct 14, 2013 6:53:21 PM
 * ------------------------------------------------------------------------
 * Description: Main header for the detection component of the MEDS module.
 *              The detection component includes the feature extraction,
 *              training, and binary classification used in order to detect
 *              mathematical symbols on a page. The emphasis of this component
 *              is to get as many true positives as possible while avoiding
 *              false positives to the greatest extent possible. It is considered
 *              better to have a false negative than a false positive in
 *              general at this stage, since it is much more difficult to correct the
 *              latter during segmentation.
 *
 *              This module is designed to cover both the training and 
 *              prediction functionality needed in order to run experiments
 *              on different classification/feature extraction/training
 *              combinations. Compile-time polymorphism is used in this 
 *              design such that the common requirements of all such 
 *              combinations are abstracted away. This makes it relatively
 *              easy to try different combinations for experimentation
 *              purposes. 
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
#ifndef TRAINER_PREDICTOR_H
#define TRAINER_PREDICTOR_H

#include <Sample.h>
#include <GTParser.h>
using namespace GT_Parser;
#include <ITrainer.h>
#include <IBinaryClassifier.h>
#include <IFeatureExtractor.h>

template <typename TrainerType,
typename BinClassType,
typename FeatExtType>
class Detector {
 public:
  // some shorthand to make things less messy
  typedef ITrainer<TrainerType, BinClassType> I_Trainer;
  typedef IBinaryClassifier<BinClassType> IClassifier;
  typedef IFeatureExtractor<FeatExtType> IFeatExt;

  Detector<TrainerType, BinClassType, FeatExtType>()
    : training_done(false), curimg(NULL), api(NULL), samples(NULL),
      feat_ext_init(false) {
    trainer = new I_Trainer(); // this constructs both the trainer and classifier
    featext = new IFeatExt();
    classifier = trainer->classifier; // classifier memory is managed by trainer interface
  }

  ~Detector<TrainerType, BinClassType, FeatExtType>() {
    if(featext != NULL) {
      cout << "in Detector destructor: deleting feature extractor\n";
      delete featext;
      featext = NULL;
    }
    if(trainer != NULL) {
      cout << "in Detector destructor: deleting trainer\n";
      delete trainer; // this also deletes the classifier object
      trainer = NULL;
    }
    // detector owns the samples
    cout << "in Detector destructor: deleting samples\n";
    destroySamples(samples);
    cout << "done deleting samples\n";
  }

  void destroySamples(vector<vector<BLSample*> >*& samples) {
    if(!samples)
      return;
    for(int i = 0; i < samples->size(); i++) {
      for(int j = 0; j < (*samples)[i].size(); j++) {
        BLSample* sample = (*samples)[i][j];
        if(sample != NULL) {
          delete sample;
          sample = NULL;
        }
      }
      (*samples)[i].clear();
    }
    samples->clear();
    delete samples;
    samples = NULL;
  }

  inline void initTrainingPaths(const string& groundtruth_path_,
      const string& training_set_path_, const string& ext_) {
    groundtruth_path = groundtruth_path_;
    training_set_path = training_set_path_;
    ext = ext_;
  }

  // Initialization required by both training and prediction
  inline void initFeatExtFull(TessBaseAPI* api, bool makenew,
      vector<string> api_init_params) {
    featext->initFeatExtFull(api, api_init_params, groundtruth_path,
        training_set_path, ext, makenew);
    feat_ext_init = true;
  }

  inline void initFeatExtSinglePage() {
    featext->initFeatExtSinglePage();
  }

  inline void setFeatExtPage(BlobInfoGrid* g, TessBaseAPI* a, PIX* i) {
    featext->setGrid(g);
    featext->setApi(a);
    featext->setImage(i);
  }

  // When doing training it is necessary to first get all the samples
  // up front. This method does feature extraction on all the blobs in
  // the given blobinfogrid and returns the feature vector (sample)
  // corresponding to each blob. This returns a vector of BLSamples
  // (Binary Labeled Samples) which includes both the features and the
  // binary label (true for math, false for non-math). The features are
  // stored inside a BLOBINFO object as a vector of doubles. What features
  // these doubles represent varies based upon what feature extractor
  // implementation is being utilized.
  inline vector<BLSample*> getAllSamples(tesseract::BlobInfoGrid* grid,
      int image_index) {
    featext->setImage(curimg);
    featext->setApi(api);
    featext->setGrid(grid);
    featext->setDBGDir(training_set_path + "../../");
    featext->initFeatExtSinglePage();
    grid = featext->extractAllFeatures(grid);
    tesseract::BlobInfoGridSearch bigs(grid);
    tesseract::M_Utils mutils;
    bigs.StartFullSearch();
    tesseract::BLOBINFO* blob = NULL;
    vector<BLSample*> samples;
    int blobnum = 0;
    while((blob = bigs.NextFullSearch()) != NULL) {
      if(!blob->features_extracted) {
        cout << "ERROR: Attempting to create a training sample from a blob "
             << "from which features haven't been extracted!>:-[\n";
        assert(false);
      }
      BLSample* lsample = new BLSample; // labeled sample
      lsample->features = blob->features;
      lsample->entry = getBlobGTEntry(blob, image_index, grid->getImg());
      TBOX tbox = blob->bounding_box();
      lsample->blobbox = mutils.tessTBoxToImBox(&tbox, grid->getImg());
      if(lsample->entry == NULL)
        lsample->label = false;
      else
        lsample->label = true;
      lsample->image_index = image_index;
      samples.push_back(lsample);
      blobnum++;
    }
    cout << "num blobs (samples) for image " << image_index << ": " << blobnum << endl;
    return samples;
  }

  // If the given blob in the given image is contained within any of the groundtruth
  // entry rectangles, then return a pointer to the entry it's contained in. Otherwise
  // just return NULL.
  GroundTruthEntry* getBlobGTEntry(tesseract::BLOBINFO* blob, int image_index,
      Pix* img) {
    // open the groundtruth file
    ifstream gtfile;
    string gtfilename = groundtruth_path;
    gtfile.open(gtfilename.c_str(), ifstream::in);
    if((gtfile.rdstate() & ifstream::failbit) != 0) {
      cout << "ERROR: Could not open Groundtruth.dat in " \
           << groundtruth_path << endl;
      assert(false);
    }
    int max = 55;
    char* curline = new char[max];
    bool found = false;
    GroundTruthEntry* entry = NULL;
    while(!gtfile.eof()) {
      gtfile.getline(curline, max);
      if(curline == NULL)
        continue;
      string curlinestr = (string)curline;
      assert(curlinestr.length() < max);
      entry = parseGTLine(curlinestr);
      if(entry == NULL)
        continue;
      if(entry->image_index == image_index) {
        // see if the entry's rectangle overlaps this blob
        tesseract::M_Utils m;
        Box* blob_bb = m.getBlobInfoBox(blob, img);
        int bb_intersects = 0; // this will be 1 if blob_bb intersects math
        boxIntersects(entry->rect, blob_bb, &bb_intersects);
        if(bb_intersects == 1) {
          // found a math blob!
          found = true;
        }
      }
      if(found)
        break;
      delete entry; // delete the entry when we're done with it
      entry = NULL;
    }
    delete [] curline;
    curline = NULL;
    gtfile.close();
    return entry;
  }

  inline void initClassifier(const string& predictor_path_,
      const string& sample_path_) {
    string feat_ext_name = featext->getFeatExtName();
    classifier->initClassifier(predictor_path_, sample_path_, feat_ext_name);
    predictor_path = classifier->getFullPredictorPath();
    sample_path = classifier->getFullSamplePath();
  }

  inline string getPredictorPath() {
    assert(predictor_path == classifier->getFullPredictorPath());
    return classifier->getFullPredictorPath();
  }

  inline string getSamplePath() {
    assert(sample_path == classifier->getFullSamplePath());
    return classifier->getFullSamplePath();
  }

  inline void initTraining(vector<vector<BLSample*> >* samples_) {
    samples = samples_;
    trainer->initTraining(classifier);
  }

  inline void setAlwaysTrain() {
    trainer->setAlwaysTrain(); // tell trainer to always retrain even if parameters can be loaded
  }

  inline void train_() {
    classifier = trainer->train_(*samples);
  }

  // prediction is just done on one page and will be using some
  // binary classifier which has already been trained with the
  // features specified for this type of trainer_predictor
  inline void initPrediction(vector<string> api_init_params) {
    featext->setDBGDir(training_set_path + "../");
    if(!feat_ext_init)
      initFeatExtFull(api, false, api_init_params); // initializes the feature extractor
    classifier->initPredictor(); // loads up the predictor, halts with an error if there is none.
  }

  inline bool predict(BLOBINFO* blob) {
    vector<double> sample = featext->extractFeatures(blob);
    blob->features = sample;
  //  if(blob->validword && !blob->ismathword) // this is cheating and also won't be correct in
                                               // many cases since valid words can be mathematical
                                               // >:-|... try and avoid this.
  //    return false;
    return classifier->predict(sample);
  }

  inline void setImage(PIX* im) {
    curimg = im;
  }

  inline void setAPI(TessBaseAPI* api_) {
    api = api_;
  }

  inline void reset() {
    classifier->reset();
    featext->reset();
    trainer->reset();
  }

  inline int numFeatures() {
    return featext->numFeatures();
  }

  inline string getTrainingPath() {
    return training_set_path;
  }

 private:
  // the training samples and their corresponding labels
  // the samples are separated into separate lists for
  // each sample image (i.e. samples[0] is the list of
  // samples for the first image, samples[1] for the
  // second, etc.)
  vector<vector<BLSample*> >* samples;

  bool training_done;
  string groundtruth_path; // path to the groundtruth file used to determine sample labels
                           // for training
  string training_set_path; // path to the set of training images being used
  string ext; // image extension
  string predictor_path; // the path where the trained classifier will be
                         // or is stored for later use in prediction
  string sample_path; // path where the samples used in training the classifier are stored

  I_Trainer* trainer;
  IFeatExt* featext;
  IClassifier* classifier;

  PIX* curimg; // the image on which detection is being carried out
  TessBaseAPI* api;

  bool feat_ext_init; // true after feature extractor has been initialized (initFeatExtFull has been called)
};

#endif
