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
#include <dlib/svm.h>

typedef dlib::matrix<double, 0, 1> sample_type;
typedef dlib::radial_basis_kernel<sample_type> kernel_type;

class cross_validation_objective
{
    /*!
        WHAT THIS OBJECT REPRESENTS
            This object is a simple function object that takes a set of model
            parameters and returns a number indicating how "good" they are.  It
            does this by performing 10 fold cross validation on our dataset
            and reporting the accuracy.

            See below in main() for how this object gets used.
    !*/
public:

    cross_validation_objective (
        const std::vector<sample_type>& samples_,
        const std::vector<double>& labels_
    ) : samples(samples_), labels(labels_) {}

    double operator() (
        const dlib::matrix<double>& params
    ) const
    {
        // Pull out the two SVM model parameters.  Note that, in this case,
        // I have setup the parameter search to operate in log scale so we have
        // to remember to call exp() to put the parameters back into a normal scale.
        const double gamma = std::exp(params(0));
        const double nu    = std::exp(params(1));

        // Make an SVM trainer and tell it what the parameters are supposed to be.
        dlib::svm_c_trainer<kernel_type> trainer;
        trainer.set_kernel(kernel_type(gamma));
        //trainer.set_c(?);

        // Finally, perform 10-fold cross validation and then print and return the results.
        dlib::matrix<double> result = cross_validate_trainer(trainer, samples, labels, 10);
        std::cout << "gamma: " << std::setw(11) << gamma << "  nu: " << std::setw(11) << nu <<  "  cross validation accuracy: " << result;

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

};


class libSVM {
 public:
  libSVM();
  void initClassifier();
  void doTraining(const vector<vector<BLSample*> >& samples);
  bool predict(const std::vector<double>& sample);
  inline bool isTrained() { return trained; }
 private:
  bool trained; // flag that's true once training complete

  void doCrossValidationTraining();

  // the training samples and their corresponding labels
  // obviously these two vectors should be the same size
  std::vector<sample_type> training_samples;
  std::vector<double> labels;

  // the radial basis function (gaussian) kernel being used
  kernel_type rbf_kernel;
};

#endif

