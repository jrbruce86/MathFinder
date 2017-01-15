/*
 * DatasetSelectionMenu.cpp
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */

#include <DatasetMenu.h>

#include <TrainingMenu.h>
#include <Utils.h>
#include <M_Utils.h>
#include <FinderTrainingPaths.h>

#include <baseapi.h>

#include <allheaders.h>

#include <vector>
#include <iostream>
#include <string>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>

DatasetSelectionMenu::DatasetSelectionMenu(TrainingMenu* const back)
: groundtruthName(""), groundtruthDirPath(""),
  groundtruthFilePath(""), isComplete_(false) {
  subMenus.push_back(back);
}

std::string DatasetSelectionMenu::getName() const {
  return "Select images to train on";
}

void DatasetSelectionMenu::doTask() {

  // Show currently chosen path and name if chosen and prompt if new path desired
  bool getNewDatasetPaths = true;
  if(groundtruthDirPath != "" && groundtruthName != "" && groundtruthFilePath != "" && groundtruthImagePaths.size() > 0) {
    std::cout << "The dataset name you have chosen is " << groundtruthName << std::endl;
    std::cout << "The dataset path you have chosen is " << groundtruthDirPath << std::endl;
    std::cout << "Would you like to edit this selection? ";
    getNewDatasetPaths = Utils::promptYesNo();
  }

  // Return if not continuing with prompt
  if(!getNewDatasetPaths) {
    return;
  }
  // Clear the form entry data
  groundtruthDirPath.clear();
  groundtruthFilePath.clear();
  groundtruthName.clear();
  groundtruthImagePaths.clear();

  // Prompt for new dataset paths
  groundtruthDirPath = promptSelectExistingGroundtruth();
  if(groundtruthDirPath == "") {
    groundtruthName = Utils::promptForValueNotOnList(
        "Enter a name for the dataset: ",
        Utils::getFileList(FinderTrainingPaths::getGroundtruthRoot()));
    groundtruthDirPath = FinderTrainingPaths::getGroundtruthRoot() + groundtruthName + "/";

    // Prompt for the new dataset dir path
    const std::string newGroundtruthDirPath =
        Utils::checkTrailingSlash(
            promptNewDatasetDirPath());

    // Copy the contents of the new dataset path to the groundtruth dir
    std::cout << "Creating a local copy of the contents of the selected dataset at "
        << groundtruthDirPath << std::endl;
    Utils::exec("mkdir -p " + groundtruthDirPath);
    Utils::exec("cp " + newGroundtruthDirPath + "* " + groundtruthDirPath);
  } else {
    if(!groundtruthDirPathIsGood(groundtruthDirPath)) {
      std::cout << "ERROR: The groundtruth directory at '" << groundtruthDirPath <<
          "' appears to be corrupted. Returning to "
          << "previous menu for more options. Try again with an updated or new directory.\n";
      groundtruthDirPath.clear();
      return;
    }
    groundtruthName = groundtruthDirPath.substr(groundtruthDirPath.find_last_of('/') + 1);
  }
  groundtruthFilePath = findGroundtruthFilePath(groundtruthDirPath);
  if(groundtruthFilePath == "") {
    // This hopefully shouldn't happen but just incase
    std::cout << "Error: Could not find the groundtruth file. You'll need to go "
        "through this menu again and import a new dataset (say no to using an "
        "existing one in this menu).\n";
    return;
  }

  groundtruthImagePaths = findGroundtruthImagePaths(groundtruthDirPath);
  std::cout << "groundtruth image paths: \n";
  for(int i = 0; i < groundtruthImagePaths.size(); ++i) {
    std::cout << groundtruthImagePaths[i] << " ";
  }
  std::cout << std::endl;
  isComplete_ = true;
}

std::string DatasetSelectionMenu::getGroundtruthDirPath() {
  return groundtruthDirPath;
}

std::string DatasetSelectionMenu::getGroundtruthName() {
  return groundtruthName;
}

std::string DatasetSelectionMenu::getGroundtruthFilePath() {
  return groundtruthFilePath;
}

std::vector<std::string> DatasetSelectionMenu::getGroundtruthImagePaths() {
  return groundtruthImagePaths;
}

bool DatasetSelectionMenu::isComplete() {
  return isComplete_;
}

/**
 * Asks user if they want to select an existing groundtruth dataset.
 * If not, or if there aren't any, return an empty string. Otherwise
 * return the selected path.
 */
std::string DatasetSelectionMenu::promptSelectExistingGroundtruth() {
  if(Utils::fileCount(FinderTrainingPaths::getGroundtruthRoot()) > 0) {
    std::cout << "The following previously used datasets at "
        << FinderTrainingPaths::getGroundtruthRoot() << " are available to use for training."
        << std::endl;
    std::vector<std::string> dirs = Utils::getFileList(FinderTrainingPaths::getGroundtruthRoot());
    Utils::displayStrVectorAsLabeledMatrix(dirs, 4);
    std::cout << "Would you like to use one of them? ";
    if(Utils::promptYesNo()) {
      const int index = Utils::promptSelectStrFromLabeledMatrix(dirs, 4);
      return Utils::checkTrailingSlash(FinderTrainingPaths::getGroundtruthRoot() + dirs[index]);
    }
  }
  return "";
}

std::string DatasetSelectionMenu::promptNewDatasetDirPath() {
  std::string datasetDirPath = "";
  while(true) {
    std::cout << "\nEnter the full path to a directory containing the images you would like to train on "
        << "as well as a file with the extension of '.rect' which contains the groundtruth bounding boxes "
        "for where the math regions are in the images. The images should be named as follows assuming "
        << "they are in .png format (images in most any format are supported): 0.png, 1.png, 2.png, etc...: ";
    Utils::getline(datasetDirPath);
    std::cout << "The line I got " << datasetDirPath << std::endl;
    std::cout << "Did it skip?? Shouldn't have. no Residual line. Remove this if not. TODO\n";
    Utils::waitForInput();
    std::cout << std::endl;

    if(groundtruthDirPathIsGood(Utils::checkTrailingSlash(datasetDirPath))) {
      break; // All tests pass. Good to go!
    } else {
      continue; // Try again....
    }
  }
  return datasetDirPath;
}

bool DatasetSelectionMenu::groundtruthDirPathIsGood(std::string groundtruthDirPath) {
  // Verify that there are files in the given path, and that the files consist
  // of one .rect file and the remainder of the non-directory files being readable images with their
  // names being numbers from 0 followed by their extension (i.e., 0.png, 1.jpg, etc..).
  // The extension doesn't really matter since leptonica handles all of that.
  std::vector<std::string> fileNames = Utils::getFileList(groundtruthDirPath);
  if(fileNames.size() < 1) {
    std::cout << "The provided groundtruth dir, " << groundtruthDirPath
        << " is empty.\n";
    return false;
  }

  // Verify that there is a .rect file and get its index
  int rectFileIndex = -1;
  for(int i = 0; i < fileNames.size(); ++i) {
    std::string fileName = fileNames[i];
    if(fileName.find(".rect") != std::string::npos) {
      rectFileIndex = i;
      break;
    }
  }
  if(rectFileIndex < 0 || rectFileIndex >= fileNames.size()) {
    std::cout << "Error: A file with the extension .rect needs to be included in the image directory. "
        << "This file should contain the bounding boxes of the groundtruth math regions in each image.\n";
    return false;
  }

  // Load the non-rect (and non-dir) file names onto their own vector
  std::vector<std::string> imageFiles;
  for(int i = 0; i < fileNames.size(); ++i) {
    if(i != rectFileIndex && !Utils::existsDirectory(groundtruthDirPath + fileNames[i])) {
      imageFiles.push_back(fileNames[i]);
    }
  }

  // Verify the non-rect files are named correctly
  bool namesAreValid = true;
  GenericVector<std::string> expectedNames;
  GenericVector<std::string> actualNames;
  {
    GenericVector<int> expectedNums;
    GenericVector<int> actualNums;
    for(int i = 0; i < imageFiles.size(); ++i) {
      expectedNums.push_back(i);
      // load on the name without the extension for a quick compare
      actualNums.push_back(atoi((imageFiles[i].substr(0, imageFiles[i].find("."))).c_str()));
    }
    expectedNums.sort();
    actualNums.sort();
    for(int i = 0; i < imageFiles.size(); ++i) {
      expectedNames.push_back(Utils::intToString(expectedNums[i]));
      actualNames.push_back(Utils::intToString(actualNums[i]));
    }
  }
  for(int i = 0; i < imageFiles.size(); ++i) {
    if(actualNames[i] != expectedNames[i]) {
      std::cout << "Error: The file, " << actualNames[i] <<
          " has an unexpected name (" << expectedNames[i] << " was expected). Names need to be numbered from 0 and " <<
          "incrementing by 1. For instance 0.png, 1.png, etc.\n";
      return false;
    }
  }

  // Verify the non-rect files are all readable images
  for(int i = 0; i < imageFiles.size(); ++i) {
    std::string fileName = imageFiles[i];
    std::string imageFullPath = groundtruthDirPath + fileNames[i];
    Pix* image = pixRead(imageFullPath.c_str());
    if(image == NULL) {
      std::cout << "Error: The provided dataset dir contains the following "
          << "file that is either not a readable image or is corrupted: "
          << imageFullPath << ". Try a different path or clean up the existing path.\n";
      pixDestroy(&image);
      return false;
    }
    pixDestroy(&image);
  }

  return true; // All the tests pass. Good to go.
}

std::string DatasetSelectionMenu::findGroundtruthFilePath(std::string path) {
  std::vector<std::string> fileNames = Utils::getFileList(path);
  for(int i = 0; i < fileNames.size(); ++i) {
    if(fileNames[i].find(".rect") != std::string::npos) {
      return Utils::checkTrailingSlash(path) + fileNames[i];
    }
  }
  return "";
}

std::vector<std::string> DatasetSelectionMenu::findGroundtruthImagePaths(std::string groundtruthDirPath) {
  std::vector<std::string> fileNames = Utils::getFileList(groundtruthDirPath);
  std::vector<std::string> trainingImagePaths;
  for(int i = 0; i < fileNames.size(); ++i) {
    if(fileNames[i].find(".rect") == std::string::npos && !Utils::existsDirectory(groundtruthDirPath + fileNames[i])) {
      trainingImagePaths.push_back(groundtruthDirPath + fileNames[i]); // assume if not the .rect file or dir then its an image
    }
  }

  // make sure they are in the right order
  std::vector<std::string> trainingImagePathsOrdered;
  for(int i = 0; i < trainingImagePaths.size(); ++i) {
    trainingImagePathsOrdered.push_back("");
  }
  for(int i = 0; i < trainingImagePaths.size(); ++i) {
    const int curNum = getFileNumFromPath(trainingImagePaths[i]);

    assert(curNum > -1 && curNum < trainingImagePaths.size());
    trainingImagePathsOrdered[curNum] = trainingImagePaths[i];
  }

  return trainingImagePathsOrdered;
}

int DatasetSelectionMenu::getFileNumFromPath(const std::string& path) {
  const std::string curPath = path;
  const int lastSlashIndex = curPath.find_last_of("/");
  const int dotIndex = curPath.find_last_of(".");
  const std::string curName = curPath.substr(lastSlashIndex + 1, dotIndex - lastSlashIndex);
  std::cout << "Training image with path " << curPath << " read in as " << curName << std::endl;
  Utils::waitForInput(); // TODO remove once confirmed this works right
  return atoi(curName.c_str());
}

