/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:		DocumentLayoutTest.cpp
 * Written by:	Jake Bruce, Copyright (C) 2013
 * History: 		Created Feb 28, 2013 9:19:01 PM 
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

#include "DocumentLayoutTest.h"
#include <assert.h>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

DocumentLayoutTester::DocumentLayoutTester() {
  equ = new EquationDetectorSVM();
  setTesseractParams();
  api.Init("/usr/local/share/", "eng");
  // Choose the page segmentation mode as PSM_AUTO
  // Fully automatic page segmentation, but no OSD
  // (Orientation and Script Detection).
  if (!api.SetVariable(page_seg_mode.c_str(), "3"))
    tessParamError(page_seg_mode);

  // Check to make sure tesseract is in the correct page
  // segmentation mode (psm)
  int psm = 0;
  api.GetIntVariable(page_seg_mode.c_str(), &psm);
  assert(psm == PSM_AUTO);
}

// set up the file structure holding all the input
// and output test data
void DocumentLayoutTester::setFileStructure(string topdir_,\
    string subdir_, string ext_) {
  topdir = topdir_;
  subdir = subdir_;
  ext = ext_;
}


// Go ahead and enable all of the true/false params
// (can come back and deactivate ones i don't want later)
void DocumentLayoutTester::activateAllBoolParams() {
  for (unsigned int i = 0; i < params.size(); i++)
    activateBoolParam(params[i]);
}

// Deactivate all the parameters that use scrollview (won't work in eclipse)
void DocumentLayoutTester::deActivateScrollView() {
  deActivateBoolParam("textord_tabfind_show_initialtabs");
  deActivateBoolParam("textord_tabfind_show_images");
  deActivateBoolParam("textord_tabfind_show_reject_blobs");
  deActivateBoolParam("textord_show_tables");
  deActivateBoolParam("textord_tabfind_show_initial_partitions");
  deActivateBoolParam("textord_tabfind_show_partitions");
  deActivateBoolParam("textord_debug_images");
  deActivateBoolParam("textord_tabfind_show_blocks");
  deActivateBoolParam("textord_show_blobs");
  deActivateBoolParam("textord_show_boxes");
}

// Activate all perameters accept for those that require the scroll view
void DocumentLayoutTester::activateNonScrollView() {
  activateAllBoolParams();
  deActivateScrollView();
}

// Get tesseract layout results on the directory structure denoted by 'subdir'
void DocumentLayoutTester::runTessLayout() {
  // set input and output paths
  if(!fileCount(topdir + "output/"))
    exec_display("mkdir " + topdir + "output/");
  const string input_subdir_path = topdir + "input/" + subdir;
  const string output_subdir_path = topdir + "output/" + subdir;
  int numfiles = fileCount(input_subdir_path);
  if (!fileCount(output_subdir_path))
    exec_display("mkdir " + output_subdir_path);

  string inputfile_name; // name of given input file
  string inputfile_path; // the path to the given file
  string outputfile_path; // path to the output for a given file

  for (int i = 1; i <= numfiles; i++) {
    // set paths
    inputfile_name = intToString(i) + ext;
    inputfile_path = input_subdir_path + inputfile_name;
    outputfile_path = output_subdir_path + inputfile_name + (string) "/";
    if (!fileCount(outputfile_path))
      exec_display("mkdir " + outputfile_path);

    // read image and run layout analysis
    Pix* img = leptReadImg(inputfile_path);
    api.SetImage(img); // set the image
    api.AnalyseLayout(); // Run Tesseract's layout analysis
    pixDestroy(&img); // destroy finished image

    // move all the output images to a directory
    exec_display((string) "mv *.png " + outputfile_path);
    exec_display((string) "mv *.tif " + outputfile_path);
  }
}

void DocumentLayoutTester::setTesseractParams() {
  /******************* Massive List of Parameters to Play With!!!!*********************************/
  // WARNING: THESE ARE HARDCODED.. DON'T MODIFY!!!!!!!!!!!!
  ////////////Settings paramaters////////////
  // Activate Tesseract's equation detection setting
  params.reserve(20);
  params.push_back("textord_equation_detect"); // turns on the equation detection

  ////////////textordering debug params ////////////
  // Should display a window showing the intial tabs detected in FindInitialTabVectors
  params.push_back("textord_tabfind_show_initialtabs");
  // Should display images detected as distinct from text by FindImagePartitions
  params.push_back("textord_tabfind_show_images");
  // Enabes their table detection!! Class for this is TableFinder in textord/tablefind.h.
 // params.push_back("textord_tabfind_find_tables");
  // In order to see a window display of the tables detected!
  // This is run after the equation detection if enabled.
  //params.push_back("textord_show_tables");
  // Displays blobs rejected as noise prior to equation or tabledetection
  params.push_back("textord_tabfind_show_reject_blobs");
  // This will show the partitions prior to the equation and tabledetection
  params.push_back("textord_tabfind_show_initial_partitions");
  // This will show the partitions after equation and table detection
  params.push_back("textord_tabfind_show_partitions");
  // Use greyed background for debug images
  params.push_back("textord_debug_images");
  // Print tabfind related debug messages
  params.push_back("textord_debug_tabfind");
  // Displays blob and block bounding boxes in a window called “Blocks”.
  // This is after equation and table detection. Also occurs during  setupandfilternoise,
  // which occurs before findblocks is called in order to filter out noise.
  params.push_back("textord_tabfind_show_blocks");
  // Display unsorted blobs during call to filterblobs made within textordpage which is
  // called after autosegmentation is carried out.
  // Displays all the blobs color-coded at ones.
  params.push_back("textord_show_blobs");
  // Displays “boxes” this displays each type of blob
  // (small, noise, big, medium size) one at a time.
  params.push_back("textord_show_boxes");

  ////////////tessedit debug params////////////////////
  // dump intermediate images during page segmentation
  params.push_back("tessedit_dump_pageseg_images");

  ///////////equationdetect debug params////////////////////
  // display the BSTT's
  params.push_back("equationdetect_save_spt_image");
  // save the results of pass 2 (identifyseedparts)
  params.push_back("equationdetect_save_seed_image");
  //  save the final math detection results
  params.push_back("equationdetect_save_merged_image");

  // print table detection output
  params.push_back("textord_dump_table_images");

  // Change from default page segmentation mode (single block)
  // to the more advanced auto-segmentation, which includes
  // the experimental equation detection module: 3 = PSM_AUTO
  page_seg_mode = "tessedit_pageseg_mode";
  /*********************End of Massive Parameter List!*********************************/
}

// Error message for Tesseract paramaters which don't exist
void DocumentLayoutTester::tessParamError(string param) {
  cout << "ERROR: Tesseract could not find a paramater called, " << param
       << endl;
}


// Tell Tesseract to set a parameter to true
void DocumentLayoutTester::activateBoolParam(string param) {
  if (!api.SetVariable(param.c_str(), "true"))
    tessParamError(param);
}

// Tell Tesseract to set a parameter back to false
void DocumentLayoutTester::deActivateBoolParam(string param) {
  if (!api.SetVariable(param.c_str(), "false"))
    tessParamError(param);
}

// execute a system command
string DocumentLayoutTester::exec(string cmd) {
  FILE* pipe = popen(cmd.c_str(), "r");
  if (!pipe)
    return "ERROR";
  char buffer[128];
  string result = "";
  while (!feof(pipe)) {
    if (fgets(buffer, 128, pipe) != NULL)
      result += buffer;
  }
  pclose(pipe);
  return result;
}

// execute system command and display output
void DocumentLayoutTester::exec_display(string cmd) {
  string res = exec(cmd);
  if (!res.empty())
    cout << "% " << res << endl;
}

// count number of files in given directory
int DocumentLayoutTester::fileCount(string dir) {
  string res = exec("ls " + dir);
  int count = 0;
  if (!res.empty()) {
    if (res.substr(3) != "ls:") {
      for (unsigned int i = 0; i < res.size(); i++) {
        if (10 == res[i])
          count++;
      }
    }
  }
  return count;
}

// convert integer to string
string DocumentLayoutTester::intToString(int i) {
  char buf[digit_count(i)];
  sprintf(buf, "%d", i);
  return (string) buf;
}

// Read in an image using Leptonica, end execution with error
// message if pixread fails
Pix* DocumentLayoutTester::leptReadImg(string fn) {
  Pix* img = pixRead(fn.c_str());
  if (img == NULL) {
    cout << "ERROR: Could not open " << fn << endl;
    exit(EXIT_FAILURE);
  }
  return img;
}

// returns the number of digits in a given integer decimal number
int DocumentLayoutTester::digit_count(int decnum) {
  int numdigits = 0;
  double ddecnum = (double) decnum;
  while (floor(ddecnum) != 0) {
    ddecnum /= (double) 10;
    numdigits++;
  }
  return numdigits;
}

