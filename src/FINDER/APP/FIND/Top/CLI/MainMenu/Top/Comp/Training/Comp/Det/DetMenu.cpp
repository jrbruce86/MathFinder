/*
 * DetectorSelectionMenu.cpp
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */

#include <DetMenu.h>

#include <TrainingMenu.h>

#include <DetFac.h>

#include <Utils.h>

#include <iostream>


DetectorSelectionMenu::DetectorSelectionMenu(
    TrainingMenu* back)
: selectedDetectorName("") {
  subMenus.push_back(back);
}

std::string DetectorSelectionMenu::getName() const {
  return "Detector selection menu";
}

void DetectorSelectionMenu::doTask() {

  // Ask if need to select again if already selected
  bool doDetectorSelection = true;
  if(selectedDetectorName != "") {
    std::cout << "You have selected the " << selectedDetectorName
        << " detector. Modify selection? ";
    doDetectorSelection = Utils::promptYesNo();
  }

  MathExpressionDetectorFactory detectorFactory;

  // Carry out the selection
  while(doDetectorSelection) {
    std::cout << "Select a detector:\n";
    const int selected = Utils::promptSelectStrFromLabeledMatrix(
        detectorFactory.getSupportedDetectorNames(), 3);
    selectedDetectorName = detectorFactory.getSupportedDetectorNames()[selected];

    // Confirm selection
    std::cout << "You have selected the " << selectedDetectorName
        << " detector. Save selection and return to previous menu? ";
    doDetectorSelection = !Utils::promptYesNo();
  }
}

std::string DetectorSelectionMenu::getSelectedDetectorName() {
  return selectedDetectorName;
}

void DetectorSelectionMenu::setToDefault() {
  this->selectedDetectorName = getDefaultDetectorName();
}

std::string DetectorSelectionMenu::getDefaultDetectorName() {
  return MathExpressionDetectorFactory().getSupportedDetectorNames()[0];
}

