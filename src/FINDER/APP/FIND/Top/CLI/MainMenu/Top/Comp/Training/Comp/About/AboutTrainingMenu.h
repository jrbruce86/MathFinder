/*
 * AboutTrainingMenu.h
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */

#ifndef ABOUTTRAININGMENU_H_
#define ABOUTTRAININGMENU_H_

#include <MenuBase.h>

class TrainingMenu;

class AboutTrainingMenu : public virtual MenuBase {

 public:

  AboutTrainingMenu(TrainingMenu* back);

  std::string getName() const ;

  void doTask();
};


#endif /* ABOUTTRAININGMENU_H_ */
