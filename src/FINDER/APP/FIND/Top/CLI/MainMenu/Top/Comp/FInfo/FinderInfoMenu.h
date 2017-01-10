/*
 * FinderInfoMenu.h
 *
 *  Created on: Dec 22, 2016
 *      Author: jake
 */

#ifndef FINDERINFOMENU_H_
#define FINDERINFOMENU_H_

#include <MenuBase.h>
#include <MainMenu.h>

#include <string>

class FinderInfoMenu : public virtual MenuBase {

 public:
  FinderInfoMenu(MainMenu* const back);

  std::string getName() const;

  void doTask();
};


#endif /* FINDERINFOMENU_H_ */
