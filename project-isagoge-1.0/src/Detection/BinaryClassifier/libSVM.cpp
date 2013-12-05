/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   CrossValidation.cpp
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Oct 25, 2013 10:51:16 AM
 * ------------------------------------------------------------------------
 * Description: DLib's implementation of Chu's soft margin SVM as described
 *              in Chih-Chung Chang and Chih-Jen Lin, LIBSVM : a library for support
 *              vector machines, 2001. Software available at
 *              http://www.csie.ntu.edu.tw/~cjlin/libsvm. Uses the RBF kernel
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

#include <libSVM.h>

using namespace dlib;

libSVM::libSVM() : trained(false), C_optimal(-1),
    predictor_loaded(false) {
#ifdef RBF_KERNEL
  gamma_optimal = -1;
#endif
}

void libSVM::initClassifier(const string& predictor_path_, bool prediction) {
  predictor_path = predictor_path_ + (string)"predictor";
  if(prediction)
    loadPredictor();
#ifdef RBF_KERNEL
#ifdef LINEAR_KERNEL
    cout << "ERROR: Can only train SVM with RBF or Linear kernel exclusively. "
         << "Both are enabled, need to disable one at the top of libSVM.h.\n";
    exit(EXIT_FAILURE);
#endif
#endif
#ifndef LINEAR_KERNEL
#ifndef RBF_KERNEL
    cout << "ERROR: Need to enable either the LINEAR_KERNEL or the RBF_KERNEL "
         << "at the top of libSVM.h\n";
    exit(EXIT_FAILURE);
#endif
#endif
}

void libSVM::doTraining(const std::vector<std::vector<BLSample*> >& samples) {
  // Convert the samples into format suitable for DLib
  int num_features = samples[0][0]->features.size();
  for(int i = 0; i < samples.size(); ++i) { // iterates through the images
    for(int j = 0; j < samples[i].size(); ++j) { // iterates the blobs in the image
      BLSample* s = samples[i][j];
      // add the sample (the feature vector)
      std::vector<double> features = s->features;
      assert(features.size() == num_features);
      sample_type sample;
      sample.set_size(num_features, 1);
      for(int k = 0; k < num_features; k++)
        sample(k) = features[k];
      training_samples.push_back(sample);
      // add the corresponding label
      if(s->label)
        labels.push_back(+1);
      else
        labels.push_back(-1);
    }
  }
  // Randomize the order of the samples so that they do not appear to be
  // from different distributions during cross validation training. The
  // samples are grouped by the image from which they came and each image
  // can, in some regards, be seen as a separate distribution.
  randomize_samples(training_samples, labels);

  // Here we normalize all the samples by subtracting their mean and dividing by their
  // standard deviation.  This is generally a good idea since it often heads off
  // numerical stability problems and also prevents one large feature from smothering
  // others.  Doing this doesn't matter much in this example so I'm just doing this here
  // so you can see an easy way to accomplish this with the library.
  vector_normalizer<sample_type> normalizer;
  // let the normalizer learn the mean and standard deviation of the samples
  normalizer.train(training_samples);
  // now normalize each sample
  for (unsigned long i = 0; i < training_samples.size(); ++i)
    training_samples[i] = normalizer(training_samples[i]);

  // Now ready to find the optimal C and Gamma parameters through a coarse
  // grid search then through a finer one. Once the "optimal" C and Gamma
  // parameters are found, the SVM is trained on these to give the final
  // predictor which can be serialized and saved for later usage.
  doCoarseCVTraining(10);
  doFineCVTraining(10);
  trainFinalClassifier();
  savePredictor();
  trained = true;
  cout << "Training Complete! The predictor has been saved to "
       << predictor_path << endl;
}

// Much of the functionality of this training is inspired by:
// [1] C.W. Hsu. "A practical guide to support vector classiﬁcation," Department of
// Computer Science, Tech. rep. National Taiwan University, 2003.
void libSVM::doCoarseCVTraining(int folds) {
  // 1. First a course grid search is carried out. As recommended in [1]
  // the grid is exponentially spaced to give an estimate of the general
  // magnitude of the parameters without taking too much time.
  // --------------------------------------------------------------------------
  // Vectors of 10 logarithmically spaced points are created for both the C and
  // Gamma parameters. The starting and ending values specified in [1] are used
  // for the C vector and Gamma vector (i.e. 2^-5,...,2^15 and 2^-15,....,2^3
  // respectively.
  matrix<double> C_vec = logspace(log10(pow(2,-5)), log10(pow(2,15)), 10);
#ifdef RBF_KERNEL
  matrix<double> Gamma_vec = logspace(log10(pow(2,-5)), log10(pow(2,3)), 10);
#endif
  // The vectors are then combined to create a 10x10 (C,Gamma) pair grid, on which
  // cross validation training is carried out for each pair to find the optimal
  // starting parameters for a finer optimization which uses the BOBYQA algorithm.
  matrix<double> grid;
#ifdef RBF_KERNEL
  grid = cartesian_product(C_vec, Gamma_vec);
#endif
#ifdef LINEAR_KERNEL

#endif
  grid = C_vec;

  //    Carry out course grid search. The grid is actually implemented as a
  //    2x100 matrix where row 1 is the C part of the grid pair and row 2
  //    is the corresponding gamma part. Each index represents a pair on the
  //    grid, just they are on two separate rows. and the column is the entire
  //    grid.
  matrix<double> best_result(2,1);
  best_result = 0;
  for(int i = 0; i < grid.nc(); i++) {
    // grab the current pair
    const double C = grid(0, i);
#ifdef RBF_KERNEL
    const double gamma = grid(1, i);
#endif
    // set up C_SVC trainer using the current parameters

#ifdef RBF_KERNEL
    svm_c_trainer<RBFKernel> trainer;
    trainer.set_kernel(RBFKernel(gamma));
#endif
#ifdef LINEAR_KERNEL
    svm_c_trainer<LinearKernel> trainer;
#endif
    trainer.set_c(C);
    // do the cross validation
    cout << "Running cross validation for " << "C: " << setw(11) << C
#ifdef RBF_KERNEL
         << "  Gamma: " << setw(11) << gamma << endl;
#endif
#ifdef LINEAR_KERNEL
         << endl;
#endif
    matrix<double> result = cross_validate_trainer_threaded(trainer, training_samples,
        labels, folds, folds);
    cout << "C: " << setw(11) << C << "  Gamma: " << setw(11) << gamma
         <<  "  cross validation accuracy (positive, negative): " << result;
    if(sum(result) > sum(best_result)) {
      best_result = result;
#ifdef RBF_KERNEL
      gamma_optimal = gamma;
#endif
      C_optimal = C;
    }
  }
#ifdef RBF_KERNEL
  cout << "Best Result: " << best_result << ". Gamma = " << setw(11) << gamma_optimal
       << ". C = " << setw(11) << C_optimal << endl;
#endif
#ifdef LINEAR_KERNEL
  cout << "Best Result: " << best_result
       << ", C = " << setw(11) << C_optimal << endl;
#endif
}

// assumes that C_optimal and gamma_optimal have already
// been initialized either manually or through doCoarseCVTraining()
// this carries out BOBYQA algorithm to find optimal C and Gamma parameters
void libSVM::doFineCVTraining(int folds) {
  // set the starting point
#ifdef RBF_KERNEL
  matrix<double, 2, 1> opt_params;
#endif
#ifdef LINEAR_KERNEL
  matrix<double, 2, 1> opt_params;
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
  matrix<double, 2, 1> lowerbound, upperbound;
  lowerbound = 1e-7, 1e-7;
  upperbound = 1000, 1000;
#endif
#ifdef LINEAR_KERNEL
  matrix<double, 2, 1> lowerbound, upperbound;
  lowerbound = 1e-7, 0;
  upperbound = 1000, 0;
#endif

  // try searching in log space like in the dlib example
  opt_params = log(opt_params);
  lowerbound = log(lowerbound);
  upperbound = log(upperbound);

  double best_score = find_max_bobyqa(
      cross_validation_objective(training_samples, labels, folds), // Function to maximize
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
  cout << "Optimal C after BOBYQA: " << setw(11) << C_optimal << endl;
#ifdef RBF_KERNEL
  cout << "Optimal gamma after BOBYQA: " << setw(11) << gamma_optimal << endl;
#endif
  cout << "BOBYQA Score: " << best_score << endl;
}

void libSVM::trainFinalClassifier() {
#ifdef RBF_KERNEL
  svm_c_trainer<RBFKernel> trainer;
  trainer.set_kernel(RBFKernel(gamma_optimal));
#endif
#ifdef LINEAR_KERNEL
  svm_c_trainer<LinearKernel> trainer;
#endif
  trainer.set_c(C_optimal);
  final_predictor = trainer.train(training_samples, labels);
  cout << "The number of support vectors in the final learned function is: "
       << final_predictor.basis_vectors.size() << endl;
}

void libSVM::savePredictor() {
  ofstream fout(predictor_path.c_str(),ios::binary);
  serialize(final_predictor,fout);
  fout.close();
}

void libSVM::loadPredictor() {
  ifstream fin(predictor_path.c_str(),ios::binary);
  deserialize(final_predictor, fin);
  predictor_loaded = true;
}

bool libSVM::predict(const std::vector<double>& sample) {
  sample_type sample_;
  sample_.set_size(sample.size(), 1);
  for(int i = 0; i < sample.size(); ++i)
    sample_(i) = sample[i];
  double result = final_predictor(sample_);
  if(result < 0)
    return false;
  else
    return true;
}

void libSVM::reset() {

}


