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

libSVM::libSVM() : trained(false) {

}

void libSVM::initClassifier() {

}

void libSVM::doTraining(const vector<vector<BLSample*> >& samples) {


  doCrossValidationTraining();
  trained = true;
}

void libSVM::doCrossValidationTraining() {

}

bool libSVM::predict(const std::vector<double>& sample) {
  return false;
}


