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
 *              false positives to the greatest extent possible. It is here considered
 *              better to have a false negative than a false positive in
 *              general at this stage, since it is much easier to correct the
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
#ifndef DETECTION_H
#define DETECTION_H

#include "Detector.h"

// Detector1 is the first Detector implementation.
// TODO Add more details!
typedef Detector<CrossValidatorSVM<libSVM>, libSVM, F_Ext1> Detector1;

#endif

