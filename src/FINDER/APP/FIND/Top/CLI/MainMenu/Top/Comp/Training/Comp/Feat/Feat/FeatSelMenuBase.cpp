/*
 * FeatureSelectionMenuBase.cpp
 *
 *  Created on: Dec 16, 2016
 *      Author: jake
 */
#include <FeatSelMenuBase.h>

#include <FeatSelMenuMain.h>
#include <BlobFeatExtCat.h>
#include <BlobFeatExtFac.h>

#include <Utils.h>

#include <baseapi.h>

#include <assert.h>
#include <string>

FeatureSelectionMenuBase::FeatureSelectionMenuBase() {}

void FeatureSelectionMenuBase::doTask() {

  // Grab the factories
  const std::vector<BlobFeatureExtractorFactory*> allFactories =
      getCategory()->getFeatureExtractorFactories();

  // Set the shared display value
  const int featureRows = 1;

  // Start off by showing which features have been selected already if some have already
  // been selected, if not just skip this
  std::vector<BlobFeatureExtractorFactory*>& selectedFactories = getSelectedFactories();
  if(selectedFactories.size() > 0) {
    std::cout << "The following features have been selected so far:\n";
    Utils::displayVectorAsLabeledMatrix(selectedFactories, featureRows);
    std::cout << "Would you like to update this selection? ";
    if(!Utils::promptYesNo()) {
      return;
    }
  }

  bool readyToReturn = false;

  // Carry out the selection options until ready to return
  while(!readyToReturn) {

    // Show the features to choose from
    std::cout << "\n\nBelow are the " << getName() << " features to choose from:\n";
    Utils::displayVectorAsLabeledMatrix(allFactories, featureRows);

    // Choose a selection method
    std::cout << "Choose a selection method:\n-------------\n";
    std::vector<std::string> options;
    std::string selectToUse = "Select features to use";
    std::string selectNotToUse = "Select which features not to use, auto-selecting all others";
    std::string selectAll = "Select all features";
    std::string deselectAll = "De-select all features";
    std::string showInfo = "Select a feature to see a description on";
    std::string goBack = "Save selection and go back to previous menu";
    options.push_back(selectToUse);
    options.push_back(selectNotToUse);
    options.push_back(selectAll);
    options.push_back(deselectAll);
    options.push_back(showInfo);
    options.push_back(goBack);
    const int selection = Utils::promptSelectStrFromLabeledMatrix(options, 1);

    // Select the features based on the selection method
    const std::string selectedOption = options[selection];
    if(selectedOption != goBack) {
      if(selectedOption == showInfo) {
        bool done = false;
        while(!done) {
          std::cout << "Select one of the following features to see information on it.\n";
          const int selectedIndex = Utils::promptSelectFromLabeledMatrix(allFactories, 4);
          BlobFeatureExtractorDescription* const description =
              allFactories[selectedIndex]->getDescription();
          std::cout << description->getName() << ":\n"
              << description->getDescriptionText() << std::endl;
          std::cout << "Select another? ";
          done = !(Utils::promptYesNo());
        }
      } else if(selectedOption == deselectAll) {
        selectedFactories.clear();
      } else if(selectedOption == selectAll) {
        for(int i = 0; i < allFactories.size(); ++i) {
          addFactoryToSelection(allFactories[i], &selectedFactories);
        }
      } else {
        std::cout << selectedOption << " by typing in one or more of the below indexes "
            << "delimited by spaces and then pressing enter.\n";
        GenericVector<int> selectedIndexes =
            Utils::promptMultiSelectFromLabeledMatrix(allFactories, featureRows);
        selectedFactories.clear();
        if(selectedOption == selectToUse) {
          for(int i = 0; i < selectedIndexes.size(); ++i) {
            const int selectedIndex = selectedIndexes[i];
            promptToAddFactoryToSelection(allFactories[selectedIndex], &selectedFactories);
          }
        } else if(selectedOption == selectNotToUse) {
          for(int i = 0; i < allFactories.size(); ++i) {
            if(!selectedIndexes.contains(i)) {
              promptToAddFactoryToSelection(allFactories[i], &selectedFactories);
            }
          }
        }
      }

      // confirm selection. if negative then repeat above, if positive return
      if(selectedFactories.size() > 0) {
        std::cout << "You have selected the below features:\n";
        Utils::displayVectorAsLabeledMatrix(selectedFactories, featureRows);
      } else {
        std::cout << "You haven't selected any features.\n";
      }
      std::cout << "Save and go back to previous menu? ";
      readyToReturn = Utils::promptYesNo();
    }
  }
}

void FeatureSelectionMenuBase::promptToAddFactoryToSelection(
    BlobFeatureExtractorFactory* const factoryToAdd,
    std::vector<BlobFeatureExtractorFactory*>* const selection) {

  // Prompt for flags if there are flags associated with the
  // feature extractor created by this factory
  if(factoryToAdd->getDescription()->getFlagDescriptions().size() > 0) {
    std::vector<FeatureExtractorFlagDescription*> allFlagDescriptions = factoryToAdd->getDescription()->getFlagDescriptions();
    std::cout << "The chosen feature extractor, " << factoryToAdd->getDescription()->getName()
        << ", has flags that need to be selected/deselected. Select one or more of the "
        << "following flags by typing in their associated indices "
        << "delimited by spaces and then pressing enter.\n";
    GenericVector<int> selectedIndexes =
        Utils::promptMultiSelectFromLabeledMatrix(
            allFlagDescriptions,
            1);
    assert(selectedIndexes.length() > 0); // sanity check.
    for(int i = 0; i < selectedIndexes.size(); ++i) {
      factoryToAdd->getSelectedFlags().push_back(
          allFlagDescriptions[selectedIndexes[i]]);
    }
  }

  selection->push_back(factoryToAdd);
}

void FeatureSelectionMenuBase::addFactoryToSelection(
    BlobFeatureExtractorFactory* const factoryToAdd,
    std::vector<BlobFeatureExtractorFactory*>* const selection) {

  std::vector<FeatureExtractorFlagDescription*> flags = factoryToAdd->getDescription()->getFlagDescriptions();
  for(int i = 0; i < flags.size(); ++i) {
    factoryToAdd->getSelectedFlags().push_back(flags[i]);
  }

  selection->push_back(factoryToAdd);
}

