/*
 * TesseractParamManager.cpp
 *
 *  Created on: Nov 6, 2016
 *      Author: jake
 */

#include<TessParamManager.h>

#include<string>
#include<assert.h>

#include<baseapi.h>

#include<Utils.h>

/**********************************************************
 * Constructor: Sets the api and the params being managed *
 **********************************************************/
TesseractParamManager::TesseractParamManager(tesseract::TessBaseAPI* api) {
  this->api = api;
  init();
}

void TesseractParamManager::init() {
  /******************* Massive List of Parameters to Play With!!!!*********************************/
  // WARNING: THESE ARE HARDCODED.. DON'T MODIFY!!!!!!!!!!!!
  ////////////Settings paramaters////////////
  // This just lists all of the possible settings. They are set or unset later.
  //boolparams.reserve(20);

  ////////////textordering debug boolparams ////////////
  // Should display a window showing the intial tabs detected in FindInitialTabVectors
  boolparams.push_back((std::string)"textord_tabfind_show_initialtabs");
  // Enables their table detection!! Class for this is TableFinder in textord/tablefind.h.
  boolparams.push_back((std::string)"textord_tabfind_find_tables");
  // In order to see a window display of the tables detected!
  // This is run after the equation detection if enabled.
  boolparams.push_back((std::string)"textord_show_tables");
  // Displays blobs rejected as noise prior to equation or tabledetection
  boolparams.push_back((std::string)"textord_tabfind_show_reject_blobs");
  // This will show the partitions prior to the equation and tabledetection
  boolparams.push_back((std::string)"textord_tabfind_show_initial_partitions");
  // This will show the partitions after equation and table detection
  boolparams.push_back((std::string)"textord_tabfind_show_partitions");
  // Use greyed background for debug images
  boolparams.push_back((std::string)"textord_debug_images");
  // Print tabfind related debug messages
  boolparams.push_back((std::string)"textord_debug_tabfind");
  // Displays blob and block bounding boxes in a window called “Blocks”.
  // This is after equation and table detection. Also occurs during  setupandfilternoise,
  // which occurs before findblocks is called in order to filter out noise.
  boolparams.push_back((std::string)"textord_tabfind_show_blocks");
  // Display unsorted blobs during call to filterblobs made within textordpage which is
  // called after autosegmentation is carried out.
  // Displays all the blobs color-coded at ones.
  boolparams.push_back((std::string)"textord_show_blobs");
  // Displays “boxes” this displays each type of blob
  // (small, noise, big, medium size) one at a time.
  boolparams.push_back((std::string)"textord_show_boxes");
  // Displays the results of ColumnFinder
  boolparams.push_back((std::string)"textord_tabfind_show_columns");

  ////////////tessedit debug boolparams////////////////////
  // dump intermediate images during page segmentation
  boolparams.push_back((std::string)"tessedit_dump_pageseg_images");

  ///////////equationdetect debug boolparams////////////////////
  // display the BSTT's
  boolparams.push_back((std::string)"equationdetect_save_spt_image");
  // save the results of pass 2 (identifyseedparts)
  boolparams.push_back((std::string)"equationdetect_save_seed_image");
  //  save the final math detection results
  boolparams.push_back((std::string)"equationdetect_save_merged_image");

  // print table detection output (requires input file to be named "test1.tif")
  boolparams.push_back((std::string)"textord_dump_table_images");

  // Change from default page segmentation mode (single block)
  // to the more advanced auto-segmentation, which includes
  // the experimental equation detection module: 3 = PSM_AUTO
  page_seg_mode = (std::string)"tessedit_pageseg_mode";

  // int params
  intparams.push_back((std::string)"textord_tabfind_show_strokewidths");
  intparams.push_back((std::string)"textord_debug_tabfind");
  // Should display images detected as distinct from text by FindImagePartitions
  intparams.push_back((std::string)"textord_tabfind_show_images");
  /*********************End of Massive Parameter List!*********************************/
}

/**********************************************************
 * Go ahead and enable both all int and all bool params    *
 **********************************************************/
void TesseractParamManager::activateAllParams() {
  activateAllBoolParams();
  activateAllIntParams();
}

/**********************************************************
 * Go ahead and enable all of the true/false params        *
 **********************************************************/
void TesseractParamManager::activateAllBoolParams() {
  for (unsigned int i = 0; i < boolparams.size(); i++)
    activateBoolParam(boolparams[i]);
}

// Turn on equation debugging in Tesseract so it outputs
// the results of layout analysis
void TesseractParamManager::activateEquOutput() {
  activateBoolParam((std::string)"equationdetect_save_spt_image");
  activateBoolParam((std::string)"equationdetect_save_seed_image");
  activateBoolParam((std::string)"equationdetect_save_merged_image");
}

/**********************************************************
 * Go ahead and enable all of the integer params           *
 **********************************************************/
void TesseractParamManager::activateAllIntParams() {
  for (unsigned int i = 0; i < intparams.size(); i++)
    activateIntParam(intparams[i]);
}

/****************************************************************************
 * Turn off all params that require the scroll view                          *
 ****************************************************************************/
void TesseractParamManager::deActivateScrollView() {
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
void TesseractParamManager::activateNonScrollView() {
  activateAllBoolParams();
  deActivateScrollView();
}

/****************************************************************************
 * Tell Tesseract to set a bool parameter to true
 ****************************************************************************/
void TesseractParamManager::activateBoolParam(std::string param) {
  if (!api->SetVariable(param.c_str(), "true"))
    tessParamError(param);
}

/****************************************************************************
 * Tell Tesseract to set an int parameter to 1
 ****************************************************************************/
void TesseractParamManager::activateIntParam(std::string param) {
  if (!api->SetVariable(param.c_str(), "5")) // for now use high debug level...
    tessParamError(param);
}

/****************************************************************************
 * Tell Tesseract to set a bool parameter back to false
 ****************************************************************************/
void TesseractParamManager::deActivateBoolParam(std::string param) {
  if (!api->SetVariable(param.c_str(), "false"))
    tessParamError(param);
}

/****************************************************************************
 * Tell Tesseract to set an int parameter back to zero
 ****************************************************************************/
void TesseractParamManager::deActivateIntParam(std::string param) {
  if (!api->SetVariable(param.c_str(), "0"))
    tessParamError(param);
}

/****************************************************************************
 * TODO: Remove this. Shouldn't be necessary since api already has function to do this
 * Tell Tesseract to set its page segmentation mode to the one provided
 ****************************************************************************/
void TesseractParamManager::setPageSegmentationMode(const tesseract::PageSegMode mode) {
  if (!api->SetVariable(page_seg_mode.c_str(), Utils::intToString(static_cast<int>(mode)).c_str()))
    tessParamError(page_seg_mode);
  // Check to make sure tesseract is in the correct page
  // segmentation mode (psm)
  int psm = 0;
  api->GetIntVariable(page_seg_mode.c_str(), &psm);
  assert(psm == mode);
}

void TesseractParamManager::tessParamError(std::string str) {
  std::cout << "ERROR " << str << std::endl;
}
