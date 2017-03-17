/*
 * AboutTrainingMenu.cpp
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */
#include <AboutTrainingMenu.h>
#include <TrainingMenu.h>

AboutTrainingMenu::AboutTrainingMenu(
    TrainingMenu* back) {
  this->subMenus.push_back(back);
}


std::string AboutTrainingMenu::getName() const {
  return "About";
}

void AboutTrainingMenu::doTask() {
  std::cout << "This software provides a framework for training, testing, and evaluating document layout analysis techniques. \
More specifically this software focuses on finding math regions \
in document pages which may or may not contain math. The purpose of this layotu analysis is for increasing the accuracy \
of OCR on a wider variety pages. [Insert more stuff from article] \n\
\
 A trained math finder using this framework consists of three primary parts:\n\n\
\
   - The feature extractor:\n\
     Extract information from each blob on the page. The same extractors are used both during training and prediction.\n\
\n\
   - The detector:\n\
     Uses the extracted information in the previous step to classify individual blobs as either math or non-math.\n\
\n\
   - The segmentor:\n\
     Takes the classified blobs and does further processing to decide how the detected blobs should be merged into complete math expressions.\n\n";
}
