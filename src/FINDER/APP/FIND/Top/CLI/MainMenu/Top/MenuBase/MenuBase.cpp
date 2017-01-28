/*
 * MenuBase.cpp
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */

#include <MenuBase.h>

#include <Utils.h>

#include <iostream>

MenuBase::MenuBase(){}

MenuBase* MenuBase::getNextSelected() {
  std::cout << "------------------------------\n"
      << getName() << ":\n------------------------------\n";
  doTask();
  const int selection =
      Utils::promptSelectFromLabeledMatrix(
          subMenus,
          getSelectionColumns());
  return subMenus[selection];
}

std::ostream& operator<<(std::ostream& stream, const MenuBase& m) {
  stream << m.getName();
  return stream;
}

int MenuBase::getSelectionColumns() {
  return 2;
}

bool MenuBase::isNotExit() {
  return true;
}

void MenuBase::doTask() {}

MenuBase::~MenuBase() {
  // The menu implementations do their own deletions if needed
}
