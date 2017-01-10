/*
 * TrainingMenu.h
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */

#ifndef TRAININGMENU_H_
#define TRAININGMENU_H_

#include <MenuBase.h>
#include <MainMenu.h>
#include <NameMenu.h>

#include <string>

class NameSelectionMenu;

class TrainingMenu : public virtual MenuBase {

 public:

  TrainingMenu(MainMenu* back);

  ~TrainingMenu();

  std::string getName() const;

  static std::string TrainingMenuName();

  std::string getInstructions();

  void doTask();

 private:

  NameSelectionMenu* nameMenu;

};


#endif /* TRAININGMENU_H_ */
