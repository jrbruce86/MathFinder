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
#include <MEDS_Types.h>
#include <equationdetectbase.h>
#include <baseapi.h> // tesseract api
#include <allheaders.h> // leptonica api
using namespace std;
using namespace tesseract;

#define debug 0

// Data structure/algorithms used for evaluation
#include "BipartiteGraph.h"

#include <Basic_Utils.h>
using namespace Basic_Utils;
#include <Lept_Utils.h>

// The DocumentLayoutTester class is designed to facilitate evaluation of
// any document layout system which detects and segments regions of a document
// image. A single DocumentLayoutTester object can test a single system on
// multiple datasets (each dataset consisting of a set of document images in
// a specified directory). In order to evaluate more than one document layout
// system, multiple DocumentLayoutTester objects should be instantiated (one for
// each system to be tested). The class assumes that the layout analysis
// system being evaluated is a component of Tesseract and is designed around that
// assumption.
template <typename MEDSType>
class DocumentLayoutTester : public Lept_Utils, TessBaseAPI {
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
  * class into some sort of comparison algorithm (or just visual
  * comparison of the results at the very least).
  **********************************************************/
  DocumentLayoutTester(EquationDetectBase* equ_detect) : layoutruns(0) {
    // set the equation detector (or just stick to the default)
    if(equ_detect) {
      new_equ_detector = equ_detect;
      eval_default = false;
    }
    else {
      new_equ_detector = NULL;
      eval_default = true;
    }

    // initialize tesseract
    api = new TessBaseAPI;
    setTesseractParams();
    api->Init("/usr/local/share/", "eng");

    // Choose the page segmentation mode as PSM_AUTO
    // Fully automatic page segmentation, but no OSD
    // (Orientation and Script Detection).
    if (!api->SetVariable(page_seg_mode.c_str(), "3"))
      tessParamError(page_seg_mode);

    // Check to make sure tesseract is in the correct page
    // segmentation mode (psm)
    int psm = 0;
    api->GetIntVariable(page_seg_mode.c_str(), &psm);
    assert(psm == PSM_AUTO);

    // Go ahead and activate equation detection
    activateBoolParam("textord_equation_detect");

    // Set the equation detector
    if(new_equ_detector)
      api->setEquationDetector(new_equ_detector);

    // initialize thresholder
    thresholder_ = new ImageThresholder();
  }

  ~DocumentLayoutTester() {
    if(thresholder_ != NULL) {
      delete thresholder_;
      thresholder_ = NULL;
    }
    if(api != NULL) {
      if(new_equ_detector != NULL) {
        cout << "Deleting the MEDS module\n";
        delete (MEDSType*)new_equ_detector; // need to typecast to delete properly
        new_equ_detector = NULL;
      }
      cout << "Deleting the api used by the evaluator\n";
      api->setEquationDetector(NULL); // avoid dangling pointer seg fault
      delete api;
      api = NULL;
    }
  }

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
  * arg3 - name of the training_set used, this distinguishes results of the same predictor
  *        trained on different data
  * arg4- the image extension (i.e. png, jpg, etc). all should be the same
  *        in a given subdir
  *
  * Assumes all images in the input dir are named 1.ext,2.ext,3.ext,.....
  * (i.e. they start from 1 not 0)
  **************************************************************************/
  void setFileStructure(string topdir_, string dataset_, string train_set_, string ext_){
    // organize the directory structure (and add directories in
    // case they may not exist yet)
    topdir = checkTrailingSlash(topdir_);
    subdir = checkTrailingSlash(dataset_);
    train_set = train_set_;
    ext = ext_;
    exec((string)"mkdir " + topdir + (string)"groundtruth/");
    groundtruthdir = topdir + (string)"groundtruth/" + subdir;
    exec((string)"mkdir " + groundtruthdir);
    groundtruthtxt = groundtruthdir + (string)"*.dat";
    groundtruthtxt = exec((string)"ls " + groundtruthtxt
          + (string)" | tail -n 1 | tr -d '\n'");
    gt_blobsdir = groundtruthdir + (string)"colorblobs/";
    exec((string)"mkdir " + gt_blobsdir);
    inputdir = topdir + (string)"input/" + subdir;
    exec((string)"mkdir " + topdir + (string)"output/");
    outputdir = topdir + (string)"output/" + checkTrailingSlash(train_set);
    numfiles = fileCount(inputdir);
    layoutruns = 0;
    assert(numfiles == DATASET_SIZE);
    exec((string)"mkdir " + outputdir);
    outputdir = outputdir + subdir;
    exec((string)"mkdir " + outputdir);
  }

  /**********************************************************
  * Go ahead and enable both all int and all bool params    *
  **********************************************************/
  void activateAllParams() {
    activateAllBoolParams();
    activateAllIntParams();
  }

  /**********************************************************
  * Go ahead and enable all of the true/false params        *
  **********************************************************/
  void activateAllBoolParams() {
    for (unsigned int i = 0; i < boolparams.size(); i++)
      activateBoolParam(boolparams[i]);
  }

  // Turn on equation debugging in Tesseract so it outputs
  // the results of layout analysis
  void activateEquOutput() {
    activateBoolParam((string)"equationdetect_save_spt_image");
    activateBoolParam((string)"equationdetect_save_seed_image");
    activateBoolParam((string)"equationdetect_save_merged_image");
  }

  /**********************************************************
  * Go ahead and enable all of the integer params           *
  **********************************************************/
  void activateAllIntParams() {
    for (unsigned int i = 0; i < intparams.size(); i++)
      activateIntParam(intparams[i]);
  }

  /****************************************************************************
  * Turn off all params that require the scroll view                          *
  ****************************************************************************/
  void deActivateScrollView() {
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
    deActivateBoolParam("textord_tabfind_show_columns");
  }

  /****************************************************************************
  * Activate all the parameters accept for those that involve the scrollview
  ****************************************************************************/
  void activateNonScrollView() {
    activateAllBoolParams();
    deActivateScrollView();
  }

  /****************************************************************************
  * Tell Tesseract to set a bool parameter to true
  ****************************************************************************/
  void activateBoolParam(string param) {
    if (!api->SetVariable(param.c_str(), "true"))
      tessParamError(param);
  }

  /****************************************************************************
  * Tell Tesseract to set an int parameter to 1
  ****************************************************************************/
  void activateIntParam(string param) {
    if (!api->SetVariable(param.c_str(), "5")) // for now use high debug level...
      tessParamError(param);
  }

  /****************************************************************************
  * Tell Tesseract to set a bool parameter back to false
  ****************************************************************************/
  void deActivateBoolParam(string param) {
    if (!api->SetVariable(param.c_str(), "false"))
      tessParamError(param);
  }

  /****************************************************************************
  * Tell Tesseract to set an int parameter back to zero
  ****************************************************************************/
  void deActivateIntParam(string param) {
    if (!api->SetVariable(param.c_str(), "0"))
      tessParamError(param);
  }

  /*************************************************************************
  * Run either the modified version of Tesseract (I am overriding the equation
  * detection module in order to improve it) or the default version on all of
  * the images that are in the [topdir]/input/[subdir]/ and put the results in
  * [topdir]/output/[train_set]/[subdir]/[output_results_subdir_]/. The
  * output_results_subdir_ is specified as an argument to this method.
  * The method can be run any number of times so that multiple modified versions
  * of Tesseract may be tested, however the same name cannot be used for the
  * results subdirectory. Thus the names are all stored after each run and
  * if the same name is seen more than once then an exception is thrown.
  * WARNING: All output .png images (and maybe .tifs?) from tesseract get
  *          cut and pasted to the test set's output directory!! So if you
  *          are expecting a debug image to come from Tesseract then
  *          look there to find it!!
  *************************************************************************/
  void runTessLayout(string out_res_subdir, bool layout_alreadydone=false) {
    layoutruns = 0;
    out_res_subdir = checkTrailingSlash(out_res_subdir);
    string out_res_dir = outputdir + out_res_subdir;
    if(!layout_alreadydone) {
      for(int i = 0; i < output_result_dirs.size(); i++) {
        if(output_result_dirs[i] == out_res_subdir) {
          cout << "ERROR: Duplicate test output directory detected.\n"
              << "       Cannot use the same output directory name without\n"
              << "       overriding previous results!\n";
          assert(false);
        }
      }
      output_result_dirs.push_back(out_res_dir);
      exec((string)"rm -r " + out_res_dir); // remove everything if it exists
      exec((string)"mkdir " + out_res_dir); // make anew
    }
    // all_results_dir is located at
    // [topdir]/output/[train_set]/[subdir]/[out_res_subdir]
    // allresults dir holds all images output by the layout analysis
    // module, even ones which may not be relevant
    string all_results_dir = out_res_dir + (string)"allresults/";
    if(!layout_alreadydone)
      exec((string)"mkdir " + all_results_dir);

    string inputfile_name; // name of given input file
    string inputfile_path; // the path to the given input file
    string outputfile_path; // path to the output for a given input file

    if(!layout_alreadydone) {
      for (int i = 1; i <= numfiles; i++) {
        // set paths
        inputfile_name = intToString(i) + ext;
        inputfile_path = inputdir + inputfile_name;
        outputfile_path = all_results_dir + inputfile_name + "/";
        exec((string)"mkdir " + outputfile_path);
        cout << "out filepath: " << outputfile_path << endl;

        // read image and run layout analysis
        Pix* img = leptReadImg(inputfile_path);
        api->SetImage(img); // set the image
        api->AnalyseLayout(); // Run Tesseract's layout analysis
        pixDestroy(&img); // destroy finished image

        // move all the output images to a directory
        exec((string) "mv *.png " + outputfile_path);
        exec((string) "mv *.tif " + outputfile_path);
        if(eval_default) {
          exec((string) "cp " + outputfile_path + "*merge*" + " "
              + out_res_dir); // move all the relevent stuff up a directory
          exec((string) "mv " + "*.rect" + " "
              + out_res_dir);
        }
        else {
          exec((string) "cp " + outputfile_path + "*MEDS_DBG_IM_*" + " "
              + out_res_dir);
          exec((string) "mv " + "*.rect" + " "
              + out_res_dir);
        }
        cout << "Ran layout analysis on file " << i << " of " << numfiles << "\n";
        layoutruns++;
      }
    }
    // mathresdir is
    // [topdir]/output/[train_set]/[subdir]/[out_res_subdir]/math_results/
    string mathresdir = out_res_dir + (string)"math_results/";
 //   if(!layout_alreadydone) {
      exec((string)"mkdir " + mathresdir); // make if not already made
      if(eval_default) {
        exec((string)"mv " + out_res_dir + "*merge*" +
            " " + mathresdir); // move all relevant outputs to mathdir
        exec((string)"mv " + out_res_dir + "*.rect" + " "
          + mathresdir);
      }
      else {
        exec((string)"mv " + out_res_dir + "*MEDS_DBG_IM_*" + " "
            + mathresdir);
        exec((string)"mv " + out_res_dir + "*.rect" + " "
            + mathresdir);
      }
      // now run script to rename all the images
      exec((string)"cp " + topdir + (string)"rename_pngs"
          + " .");
      exec((string)"chmod +x rename_pngs", true);
      string tmpdir = mathresdir + (string)"tmp/";
      exec((string)"mkdir " + tmpdir);
      cout << "Renaming the output tif's/png's in " << mathresdir
          << " so that they are simply 1.tif, 2.tif, etc.\n";
      exec_display((string)"./rename_pngs " + mathresdir
          + (string)" " + tmpdir + (eval_default ? (string)" tif" : (string)" png"));
      exec((string)"rm " + mathresdir +  (eval_default ? (string)"*tif*" : (string)"*png*"));
      exec((string)"mv " + tmpdir + (eval_default ? (string)"*tif*" : (string)"*png*")
          + (string)" " + mathresdir);
      exec((string)"rm -r " + tmpdir);
      exec((string)"rm rename_pngs");
  //  }

    //------------------------------------------------------------------------------
    // The following only applies to evaluation of Tesseract's default Equation Detector
    //------------------------------------------------------------------------------
    // Now convert the results into ones that will be useful for
    // pixel-accurate evaluation purposes (change the color of detected
    // foreground math regions such that they use the same convention as
    // the groundtruth: red = displayed, green = label, blue = inline).
    // This is complicated by the fact that Tesseract inherently uses
    // a different coloring convention for their results:
    // Red = displayed, Green = inline, Blue = non-math.
    // Thus for Tesseract results, all that is significant is Red and
    // Green bounding boxes. Blue ones can be ignored.
    // --This also writes all the bounding boxes of the colored regions
    // to a file.
   /* if(eval_default) {
      vector<LayoutEval::Color> significantcolors;
      significantcolors.push_back(LayoutEval::RED);
      significantcolors.push_back(LayoutEval::GREEN);
      colorFoundBlobs(mathresdir, inputdir, 5,
          significantcolors, (string)".tif");
    }*/
  }

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
  * system created in this word will be compared in a separate method.            *
  *                                                                               *
  * If runTessLayout has not been run then throws an exception. Also assumes      *
  * that the groundtruth data is available in the subdir of interest (the one     *
  * for which layout analysis has already been run). Takes the groundtruth data   *
  * and uses it to color the blobs of interest appropriately based on their type. *
  * See setFileStructure() for more details                                       *
  * on the directory structure. Beneath the topdir/ there is not only the         *
  * groundtruth/ but also input/ and output/ which contains the same subdirs as   *
  * in the groundtruth. For instance, for subdir1/ there will be a subdir1/ in    *
  * the input/ directory with all of the input images. There will also be a       *
  * subdir1/ in the output/ directory which will have the results that need to be *
  * evaluated for the given dataset represented by the groundtruth.               *
  * The subdir1/ located in the groundtruth/ is used to evaluate                  *
  * the results for a given test in subdir1/ located in the output/.              *
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
  vector<vector<HypothesisMetrics> > evalTessLayout(string testname_,
      bool layoutdone=false) {
    const string testname = checkTrailingSlash(testname_);
    if(!layoutdone) {
      if(layoutruns != numfiles) {
        cout << "ERROR: evalTessLayout requires layout analysis to"
            << " be done in advance on all " << numfiles << " of the "
            << "input files for the current test. Layout analysis has been done "
            << "for " << layoutruns << " of them.\n";
        assert(false);
      }
    }
    // make sure the groundtruth is ready by making sure there is a
    // file in gt_blobsdir for each input image
    bool groundtruthready = true;
    for(int i = 1; i <= numfiles; i++) {
      string filename = gt_blobsdir + intToString(i) + ext;
      if(!existsFile(filename)) {
        groundtruthready = false;
        break;
      }
    }
    if(groundtruthready) {
      // check to make sure the GroundTruth.dat file exists where it should
      if(!existsFile(groundtruthtxt)) {
        cout << "ERROR: a file ending with the .dat extension is required in "
             << groundtruthdir << endl;
        assert(false);
      }
    }
    if(!groundtruthready) {
      cout << "Groundtruth appears not to have been prepared yet!"
           << " Would you like to prepare it now (y/n)?\n";
      char answer[] = "?";
      while(answer[0] != 'y' && answer[0] != 'n')
        cin >> answer;
      if(answer[0] == 'n') {
        cout << "Cannot continue evaluation without preparing the "
             << "Groundtruth images. Exiting now!\n";
        assert(false);
      }
      else {
        cout << "Preparing the Groundtruth images...\n";
        colorGroundTruthBlobs();
      }
    }

    // Make sure the hypothesis test results are ready for evaluation
    const string results_type = "math_results/";
    const string colorblobs = "colorblobs/";
    string eval_dir_;
  //  if(eval_default)
   //   eval_dir_ = outputdir + testname + results_type
   //     + colorblobs;
  //  else
    eval_dir_ = outputdir + testname + results_type;
    const string evaldir = eval_dir_;
    const string hypboxfile = evaldir + (string)"Rectangles.dat";
    if(!existsDirectory(evaldir)) {
      cout << "ERROR: The directory, " + evaldir << ", doesn't exist!\n";
      assert(false);
    }
    bool readyfortest = true;
    string tmp;
    for(int i = 1; i <= numfiles; ++i) {
      tmp = evaldir + intToString(i) + (eval_default ? (string)".tif" : ext);
      if(!existsFile(tmp)) {
        readyfortest = false;
        break;
      }
    }
    if(readyfortest) {
    //  if(eval_default) {
        // make sure the Rectangles.dat file is where it should be
   //     if(!existsFile(hypboxfile)) {
   //       cout << "ERROR: the file " << hypboxfile << ", couldn't be found!\n";
   //       exit(EXIT_FAILURE);
   //     }
  //    }
  //    else {
        // move all the contents in the .rect files to the Rectangles.dat
        exec((string)"rm " + hypboxfile); // start anew
        ofstream hboxstream(hypboxfile.c_str());
        vector<string> rects;
        for(int i = 1; i <= numfiles; ++i) {
          string curboxfile = evaldir + intToString(i) + (string)".rect";
          ifstream s(curboxfile.c_str());
          if(!s.is_open()) {
            cout << "ERROR: Could not open boxfile " << curboxfile << endl;
            assert(false);
          }
          int maxlen = 100;
          char ln[maxlen];
          while(!s.eof()) {
            s.getline(ln, maxlen);
            if(!s)
              continue;
            rects.push_back((string)ln);
          }
        }
        // get rid of any duplicates
        for(int i = 0; i < rects.size(); ++i) {
          for(int j = i+1; j < rects.size(); ++j) {
            if(rects[i] == rects[j]) {
              rects.erase(rects.begin() + j);
              --j;
            }
          }
        }
        // write to file
        for(int i = 0; i < rects.size(); ++i)
          hboxstream << rects[i] << endl;
   //   }
    }
    if(!readyfortest) {
      cout << "ERROR: The following file, " << tmp << " is missing!\n";
      assert(false);
    }

    // Now we are ready to carry out the evaluation!
    vector<vector<HypothesisMetrics> > all_page_metrics;
    for(int i = 1; i <= numfiles; i++) {
      // page metrics can be separated out by the type of element being evaluated
      // or all combined together. in the former case there will be multiple metrics
      // per page and in the latter just one per page.
      vector<HypothesisMetrics> page_metrics;
      if(type_eval) {
        // evaluate each type separately
        HypothesisMetrics disp_metrics = getEvaluationMetrics(testname_,
            results_type, (string)"displayed", i, hypboxfile);
        page_metrics.push_back(disp_metrics);
        HypothesisMetrics emb_metrics = getEvaluationMetrics(testname_, results_type,
            (string)"embedded", i, hypboxfile);
        page_metrics.push_back(emb_metrics);
        HypothesisMetrics label_metrics = getEvaluationMetrics(testname_,
            results_type, (string)"label", i, hypboxfile);
        page_metrics.push_back(label_metrics);
      }
      else {
        // evaluate all of the above types (considering them all the same)
        HypothesisMetrics all_metrics = getEvaluationMetrics(testname_,
            results_type, (string)"all", i, hypboxfile);
        page_metrics.push_back(all_metrics);
      }
      all_page_metrics.push_back(page_metrics);
    }
    assert(all_page_metrics.size() == numfiles);
    cout << "Finished with evalTessLayout()\n";
    // return the metrics for each page of the dataset being evaluated
    return all_page_metrics;
  }

  /*************************************************************************
  * For whatever test is currently being run (the test name is specified by the
  * string, "subdir", which is assumed to have already been set by a call to
  * setFileStructure()), read from it's GroundTruth.dat file to find all of
  * the rectangles and then color the foreground of the corresponding input
  * image within these rectangles based on the color of interest. The resulting
  * image(s) are then saved in [topdir]/groundtruth/[subdir]/colorblobs. These
  * images can be used for pixel accurate evaluation for layout analysis.
  *************************************************************************/
  void colorGroundTruthBlobs() {
    // open the groundtruth text file which holds all of the math rectangles
    ifstream gtfile;
    string gtfilename = groundtruthtxt;
    gtfile.open(gtfilename.c_str(), ifstream::in);
    if((gtfile.rdstate() & ifstream::failbit) != 0) {
      cout << "ERROR: Could not open Groundtruth.dat in "
           << groundtruthdir << endl;
      exit(EXIT_FAILURE);
    }

    // Now start reading and coloring
    int max = 55;
    char* curline = new char[max];
    int curfilenum = -1;
    int expectedfilenum = 1;
    Pix* curimg;
    int linenum = 1;
    while(!gtfile.eof()) {
      gtfile.getline(curline, max);
      string curlinestr = (string)curline;
      assert(curlinestr.length() < max);
      // parse the line
      if(curlinestr.empty()) {
        continue;
      }
      vector<string> splitline = stringSplit(curlinestr);

      string filename   = splitline[0];
      string recttype   = splitline[1];
      int rectleft   = atoi(splitline[2].c_str());
      int recttop    = atoi(splitline[3].c_str());
      int rectright  = atoi(splitline[4].c_str());
      int rectbottom = atoi(splitline[5].c_str());
      vector<string> tmp = stringSplit(filename, '.');
      int filenum = atoi(tmp[0].c_str());

      // open the image if it isn't already opened
      // save the previous one if applicable
      if(curfilenum != filenum) {
        cout << "Coloring groundtruth blobs for image " << filenum << endl;
        if(curfilenum != -1) {
          // save previous and then deallocate it
          string savename = gt_blobsdir + intToString(curfilenum) + ext;
          pixWrite(savename.c_str(), curimg, IFF_PNG);
          pixDestroy(&curimg);
          thresholder_->Clear();
        }
        string imgname = inputdir + intToString(filenum) + ext;
        curimg = leptReadImg(imgname);
        thresholder_->SetImage(curimg);
        if (!thresholder_->IsBinary()) {
          pixDestroy(&curimg); // thresholder has a clone so destroy this one
          thresholder_->ThresholdToPix(&curimg);
        }
        curimg = pixConvertTo32(curimg);
        curfilenum = filenum;
      }
      // now color the foreground regions of the image in the
      // rectangle based on the rectangle's type
      if(rectleft == -1 || recttop == -1 ||
          rectright == -1 || rectbottom == -1)
        continue; // if the image has nothing then it should have a single entry with -1's
      BOX* box = boxCreate(rectleft, recttop,
          rectright - rectleft, rectbottom - recttop);
      LayoutEval::Color color;
      if(recttype == "displayed")
        color = LayoutEval::RED;
      else if(recttype == "embedded")
        color = LayoutEval::BLUE;
      else if(recttype == "label")
        color = LayoutEval::GREEN;
      else {
        cout << "ERROR: Rectangle of unknown type in GroundTruth.dat!\n";
        assert(false);
      }
      fillBoxForeground(curimg, box, color);
      boxDestroy(&box);
      linenum++;
    }
    delete [] curline;
    curline = NULL;
    // save and destroy the final image
    string savename = gt_blobsdir + intToString(curfilenum) + ext;
    pixWrite(savename.c_str(), curimg, IFF_PNG);
    pixDestroy(&curimg);
    gtfile.close();
  }

  int dbgColorCount(string imname, LayoutEval::Color color) {
    PIX* im = leptReadImg(imname);
    return colorPixCount(im, color);
  }

  inline TessBaseAPI* getTessApi() {
    return api;
  }

  inline void turnOffTypeEval() {
    type_eval = false;
  }

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
  HypothesisMetrics getEvaluationMetrics(string testname, string results_type,
      string typenamespec, int filenum, string hypboxfile) {
    // set the name of the files we'll output the results to
    testname = checkTrailingSlash(testname);
    results_type = checkTrailingSlash(results_type);
    string typenamespecdir = checkTrailingSlash(typenamespec);
    string outfile_dir = outputdir + testname + results_type + typenamespecdir;
    if(!existsDirectory(outfile_dir)) {
      exec((string)"mkdir " + outfile_dir);
    }
    string outfile_dir_verbose = outfile_dir + (string)"verbose/";
    if(!existsDirectory(outfile_dir_verbose))
      exec((string)"mkdir " + outfile_dir_verbose);
    string dbgdir = checkTrailingSlash(outfile_dir) + (string)"dbg/";
    if(!existsDirectory(dbgdir))
      exec((string)"mkdir " + dbgdir);
    string this_dbgdir = checkTrailingSlash(dbgdir) + intToString(filenum) + "/";
    if(!existsDirectory(this_dbgdir))
      exec((string)"mkdir " + this_dbgdir);
    string outfile = outfile_dir + intToString(filenum) + (string)"_metrics";
    FILE* out;
    if(!(out = fopen(outfile.c_str(), "w"))) {
      cout << "ERROR: Could not create " << outfile << " file\n";
      exit(EXIT_FAILURE);
    }
    string outfile_verbose = outfile_dir_verbose + intToString(filenum) +
        (string)"_metrics_verbose";
    FILE* out_verbose;
    if(!(out_verbose = fopen(outfile_verbose.c_str(), "w"))) {
      cout << "ERROR: Could not create " << outfile_verbose << " file\n";
      exit(EXIT_FAILURE);
    }
    const string colorblobs  = "colorblobs/";
    string evaldir_;
  //  if(eval_default)
   //   evaldir_ = checkTrailingSlash(outputdir +
   //     testname + results_type + colorblobs);
   // else
      evaldir_ = checkTrailingSlash(outputdir + testname + results_type);
    const string evaldir = evaldir_;

    // The first step is to create the bipartite graph data structure
    // for the image
    string filename = intToString(filenum);
    GraphInput gi;
    gi.gtboxfile = groundtruthtxt;
    gi.gtimg = leptReadImg(gt_blobsdir + filename + ext);
    gi.hypboxfile = hypboxfile;;
    gi.hypimg = leptReadImg(evaldir + filename  + (eval_default ? (string)".tif" : ext));
    gi.imgname = filename;
    gi.dbgdir = this_dbgdir;
    Pix* inimg = leptReadImg(inputdir + filename + ext);
    inimg = pixConvertTo32(inimg);
    gi.inimg = inimg;
    BipartiteGraph pixelGraph(typenamespec, gi);

    // Now get and print the metrics
    HypothesisMetrics metrics = pixelGraph.getHypothesisMetrics();
    // move the dbg image to its dir
   // exec((string)"cp " + (string)"Eval_DBG_" + typenamespec +
   //     (string)"* " + checkTrailingSlash(dbgdir)); // TODO: Remove this
    pixelGraph.printMetrics(out);
    pixelGraph.printMetricsVerbose(out_verbose);

    // Clear the memory and close the file
    pixelGraph.clear();
    fclose(out);
    cout << "Finished evaluating " << typenamespec << " type of "
        << results_type << " for image " << filename << endl;
    return metrics;
  }

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
  void colorFoundBlobs(string labeledinputdir,
      string unlabeledinputdir, l_uint32 line_thickness,
      vector<LayoutEval::Color> significantcolors,
      string ext_, int startimg=1, int finalimg=-1) {
    labeledinputdir = checkTrailingSlash(labeledinputdir);
    assert(numfiles > 0);
    string realext = ext;
    ext = ext_;
    thickness = line_thickness;
    assert(sizeof(l_uint32) == 4); // should be 32 bits

    if(finalimg != -1) {
      assert(finalimg <= numfiles);
      assert(finalimg > 0);
    }
    else
      finalimg = numfiles; // just do all of them by default

    if(startimg != 1) {
      assert(startimg > 0);
      assert(startimg <= numfiles);
    }

    // where the colored blob images (the output of this function)
    // will be stored. see .h file comments for more details
    string colorblobsdir = labeledinputdir + (string)"colorblobs/";
    exec((string)"mkdir " + colorblobsdir, false);

    /* create the colored blobs images from the boxed ones. also
     * write the coordinates of the boxes to a file*/
    BOXAA* colorboxlist = boxaaCreate(0);
    FILE* rectfile;
    string rectfilename = colorblobsdir + (string)"Rectangles.dat";
    if(!(rectfile = fopen(rectfilename.c_str(), "w"))) {
      cout << "ERROR: Could not create the file: " << rectfilename << endl;
      exit(EXIT_FAILURE);
    }
    for(int i = startimg; i <= finalimg; i++) {
      cout << "Coloring math foreground regions for image " << i << " of "
           << finalimg << endl;
      // get the current file name
      string filename = intToString(i);

      /* read in the groundtruth image (has regions of interest
       * enclosed by colored bounding boxes) */
      // ground truth image (read only!!!)
      string full_labeled_pathname = labeledinputdir + filename + ext;
      Pix* rect_labeled_im = leptReadImg(full_labeled_pathname);

      // copy the original unlabeled document image (edit the copied
      // one so the original is unharmed) reads in the copied image
      // and colors the blobs of interest on that one
      string inputimpath = unlabeledinputdir + filename + realext; // input image name
      exec("rm " + colorblobsdir + filename + realext, false); // delete existing (if exists)
      exec("cp " + inputimpath + " " + colorblobsdir, true);
      string fulleditpathname = colorblobsdir + filename + realext; // new image
      Pix* color_blobs_im = leptReadImg(fulleditpathname); // read/write
      color_blobs_im = pixConvertTo32(color_blobs_im); // make sure its rgb

      // make sure the groundtruth and input image have the
      // same dimensions
      assert(rect_labeled_im->w == color_blobs_im->w
          && rect_labeled_im->h == color_blobs_im->h);

      // find the colored bounding boxes in the rect_labeled_im
      // find the boxes for each color (red is displayed eq, blue
      // is inline eq, and green is labeled)
      BOXA* curboxlist;
      int j = 0;
      for (vector<LayoutEval::Color>::iterator it = significantcolors.begin();
          it != significantcolors.end(); ++it) {
        curboxlist = getAllColoredBoundingBoxes(rect_labeled_im, *it);
        boxaaInsertBoxa(colorboxlist, (l_int32)j, curboxlist);
        j++;
      }

      // color the foreground inside all of the boxes that
      // were just found. also write the coordinates to
      // a file for quick access to the rectangles
      j = 0;
      for (vector<LayoutEval::Color>::iterator it = significantcolors.begin();
          it != significantcolors.end(); ++it) {
        LayoutEval::Color color = *it;
        if(color == LayoutEval::GREEN)
          color = LayoutEval::BLUE; // tesseract labels inline as green, i use blue
        BOXA* curboxlist = boxaaGetBoxa(colorboxlist, (l_int32)j, L_CLONE);
        color_blobs_im = fillBoxesForeground(color_blobs_im,
            curboxlist, color);
        writeBoxesToFile(rectfile, curboxlist, color, filename + ext);
        j++;
      }

      // save the resulting image
      string savename = colorblobsdir + filename + realext;
      pixWrite(savename.c_str(), color_blobs_im, IFF_PNG);

      // Avoid memory leaks!!!!
      boxaDestroy(&curboxlist);
      pixDestroy(&color_blobs_im);
      pixDestroy(&rect_labeled_im);
      // clear all the box arrays after each iteration
      l_int32 numboxes = boxaaGetCount(colorboxlist);
      for(l_int32 j = 0; j < numboxes; j++) {
        BOXA* curboxes = boxaaGetBoxa(colorboxlist, j, L_CLONE);
        boxaClear(curboxes);
      }
    }
    ext = realext; // put the ext back the way it was originally
    fclose(rectfile);
  }

  // write the boxes in box list to the file in the following
  // format:
  // [imagename][ext] [type] [left] [top] [right] [bottom]
  void writeBoxesToFile(FILE* file, BOXA* boxes,
      LayoutEval::Color color, string imname) {
    string type;
    switch (color) {
    case LayoutEval::RED : type = (string)"displayed"; break;
    case LayoutEval::BLUE : type = (string)"embedded"; break;
    case LayoutEval::GREEN : type  = (string)"label"; break;
    default : cout << "ERROR Unknown type\n"; exit(EXIT_FAILURE); break;
    }
    int numboxes = (int)boxaGetCount(boxes);
    for(int i = 0; i < numboxes; i++) {
      l_int32 t, l, w, h; // top, left, width, height
      BOX* curbox = boxaGetBox(boxes, (l_int32)i, L_COPY);
      boxGetGeometry(curbox, &l, &t, &w, &h);
      string left = intToString((int)l);
      string top = intToString((int)t);
      string right = intToString((int)l + (int)w);
      string bottom = intToString((int)t + (int)h);
      string _ = (string)" ";
      string txt = imname + _ + type + _ + left + _ + top +
          _ + right + _ + bottom + (string)"\n";
      fprintf(file, txt.c_str());
    }
  }

 /****************************************
  * For Tesseract parameter manipulation *
  ***************************************/
  // set up the parameter list (they are hardcoded here)
  void setTesseractParams() {
    /******************* Massive List of Parameters to Play With!!!!*********************************/
    // WARNING: THESE ARE HARDCODED.. DON'T MODIFY!!!!!!!!!!!!
    ////////////Settings paramaters////////////
    // This just lists all of the possible settings. They are set or unset later.
    //boolparams.reserve(20);

    ////////////textordering debug boolparams ////////////
    // Should display a window showing the intial tabs detected in FindInitialTabVectors
    boolparams.push_back((string)"textord_tabfind_show_initialtabs");
    // Enables their table detection!! Class for this is TableFinder in textord/tablefind.h.
    boolparams.push_back((string)"textord_tabfind_find_tables");
    // In order to see a window display of the tables detected!
    // This is run after the equation detection if enabled.
    boolparams.push_back((string)"textord_show_tables");
    // Displays blobs rejected as noise prior to equation or tabledetection
    boolparams.push_back((string)"textord_tabfind_show_reject_blobs");
    // This will show the partitions prior to the equation and tabledetection
    boolparams.push_back((string)"textord_tabfind_show_initial_partitions");
    // This will show the partitions after equation and table detection
    boolparams.push_back((string)"textord_tabfind_show_partitions");
    // Use greyed background for debug images
    boolparams.push_back((string)"textord_debug_images");
    // Print tabfind related debug messages
    boolparams.push_back((string)"textord_debug_tabfind");
    // Displays blob and block bounding boxes in a window called “Blocks”.
    // This is after equation and table detection. Also occurs during  setupandfilternoise,
    // which occurs before findblocks is called in order to filter out noise.
    boolparams.push_back((string)"textord_tabfind_show_blocks");
    // Display unsorted blobs during call to filterblobs made within textordpage which is
    // called after autosegmentation is carried out.
    // Displays all the blobs color-coded at ones.
    boolparams.push_back((string)"textord_show_blobs");
    // Displays “boxes” this displays each type of blob
    // (small, noise, big, medium size) one at a time.
    boolparams.push_back((string)"textord_show_boxes");
    // Displays the results of ColumnFinder
    boolparams.push_back((string)"textord_tabfind_show_columns");

    ////////////tessedit debug boolparams////////////////////
    // dump intermediate images during page segmentation
    boolparams.push_back((string)"tessedit_dump_pageseg_images");

    ///////////equationdetect debug boolparams////////////////////
    // display the BSTT's
    boolparams.push_back((string)"equationdetect_save_spt_image");
    // save the results of pass 2 (identifyseedparts)
    boolparams.push_back((string)"equationdetect_save_seed_image");
    //  save the final math detection results
    boolparams.push_back((string)"equationdetect_save_merged_image");

    // print table detection output (requires input file to be named "test1.tif")
    boolparams.push_back((string)"textord_dump_table_images");

    // Change from default page segmentation mode (single block)
    // to the more advanced auto-segmentation, which includes
    // the experimental equation detection module: 3 = PSM_AUTO
    page_seg_mode = (string)"tessedit_pageseg_mode";

    // int params
    intparams.push_back((string)"textord_tabfind_show_strokewidths");
    intparams.push_back((string)"textord_debug_tabfind");
    // Should display images detected as distinct from text by FindImagePartitions
    intparams.push_back((string)"textord_tabfind_show_images");
    /*********************End of Massive Parameter List!*********************************/
  }

  // Error message for Tesseract paramaters which don't exist
  void tessParamError(string param) {
    cout << "ERROR: Tesseract could not find a paramater called, " << param
         << endl;
    exit(EXIT_FAILURE);
  }

  /****************************************
   * Result Box Detection Utilities (for detecting
   * the already labeled rectangles on the output of
   * Tesseract correctly)
   ***************************************/
  // Deals with overlapping rectangles
  inline void detectOverlappingRect(const BOX* const &box,
      l_uint32* &pixel, const l_uint32& pixwidth,
      const l_uint32& pixheight, const l_uint32& row, l_uint32& col,
      const LayoutEval::Color& color, bool& overlapfound,
      BOXA* processingboxbin, BOXA* finishedboxbin, bool& onbottom,
      bool& looking_for_tr) {
    // look for overlap going downward (top of lower rect may be attached to bottom of current rect)
    // or look for overlap going upward (bottom of upper rect may be attached to top of current rect)
    LayoutEval::Color pixbeloworabove;
    if(onbottom)
      pixbeloworabove = getRGBBelow((l_int32)row, pixel, pixwidth);
    else
      pixbeloworabove = getRGBAbove((l_int32)row, pixel, pixwidth);
    if(pixbeloworabove == color && !overlapfound) {
      if(debug)
        cout << "Overlap detected on bottom/top of rect at x: " << col << ", y: " << row << endl;
      // found the top/bottom left of an overlapping rectangle on the bottom/top
      // of current one. go ahead and look for its top right if looking at the bottom
      // of current rectangle. if looking at the top of current rectangle then we already
      // have the top parts (and now have the bottom) of the overlapping rectangle and
      // need to remove it from the processing bin and add it to the finished bin with
      // the updated information
      if(onbottom)  {
        if(debug)
          cout << "possible top left corner detected at x: " << col << ", y: " << row << endl;
        // make sure it isn't already being processed first!
        if(isBeingProcessed(col, processingboxbin)) {
          if(debug)
            cout << "found its actually already being processed, so moving on\n";
          overlapfound = true;
          return;
        }
        overlapfound = true;
        // top left corner of a rectangle detected!
        addNewBox((l_int32)col, (l_int32)row, processingboxbin);
        // need to go ahead and find the tr for this one before causing any confusion
        // this is because of how QT draws things
        // figure out what the offset is from the overlap to the bottom of the new rect's line
        //cout << " at x: " << col << endl;
        l_uint32* stpx = (l_uint32*)pixel + thickness + 1; // move to the right by the thickness plus one
        l_uint32 tmpindex_x = col + thickness + 1;
        l_uint32 tmpindex_y = row;// + 2;
        l_uint32* tmppix = stpx;
        l_int32 offset = 0;
        for(l_int32 k = tmpindex_y; k < pixheight; k++) {
          tmppix = stpx + (pixwidth*offset); // go down to the offset
          // get the current pixel's color
          LayoutEval::Color c = getPixelColor(tmppix);
          if(c != color) {
            tmpindex_y += offset;
            break;
          }
          offset++;
        }
        stpx = tmppix;
        bool trfound = false;
        for(l_uint32 k = tmpindex_x; k < box->x+box->w; k++) {
          tmppix = stpx + (k-tmpindex_x);
          // need to look down to find the top right
          // in case the top right of the box is hidden
          // by the bottom rectangle it overlaps
          //cout << "checking color below x: " << k << ", y: " << tmpindex_y << endl;
          LayoutEval::Color colorbelow = getRGBBelow(tmpindex_y, tmppix, pixwidth);
          if(colorbelow == color) {
            // top right detected!
            trfound = true;
            if(debug)
              cout << "top right detected at x: " << k+thickness << ",y : " << tmpindex_y << endl;
            setTopRightLastBox(processingboxbin, k+thickness);
            break;
          }
        }
        if(!trfound)
          looking_for_tr = true;
      }
      else { // if on the top of the current rectangle, we have bottom left of overlapping
        if(debug)
          cout << "detected possibly the bottom left of an overlapping rectangle at x: " <<
            col << ", y: " << row << endl;
        // need to find the rectangle in the processing

        if(!isBeingProcessed(col, processingboxbin)) {
          if(debug)
            cout << "Can't be a bottom left, trying bottom right instead\n";
          l_uint32* tmppx = (l_uint32*)pixel;
          l_uint32 tmprow = (l_uint32)row;
          l_uint32 tmpcol = (l_uint32)col;
          tmpcol += thickness; // move to what may be a pixel from the outside
          tmppx += thickness;
          bool topfound = false;
          // follow up to the top
          for(l_int32 k = 0; k < (l_int32)pixheight - (l_int32)row; k++) {
            tmprow = row - k;
            tmppx = (l_uint32*)pixel - (pixwidth*(l_int32)k);
            if(getPixelColor(tmppx) != color) {
              tmprow++;
              tmppx += pixwidth;
              topfound = true;
              break;
            }
          }
          if(debug)
            cout << "moved up to row " << (l_int32)tmprow << endl;
          if(!topfound) {
            cout << "ERROR: label goes beyond top of image!\n";
            exit(EXIT_FAILURE);
          }
          if(getPixelColor(tmppx-thickness) == color) {
            // bottom right found.. defer for later processing
            pixel = pixel + tmpcol + 1 - col;
            col = tmpcol + 1;
            if(debug)
              cout << "moving the col forward to " << tmpcol << endl;
            looking_for_tr = true;
            return;
          }
          else {
            if(debug)
              cout << "Was not the bottom right either... At a loss.\n";
          }
        }
        else {
          if(debug)
            cout << "still considering potential bottom left.. looking down to see where it ends\n";
          // look down the thickness
          bool bottomleftfound = false;
          l_uint32* tmppx = (l_uint32*)pixel;
          l_uint32 tmprow = (l_uint32)row;
          l_uint32 tmpcol = (l_uint32)col;
          for(l_uint32 k = 0; k < thickness+2; k++) {
            if(getPixelColor(tmppx) != color) {
              bottomleftfound = true;
              break;
            }
            tmprow = row + k;
            tmppx = pixel + (k*pixwidth);
          }
          if(bottomleftfound) {
            if(debug)
              cout << "found bottom left at x: " << tmpcol << ", y: " << tmprow << endl;
            if(!finishRectangle(processingboxbin, finishedboxbin, col, tmprow)) {
              cout << "ERROR: Could not finish a rectangle that is being processed!\n";
              exit(EXIT_FAILURE);
            }
          }
        }
        overlapfound = true;
      }
    }
  }

  // determine whether or not a box at the column given is already being processed
  bool isBeingProcessed(const l_uint32& col, const BOXA* const &processingboxbin) {
    BOX* bbox;
    l_int32 boxcount = boxaGetCount((BOXA*)processingboxbin);
    if(boxcount < 1)
      return false;
    bool success = false;
    for(l_int32 k = 0; k < boxcount; k++) {
      bbox = boxaGetBox((BOXA*)processingboxbin, k, L_COPY);
      if(bbox->x == col) {
        success = true;
        break;
      }
    }
    boxDestroy(&bbox);
    return success;
  }

  // Adds the bottom left of the rectangle (the final information needed)
  // and move s the completed rectangle from the processing bin to the
  // completed bin
  bool finishRectangle(BOXA* processingboxbin, BOXA* finishedboxbin,
      l_uint32 col, l_uint32 row) {
    bool success = false;
    l_int32 pboxcount = boxaGetCount(processingboxbin);
    for(l_int32 k = 0; k < pboxcount; k++) {
      BOX* bbox = boxaGetBox(processingboxbin, k, L_CLONE);
      if(bbox->x == col) {
        // this is the one, add it to the processed boxes with
        // the height that was just found
        if(debug)
          cout << "bottom left corner detected at x: " << col << ", y: " << row << endl;
        bbox->h = row - bbox->y;
        if(debug)
          cout << "finishing a rectangle whose topleft is (x,y) = ("
          << bbox->x << ", " << bbox->y << "). and its bottomright is " <<
          "(x,y) = " << bbox->x+bbox->w << ", " << bbox->y+bbox->h <<
          ")\n";
        boxaAddBox(finishedboxbin, bbox, L_CLONE);
        boxaRemoveBox(processingboxbin, k);
        success = true;
        boxDestroy(&bbox);
        break;
      }
    }
    return success;
  }

  // Sets the top right corner of the box at the top of the list
  // to the given column (the y coordinate is known from the
  // top left corner)
  inline void setTopRightLastBox(BOXA* bboxes, l_uint32 col) {
    l_int32 boxindex = boxaGetCount(bboxes) - 1;
    BOX* bbox = boxaGetBox(bboxes, boxindex, L_CLONE);

    // At this point already one pixel outside so just adding one
    if(debug)
      cout << "top right corner detected at x: " << col << ", y: " << bbox->y << endl;
    l_int32 w = (l_int32)col - bbox->x;
    boxSetGeometry(bbox, bbox->x, bbox->y, w, 0);
    boxDestroy(&bbox); // destroy clone
  }

  // Finds all of the bounding boxes within a labeled
  // image provided with the specified color
  inline BOXA* getAllColoredBoundingBoxes(const Pix* const &img,
      LayoutEval::Color color) {
    // first initialize a redblack tree for quick access into which pixels
    // have been marked and unmarked for the current groundtruth image
    l_uint32 pixheight, pixwidth;
    pixGetDimensions((Pix*)img, (int*)&pixwidth, (int*)&pixheight, NULL);

    // first need to find all of the top-left corners and which ones overlap
    // with other rectangles. the overlapping is only problematic when the
    // top-left corner of a rectangle overlaps with the bottom of another.
    // thus, it is necessary to specify which rectangle (represented here
    // as a top-left corner) overlaps the top-left corner of another rectangle.
    // either the bottom or right of the rectangle may overlap (the other
    // conditions are severe groundtruthing errors that are not going
    // to be accounted for here.
    l_uint32* startpixel = img->data; //pixGetData((Pix*)img);
    rgbtype rgb[3];
    LayoutEval::Color prevpixcolorx = LayoutEval::NONE;
    LayoutEval::Color prevpixcolory = LayoutEval::NONE;
    BOXA* finishedboxbin = boxaCreate(0);
    BOXA* processingboxbin = boxaCreate(0);
    bool overlapfound = false; // true if overlap was already found on bottom of rectangle
    bool looking_for_tr = false; // looking for the top right if true
    bool on_bottom_rect = false; // currently at the bottom of a rect if true
    bool continuingbottomrect = false; // true if still searching for tr along br after br ended
    l_int32 bottom_rect_index = -1;
    for(l_uint32 i = 0; i < pixheight; i++) {
      for(l_uint32 j = 0; j < pixwidth; j++) {
        l_uint32 index = i*pixwidth + j;
        l_uint32* curpixel = startpixel + index;
        getPixelRGB(curpixel, rgb);
        LayoutEval::Color pixcolor = getColor(rgb);
        if(looking_for_tr) {
          if(pixcolor == color && j < (pixwidth-1) && !on_bottom_rect && !continuingbottomrect) {
            // check for overlap
            l_int32 topindex = boxaGetCount(processingboxbin) - 1;
            BOX* bbox = boxaGetBox(processingboxbin, topindex, L_CLONE);
            detectOverlappingRect(bbox, curpixel, pixwidth, pixheight, i, j,
                color, overlapfound, processingboxbin, finishedboxbin, on_bottom_rect,
                looking_for_tr);
            boxDestroy(&bbox);
            continue; // pix color hasn't changed keep looking
          }
          // pix color changed (or on the bottom of another rect). potential top right found
          if(isColorSignificant(pixcolor)) {
            // need to go down until we get back to our own territory
            if(debug)
              cout << "found a potential top right, at x: " << j
                   << ", but not sure if it's just an overlapping rectangle or not\n";
            l_uint32* tmppix = curpixel;
            l_uint32 tmpindex_x = j;
            l_uint32 tmpindex_y = i;
            if(debug)
              cout << "looking down the thickness of the line to see if it is just partially overlapped\n";
            bool partialyoverlap = false;
            bool partialxoverlap = false;
            for(l_uint32 k = 0; k < thickness; k++) {
              if(getPixelColor(tmppix) == color) {
                if(debug)
                  cout << "color changed back to normal at y: " << tmpindex_y << endl;
                partialyoverlap = true;
                break;
              }
              tmppix = curpixel + (k*pixwidth);
              tmpindex_y = i + k;
            }
            // see if it is just overlapping by moving to the right
            if(tmpindex_y >= i + thickness - 1) {
              if(debug)
                cout << "not partially overlapped down the thickness, moving back up and looking to the right instead\n";
              l_uint32* startpx = curpixel + pixwidth;
              tmpindex_y = i+1;
              for(l_uint32 k = 0; k < thickness + 1; k++) {
                if(getPixelColor(tmppix) == color) {
                  if(debug)
                    cout << "color back to normal at x: " << tmpindex_x << endl;
                  partialxoverlap = true;
                  break;
                }
                tmppix = startpx + k;
                tmpindex_x = j + k;
              }
            }
            l_uint32* stpx = tmppix;
            // now we know we have a top right if there wasn't overlap
            if(!partialyoverlap && !partialxoverlap) {
              if(debug)
                cout << "top right detected at x: " << tmpindex_x << ", y: " << tmpindex_y << endl;
              setTopRightLastBox(processingboxbin, tmpindex_x);
              looking_for_tr = false;
              overlapfound = false;
              continuingbottomrect = false;
            }
            else {
              if(debug)
                cout << "no top right detected at x: " << j << ". it was only overlap\n";
            }
          }
          else {
            if(debug)
              cout << "top right detected at x: " << j << ", y: " << i << endl;
            setTopRightLastBox(processingboxbin, j);
            looking_for_tr = false;
          }
        }
        if(on_bottom_rect) {
          BOX* bbox = boxaGetBox(finishedboxbin, bottom_rect_index, L_COPY);
          if((l_int32)j < (bbox->x+bbox->w-thickness)) {
            detectOverlappingRect(bbox, curpixel, pixwidth, pixheight, i, j,
                color, overlapfound, processingboxbin, finishedboxbin, on_bottom_rect,
                looking_for_tr);
            l_int32 boxsize = boxaGetCount(finishedboxbin);
            prevpixcolorx = pixcolor;
            boxDestroy(&bbox);
            continue;
          }
          if(debug)
            cout << "found bottom right at col " << j << ", row " << i << endl;
          on_bottom_rect = false;
          overlapfound = false;
          if(looking_for_tr)
            continuingbottomrect = true;
          boxDestroy(&bbox);
      /*    // check to see if there's something to the right!
          l_uint32 tmpcol = j;
          l_uint32* tmppx = curpixel;
          tmpcol += thickness + 1; // this is the rightmost of the rectangle
          tmppx += thickness + 1;
          tmpcol++; // move outside
          tmppx++;
          if(getPixelColor(tmppx) == color) {
            if(dbg)
              cout << "something was detected to the right!!"
          }*/
        }
        prevpixcolory = getRGBAbove(i, curpixel, pixwidth);
        // check for top left
        if(pixcolor == color && prevpixcolorx != color
            && prevpixcolory != color) {
          if(debug)
            cout << "top left corner detected at x: " << j << ", y: " << i << endl;
          // top left corner of a rectangle detected!
          addNewBox((l_int32)j, (l_int32)i, processingboxbin);
          looking_for_tr = true;
          overlapfound = false;
        }
        // check for bottom left corner
        else if(pixcolor == color && prevpixcolory == color
            && prevpixcolorx != color) {
          LayoutEval::Color colorbelow = getRGBBelow(i, curpixel, pixwidth);
          if(colorbelow != color) {
            if((getPixelColor(curpixel+thickness/2) == color)
                && getPixelColor(curpixel-(pixwidth*(thickness/2)))
                == color && getPixelColor(curpixel+
                    (pixwidth*(thickness/2))) != color &&
                    isColorSignificant(getPixelColor
                        (curpixel+(pixwidth*(thickness/2))))) {

              // check to make sure there aren't any holes in between vertically
              // this is necessary for finding out whether or not the boxes are
              // overlapping horizontally (such that the left is in the same x
              // position as the right. either that condition or where there is
              // a gap between separate boxes which spans the entire width of the
              // current box
              bool amisure = true;
              l_uint32* tmppx = startpixel+j+(i*pixwidth);
              l_uint32 tmpcol = j;
              l_uint32 tmprow = i;
              for(l_uint32 k = 0; k < thickness+1; k++) {
                tmppx = curpixel + (k*pixwidth);
                tmprow++;
                if(getPixelColor(tmppx) != color && isColorSignificant(getPixelColor(tmppx))) {
                  // could be an overlap if this is the case
                  amisure = false;
                  break;
                }
                if(debug)
                  cout << "color at x: " << tmpcol << ", y: " << tmprow << " is insignificant\n";
              }
              l_uint32* stpx = tmppx;
              if(!amisure) {
                if(debug) {
                  cout << "detected potential bottom left at x: " << j << ", y: " << i <<
                      ", but not completely sure yet\n";
                }

                // lets take a closer look first get the box
                l_int32 count = boxaGetCount(processingboxbin);
                BOX* bbox;// = boxCreate(0);
                bool bboxfound = false;
                for(l_uint32 k = 0; k < count; k++) { // get the box first
                  bbox = boxaGetBox(processingboxbin, k, L_CLONE);
                  if(bbox->x == (l_int32)j /*&& bbox->y < (l_int32)i*/) {
                    bboxfound = true;
                    break;
                  }
                }
                // check to see if there's white space spanning
                // the entire width below the supposed rectangle
                if(bboxfound) {
                  amisure = true;
                  if(debug)
                    cout << "taking a closer look\n";
                  // move down by one to get to white space
                  stpx += pixwidth;
                  tmprow ++;
                  for(l_uint32 k = 0; k < bbox->w; k++) {
                    tmppx = stpx + k;
                    if(getPixelColor(tmppx) == color) {
                      amisure = false; // not so sure anymore that
                                       // I am at a bottom left
                      break;
                    }
                  }
                    // if it can be followed up a couple pixels, then back down and
                    // to the right a couple pixels then we know its the bottom left!!
                  tmprow = i;
                  tmpcol = j;
                  tmppx = curpixel; // go back to where we were at the beginning
                  if(getPixelColor(curpixel-pixwidth) == color
                      && getPixelColor(curpixel-(pixwidth*2)) == color
                      && getPixelColor(curpixel+1) == color
                      && getPixelColor(curpixel+2) == color
                      && getPixelColor(curpixel+pixwidth) != color) {
                    amisure = true;
                    if(debug)
                      cout << "the pixel is likely to be a bottom left based on its surrounding shape\n";

                  }
                  // check to see if there's any weird gaps should span the width
                  // of the rectangle
                  if(bbox->w) { // if the width is known try to follow to the width
                    // go back to start
                    l_uint32* stpx = startpixel+j+(i*pixwidth);
                    tmppx = stpx;
                    tmpcol = j;
                    tmprow = i;
                    amisure = true;
                    for(l_uint32 k = 0; k < bbox->w; k++) {
                      LayoutEval::Color colorpx = getPixelColor(tmppx);
                      if(colorpx != color && !isColorSignificant(colorpx)) {
                        if(debug)
                          cout << "however there was a weird gap found at x = " << tmpcol << endl;
                        amisure = false;
                        break;
                      }
                      tmppx = stpx + k;
                      tmpcol = j + k;
                    }
                  }
                }
                boxDestroy(&bbox);
              }
              if(amisure) {
                l_int32 blcornerx = (l_uint32)j;
                l_int32 blcornery = (l_uint32)i;
                on_bottom_rect = finishRectangle(processingboxbin, finishedboxbin,
                    blcornerx, blcornery);
                //overlapfound = true;
                bottom_rect_index = boxaGetCount(finishedboxbin) - 1;
              }
            }
            else {
              l_int32 blcornerx = (l_uint32)j;
              l_int32 blcornery = (l_uint32)i;
              on_bottom_rect = finishRectangle(processingboxbin, finishedboxbin,
                  blcornerx, blcornery);
              bottom_rect_index = boxaGetCount(finishedboxbin) - 1;
            }
          }
        }
        // check for overlap on the right side of a rectangle
        else if(pixcolor == color && prevpixcolorx == color &&
            prevpixcolory != color && !looking_for_tr) {
          l_int32 pboxcount = boxaGetCount(processingboxbin);
          BOX* bbox;
          for(l_int32 k = 0; k < pboxcount; k++) {
            bbox = boxaGetBox(processingboxbin, k, L_CLONE);
            if((bbox->x + bbox->w + 1) == (l_int32)j) {
              // found one!
              if(debug)
                cout << "overlap detected to right of rect at x: "
                << j << ", and y: " << i << ". current rect's top "
                << "left is x: " << bbox->x << ", y: " << bbox->y << endl;
              // make sure its the top left first and if not leave it alone
              // follow to the left to see if its overlapping through
              bool nottopleft = false;
              if(i > bbox->y + thickness) { // make sure we're not about to follow ourself
                l_uint32* tmppx = curpixel;
                l_uint32 tmpcol = j;
                for(l_uint32 k = j; k > bbox->x; k--) {
                  tmppx--;
                  tmpcol = k;
                  if((k == (j - 1 - thickness))) {
                    if(getPixelColor(tmppx) == color) {
                      if(debug) {
                        cout << "at col " << k << " and row " << i << " and color hasn't changed\n";
                        cout << " the column " << k << " is less than " << j - 1 - thickness << endl;
                      }
                      nottopleft = true;
                      if(debug)
                        cout << "the overlap is not the top left however so leaving it alone\n";
                      break;
                    }
                    else
                      break;
                  }
                }
              }
              if(nottopleft) // leave it (britney) alone!
                break;
              addNewBox((l_int32)j-thickness-1, (l_int32)i, processingboxbin);
              looking_for_tr = true;
              break;
            }
          }
          boxDestroy(&bbox);
          // overlap detected to right of a rect
          // top left corner of a rectangle detected!
        }
        prevpixcolorx = pixcolor;
      }
      prevpixcolorx = LayoutEval::NONE;
    }
    return finishedboxbin;
  }

  inline void addNewBox(const l_int32& x, const l_int32& y, BOXA* boxes) {
    if(debug)
      cout << "creating new box at x: " << x << ", and y: " << y << endl;
    BOX* bbox = boxCreate((l_int32)x, (l_int32)y, 0, 0);
    boxaAddBox(boxes, bbox, L_COPY);
    boxaSort(boxes, L_SORT_BY_X, L_SORT_INCREASING, NULL);
    boxDestroy(&bbox);
  }

  // for debugging display the particular type of region
  // found for a given image
  void dbgDrawRectangles(Pix* orig_img,
      const Pix* const gt_img, BOXA* boxes,
      const l_uint32& img_index, string groupname) {
    Pix* img = pixDrawBoxa(orig_img, boxes, 8, (l_uint32)0xff000000);
    string title2 = " for " + groupname + "in image " +
        intToString(img_index+1);
    string title1 = "Rectangles Found";
    pixDisplayWithTitle(img, 100, 100, (title1+title2).c_str(), 1);
    title1 = "Ground Truth";
    pixDisplayWithTitle((Pix*)gt_img, 500, 100,
        (title1+title2).c_str(), 1);
    pixDestroy(&img);
  }


  /****************************************
  * Data members                         **
  ****************************************/
  int layoutruns; // keeps track of how many times layout analysis
                  // has been run (should only happen once per object)
  string topdir; // top directory of file structure to be processed
  string subdir; // the name of the subdirectory file structure
  string ext; // image extension (ie png, jpg, etc)
  string groundtruthdir; // this points to [topdir]/groundtruth/[subdir]/
  string groundtruthtxt; // this points to [topdir]/groundtruth/[subdir]/[*.dat]
  string gt_blobsdir; // points to [topdir]/groundtruth/[subdir]/colorblobs/
  string inputdir; // this points to [topdir]/input/[subdir]/
  string outputdir; // this points to [topdir]/output/[train_set]/[subdir]/
  string train_set; // this is the name of the training data that was used to train the predictor being evaluated
  vector<string> output_result_dirs; // this is the name of the full
                               // path to the current test results
                               // [topdir]/output/[train_set]/[subdir]/[test]. The [train_set]
                               // specifies what training images the detector to be evaluated
                               // was trained with. The [subdir]
                               // specifies what data the test is being on run
                               // (multiple tests can be run on the same data)
                               // and the [test] dir specifies the specific MEDS module
                               // currently being run and evaluated. The same test can be run
                               // on multiple datasets in which case the results
                               // for a test will occur in the [test] directory
                               // inside a different [subdir] directory corresponding
                               // to the different dataset. This list is kept to ensure that a
                               // result dir is not overwritten during a single test
                               // run which may include tests on multiple datasets.
  int numfiles; // this is the number of files in the [topdir]/input/[subdir]/
                // (the number of files in the testset)
  l_uint32 thickness; // thickness of a line while grabbing colored bounding boxes
  vector<string> boolparams; // array of true/false tesseract parameters
  vector<string> intparams; // array of int tesseract params
  string page_seg_mode; // tesseract's page segmentation mode
                        // (rather than just resetting all of them on each iteration)
  TessBaseAPI* api; // the Tesseract API
  EquationDetectBase* new_equ_detector; // ownership is outside the scope of this class
  bool eval_default; // true if evaluating the tesseract default implementation
                     // otherwise evaluating a MEDS module from this work
  bool type_eval; // if this is on then each type of expression (displayed/embedded/label)
                  // is separately evaluted. if this is off then math/non-math is all that
                  // is evaluated
};


#endif /* DOCUMENTLAYOUTTEST_H_ */
