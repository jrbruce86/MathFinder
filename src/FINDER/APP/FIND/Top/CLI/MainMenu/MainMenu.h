/*
 * MainMenu.h
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */

#ifndef MAINMENU_H_
#define MAINMENU_H_

#include <MenuBase.h>

#include <string>

class MainMenu : public virtual MenuBase {

 public:

  MainMenu();

  ~MainMenu();

  std::string getName() const;

  static std::string MainMenuName();
};


#endif /* MAINMENU_H_ */
