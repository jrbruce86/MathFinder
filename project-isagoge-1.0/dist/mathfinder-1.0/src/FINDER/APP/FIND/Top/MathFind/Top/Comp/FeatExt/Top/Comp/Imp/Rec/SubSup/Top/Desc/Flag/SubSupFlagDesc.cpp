/*
 * SubOrSuperscriptsFeatureExtractorFlagDescription.cpp
 *
 *  Created on: Dec 11, 2016
 *      Author: jake
 */

#include <SubSupFlagDesc.h>

#include <string>

std::string HasSubscriptFlagDescription
::getName() {
  return "HasSubscript";
}
std::string HasSubscriptFlagDescription
::getDescriptionText() {
  return "Binary feature expressing that the blob has a subscript if set nonzero.";
}

std::string IsSubscriptFlagDescription
::getName() {
  return "IsSubscript";
}
std::string IsSubscriptFlagDescription
::getDescriptionText() {
  return "Binary feature expressing that the blob is a subscript if set nonzero.";
}

std::string HasSuperscriptFlagDescription
::getName() {
  return "HasSuperscript";
}
std::string HasSuperscriptFlagDescription
::getDescriptionText() {
  return "Binary feature expressing that the blob has a superscript if set nonzero.";
}

std::string IsSuperscriptFlagDescription
::getName() {
  return "IsSuperscript";
}
std::string IsSuperscriptFlagDescription
::getDescriptionText() {
  return "Binary feature expressing that the blob is a superscript if set nonzero.";
}
