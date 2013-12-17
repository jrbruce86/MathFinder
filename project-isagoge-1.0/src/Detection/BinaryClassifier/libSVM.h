/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   CrossValidation.cpp
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Oct 25, 2013 10:50:29 AM
 * ------------------------------------------------------------------------
 * Description: DLib's implementation of Chu's soft margin SVM as described
 *              in Chih-Chung Chang and Chih-Jen Lin, LIBSVM : a library for support
 *              vector machines, 2001. Software available at
 *              http://www.csie.ntu.edu.tw/~cjlin/libsvm. Uses RBF kernel
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
#ifndef LIBSVM_H
#define LIBSVM_H

#include <Sample.h>
#include <Basic_Utils.h>
#include <dlib/svm_threaded.h>

// only one of the following should be enabled!
// the chosen kernel is used for training
#define RBF_KERNEL
//#define LINEAR_KERNEL

typedef dlib::matrix<double, 0, 1> sample_type;

// RBF SVM Typedefs
typedef dlib::radial_basis_kernel<sample_type> RBFKernel;
typedef dlib::decision_function<RBFKernel> RBFSVMPredictor;
typedef dlib::normalized_function<RBFSVMPredictor> RBFSVMNormalizedPredictor;

// Linear SVM Typedefs
typedef dlib::linear_kernel<sample_type> LinearKernel;
typedef dlib::decision_function<LinearKernel> LinearSVMPredictor;
typedef dlib::normalized_function<LinearSVMPredictor> LinearSVMNormalizedPredictor;

// Copied from dlib's model_selection_ex.cpp with the following modifications:
// - Divides the data into a variable number of subsets for cross validation.
// - Uses C-SVM rather than Nu-SVM
// - Uses multiple threads (one for each fold of cross validation)
// - TODO: Modify cross validation return value to be maximized.
class cross_validation_objective
{
 public:
  cross_validation_objective (const std::vector<sample_type>& samples_,
      const std::vector<double>& labels_, int folds_) :
        samples(samples_), labels(labels_), folds(folds_) {}

  double operator() (const dlib::matrix<double>& params) const {
    // Pull out the two SVM model parameters.  Note that, in this case,
    // I have setup the parameter search to operate in log scale so we have
    // to remember to call exp() to put the parameters back into a normal scale.
    const double C = std::exp(params(0));
#ifdef RBF_KERNEL
    const double gamma    = std::exp(params(1));

    // Make an SVM trainer and tell it what the parameters are supposed to be.
    dlib::svm_c_trainer<RBFKernel> trainer;
    trainer.set_kernel(RBFKernel(gamma));
#endif
#ifdef LINEAR_KERNEL
    dlib::svm_c_trainer<LinearKernel> trainer;
#endif
    trainer.set_c(C);

    // Finally, perform 10-fold cross validation and then print and return the results.
#ifdef RBF_KERNEL
    cout << "Running cross validation on RBF Kernel SVM with ";
    cout << "C: " << setw(11) << C << ", gamma: " << setw(11) << gamma << endl;
#endif
#ifdef LINEAR_KERNEL
    cout << "Running cross validation on Linear Kernel SVM with ";
    cout << "C: " << setw(11) << C << endl;
#endif
    dlib::matrix<double> result = dlib::cross_validate_trainer_threaded(trainer, samples, labels, folds, folds);
#ifdef RBF_KERNEL
    std::cout << "C: " << std::setw(11) << C << "  gamma: " << std::setw(11) << gamma
              <<  "  cross validation accuracy: " << result;
#endif
#ifdef LINEAR_KERNEL
    std::cout << "C: " << std::setw(11) << C
              << "   cross validation accuracy:  " << result;
#endif


    // Here I'm just summing the accuracy on each class.  However, you could do something else.
    // For example, your application might require a 90% accuracy on class +1 and so you could
    // heavily penalize results that didn't obtain the desired accuracy.  Or similarly, you
    // might use the roc_c1_trainer() function to adjust the trainer output so that it always
    // obtained roughly a 90% accuracy on class +1.  In that case returning the sum of the two
    // class accuracies might be appropriate.
    return sum(result);
  }

  const std::vector<sample_type>& samples;
  const std::vector<double>& labels;
  int folds;
};


class libSVM {
 public:
  libSVM();
  ~libSVM();
  void initClassifier(const string& predictor_path_,
      const string& sample_path_, const string& featext_name);
  void initPredictor();
  void doTraining(const vector<vector<BLSample*> >& samples);
  bool predict(const std::vector<double>& sample);
  inline bool isTrained() { return trained; }
  void reset();
  inline string getFullPredictorPath() {
    return predictor_path;
  }
  inline string getFullSamplePath() {
    return sample_path;
  }
  inline string getFeatExtName() {
    return feat_ext_name;
  }
 private:
  bool trained; // flag that's true once training complete

  void doCoarseCVTraining(int folds); // coarse grid search to find starting params for doFineCVTraining
  void doFineCVTraining(int folds); // uses BOBYQA to get final optimized params

  void saveOptParams(); // save optimal parameters for later use
  bool loadOptParams(); // load optimal paramaters for given predictor if they don't exist
                        // returns false

  void trainFinalClassifier();

  void savePredictor(); // serialize and save the predictor for later use
  void loadPredictor(); // read in a previously serialized predictor

  // the training samples and their corresponding labels
  // obviously these two vectors should be the same size
  std::vector<sample_type> training_samples;
  std::vector<double> labels;

  dlib::vector_normalizer<sample_type> normalizer;

  // the optimal gamma and C parameters for the SVM
#ifdef RBF_KERNEL
  double gamma_optimal;
#endif
  double C_optimal;

  // the final trained classifier which was trained with the
  // optimal gamma and C parameters
#ifdef RBF_KERNEL
  RBFSVMNormalizedPredictor final_predictor;
#endif
#ifdef LINEAR_KERNEL
  LinearSVMNormalizedPredictor final_predictor;
#endif


  string predictor_path; // path to save the final predictor in
  string sample_path; // path to save the samples for training (so feature extraction
                      // doesn't need to be re-run every time training occurs)

  string feat_ext_name;

  bool predictor_loaded; // true only after loading the predictor
  bool init_done; // true after initialization successfully completed
};

#endif

