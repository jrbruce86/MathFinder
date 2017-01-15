/*
 * SegmentorSelectionMenu.h
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */

#ifndef SEGMENTORSELECTIONMENU_H_
#define SEGMENTORSELECTIONMENU_H_

#include <MenuBase.h>

#include <string>

class TrainingMenu;

class SegmentorSelectionMenu: public MenuBase {

 public:

  SegmentorSelectionMenu(TrainingMenu* back);

  std::string getName() const;

  void doTask();

  std::string getSelectedSegmentorName();

  void setToDefault();

  std::string getDefaultSegmentorName();

 private:

  std::string selectedSegmentorName;
};


#endif /* SEGMENTORSELECTIONMENU_H_ */
