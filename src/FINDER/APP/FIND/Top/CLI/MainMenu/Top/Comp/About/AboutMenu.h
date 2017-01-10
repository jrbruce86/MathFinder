/*
 * AboutMenu.h
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */

#ifndef ABOUTMENU_H_
#define ABOUTMENU_H_

#include <MenuBase.h>
#include <MainMenu.h>

#include <string>

class AboutMenu : public virtual MenuBase {

 public:
  AboutMenu(MainMenu* const mainMenu);

  std::string getName() const;

  void doTask();
};


#endif /* ABOUTMENU_H_ */
