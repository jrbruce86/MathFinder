/*
 * DetectorSelectionMenu.h
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */

#ifndef DETECTORSELECTIONMENU_H_
#define DETECTORSELECTIONMENU_H_

#include <MenuBase.h>

class TrainingMenu;

class DetectorSelectionMenu: public MenuBase {

 public:

  DetectorSelectionMenu(TrainingMenu* back);

  std::string getName() const;

  void doTask();

  std::string getSelectedDetectorName();

  std::string getDefaultDetectorName();

 private:

  std::string selectedDetectorName;
};


#endif /* DETECTORSELECTIONMENU_H_ */
