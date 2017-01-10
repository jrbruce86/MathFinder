/*
 * TesseractParamManager.h
 *
 *  Created on: Nov 6, 2016
 *      Author: jake
 */

#ifndef TESSERACTPARAMMANAGER_H_
#define TESSERACTPARAMMANAGER_H_

#include <string>
#include <vector>

#include <baseapi.h>

/**
 * Convenience class to help with managing Tesseract parameters
 */
class TesseractParamManager {
public:

  /**********************************************************
   * Constructor: Sets the api.                             *
   **********************************************************/
  TesseractParamManager(tesseract::TessBaseAPI* const api);

  /**********************************************************
   * Go ahead and enable both all int and all bool params    *
   **********************************************************/
  void activateAllParams();

  /**********************************************************
   * Go ahead and enable all of the true/false params        *
   **********************************************************/
  void activateAllBoolParams();

  // Turn on equation debugging in Tesseract so it outputs
  // the results of layout analysis
  void activateEquOutput();

  /**********************************************************
   * Go ahead and enable all of the integer params           *
   **********************************************************/
  void activateAllIntParams();

  /****************************************************************************
   * Turn off all params that require the scroll view                          *
   ****************************************************************************/
  void deActivateScrollView();

  /****************************************************************************
   * Activate all the parameters accept for those that involve the scrollview
   ****************************************************************************/
  void activateNonScrollView();

  /****************************************************************************
   * Tell Tesseract to set a bool parameter to true
   ****************************************************************************/
  void activateBoolParam(std::string param);

  /****************************************************************************
   * Tell Tesseract to set an int parameter to 1
   ****************************************************************************/
  void activateIntParam(std::string param);

  /****************************************************************************
   * Tell Tesseract to set a bool parameter back to false
   ****************************************************************************/
  void deActivateBoolParam(std::string param);

  /****************************************************************************
   * Tell Tesseract to set an int parameter back to zero
   ****************************************************************************/
  void deActivateIntParam(std::string param);

  /****************************************************************************
   * Tell Tesseract to set its page segmentation mode to the one provided
   ****************************************************************************/
  void setPageSegmentationMode(const tesseract::PageSegMode mode);

 private:

  void init();

  void tessParamError(std::string str);

  tesseract::TessBaseAPI* api;

  std::vector<std::string> boolparams; // array of true/false tesseract parameters
  std::vector<std::string> intparams; // array of int tesseract params
  std::string page_seg_mode; // tesseract's page segmentation mode
};


#endif /* TESSERACTPARAMMANAGER_H_ */
