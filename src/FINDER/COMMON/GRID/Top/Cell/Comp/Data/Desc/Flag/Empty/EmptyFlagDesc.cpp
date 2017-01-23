/*
 * EmptyFlagDesc.cpp
 *
 *  Created on: Jan 22, 2017
 *      Author: jake
 */

#include <EmptyFlagDesc.h>

#include <string>

EmptyFlagDescription::EmptyFlagDescription() {}

std::string EmptyFlagDescription::getName() {
  return "None";
}

std::string EmptyFlagDescription::getDescriptionText() {
  return "N/A";
}

