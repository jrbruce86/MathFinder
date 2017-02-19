/*
 * MathExpressionFinderMain.h
 *
 *  Created on: Dec 22, 2016
 *      Author: jake
 */

#ifndef MATHEXPRESSIONFINDERMAIN_H_
#define MATHEXPRESSIONFINDERMAIN_H_

#include <string>

void runInteractiveMenu();

void runFinder(char* path, bool doJustDetection=false);

// Runs trainer in isolation (for debug/experiment purposes)
static void runTrainer();

static std::string getResultsNameFromPath(std::string path);

#endif /* MATHEXPRESSIONFINDERMAIN_H_ */
