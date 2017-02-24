/*
 * DatasetSelectionMenu.h
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */

#ifndef DATASETSELECTIONMENU_H_
#define DATASETSELECTIONMENU_H_

#include <MenuBase.h>

#include <vector>
#include <string>

class TrainingMenu;

class DatasetSelectionMenu : public virtual MenuBase {

 public:

  DatasetSelectionMenu(TrainingMenu* back);

  std::string getName() const;

  void doTask();

  std::string getGroundtruthDirPath();

  std::string getGroundtruthName();

  std::string getGroundtruthFilePath();

  std::vector<std::string> getGroundtruthImagePaths();

  bool isComplete();

  static bool groundtruthDirPathIsGood(std::string groundtruthDirPath);

  static int getFileNumFromPath(const std::string& path);

  static std::string getFileNameFromPath(const std::string& path);

  static std::vector<std::string> findImagePaths(const std::string& dirPath);


  static std::vector<std::string> findGroundtruthImagePaths(std::string groundtruthDirPath);

  static std::string findGroundtruthFilePath(std::string path);

 private:

  static std::vector<std::string> sortFilePathsNumerically(const std::vector<std::string>& strings);

  std::string promptNewDatasetDirPath();

  std::string promptSelectExistingGroundtruth();

  std::string groundtruthDirPath;

  std::string groundtruthName;

  std::string groundtruthFilePath;

  std::vector<std::string> groundtruthImagePaths;

  bool isComplete_;
};


#endif /* DATASETSELECTIONMENU_H_ */
