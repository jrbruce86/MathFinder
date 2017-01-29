/*
 * TrainedSvmDetector.cpp
 *
 *  Created on: Oct 30, 2016
 *      Author: jake
 */

#include <SvmDetector.h>

#include <BlobDataGrid.h>
#include <BlobData.h>
#include <M_Utils.h>
#include <Sample.h>
#include <Utils.h>

#include <baseapi.h>
#include <scrollview.h>

// dlib includes
#include <dlib/svm_threaded.h>

// standard includes
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <assert.h>
#include <ios>

//#define SHOW_GRID

#define PROGRESS_TO_FILE
//#define RUNNING_BACKGROUND

// ***********
// Note see http://dlib.net/svm_ex.cpp.html
// Much of this is copied from that example.
// ************
TrainedSvmDetector::TrainedSvmDetector(
    const std::string& detectorDirPath) {
  std::string classifierName =
#ifdef RBF_KERNEL
      (std::string)"RBFSVM";
#endif
#ifdef LINEAR_KERNEL
      (std::string)"LinearSVM";
#endif
  predictorPath = std::string(Utils::checkTrailingSlash(detectorDirPath)) + classifierName + "Predictor";
  progressFilePath = std::string(Utils::checkTrailingSlash(detectorDirPath)) + classifierName + "Progress";
#ifdef RBF_KERNEL
#ifdef LINEAR_KERNEL
    cout << "ERROR: Can only train SVM with RBF or Linear kernel exclusively. "
         << "Both are enabled, need to disable one at the top of libSVM.h.\n";
    assert(false);
#endif
#endif
#ifndef LINEAR_KERNEL
#ifndef RBF_KERNEL
    cout << "ERROR: Need to enable either the LINEAR_KERNEL or the RBF_KERNEL "
         << "at the top of libSVM.h\n";
    assert(false);
#endif
#endif
}

void TrainedSvmDetector::detectMathExpressions(
    BlobDataGrid* const blobDataGrid) {

  // Start up the predictor
  loadPredictor();

  // Run the predictor on each blob
  BlobData* blob = NULL;
  BlobDataGridSearch bdgs(blobDataGrid);
  bdgs.StartFullSearch();
  while((blob = bdgs.NextFullSearch()) != NULL) {
    blob->setMathExpressionDetectionResult(predict(blob->getExtractedFeatures()));
  }

#ifdef SHOW_GRID
  std::cout << "Done running predictions for image " << blobDataGrid->getImageName() << std::endl;
  std::string winname = "BlobInfoGrid for Image " + blobDataGrid->getImageName();
  ScrollView* gridviewer = blobDataGrid->MakeWindow(100, 100, winname.c_str());
  blobDataGrid->DisplayBoxes(gridviewer);
  M_Utils::waitForInput();
  delete gridviewer;
  gridviewer = NULL;
#endif
}

std::string TrainedSvmDetector::getDetectorPath() {
  return predictorPath;
}

bool TrainedSvmDetector::doTraining(const std::vector<std::vector<BLSample*> >& samples) {

#ifdef PROGRESS_TO_FILE
    progressFile.open(progressFilePath.c_str());
    if(!progressFile.is_open()) {
      std::cout << "ERROR: Failed to open " << progressFilePath << std::endl <<
          "Training failed.\n";
      // I wouldn't want to run this thing unless I get progress (it can take hours)
      // This is for if I'm on a remote server and the pipe goes down. I'd want this
      // as a background process so I can just leave the server and let it do it's thing
      // and come back to check on the file later to see the progress.
      return false;
    }
#endif

  // Convert the samples into format suitable for DLib
  std::cout << "started libSVM's doTraining\n";
  int num_features = samples[0][0]->features.size();
  for(int i = 0; i < samples.size(); ++i) { // iterates through the images
    for(int j = 0; j < samples[i].size(); ++j) { // iterates the samples in the image
      BLSample* const s = samples[i][j];
      // add the sample (the feature vector)
      std::vector<DoubleFeature*> features = s->features;
      assert(features.size() == num_features);
      sample_type sample;
      sample.set_size(num_features, 1);
      for(int k = 0; k < num_features; ++k) {
        sample(k) = features[k]->getFeature();
      }
      training_samples.push_back(sample);
      // add the corresponding label
      if(s->label)
        labels.push_back(+1);
      else
        labels.push_back(-1);
    }
  }
  outputProgress("done pushing back samples\n");

  // Randomize the order of the samples so that they do not appear to be
  // from different distributions during cross validation training. The
  // samples are grouped by the image from which they came and each image
  // can, in some regards, be seen as a separate distribution.
  // *** see http://dlib.net/svm_ex.cpp.html for a better explanation.
  // *** basically need to randomize the ordering of the samples to avoid
  // *** screwing up cross validation
  randomize_samples(training_samples, labels);
  outputProgress("done randomizing samples\n");

  // Here we normalize all the samples by subtracting their mean and dividing by their
  // standard deviation.  This is generally a good idea since it often heads off
  // numerical stability problems and also prevents one large feature from smothering
  // others.  Doing this doesn't matter much in this example so I'm just doing this here
  // so you can see an easy way to accomplish this with the library.
  // let the normalizer learn the mean and standard deviation of the samples
  normalizer.train(training_samples);
  outputProgress("done setting up normalizing\n");

  // now normalize each sample
  for (unsigned long i = 0; i < training_samples.size(); ++i)
    training_samples[i] = normalizer(training_samples[i]);
  outputProgress("done normalizing each sample\n");

  // Now ready to find the optimal C and Gamma parameters through a coarse
  // grid search then through a finer one. Once the "optimal" C and Gamma
  // parameters are found, the SVM is trained on these to give the final
  // predictor which can be serialized and saved for later usage.
  bool doParamCalc = true;
  if(loadOptParams()) {
#ifdef RUNNING_BACKGROUND
    doParamCalc = true;
#endif
#ifndef RUNNING_BACKGROUND
    std::cout << "Training previously carried out resulted in the following optimal "
        << "parameters being found: C->" << C_optimal << ", gamma->" << gamma_optimal
        << ". Would you like to recompute these paramaters? If you answer yes, then "
        << "the parameters will be recomputed with all training starting from scratch. "
        << "If you answer no then the parameters shown above will be re-used and the part "
        << "of training which calculates them will be skipped. ";
    doParamCalc = Utils::promptYesNo();
#endif
  }
  if(doParamCalc) {
    doCoarseCVTraining(10);
    doFineCVTraining(10);
    saveOptParams();
  }
  outputProgress("about to do training\n");
  trainFinalClassifier();
  outputProgress("done with trainFinalClassifier\n");
  savePredictor();
  outputProgress("done with savePredictor\n");
  outputProgress(std::string("Training Complete! The predictor has been saved to ")
       + predictorPath + std::string("\n"));
  return true;
}

// Much of the functionality of this training is inspired by:
// [1] C.W. Hsu. "A practical guide to support vector classiﬁcation," Department of
// Computer Science, Tech. rep. National Taiwan University, 2003.
void TrainedSvmDetector::doCoarseCVTraining(int folds) {
  // 1. First a course grid search is carried out. As recommended in [1]
  // the grid is exponentially spaced to give an estimate of the general
  // magnitude of the parameters without taking too much time.
  // --------------------------------------------------------------------------
  // Vectors of 10 logarithmically spaced points are created for both the C and
  // Gamma parameters. The starting and ending values were chosen to cover a wide
  // range. But since it was noticed that numerical difficulties are experienced for
  // values over 1000 they are capped to that (numerical difficulties being that it
  // was taking hours to do a single fold of cross validation...).
  dlib::matrix<double> C_vec = dlib::logspace(log10(1e-3), log10(1000), 10);
#ifdef RBF_KERNEL
  outputProgress("Started coarse training for RBF Kernel.\n");
  dlib::matrix<double> Gamma_vec = dlib::logspace(log10(1e-7), log10(1000), 10);
#endif
  // The vectors are then combined to create a 10x10 (C,Gamma) pair grid, on which
  // cross validation training is carried out for each pair to find the optimal
  // starting parameters for a finer optimization which uses the BOBYQA algorithm.
  dlib::matrix<double> grid;
#ifdef RBF_KERNEL
  grid = cartesian_product(C_vec, Gamma_vec);
#endif
#ifdef LINEAR_KERNEL
  std::cout << "Started coarse training for linear kernel\n";
  grid = C_vec;
#endif

  outputProgress(std::string("Predictor Path: ") +
      predictorPath + std::string("\n"));

  //    Carry out course grid search. The grid is actually implemented as a
  //    2x100 matrix where row 1 is the C part of the grid pair and row 2
  //    is the corresponding gamma part. Each index represents a pair on the
  //    grid, just they are on two separate rows. and the column is the entire
  //    grid.
  dlib::matrix<double> best_result(2,1);
  best_result = 0;
  for(int i = 0; i < grid.nc(); i++) {
    // grab the current pair
    const double C = grid(0, i);
#ifdef RBF_KERNEL
    const double gamma = grid(1, i);
#endif
    // set up C_SVC trainer using the current parameters

#ifdef RBF_KERNEL
    dlib::svm_c_trainer<RBFKernel> trainer;
    trainer.set_kernel(RBFKernel(gamma));
#endif
#ifdef LINEAR_KERNEL
    svm_c_trainer<LinearKernel> trainer;
#endif
    trainer.set_c(C);
    // do the cross validation
    outputProgress(std::string("Running cross validation for ") +
        std::string("C: ") + Utils::doubleToString(C, 11) +
#ifdef RBF_KERNEL
         std::string("  Gamma: ") + Utils::doubleToString(gamma, 11) +
         std::string("\n"));
#endif
#ifdef LINEAR_KERNEL
         std::string("\n"));
#endif
    dlib::matrix<double> result = cross_validate_trainer_threaded(trainer, training_samples,
        labels, folds, folds); // last arg is the number of threads (using same as folds)
    outputProgress(std::string("C: ") +  Utils::doubleToString(C, 11) +
#ifdef RBF_KERNEL
         std::string("  Gamma: ") + Utils::doubleToString(gamma, 11) +
#endif
         std::string("  cross validation accuracy (positive, negative): ") +
         Utils::doubleToString(result(0,0)) + std::string(", ") +
         Utils::doubleToString(result(0,1)) + std::string("\n"));
    if(sum(result) > sum(best_result)) {
      best_result = result;
#ifdef RBF_KERNEL
      gamma_optimal = gamma;
#endif
      C_optimal = C;
    }
  }
#ifdef RBF_KERNEL
  outputProgress(std::string("Best Result: (positive, negative)") +
      Utils::doubleToString(best_result(0,0)) + std::string(", ") +
      Utils::doubleToString(best_result(0,1)) + std::string("\n") +
      std::string(". Gamma = ") + Utils::doubleToString(gamma_optimal, 11) +
       std::string(". C = ") + Utils::doubleToString(C_optimal, 11) + std::string("\n"));
#endif
#ifdef LINEAR_KERNEL
  std::cout << "Best Result: " << best_result
       << ", C = " << std::setw(11) << C_optimal << std::endl;
#endif
}

// assumes that C_optimal and gamma_optimal have already
// been initialized either manually or through doCoarseCVTraining()
// this carries out BOBYQA algorithm to find optimal C and Gamma parameters
void TrainedSvmDetector::doFineCVTraining(int folds) {
  // set the starting point
#ifdef RBF_KERNEL
  dlib::matrix<double, 2, 1> opt_params;
#endif
#ifdef LINEAR_KERNEL
  dlib::matrix<double, 2, 1> opt_params;
#endif
  opt_params(0) = C_optimal;
#ifdef RBF_KERNEL
  opt_params(1) = gamma_optimal;
#endif
#ifdef LINEAR_KERNEL
  opt_params(1) = 0; //fixed
#endif

  // set the upper and lower limits
#ifdef RBF_KERNEL
  dlib::matrix<double, 2, 1> lowerbound, upperbound;
  lowerbound = 1e-7, 1e-7;
  upperbound = 1000, 1000;
#endif
#ifdef LINEAR_KERNEL
  dlib::matrix<double, 2, 1> lowerbound, upperbound;
  lowerbound = 1e-7, 0;
  upperbound = 1000, 0;
#endif

  // try searching in log space like in the dlib example
  opt_params = dlib::log(opt_params);
  lowerbound = dlib::log(lowerbound);
  upperbound = dlib::log(upperbound);

  double best_score = dlib::find_max_bobyqa(
      cross_validation_objective(training_samples, labels, folds, &progressFile), // Function to maximize
      opt_params,                                      // starting point
      opt_params.size()*2 + 1,                         // See BOBYQA docs, generally size*2+1 is a good setting for this
      lowerbound,                                 // lower bound
      upperbound,                                 // upper bound
      min(upperbound-lowerbound)/10,             // search radius
      0.01,                                        // desired accuracy
      100                                          // max number of allowable calls to cross_validation_objective()
  );
  opt_params = exp(opt_params); // convert back to normal scale from log scale
  C_optimal = opt_params(0);
#ifdef RBF_KERNEL
  gamma_optimal = opt_params(1);
#endif
  outputProgress(std::string("Optimal C after BOBYQA: ") +
      Utils::doubleToString(C_optimal, 11) +
      std::string("\n"));
#ifdef RBF_KERNEL
  outputProgress(std::string("Optimal gamma after BOBYQA: ") +
      Utils::doubleToString(gamma_optimal, 11) +
      std::string("\n"));
#endif
  outputProgress(std::string("BOBYQA Score: ") +
      Utils::doubleToString(best_score) + std::string("\n"));

  // Get and display the true positive and negative rate of best result (have to do another round of
  // cross validation since bobyqa algorithm has no easy way of storing this
  // that I know of...
  outputProgress(std::string("Running cross validation again on optimal c and gamma") +
      std::string(" to get the true positive and true negative rate (should sum") +
      std::string(" up to the score above.\n"));
  dlib::svm_c_trainer<RBFKernel> trainer;
  trainer.set_kernel(RBFKernel(gamma_optimal));
  trainer.set_c(C_optimal);
  dlib::matrix<double> result = cross_validate_trainer_threaded(trainer, training_samples,
      labels, folds, folds); // last arg is the number of threads (using same as folds)
  outputProgress(std::string("C: ") +  Utils::doubleToString(C_optimal, 11) +
       std::string("  Gamma: ") + Utils::doubleToString(gamma_optimal, 11) +
       std::string("  cross validation accuracy (positive, negative): ") +
       Utils::doubleToString(result(0,0)) + std::string(", ") +
       Utils::doubleToString(result(0,1)) + std::string("\n"));
}

void TrainedSvmDetector::saveOptParams() {
  std::string param_path = predictorPath + (std::string)"_params";
  std::ofstream s(param_path.c_str());
  s << C_optimal << std::endl;
#ifdef RBF_KERNEL
  s << gamma_optimal << std::endl;
#endif
}

bool TrainedSvmDetector::loadOptParams() {
  std::string param_path = predictorPath + (std::string)"_params";
  std::ifstream s(param_path.c_str());
  if(!s.is_open())
    return false;
  int maxlen = 55;
  char ln[maxlen];
  std::vector<double> params;
  while(!s.eof()) {
    s.getline(ln, maxlen);
    if(!s)
      continue;
    double p = atof(ln);
    params.push_back(p);
  }
#ifdef RBF_KERNEL
  if(params.size() > 2 || params.size() == 0) {
    std::cout << "ERROR: Wrong number of parameters loaded in for RBF Kernel.\n";
    assert(false);
  }
  C_optimal = params[0];
  gamma_optimal = params[1];
#endif
#ifdef LINEAR_KERNEL
  if(params.size() > 1 || params.size() == 0) {
    std::cout << "ERROR: Wrong number of parameters loaded in for Linear Kernel.\n";
    assert(false);
  }
  C_optimal = params[0];
#endif
  return true;
}

void TrainedSvmDetector::trainFinalClassifier() {
#ifdef RBF_KERNEL
  dlib::svm_c_trainer<RBFKernel> trainer;
  trainer.set_kernel(RBFKernel(gamma_optimal));
#endif
#ifdef LINEAR_KERNEL
  dlib::svm_c_trainer<LinearKernel> trainer;
#endif
  trainer.set_c(C_optimal);
  outputProgress("setting normalizer\n");
  final_predictor.normalizer = normalizer;
  outputProgress("calling trainer.train()\n");
  final_predictor.function = trainer.train(training_samples, labels);
  outputProgress(std::string("The number of support vectors in the final learned function is: ") +
      Utils::intToString(final_predictor.function.basis_vectors.size()) +
      std::string("\n"));
}

void TrainedSvmDetector::savePredictor() {
  std::ofstream fout(predictorPath.c_str(), std::ios::binary);
  serialize(final_predictor, fout);
  fout.close();
}

void TrainedSvmDetector::loadPredictor() {
  std::ifstream fin(predictorPath.c_str(), std::ios::binary);
  if(!fin.is_open()) {
    std::cout << "ERROR: Could not open the predictor at " << predictorPath << std::endl;
    assert(false);
  }
  deserialize(final_predictor, fin);
  std::cout << "Predictor at " << predictorPath << " was successfully loaded!\n";
}

bool TrainedSvmDetector::predict(const std::vector<DoubleFeature*>& sample) {
  sample_type sample_;
  sample_.set_size(sample.size(), 1);
  for(int i = 0; i < sample.size(); ++i)
    sample_(i) = sample[i]->getFeature();
  double result = final_predictor(sample_);
  if(result < 0)
    return false;
  else
    return true;
}

void TrainedSvmDetector::outputProgress(std::string progressStr) {

  std::cout << progressStr << std::endl;

#ifdef PROGRESS_TO_FILE
  progressFile << progressStr << std::endl;
  progressFile.flush();
#endif
}
