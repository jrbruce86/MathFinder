/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   Detection.h
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Oct 14, 2013 7:36:22 PM
 * ------------------------------------------------------------------------
 * Description: Main header for the detection component of the MEDS module.
 *              The detection component includes the feature extraction,
 *              training, and binary classification used in order to detect
 *              mathematical symbols on a page. The emphasis of this component
 *              is to get as many true positives as possible while avoiding
 *              false positives to the greatest extent possible. It is considered
 *              better to have a false negative than a false positive in
 *              general at this stage, since it is much easier to correct the
 *              latter during segmentation.
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

// includes for all FeatureExtractor implementations
#include <IFeatureExtractor.h>

// includes for all BinaryClassification implementations
#include <IBinaryClassifier.h>

// includes for all Trainer implementations
#include <ITrainer.h>

#endif
