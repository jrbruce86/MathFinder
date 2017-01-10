/*
 * MenuBase.h
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */

#ifndef MENUBASE_H_
#define MENUBASE_H_

#include <iostream>
#include <vector>
#include <string>

class MenuBase {

 public:

  MenuBase();

  /**
   * Primary function for a menu. Calling code
   * invokes in a while loop until a "poison" menu
   * is found.
   */
  MenuBase* getNextSelected();

  /**
   * Gets the name of this menu, displayed while showing options
   */
  virtual std::string getName() const = 0;

  /**
   * Callback for doing some task within the getNextSelected method
   * before populating the menu options.
   */
  virtual void doTask();

  /**
   * Return true only for menus indicating an exit condition
   * to calling logic
   */
  virtual bool isNotExit();
  /**
   * Override for displaying menu options
   */
  friend std::ostream& operator<<(std::ostream& stream, const MenuBase& m);

  /**
   * Gets the number of columns for displaying the menu options
   */
  virtual int getSelectionColumns();

  virtual ~MenuBase();

 protected:

  // The menu options
  std::vector<MenuBase*> subMenus;
};

/**
 * "Poison" menu for convenient exit by calling code
 */
class ExitMenu : public virtual MenuBase {
 public:
  std::string getName() const { return "Exit"; }
  bool isNotExit() { return false; }
};

#endif /* MENUBASE_H_ */
