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

#include <assert.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <baseapi.h> // tesseract api
#include <allheaders.h> // leptonica api
using namespace std;
using namespace tesseract;

// My equation detection implementations:
#include "EquationDetectorSVM.h"

// Data structure/algorithms used for evaluation
#include "BipartiteGraph.h"

#include "Basic_Utils.h"
using namespace Basic_Utils;
#include "Lept_Utils.h"

class DocumentLayoutTester : public Lept_Utils {
 public:

  /**********************************************************
  * Initializes all potentially desired Tesseract parameters
  * then initalizes Tesseract in auto-segmentation mode so
  * that the appropriate equation detector will be utilized on
  * the input document. Terminates program execution if the
  * segmentation mode of Tesseract was not properly set. By
  * default the test will be run on Tesseract's 2011 equation
  * detector. If an equation detector is passed into the
  * constructor, however, then that one will be used by
  * Tesseract instead.
  *
  * This is designed to be used for a single evaluation only.
  * Comparing the results of two tests involves passing in
  * results of the two corresponding instantiations of this
  * class into the comparison algorithm.
  **********************************************************/
  DocumentLayoutTester(EquationDetectBase* equ_detect);

  /**************************************************************************
  * Set the desired file directory structure so that the right
  * files can be processed by runTessLayout.
  * WARNING: Make sure to include trailing slash for all directory names!!
  * (use checkTrailingSlash() method of BasicUtils.h)
  * The directory structure format is as follows:
  *                               topdir
  *           input                             output
  *   subdir1      subdir2 ...etc       sibdir1         subdir2    ...etc
  * [imagelst1]   [imagelst2]           resultdirlst1   resultdirs2
  *                                     res1lst1 res1lst2  res2lst1 res2lst2
  * arg1 - relative path to root of directory structure
  * arg2 - name of the subdirectory within both input and output dirs
  * arg3- the image extension (i.e. png, jpg, etc). all should be the same
  *        in a given subdir
  *
  * Assumes all images in the input dir are named 1.ext,2.ext,3.ext,.....
  * (i.e. they start from 1 not 0)
  **************************************************************************/
  void setFileStructure(string topdir_, string subdir_, string ext_);

  /**********************************************************
  * Go ahead and enable both all int and all bool params    *
  **********************************************************/
  void activateAllParams();

  /**********************************************************
  * Go ahead and enable all of the true/false params        *
  **********************************************************/
  void activateAllBoolParams();

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
  void activateBoolParam(string param);

  /****************************************************************************
  * Tell Tesseract to set an int parameter to 1
  ****************************************************************************/
  void activateIntParam(string param);

  /****************************************************************************
  * Tell Tesseract to set a bool parameter back to false
  ****************************************************************************/
  void deActivateBoolParam(string param);

  /****************************************************************************
  * Tell Tesseract to set an int parameter back to zero
  ****************************************************************************/
  void deActivateIntParam(string param);

  /*************************************************************************
  * Run either the modified version of Tesseract (I am overriding the equation
  * detection module in order to improve it) or the default version on all of
  * the images that are in the [topdir]/input/[subdir]/ and put the results in
  * [topdir]/output/[subdir]/[output_results_subdir_]/. The
  * output_results_subdir_ is specified as an argument to this method.
  * The method can be run any number of times so that multiple modified versions
  * of Tesseract may be tested, however the same name cannot be used for the
  * results subdirectory. Thus the names are all stored after each run and
  * if the same name is seen more than once then an exception is thrown.
  *************************************************************************/
  void runTessLayout(string output_results_subdir_, bool layout_alreadydone=false);

  /*************************************************************************
  * For whatever test is currently being run (the test name is specified by the
  * string, "subdir", which is assumed to have already been set by a call to
  * setFileStructure()), read from it's GroundTruth.dat file to find all of
  * the rectangles and then color the foreground of the corresponding input
  * image within these rectangles based on the color of interest. The resulting
  * image(s) are then saved in [topdir]/groundtruth/[subdir]/colorblobs. These
  * images can be used for pixel accurate evaluation for layout analysis.
  *************************************************************************/
  void colorGroundTruthBlobs();

  /********************************************************************************
  * Evaluate layout accuracy (assumes that runTessLayout was already run).        *
  *                                                                               *
  * Can be used to evaluate either the results of Tesseract's default             *
  * implementation or one that has certain methods overridden, i.e. for my        *
  * purposes I will override the math detection module. This method should be run *
  * on both the default Tesseract implementation and the modified one. The output *
  * is a data structure which represents the layout accuracy. The evaluation      *
  * technique is primarily based off of the evaluation used in:                   *
  *                                                                               *
  *  - F. Shafait, D. Keysers, and T. M. Breuel. "Pixel-Accurate Representation   *
  *  - and Evaluation of Page Segmentation in Document Images," 18th Int. Conf.   *
  *  - on Pattern Recognition, (Hong Kong, China), pp. 872–875, Aug. 2006.        *
  *                                                                               *
  * The output data structures representing the default Tesseract results and the *
  * modified ones will be compared in a separate method.                          *
  *                                                                               *
  * If runTessLayout has not been run then throws an exception. Also assumes      *
  * that the groundtruth data is available in the subdir of interest (the one     *
  * for which layout analysis has already been run). Takes the groundtruth data   *
  * and uses it to color the blobs of interest appropriately based on whether     *
  * they are part of a displayed equation (red), inline/embedded  equation (blue) *
  * or displayed equation label (green). See setFileStructure() for more details  *
  * on the directory structure. Beneath the topdir/ there is not only the         *
  * groundtruth/ but also input/ and output/ which contains the same subdirs as   *
  * in the groundtruth. For instance, for subdir1/ there will be a subdir1/ in    *
  * the input/ directory with all of the input images. There will also be a       *
  * subdir1/ in the output/ directory which will have the results that need to be *
  * evaluated. The subdir1/ located in the groundtruth/ is used to evaluate       *
  * the results in subdir1/ located in the output/.                               *
  *                                                                               *
  * Directory structure looks as follows (topdir same as in setFileStructure())   *
  *                          for the groundtruth:                                 *
  *                               topdir/                                         *
  *                             groundtruth/                                      *
  *           subdir1/                             subdir2/                       *
  * 1.png 2.png 3.png etc. colorblobs/    1.png 2.png 3.png etc. colorblobs/      *
  *                                                                               *
  * Arguments are as follows:                                                     *
  * arg1 - output_results_subdir_ -> same as specified in runTessLayout(). This   *
  *                                  is the name of the test being run which also *
  *                                  specifies the directory where the results    *
  *                                  for this test will be placed. The test is    *
  *                                  run on the images in the                     *
  ********************************************************************************/
  void evalTessLayout(string testname, bool layoutdone=false);

  int dbgColorCount(string imname, LayoutEval::Color color);

 protected:
  // -----------------getEvaluationMetrics--------------------------
  // :::::::::::Implementation info (skip to "Function output directory"
  // :::::::::::section below to see where the resulting metrics are
  // :::::::::::placed::::::::::::::::::::::::::::::::::::::::::::::
  // Creates a bipartite graph for the given filename and type
  // by reading in the groundtruth and Tesseract results
  // files. Results files are assumed to have already been stored in
  // [topdir]/output/[subdir]/[testname]/[results_type]/color_blobs
  // results_type could, for instance, be "math_results" or
  // "table_results" depending on what is being evaluated.
  // The typenamespec refers to the specific kind of element of the given
  // "results_type" being evaluated (i.e for math equations this
  // could be displayed, embedded, or label). For some evaluations
  // this part may be unnecessary and could thus be named arbitrarily
  // (i.e. if there is only one specific kind of element being detected
  // for the given results_type).
  // Uses this graph in order to attain metrics on the
  // accuracy of the results (termed hypothesis here) with respect
  // to the groundtruth.
  // :::::::::::::::::Function output directory:::::::::::::::::::::::
  // The output metrics are placed in the
  // [topdir]/output/[subdir]/[testname]/[results_type]/[typenamespec]/ directory
  // The [subdir] here refers to the subdirectory name which holds
  // the input images in [topdir]/[input]/[subdir]/. The [testname]
  // is what was provided to the evalTessLayout method from which
  // this is called, and its purpose is to allow for different
  // tests to be run on the same input dataset. [typenamespec] is
  // the specific type of element of the given [results_type] which
  // is being evaluated here. The hypboxfile is the full path and name
  // of the file in the colorblobs directory which holds all of the
  // rectangles detected for the hypothesis being evaluated.
  void getEvaluationMetrics(string testname, string results_type, \
      string typenamespec, int filenum, string hypboxfile);

  // ----------------colorFoundBlobs--------------------------
  // Takes all of the hypothesis images which have been generated
  // by Tesseract's layout evaluation (this was implemented specifically
  // for the math recognition results, which have displayed equations
  // labeled with red rectangles and inline ones labeled with green).
  // Colors the blobs on the inside of each such rectangle. Blobs
  // labeled as displayed equations are colored red, blobs labeled
  // as embedded are colored blue. Does this for all the images for
  // which layout analysis was run. The resulting
  // images are stored in a subdirectory of the hypothesis
  // one called "colorblobs".
  // labeled image dir   -> this is the dir where the input
  //                        labeled images are stored, named
  //                        1.png, 2.png, 3.png, etc. The
  // unlabeled image dir -> this is the dir where the input
  //                        unlabeled images are stored.
  //                        like with the groundtruth dir
  //                        these images are named 1.png,
  //                        2.png, 3.png, etc.
  // line_thickness      -> this is the pixel thickness of each
  //                        rectangle line in the image
  // significant colors  -> these are the colors that need
  //                        to be processed (all others will
  //                        be ignored.
  // ext                 -> the extension (i.e. tif, png, etc)
  // startimg (optional) -> the index of the first image to be
  //                        processed. if not specified then
  //                        starts from 1.png.
  // finalimg (optional) -> the index of the last image to
  //                        process. if not specified then
  //                        processes all the images in the
  //                        given groundtruth directory
  void colorFoundBlobs(string labeledinputdir, \
      string unlabeledinputdir, l_uint32 line_thickness, \
      vector<LayoutEval::Color> sigcolors, \
      string ext_, int startimg=1, int finalimg=-1);

  // write the boxes in box list to the file in the following
  // format:
  // [imagename][ext] [type] [left] [top] [right] [bottom]
  void writeBoxesToFile(FILE* file, BOXA* boxes, \
      LayoutEval::Color color, string imname);

 /****************************************
  * For Tesseract parameter manipulation *
  ***************************************/
  // set up the parameter list (they are hardcoded here)
  void setTesseractParams();

  // Error message for Tesseract paramaters which don't exist
  void tessParamError(string param);

  /****************************************
   * Result Box Detection Utilities (for detecting
   * the already labeled rectangles on the output of
   * Tesseract correctly)
   ***************************************/
  // Deals with overlapping rectangles
  inline void detectOverlappingRect(const BOX* const &box, \
      l_uint32* &pixel, const l_uint32& pixwidth, \
      const l_uint32& pixheight, const l_uint32& row, l_uint32& col, \
      const LayoutEval::Color& color, bool& overlapfound, \
      BOXA* processingboxbin, BOXA* finishedboxbin, bool& onbottom, \
      bool& looking_for_tr);

  // determine whether or not a box at the column given is already being processed
  bool isBeingProcessed(const l_uint32& col, const BOXA* const &processingboxbin);

  // Adds the bottom left of the rectangle (the final information needed)
  // and move s the completed rectangle from the processing bin to the
  // completed bin
  bool finishRectangle(BOXA* processingbin, BOXA* finishedboxbin, \
      l_uint32 col, l_uint32 row);

  // Sets the top right corner of the box at the top of the list
  // to the given column (the y coordinate is known from the
  // top left corner)
  inline void setTopRightLastBox(BOXA* bboxes, l_uint32 col);

  // Finds all of the bounding boxes within a labeled
  // image provided with the specified color
  inline BOXA* getAllColoredBoundingBoxes(const Pix* const &img, \
      LayoutEval::Color color);

  inline void addNewBox(const l_int32& x, const l_int32& y, BOXA* boxa);

  // for debugging display the particular type of region
  // found for a given image
  void dbgDrawRectangles(Pix* orig_img, \
      const Pix* const gt_img, BOXA* boxes, \
      const l_uint32& img_index, string groupname);

  /****************************************
  * Data members                         **
  ****************************************/
  int layoutruns; // keeps track of how many times layout analysis
                  // has been run (should only happen once per object)
  string topdir; // top directory of file structure to be processed
  string subdir; // the name of the subdirectory file structure
  string ext; // image extension (ie png, jpg, etc)
  string groundtruthdir; // this points to [topdir]/groundtruth/[subdir]/
  string groundtruthtxt; // this points to [topdir]/groundtruth/[GroundTruth.dat]
  string gt_blobsdir; // points to [topdir]/groundtruth/[subdir]/colorblobs/
  string inputdir; // this points to [topdir]/input/[subdir]/
  string outputdir; // this points to [topdir]/output/[subdir]/
  vector<string> output_result_subdirs; // this is the name of the subdirectory
                               // which will be placed in
                               // [topdir]/output/[subdir]/ upon completion
                               // of layout analysis. This name is
                               // specified as an argument to runTessLayout().
                               // It could either correspond to a modified
                               // Tesseract result or the default ones, but
                               // the same name cannot be used for more than
                               // one test (this would overwrite a previous
                               // result). Thus a new directory name is allocated
                               // on each run, and if it matches a previous one
                               // then an exception is thrown.
  int numfiles; // this is the number of files in the [topdir]/input/[subdir]/
                // (the number of files in the testset)
  l_uint32 thickness; // thickness of a line while grabbing colored bounding boxes
  vector<string> boolparams; // array of true/false tesseract parameters
  vector<string> intparams; // array of int tesseract params
  string page_seg_mode; // tesseract's page segmentation mode
                        // (rather than just resetting all of them on each iteration)
  TessBaseAPI api; // the Tesseract API
  EquationDetectBase* new_equ_detector;
  //EquationDetectorSVM* equ;
};

// TODO: Move all defunct code to some other file (incase it may
// come in handy later.. and also to tidy up this one a bit...)
/*
  // Returns bounding box corresponding to labeled groundtruth image
  // by following the edges of the rectangle and marking processed
  // pixels. arg1: starting row, arg2: starting column, arg3:
  // the image of iterest, arg4: bounding box color to follow in
  // the image. WARNING: the limitation of this is that no topleft
  // corner of a bounding box may overlap another. Check for these
  // and try to prevent them while creating the initial groundtruth
  BOX* getColoredBoundingBox(const l_uint32& srow, const l_uint32& scol, \
      const PIX* const &image, const LayoutEval::Color& color);
*/

/*  map<l_uint32, bool> pixel_markers; // quick indexing to which pixels
                                     // have been processed within a PIX structure
                                     // NOTE: This was retarded because it took
                                     // up way too much memory
  vector<l_uint32> markedpix; // list of the pixels that will have to be reset*/

/*
// Helper function for getColoredBoundingBox which searches right
// or downward to find the rightmost or bottom-most point respectively
inline void followBBox(l_uint32& row_or_col, \
    const l_uint32& inc, l_uint32** pixel, \
    const l_uint32& width_or_height, \
    const LayoutEval::Color& color, bool isrow, \
    const l_uint32& col_or_row, const l_uint32& height_or_width, \
    bool forward, bool recurse=false, bool recursemode=true, \
    bool firstpix_processed=true);
*/

/*
// mark a pixel as already having been processed
// uses pixel_markers map to keep track of what
// has and hasn't been marked markedpix list to
// keep a list of what has been marked so resetting the
// map is faster on each iteration
inline void markPixel(l_uint32 index);
*/

/*
inline void resetPixelMarkers(const Pix* const &img);
*/

#endif /* DOCUMENTLAYOUTTEST_H_ */
