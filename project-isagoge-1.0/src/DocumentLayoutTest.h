/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:		DocumentLayoutTest.h
 * Written by:	Jake Bruce, Copyright (C) 2013
 * History: 		Created Feb 28, 2013 9:15:45 PM 
 * Description: TODO
 * 
 * This file is part of Project Isagoge.
 * 
 * Project Isagoge is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Project Isagoge is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Project Isagoge.  If not, see <http://www.gnu.org/licenses/>.  
 ***************************************************************************/

#ifndef DOCUMENTLAYOUTTEST_H_
#define DOCUMENTLAYOUTTEST_H_

#include <string>
#include <vector>
#include <baseapi.h> // tesseract api
#include <allheaders.h> // leptonica api
#include "EquationDetectorSVM.h"
using namespace std;
using namespace tesseract;

class DocumentLayoutTester {
 public:

  /**********************************************************
  * Initializes all potentially desired Tesseract parameters
  * then initalizes Tesseract in auto-segmentation mode so
  * that the appropriate equation detector will be utilized on
  * the input document. Terminates program execution if the
  * segmentation mode of Tesseract was not properly set
  **********************************************************/
  DocumentLayoutTester();

  /**************************************************************************
  * Set the desired file directory structure so that the right
  * files can be processed by runTessLayout.
  *
  * The directory structure format is as follows:
  *                              topdir
  *           input                             output
  *   subdir1      subdir2 ...etc       sibdir1         subdir2    ...etc
  * [imagelst1]   [imagelst2]           resultdirlst1   resultdirs2
  *                                     res1lst1 res1lst2  res2lst1 res2lst2
  * arg1 - relative path to root of directory structure
  * arg2 - name of the subdirectory with the inputs and outputs
  * arg3- the image extension (i.e. png, jpg, etc). all should be the same
  *        in a given subdir
  *
  * Assumes all images in the input dir are named 1.ext,2.ext,3.ext,.....
  * (i.e. they start from 1 not 0)
  **************************************************************************/
  void setFileStructure(string topdir_, string subdir_, string ext_);

  /**********************************************************
  * Go ahead and enable all of the true/false params        *
  **********************************************************/
  void activateAllBoolParams();

  /****************************************************************************
  * Turn off all params that require the scroll view (won't work in eclipse..)*
  ****************************************************************************/
  void deActivateScrollView();

  /****************************************************************************
  * Activate all the parameters accept for those that involve the scrollview
  ****************************************************************************/
  void activateNonScrollView();

  /*************************************************************************
  * Get Tesseract layout results on the entire directory structure         *
  *************************************************************************/
  void runTessLayout();

 protected:
 /****************************************
  * For Tesseract parameter manipulation *
  ***************************************/
  // set up the parameter list
  void setTesseractParams();

  // Error message for Tesseract paramaters which don't exist
  void tessParamError(string param);

  // Tell Tesseract to set a parameter to true
  void activateBoolParam(string param);

  // Tell Tesseract to set a parameter back to false
  void deActivateBoolParam(string param);

  /****************************************
   * Linux system command utilities      **
   ***************************************/
  // execute a system command
  string exec(string cmd);

  // execute system command and display output
  void exec_display(string cmd);

  // count number of files in given directory
  int fileCount(string dir);


  /****************************************
   * Some very basic helper utilities    **
   ***************************************/
  // convert integer to string
  string intToString(int i);

  // Read in an image using Leptonica, end execution with error message if pixread fails
  Pix* leptReadImg(string fn);

  // returns the number of digits in a given integer decimal number
  int digit_count(int decnum);

  /****************************************
  * Data members                         **
  ****************************************/
  string topdir; // top directory of file structure to be processed
  string subdir; // the name of the subdirectory file structure
  string ext; // image extension (ie png, jpg, etc)
  vector<string> params; // array of true/false tesseract parameters
  string page_seg_mode; // tesseract's page segmentation mode
  TessBaseAPI api; // the Tesseract API
  EquationDetectorSVM* equ;
};

#endif /* DOCUMENTLAYOUTTEST_H_ */
