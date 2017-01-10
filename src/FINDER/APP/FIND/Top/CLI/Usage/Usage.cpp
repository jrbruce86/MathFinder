/*
 * MathExpressionFinderUsage.cpp
 *
 *  Created on: Dec 22, 2016
 *      Author: jake
 */

#include <Usage.h>

#include <iostream>

void MathExpressionFinderUsage::printUsage() {
  std::cout << "-----MathFinder usage------\n\n"
      << "In order to run the math finder on an image or multiple images "
      << "run as follows:\n"
      << "MathFinder [path]\n"
      << "Where [path] is the path either to a single image or a directory "
      << "containing multiple images.\n\n"
      << "For all other options including training, evaluation, groundtruth "
      << "generation, and documentation, there is an interactive menu which can "
      << "be run as follows:\n"
      << "MathFinder -menu\n";
}

