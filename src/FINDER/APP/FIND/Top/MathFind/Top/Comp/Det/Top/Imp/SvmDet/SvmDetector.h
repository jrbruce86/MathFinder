/*
 * TrainedSvmDetector.h
 *
 *  Created on: Oct 30, 2016
 *      Author: jake
 */

#ifndef TRAINEDSVMDETECTOR_H_
#define TRAINEDSVMDETECTOR_H_

#include <Detector.h>
#include <BlobDataGrid.h>

#include <dlib/svm_threaded.h>

#include <iostream>
#include <vector>
#include <string>

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
class cross_validation_objective {
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
    std::cout << "Running cross validation on RBF Kernel SVM with ";
    std::cout << "C: " << std::setw(11) << C << ", gamma: " << std::setw(11) << gamma << std::endl;
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

class TrainedSvmDetector : virtual public MathExpressionDetector {
 public:

  TrainedSvmDetector(const std::string&);

  /**
   * See base class for docs
   */
  void detectMathExpressions(
      BlobDataGrid* const featureExtractionOutput);

  std::string getDetectorPath();

  bool doTraining(const std::vector<std::vector<BLSample*> >& samples);

 private:

  void doCoarseCVTraining(int folds); // coarse grid search to find starting params for doFineCVTraining
  void doFineCVTraining(int folds); // uses BOBYQA to get final optimized params

  void saveOptParams(); // save optimal parameters for later use
  bool loadOptParams(); // load optimal paramaters for given predictor if they don't exist
                        // returns false

  void trainFinalClassifier();

  void savePredictor(); // serialize and save the predictor for later use
  void loadPredictor(); // read in a previously serialized predictor

  bool predict(const std::vector<DoubleFeature*>& sample);

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

  std::string predictorPath;

  std::string progressFilePath;
  std::ofstream progressFile;
  void outputProgress(std::string progressStr);
};


#endif /* TRAINEDSVMDETECTOR_H_ */
