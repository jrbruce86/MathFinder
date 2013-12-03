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

libSVM::libSVM() : trained(false), gamma_optimal(-1), C_optimal(-1) {

}

void libSVM::initClassifier() {

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
  // Now ready to find the optimal C and Gamma parameters through a course
  // grid search then through a finer one. Once the "optimal" C and Gamma
  // parameters are found, the SVM is trained on these to give the final
  // predictor which can be serialized and saved for later usage.
  doCrossValidationTraining(10);
  trained = true;
}

// Much of the functionality of this training is inspired by:
// [1] C.W. Hsu. "A practical guide to support vector classiﬁcation," Department of
// Computer Science, Tech. rep. National Taiwan University, 2003.
void libSVM::doCrossValidationTraining(int folds) {
  // 1. First a course grid search is carried out. As recommended in [1]
  // the grid is exponentially spaced to give an estimate of the general
  // magnitude of the parameters without taking too much time.
  // --------------------------------------------------------------------------
  // Vectors of 10 logarithmically spaced points are created for both the C and
  // Gamma parameters. The starting and ending values specified in [1] are used
  // for the C vector and Gamma vector (i.e. 2^-5,...,2^15 and 2^-15,....,2^3
  // respectively.
  matrix<double> C_vec = logspace(log10(pow(2,-5)), log10(pow(2,15)), 10);
  matrix<double> Gamma_vec = logspace(log10(pow(2,-15)), log10(pow(2,3)), 10);
  // The vectors are then combined to create a 10x10 (C,Gamma) pair grid, on which
  // cross validation training is carried out for each pair to find the optimal
  // starting parameters for a finer optimization which uses the BOBYQA algorithm.
  matrix<double> C_Gamma_Grid = cartesian_product(C_vec, Gamma_vec);
  //    Carry out course grid search. The grid is actually implemented as a
  //    2x100 matrix where row 1 is the C part of the grid pair and row 2
  //    is the corresponding gamma part. Each index represents a pair on the
  //    grid, just they are on two separate rows. and the column is the entire
  //    grid.
  matrix<double> best_result(2,1);
  best_result = 0;
  for(int i = 0; i < C_Gamma_Grid.nc(); i++) {
    // grab the current pair
    const double C = C_Gamma_Grid(0, i);
    const double gamma = C_Gamma_Grid(1, i);
    // set up C_SVC trainer using the current parameters
    svm_c_trainer<kernel_type> trainer;
    trainer.set_kernel(kernel_type(gamma));
    trainer.set_c(C);
    // do the cross validation
    matrix<double> result = cross_validate_trainer(trainer, training_samples, labels, folds);
    cout << "C: " << setw(11) << C << "  Gamma: " << setw(11) << gamma
         <<  "  cross validation accuracy (positive, negative): " << result;
    if(result(0) > .9) {
      if(sum(result) > sum(best_result))
        best_result = result;
    }

  }


}

bool libSVM::predict(const std::vector<double>& sample) {
  cout << "in the svm predictor!!\n";
  return false;
}

void libSVM::reset() {

}


