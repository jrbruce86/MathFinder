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

// SVMDectector is the first Detector implementation.
// Uses a binary SVM classifier trained through 10-fold cross validation first on 15 training
// pages randomly selected from E. Bidwell, Advanced Calculus. (1911), and then experiments
// were carried out on other datasets. The following 23 features are implemented during classifier
// training and prediction (See thesis for more details on each feature):
//   1.  Rightward horizontally adjacent blobs covered (rhabc)
//   2.  Upward vertically adjacent blobs covered (uvabc)
//   3.  Downard vertically adjacent blobs covered (dvabc)
//   4.  Number of completely nested characters (cn)
//   5.  Has a superscript
//   6.  Has a subscript
//   7.  Is a superscript
//   8.  Is a subscript
//   9.  Blob height (h)
//   10. Blob width/height ratio (whr)
//   11. Vertical distance above row baseline (vdarb)
//   12. Count of stacked blobs at blob position (cosbabp)
//   13. Is blob in math word (imw)
//   14. Italicized text (is_italic)
//   15. OCR confidence rating (ocr_conf)
//   16. Unigram Feature (unigram)
//   17. Bigram Feature (bigram)
//   18. Trigram Feature (trigram)
//   19. Blob belongs to row with normal text (in_valid_row)
//   20. Blob belongs to normal text (in_valid_word)
//   21. Page Doesn't Have Normal Text (bad_page)
//   22. Blob belongs to a stop word (stop_word)
//   23. Blob belongs to a valid word (valid_word)
typedef Detector<CrossValidatorSVM<libSVM>, libSVM, F_Ext1> SVMDetector;

// The following are variations of the SVMDetector that use slightly different feature combinations
// (See the F_Ext2, F_Ext3, etc. for more details):
typedef Detector<CrossValidatorSVM<libSVM>, libSVM, F_Ext2> SVMDetector1; // no valid_word feature
typedef Detector<CrossValidatorSVM<libSVM>, libSVM, F_Ext3> SVMDetector2; // no n-grams or valid_row
typedef Detector<CrossValidatorSVM<libSVM>, libSVM, F_Ext4> SVMDetector3; // no italics feature



#endif

