/*
 * SegmentorSelectionMenu.cpp
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */

#include <SegMenu.h>

#include <TrainingMenu.h>

#include <SegFac.h>

#include <Utils.h>

#include <string>
#include <iostream>

SegmentorSelectionMenu::SegmentorSelectionMenu(
    TrainingMenu* back)
: selectedSegmentorName("") {
  subMenus.push_back(back);
}

std::string SegmentorSelectionMenu::getName() const {
  return "Segmentor selection menu";
}

void SegmentorSelectionMenu::doTask() {
  // Ask if need to select again if already selected
  bool doSegmentorSelection = true;
  if(selectedSegmentorName != "") {
    std::cout << "You have selected the " << selectedSegmentorName
        << " detector. Modify selection? ";
    doSegmentorSelection = Utils::promptYesNo();
  }

  MathExpressionSegmentorFactory segmentorFactory;

  // Carry out the selection
  while(doSegmentorSelection) {
    std::cout << "Select a segmentor:\n";
    const int selected = Utils::promptSelectStrFromLabeledMatrix(
        segmentorFactory.getSupportedSegmentorNames(), 3);
    selectedSegmentorName = segmentorFactory.getSupportedSegmentorNames()[selected];

    // Confirm selection
    std::cout << "You have selected the " << selectedSegmentorName
        << " detector. Save selection and return to previous menu? ";
    doSegmentorSelection = !Utils::promptYesNo();
  }
}

std::string SegmentorSelectionMenu::getSelectedSegmentorName() {
  return selectedSegmentorName;
}

void SegmentorSelectionMenu::setToDefault() {
  this->selectedSegmentorName = getDefaultSegmentorName();
}

std::string SegmentorSelectionMenu::getDefaultSegmentorName() {
  return MathExpressionSegmentorFactory().getSupportedSegmentorNames()[0];
}
