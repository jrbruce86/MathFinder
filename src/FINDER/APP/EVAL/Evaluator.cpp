/*
 * Evaluator.cpp
 *
 *  Created on: Dec 31, 2016
 *      Author: jake
 */

#include <Evaluator.h>

#include <MetricsPrinter.h>

#include <DatasetMenu.h>

#include <Utils.h>
#include <Lept_Utils.h>

#include <allheaders.h>

#include <assert.h>
#include <exception>
#include <string>
#include <vector>

Evaluator::Evaluator(
    std::string resultsDirPath,
    std::string groundtruthDirPath,
    bool typeSpecificMode)
: coloredImageSubDirName("coloredImages") {
  this->resultsDirPath = Utils::checkTrailingSlash(resultsDirPath);
  this->groundtruthDirPath = Utils::checkTrailingSlash(groundtruthDirPath);
  this->typeSpecificMode = typeSpecificMode;

  this->coloredGroundtruthImageDirPath = Utils::checkTrailingSlash(this->groundtruthDirPath) + coloredImageSubDirName;

  this->groundtruthRectFilePath = this->groundtruthDirPath + std::string("groundtruth.rect");
  this->resultsRectFilePath = this->resultsDirPath + std::string("results.rect");
}


void Evaluator::evaluateSingleRun() {

  // 1. Verify the groundtruth dir and results dir are in the correct formats
  if(!verifyResultsAndGroundtruthPaths()) {
    return;
  }

  // 2. Grab paths to the input images
   inputImagePaths = DatasetSelectionMenu::findGroundtruthImagePaths(groundtruthDirPath);

  // 3. Copy the original images in the groundtruth dir to a subdir and color
  //    the foreground pixels of the copies based on the contents of the .rect file
  //    if not done already. The colored foreground pixels can be used for pixel
  //    accurate evaluation.
  //    Create the colored copies of each image if they don't already exist
  //    This involves coloring the foreground pixels of each input image based on the .rect file contents
  if(!coloredImageCopiesExist(inputImagePaths)) {
    if(!colorGroundtruthBlobs()) {
      return;
    }
  }

  // Grab the paths to the colored images
  coloredGroundtruthImagePaths = DatasetSelectionMenu::findGroundtruthImagePaths(coloredGroundtruthImageDirPath);
  coloredResultsImagePaths = DatasetSelectionMenu::findGroundtruthImagePaths(resultsDirPath);
  if(coloredGroundtruthImagePaths.size() != coloredResultsImagePaths.size()) {
    std::cout << "ERROR there are different numbers of colored images in the groundtruth from the results dirs.\n";
    return;
  }
  if(coloredGroundtruthImagePaths.size() != inputImagePaths.size()) {
    std::cout << "ERROR there are different numbers of colored images than there are input images.\n";
    return;
  }

  // 4. Run the evaluation logic
  std::vector<std::vector<HypothesisMetrics> > all_dataset_metrics;   // the metrics for each page of the dataset being evaluated
  for(int i = 0; i < inputImagePaths.size(); ++i) {

    // sanity checks
    if(!checkImNameMatchesIndex(inputImagePaths[i], i) ||
        !checkImNameMatchesIndex(coloredGroundtruthImagePaths[i], i) ||
        !checkImNameMatchesIndex(coloredResultsImagePaths[i], i)) {
      std::cout << "One of the results/input image names doesn't match its index. This would cause problems so exiting.\n";
      return;
    }

    // page metrics can be separated out by the type of element being evaluated
    // or all combined together. in the former case there will be multiple metrics
    // per page and in the latter just one per page.
    std::vector<HypothesisMetrics> page_metrics;
    try {
    if(typeSpecificMode) {
      // evaluate each type separately
      HypothesisMetrics disp_metrics = getEvaluationMetrics(
          (std::string)"displayed", i);
      page_metrics.push_back(disp_metrics);
      HypothesisMetrics emb_metrics = getEvaluationMetrics(
          (std::string)"embedded", i);
      page_metrics.push_back(emb_metrics);
      HypothesisMetrics label_metrics = getEvaluationMetrics(
          (std::string)"label", i);
      page_metrics.push_back(label_metrics);
    }
    else {
      // evaluate all of the above types (considering them all the same)
      HypothesisMetrics all_metrics = getEvaluationMetrics(
          (std::string)"all", i);
      page_metrics.push_back(all_metrics);
    }
    } catch (std::exception& e) {
      std::cout << "ERROR: Exception caught while running the evaluation.\n";
    }
    all_dataset_metrics.push_back(page_metrics);
  }
  assert(all_dataset_metrics.size() == inputImagePaths.size());
  std::cout << "Finished evaluating images in " << resultsDirPath << std::endl;



  std::vector<DatasetMetrics> avg_dataset_metrics = getDatasetAverages(all_dataset_metrics);

  // print all the metrics to a file in a subdir of the results directory
  string final_res_dir = resultsDirPath + "eval/";
  exec("rm -rf " + final_res_dir);
  exec("mkdir " + final_res_dir);
  string final_res_file = final_res_dir + "metrics";
  ofstream metric_stream(final_res_file.c_str());
  MetricsPrinter::printDatasetMetrics(all_dataset_metrics, metric_stream);
  metric_stream << "-----------------------------------------\n";
  MetricsPrinter::printAvgMetrics(avg_dataset_metrics, metric_stream);
}

vector<DatasetMetrics> Evaluator::getDatasetAverages(
    const vector<vector<HypothesisMetrics> >& all_metrics) {
  const int num_pages = all_metrics.size(); // number of pages in the dataset (all datasets should have the same number of pages)
  int num_res_types = all_metrics[0].size(); // the number of HypothesisMetrics on the first result type of the dataset (should all be this number), this is asserted in the loop below

  // Initialize the output
  vector<DatasetMetrics> datasetAverages;
  datasetAverages.reserve(num_res_types);
  for(int i = 0; i < num_res_types; ++i) {
    DatasetMetrics metric;
    metric.res_type_name = all_metrics[0][i].res_type_name;
    datasetAverages.push_back(metric);
  }

  // Aggregate the metrics for each expression type over all of the images in the dataset
  for(int i = 0; i < num_pages; ++i) { // iterate the pages of the dataset
    const vector<HypothesisMetrics>& page_metrics = all_metrics[i]; // metrics on a single page (consists of metrics for one or more expression type being evaluated)
      assert(page_metrics.size() == num_res_types); // expecting all pages to be evaluated on the same expression type(s)
      for(int k = 0; k < page_metrics.size(); ++k) { // iterate the expression types of i'th page
        const HypothesisMetrics& expressionTypeMetrics = page_metrics[k];
        DatasetMetrics& datasetAverage = datasetAverages[k];
        assert(expressionTypeMetrics.res_type_name == datasetAverage.res_type_name);
        datasetAverage.appendHypMetrics(expressionTypeMetrics);
      }
    }

  // Divide the dataset's aggregation by number of images to get the averages for each expression type
  for(int k = 0; k < num_res_types; ++k) {
    DatasetMetrics& dataset_restype_average = datasetAverages[k];
    dataset_restype_average.divideMetrics(num_pages);
  }

  return datasetAverages;
}


bool Evaluator::checkImNameMatchesIndex(const std::string& imName, const int index) {
  if(Utils::getNameFromPath(imName) != Utils::intToString(index)) {
    std::cout << "Error: " << imName << " is at the " << index << " index in its vector which doesn't match its name.\n";
    return false;
  }
  return true;
}

bool Evaluator::coloredImageCopiesExist(const std::vector<std::string>& inputImagePaths) {
  if(!Utils::existsDirectory(coloredGroundtruthImageDirPath)) {
    return false;
  }
  for(int i = 0; i < inputImagePaths.size(); ++i) {
    if(!Utils::existsFile(Utils::checkTrailingSlash(coloredGroundtruthImageDirPath) +
        Utils::intToString(i) + std::string("*"))) {
      return false;
    }
  }
  return true;
}

// Actually may not have time for this... Oh well.
void Evaluator::evaluateMultipleRuns() {
//  std::vector<std::string>

//  vector<DatasetMetrics> overall_averages = getFullAverage(avg_dataset_metrics);
//  metric_stream << "-----------------------------------------\n";
//  printOverallAvg(overall_averages, metric_stream);
//  cout << "Final metrics were printed to " << final_res_file << endl;

}

/**
 * Throws exception if fails
 */
HypothesisMetrics Evaluator::getEvaluationMetrics(
    const std::string typenamespec, const int i) {

  std::string evalTopDir = resultsDirPath + std::string("MathFinderEvaluationResults/");
  if(!Utils::existsDirectory(evalTopDir)) {
    Utils::exec((std::string)"mkdir " + evalTopDir);
  }
  std::string outfile_dir_verbose = evalTopDir + (std::string)"verbose/";
  if(!Utils::existsDirectory(outfile_dir_verbose))
    Utils::exec((std::string)"mkdir " + outfile_dir_verbose);
  std::string dbgdir = Utils::checkTrailingSlash(evalTopDir) + (std::string)"dbg/";
  if(!Utils::existsDirectory(dbgdir))
    Utils::exec((std::string)"mkdir " + dbgdir);
  std::string this_dbgdir = Utils::checkTrailingSlash(dbgdir) + inputImagePaths[i] + std::string("/");
  if(!Utils::existsDirectory(this_dbgdir))
    Utils::exec((std::string)"mkdir " + this_dbgdir);
  std::string outfile = evalTopDir + inputImagePaths[i] + (std::string)"_metrics";
  FILE* out;
  if(!(out = fopen(outfile.c_str(), "w"))) {
    std::cout << "ERROR: Could not create " << outfile << " file\n";
    throw std::exception();
  }
  std::string outfile_verbose = outfile_dir_verbose + inputImagePaths[i] +
      (std::string)"_metrics_verbose";
  FILE* out_verbose;
  if(!(out_verbose = fopen(outfile_verbose.c_str(), "w"))) {
    std::cout << "ERROR: Could not create " << outfile_verbose << " file\n";
    throw std::exception();
  }

  // The first step is to create the bipartite graph data structure
  // for the image
  GraphInput gi;

  gi.gtboxfile = groundtruthRectFilePath;
  gi.gtimg = readInColorImage(coloredGroundtruthImagePaths[i]);
  gi.hypboxfile = resultsRectFilePath;
  gi.hypimg = readInColorImage(coloredResultsImagePaths[i]);
  gi.imgname = Utils::getNameFromPath(inputImagePaths[i]);
  gi.evalTopDir = evalTopDir;
  gi.dbgdir = this_dbgdir;
  gi.inimg = readInColorImage(inputImagePaths[i]);
  BipartiteGraph pixelGraph(typenamespec, gi);

  // Now get and print the metrics
  HypothesisMetrics metrics = pixelGraph.getHypothesisMetrics();

  pixelGraph.printMetrics(out);
  pixelGraph.printMetricsVerbose(out_verbose);

  // Clear the memory and close the file
  pixelGraph.clear();
  fclose(out);
  std::cout << "Finished evaluating " << typenamespec << " for image " << gi.imgname << endl;
  return metrics;
}



Pix* Evaluator::readInColorImage(const std::string& imagePath) {
  Pix* image = Utils::leptReadImg(imagePath);
  Pix* convertedImage = pixConvertTo32(image);
  pixDestroy(&image);
  return convertedImage;
}


/*************************************************************************
* Reads in the .rect file and the input images in the user-specified groundtruth
* directory. Colors the foreground pixels of the input images based on the
* groundtruth data in the .rect file (i.e., color displayed math as red,
* embedded as blue, label as green). The colored images are stored in a subdir of the
* specified groundtruth. These colored images allow for pixel-accurate evaluation.
*
* Returns true on success, false if there's any error
*************************************************************************/
bool Evaluator::colorGroundtruthBlobs() {

  // Make sure the directory is fresh
  if(Utils::existsDirectory(coloredGroundtruthImageDirPath)) {
    Utils::exec(std::string("rm -rf ") + coloredGroundtruthImageDirPath);
    Utils::exec(std::string("mkdir -p ") + coloredGroundtruthImageDirPath);
  }

  // open the groundtruth text file which holds all of the math rectangles
  std::ifstream gtFileStream;
  gtFileStream.open(groundtruthRectFilePath.c_str());
  if(!gtFileStream.is_open()) {
    std::cout << "ERROR: Could not open the file at "
         << groundtruthRectFilePath << endl;
    return false;
  }

  // Now start reading and coloring
  std::string line;

  int curfilenum = -1;
  int expectedfilenum = 1;
  Pix* curimg;
  int linenum = 1;


  while(getline(gtFileStream, line)) {
    // parse the line
    if(line.empty()) {
      continue;
    }
    std::vector<std::string> splitline = Utils::stringSplit(line);
    const std::string filename   = splitline[0];
    const std::string recttype   = splitline[1];
    const int rectleft = atoi(splitline[2].c_str());
    const int recttop = atoi(splitline[3].c_str());
    const int rectright = atoi(splitline[4].c_str());
    const int rectbottom = atoi(splitline[5].c_str());
    int fileNumTmp; // throw-away variable
    {
      std::vector<std::string> tmp = Utils::stringSplit(filename, '.');
      fileNumTmp = atoi(tmp[0].c_str());
    }
    const int fileNum = fileNumTmp;

    // open the image if it isn't already opened
    // save the previous one if applicable
    if(curfilenum != fileNum) {
      std::cout << "Coloring groundtruth blobs for image " << fileNum << endl;
      if(curfilenum != -1) {
        // save previous and then deallocate it
        std::string savename = coloredGroundtruthImageDirPath + Utils::intToString(curfilenum) + ".png";
        pixWrite(savename.c_str(), curimg, IFF_PNG);
        pixDestroy(&curimg);
        imageThresholder.Clear();
      }
      curimg = Utils::leptReadImg(groundtruthDirPath + filename);
      imageThresholder.SetImage(curimg);
      if (!imageThresholder.IsBinary()) {
        pixDestroy(&curimg); // thresholder clones/copies/converts the input in SetImage, so no need to hold onto the original
        imageThresholder.ThresholdToPix(&curimg); // replace original image with the binarized one
      }
      curimg = pixConvertTo32(curimg);
      curfilenum = fileNum;
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
      std::cout << "ERROR: Rectangle of unknown type in .rect file.\n";
      return false;
    }
    Lept_Utils::fillBoxForeground(curimg, box, color);
    boxDestroy(&box);
    linenum++;
  }

  // save and destroy the final image
  const std::string savename = coloredGroundtruthImageDirPath + Utils::intToString(curfilenum) + ".png";
  pixWrite(savename.c_str(), curimg, IFF_PNG);
  pixDestroy(&curimg);
  gtFileStream.close();
  return true; // success
}

bool Evaluator::verifyResultsAndGroundtruthPaths() {

  // Make sure the folders aren't identical
  if(resultsDirPath == groundtruthDirPath) {
    std::cout << "ERROR: The results directory cannot be the same as the groundtruth one.\n";
    return false;
  }

  // Make sure the results dir contains all the results in one .rect file
  // and that the groundtruth dir also contains its groundtruth in one .rect file
  if(!verifyOneRectFileAt(resultsDirPath) ||
      !verifyOneRectFileAt(groundtruthDirPath)) {
    return false;
  }

  // Verify the groundtruth .rect file has the expected name
  if(!Utils::existsFile(groundtruthRectFilePath)) {
    std::cout << "ERROR: The groundtruth .rect file is expected to be named as follows: "
        << groundtruthRectFilePath << ". A file with that path could not be found.\n";
    return false;
  }

  // Verify the results .rect file has the expected name
  if(!Utils::existsFile(resultsRectFilePath)) {
    std::cout << "ERROR: The groundtruth .rect file is expected to be named as follows: "
        << groundtruthRectFilePath << ". A file with that path could not be found.\n";
    return false;
  }

  // Make sure the groundtruth dir is in the expected format
  return DatasetSelectionMenu::groundtruthDirPathIsGood(groundtruthDirPath);
}

bool Evaluator::verifyOneRectFileAt(const std::string& dirPath) {
  std::vector<std::string> rectFiles = Utils::getFileList(dirPath + "*.rect");
  if(rectFiles.size() != 1) {
    std::cout << "ERROR: The directory, " << dirPath << ", needs to have one and only one "
        << ".rect file. It currently has " << rectFiles.size() << " such files.\n";
    return false;
  }
  return true;
}
