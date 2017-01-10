/*
 * AboutMenu.cpp
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */

#include <AboutMenu.h>

#include <MainMenu.h>

#include <string>
#include <iostream>

AboutMenu::AboutMenu(MainMenu* const mainMenu) {
  subMenus.push_back(mainMenu);
}

std::string AboutMenu::getName() const {
  return "About";
}

void AboutMenu::doTask() {
  std::cout << "This software provides a framework for training, testing, and evaluating document layout analysis techniques.\
[insert canned description on document layout analysis]. More specifically this software focuses on finding math regions \
in document pages which may or may not contain math. The purpose of this layotu analysis is for increasing the accuracy \
of OCR on a wider variety pages. [Insert more stuff from article] \n\
\n\
\n\
A trained math finder using this framework consists of three primary parts:\n\
\n\
- The feature extractor:\n\
  Extracts information from each blob on the page. The same extractors are used both during training and prediction.\n\
\n\
- The detector:\n\
  Uses the extracted information in the previous step to classify individual blobs as either math or non-math.\n\
- The segmentor:\n\
  Takes the classified blobs and does further processing to decide how the detected blobs should be merged into complete math expressions.\n";

  std::cout << "To run an already trained detector, run this executable with the path or directory path of the image/images to find math for as "
      << "the commandline argument (e.g. MathFinder [path to image/images]).\n";
}

