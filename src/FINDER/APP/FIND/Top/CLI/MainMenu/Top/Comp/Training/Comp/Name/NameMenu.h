/*
 * NameSelectionMenu.h
 *
 *  Created on: Dec 16, 2016
 *      Author: jake
 */

#ifndef NAMESELECTIONMENU_H_
#define NAMESELECTIONMENU_H_

#include <MenuBase.h>

#include <string>

class TrainingMenu;

class NameSelectionMenu : public virtual MenuBase {

 public:

  NameSelectionMenu(TrainingMenu* const back);

  std::string getName() const;

  /**
   * Prompts user to set finder name
   */
  void promptSetFinderName();

  /**
   * Gets the user-entered finder name
   */
  std::string getFinderName();

  void doTask();

 private:

  std::string finderName;
};


#endif /* NAMESELECTIONMENU_H_ */
