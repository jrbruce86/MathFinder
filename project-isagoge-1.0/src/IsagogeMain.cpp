/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:		isagoge_main.cpp
 * Written by:	Jake Bruce, Copyright (C) 2013
 * History: 		Created Feb 28, 2013 9:34:34 PM 
 * ------------------------------------------------------------------------
 * Description: Main file from which evaluations of various equation
 *              detectors on various datasets and experiments are run.
 *              Primarily uses DocumentLayoutTester class functions to
 *              perform tests. See DocumentLayoutTest.h for more info.
 * ------------------------------------------------------------------------
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

#include <allheaders.h>
#include <MEDS_Trainer.h>
#include <DocumentLayoutTest.h>
#include <MEDS_Types.h>

#include <iostream>
#include <string>
using namespace std;

/**************************************************************************
 * Uncomment out the MEDS module to be tested. Make sure that one and only
 * one of them are uncommented!
 *************************************************************************/
//#define MEDS1TEST
//#define MEDS2TEST
//#define MEDS3TEST
#define MEDS4TEST
//#define TESSERACT

#ifdef MEDS1TEST
typedef MEDS1 MEDSType;
typedef MEDS1Detector DetectorType;
#define MEDSNUM 1;
#endif
#ifdef MEDS2TEST
typedef MEDS2 MEDSType;
typedef MEDS2Detector DetectorType;
#define MEDSNUM 2;
#endif
#ifdef MEDS3TEST
typedef MEDS3 MEDSType;
typedef MEDS3Detector DetectorType;
#define MEDSNUM 3;
#endif
#ifdef MEDS4TEST
typedef MEDS4 MEDSType;
typedef MEDS4Detector DetectorType;
#define MEDSNUM 4;
#endif
#ifdef TESSERACT
typedef int MEDSType;
#endif

// The averaged metrics for an entire dataset (or for all of the datasets combined)
struct DatasetMetrics {
  DatasetMetrics() : TPR(0), FPR(0), ACC(0), TNR(0), PPV(0), FDR(0), NPV(0),
      avg_overseg(0), avg_underseg(0), overseg_severity(0), underseg_severity(0),
      avg_correct_ratio(0), avg_missed_ratio(0), avg_false_regions(0) {}
  // pixel accurate statistics
  double TPR; // True Positive Rate (Sensitivity, Hit Rate, Recall): TPR = TP/P
  double FPR; // False Positive Rate (Fallout): FPR = FP/N
  double ACC; // Accuracy: ACC = (TP+TN)/(P+N)
  double TNR; // True Negative Rate (Specificity): SPC = TN/N = 1-FPR
  double PPV; // Positive Predictive Value (Precision): PPV = TP/(TP+FP)
  double FDR; // False Discovery Rate: FDR = FP/(TP+FP)
  double NPV; // Negative Predictive Value: NPV = TN/(TN+FN)
  // over/undersegmentation statistics
  double avg_overseg; // Average count of groundtruth rectangles that were oversegmented by the hypothesis
  double avg_underseg; // Average count of hypothesis rectangles that were unersegmented (i.e., that have more than one groundtruth rectangle)
  double overseg_severity; // Average severity of oversegmented regions (i.e., how many hypothesis regions an oversegmented groundtruth region was split into)
  double underseg_severity; // Average severity of undersegmented regions (i.e., how many grountruth regions were improperly merged into a hypothesis region)
  // region-wide statistics
  double avg_correct_ratio; // ratio of correctly segmented groundtruth regions to total groundtruth regions
  double avg_missed_ratio; // ratio of completely missed groundtruth regions to total groundtruth regions
  double avg_false_regions; // average # completely false regions per page

  string res_type_name;
  //TODO add oversegmentation and undersegmentation statistics

  inline void appendHypMetrics(const HypothesisMetrics& hypmetrics) {
    assert(res_type_name == hypmetrics.res_type_name);
    TPR += hypmetrics.total_recall;
    FPR += hypmetrics.total_fallout;
    ACC += hypmetrics.accuracy;
    TNR += hypmetrics.specificity;
    PPV += hypmetrics.total_precision;
    FDR += hypmetrics.total_fdr;
    NPV += hypmetrics.negative_predictive_val;
    avg_overseg += (double)(hypmetrics.oversegmentedcomponents);
    avg_underseg += (double)(hypmetrics.undersegmentedcomponents);
    overseg_severity += hypmetrics.avg_oversegmentations_perbox;
    underseg_severity += hypmetrics.avg_undersegmentations_perbox;
    assert(hypmetrics.correctsegmentations >= 0);
    if(hypmetrics.correctsegmentations > 0)
      avg_correct_ratio += ((double)(hypmetrics.correctsegmentations)
          / (double)(hypmetrics.total_gt_regions));
    assert(hypmetrics.falsenegatives >= 0);
    if(hypmetrics.falsenegatives > 0)
      avg_missed_ratio += ((double)hypmetrics.falsenegatives
          / (double)(hypmetrics.total_gt_regions));
    avg_false_regions += (double)(hypmetrics.falsepositives);
  }

  inline void appendDatasetMetrics(const DatasetMetrics& dm) {
    assert(res_type_name == dm.res_type_name);
    TPR += dm.TPR;
    FPR += dm.FPR;
    ACC += dm.ACC;
    TNR += dm.TNR;
    PPV += dm.PPV;
    FDR += dm.FDR;
    NPV += dm.NPV;
    avg_overseg += dm.avg_overseg;
    avg_underseg += dm.avg_underseg;
    overseg_severity += dm.overseg_severity;
    underseg_severity += dm.underseg_severity;
    avg_correct_ratio += dm.avg_correct_ratio;
    avg_missed_ratio += dm.avg_missed_ratio;
    avg_false_regions += dm.avg_false_regions;
  }

  inline void divideMetrics(const int& denom_) {
    const double denom = (double)denom_;
    TPR /= denom;
    FPR /= denom;
    ACC /= denom;
    TNR /= denom;
    PPV /= denom;
    FDR /= denom;
    NPV /= denom;
    avg_overseg /= denom;
    avg_underseg /= denom;
    overseg_severity /= denom;
    underseg_severity /= denom;
    if(!(avg_correct_ratio >= 0))
      cout << avg_correct_ratio << endl;
    assert(avg_correct_ratio >= 0);
    if(avg_correct_ratio > 0)
      avg_correct_ratio /= denom;
    assert(avg_missed_ratio >= 0);
    if(avg_missed_ratio > 0)
      avg_missed_ratio /= denom;
    avg_false_regions /= denom;
  }

  inline void printMetrics(const int& numspaces) const {
    const int typenamespaces = numspaces;
    const int metricspaces = numspaces + 2;
    printSpaces(typenamespaces); cout << "Result Type: " << res_type_name << endl;
    printSpaces(metricspaces); cout << "TPR: " << TPR << endl;
    printSpaces(metricspaces); cout << "FPR: " << FPR << endl;
    printSpaces(metricspaces); cout << "ACC: " << ACC << endl;
    printSpaces(metricspaces); cout << "TNR: " << TNR << endl;
    printSpaces(metricspaces); cout << "PPV: " << PPV << endl;
    printSpaces(metricspaces); cout << "FDR: " << FDR << endl;
    printSpaces(metricspaces); cout << "NPV: " << NPV << endl;
    printSpaces(metricspaces);
    cout << "Average Oversegmentations: " << avg_overseg << endl;
    printSpaces(metricspaces);
    cout << "Average Oversegmentation Severity: " << overseg_severity << endl;
    printSpaces(metricspaces);
    cout << "Average Undersegmentations: " << avg_underseg << endl;
    printSpaces(metricspaces);
    cout << "Average Undersegmentation Severity: " << underseg_severity << endl;
    printSpaces(metricspaces);
    cout << "Average Correctly Segmented Ratio: " << avg_correct_ratio << endl;
    printSpaces(metricspaces);
    cout << "Average Completely Missed Region Ratio: " << avg_missed_ratio << endl;
    printSpaces(metricspaces);
    cout << "Average Falsely Detected Region Count: " << avg_false_regions << endl;
  }

  inline void printSpaces(const int& numspaces) const {
    for(int i = 0; i < numspaces; ++i)
      cout << " ";
  }
};
// takes the results for all images in all the datasets, computes the average for each dataset.
// Each dataset could have multiple averages if more than one result type is being evaluated for
// each image hence then 2D vector. The input vector is 3D because it can have more than one
// dataset, each having more than one image, each of which can have more than one result type.
vector<vector<DatasetMetrics> >
getDatasetAverages(const vector<vector<vector<HypothesisMetrics> > >&);
// takes the averages for each dataset and computes the average metrics over all of the datasets
// for each result type that was evaluated.
vector<DatasetMetrics> getFullAverage(const vector<vector<DatasetMetrics> >&);

// Runs evaluation test on the given detector for the given dataset
// which should be within the "topdir" specified (if detector is null
// then uses Tesseract's default one. The testname is the name of
// the test to be run on the dataset and the name of directory
// wherein the test results will be held (this name should be indicative
// of which equation detector is being used). if the bool argument, type_eval
// is turned off then only math/non-math is evaluated, if it is turned on then
// displayed, embedded, and labels are evaluated seperately.
vector<vector<vector<HypothesisMetrics> > >
evaluateDataSets(EquationDetectBase*& detector, string topdir,
    vector<string> datasets, string train_set, string testname, bool type_eval=true,
    string extension=(string)".png");

// prints the metrics for each result type of each image of each dataset
void printAllMetrics(const vector<vector<vector<HypothesisMetrics> > >&);
// prints the average metrics for each result type of each dataset
void printAvgMetrics(const vector<vector<DatasetMetrics> >&);
// prints the average for each result type over all the datasets
void printOverallAvg(const vector<DatasetMetrics>&);

int main() {
  string topdir = "../test_sets/";
  string dataset = "test";
  string train_dir = "training/MainTrainingDir/";
  string train_set = "AdvCalc1";
  const string trainpath = topdir + train_dir + (string)"training_sets/" + train_set;

  // set this to false if only want to train the module
  // if it hasn't been trained yet.
  bool train_always = false;
  // if true then extract features during training even
  // if they've already been written to a file
  bool new_samples = true;

  // specify how the tesseract api should always be initialized
  // i.e. what language and the path to the training files necessary
  vector<string> api_init_params;
  api_init_params.push_back((string)"/usr/local/share/"); // tesseract training file path
  api_init_params.push_back((string)"eng"); // tesseract language

  // Pick a detector/segmentor combo and train if necessary
#ifndef TESSERACT
  MEDS_Trainer<DetectorType> trainer(train_always, trainpath, train_set, new_samples, api_init_params);
  trainer.trainDetector();
  cout << "Finished training the detector!\n";

  // Instantiate a MEDS module which uses the detector that was
  // initialized by the trainer
  EquationDetectBase* meds = new MEDSType(); // MEDS is owned by the evaluator
  DetectorType* detector = trainer.getDetector();
  ((MEDSType*)meds)->setDetector(detector); // MEDS owns the detector
  cout << "Finished initializing MEDS class\n";
#endif
#ifdef TESSERACT
  EquationDetectBase* meds = NULL;
#endif
  // Test it
  vector<string> datasets;
  for(int i = 0; i < 4; ++i) {
    string dataset_ = dataset + intToString(i+1);
    datasets.push_back(dataset_);
  }
#ifndef TESSERACT
  int testnum_ = MEDSNUM;
  string testnum = intToString(testnum_);
#endif
  cout << "About to run evaluation.\n";
  vector<vector<vector<HypothesisMetrics> > > all_dataset_metrics =
      evaluateDataSets(meds, topdir, datasets, train_set,
#ifdef TESSERACT
          "default",
#endif
#ifndef TESSERACT
          "myMEDS" + testnum,
#endif
          false);
  vector<vector<DatasetMetrics> > avg_dataset_metrics = getDatasetAverages(all_dataset_metrics);
  vector<DatasetMetrics> overall_averages = getFullAverage(avg_dataset_metrics);

  printAllMetrics(all_dataset_metrics);
  cout << "-----------------------------------------\n";
  printAvgMetrics(avg_dataset_metrics);
  cout << "-----------------------------------------\n";
  printOverallAvg(overall_averages);
  return 0;
}

vector<vector<vector<HypothesisMetrics> > > evaluateDataSets(EquationDetectBase*& meds,
    string topdir, vector<string> datasets, string train_set, string testname,
    bool type_eval, string extension) {
  bool meds_given = false;
  if(meds)
    meds_given = true;
  DocumentLayoutTester<MEDSType> test(meds);
  cout << "Finished constructing evaluator class.\n";
  if(!type_eval)
    test.turnOffTypeEval();
  test.activateEquOutput();
  vector<vector<vector<HypothesisMetrics> > > all_dataset_metrics;
  for(int i = 0; i < datasets.size(); ++i) {
    string dataset = datasets[i];
    test.setFileStructure(topdir, dataset, train_set, extension);
    test.runTessLayout(testname, false);
    vector<vector<HypothesisMetrics> > dataset_metrics =
        test.evalTessLayout(testname, false);
    all_dataset_metrics.push_back(dataset_metrics);
  }
  // return the metrics for each dataset
  return all_dataset_metrics;
}

vector<vector<DatasetMetrics> >
getDatasetAverages(const vector<vector<vector<HypothesisMetrics> > >& all_metrics) {
  vector<vector<DatasetMetrics> > average_metrics_datasets; // averages for all datasets
  const int num_pages = all_metrics[0].size(); // number of pages in the first dataset (all datasets should have the same number of pages)
  const int num_res_types = all_metrics[0][0].size(); // the number of HypothesisMetrics on the first page of first dataset (should all be this number)
  // a page may have more than one metric if multiple result types are being evaluated separately
  // for each page. currently only either 1 or 3 types are expected, at least for the current
  // application. if something else is being evaluated however, this assertion needs to be modified.
  assert(num_res_types == 1 || num_res_types == 3);
  // first allocate and initialize each average to zero
  for(int i = 0; i < all_metrics.size(); ++i) {
    const vector<vector<HypothesisMetrics> >& dataset_metrics = all_metrics[i]; // all metrics for i'th dataset consists of images, each with metrics for all the types being evaluated
    vector<DatasetMetrics> average_metric_dataset; // average metric for i'th dataset (consists of an average for each type evaluated)
    for(int k = 0; k < num_res_types; ++k) {
      DatasetMetrics type_avg;
      type_avg.res_type_name = all_metrics[0][0][k].res_type_name;
      average_metric_dataset.push_back(type_avg); // each dataset has averages for all the types being evaluated
    }
    average_metrics_datasets.push_back(average_metric_dataset); // append initialized dataset average to full list
  }
  // now ready to compute the averages for each result type within each dataset
  for(int i = 0; i < all_metrics.size(); ++i) { // iterate the datasets
    const vector<vector<HypothesisMetrics> >& dataset_metrics = all_metrics[i];
    vector<DatasetMetrics>& dataset_averages = average_metrics_datasets[i];
    assert(dataset_averages.size() == num_res_types); // should have entry for each result type
    assert(dataset_metrics.size() == num_pages); // dataset should have the right number of pages
    for(int j = 0; j < num_pages; ++j) { // iterate the pages of the i'th dataset
      const vector<HypothesisMetrics>& page_metrics = dataset_metrics[j];
      assert(page_metrics.size() == num_res_types);
      for(int k = 0; k < page_metrics.size(); ++k) { // iterate the result types of j'th page of i'th dataset
        const HypothesisMetrics& page_restype_metrics = page_metrics[k];
        DatasetMetrics& dataset_restype_average = dataset_averages[k];
        dataset_restype_average.appendHypMetrics(page_restype_metrics);
      }
    }
    // now divide the dataset's aggregation for each result type by the number of pages in the dataset
    for(int k = 0; k < num_res_types; ++k) {
      DatasetMetrics& dataset_restype_average = dataset_averages[k];
      dataset_restype_average.divideMetrics(num_pages);
    }
  }
  return average_metrics_datasets;
}

vector<DatasetMetrics> getFullAverage(const vector<vector<DatasetMetrics> >& all_averages) {
  vector<DatasetMetrics> full_average;
  // first initialize average entry for each result type
  const int num_datasets = all_averages.size();
  const int num_res_types = all_averages[0].size(); // should all have the same # result types
  for(int i = 0; i < num_res_types; ++i) {
    DatasetMetrics res_type_avg;
    res_type_avg.res_type_name = all_averages[0][i].res_type_name;
    full_average.push_back(res_type_avg);
  }
  assert(full_average.size() == num_res_types);
  // now aggregate the metrics for each result type for all the datasets
  for(int i = 0; i < num_datasets; ++i) { // iterate the datasets
    const vector<DatasetMetrics>& dataset_avg = all_averages[i];
    assert(dataset_avg.size() == num_res_types);
    for(int j = 0; j < num_res_types; ++j) {
      const DatasetMetrics& dataset_res_type = dataset_avg[j];
      DatasetMetrics& res_type_avg = full_average[j];
      res_type_avg.appendDatasetMetrics(dataset_res_type);
    }
  }
  // now divide the full averages (for each result type) by the number of datasets
  for(int i = 0; i < num_res_types; ++i) {
    DatasetMetrics& res_type_avg = full_average[i];
    res_type_avg.divideMetrics(num_datasets);
  }
  return full_average;
}

void printAllMetrics(const vector<vector<vector<HypothesisMetrics> > >& allmetrics) {
  for(int i = 0; i < allmetrics.size(); ++i) {
    const vector<vector<HypothesisMetrics> >& dataset_metrics = allmetrics[i];
    cout << "Metrics for dataset " << i << ":\n";
    for(int j = 0; j < dataset_metrics.size(); ++j) {
      cout << "  Page " << j << ":\n";
      const vector<HypothesisMetrics>& page_metrics = dataset_metrics[j];
      for(int k = 0; k < page_metrics.size(); ++k) {
        const HypothesisMetrics& page_type_metrics = page_metrics[k];
        cout << "    Result type '" << page_type_metrics.res_type_name << "' metrics:\n";
        cout << "      TPR: " << page_type_metrics.total_recall << endl;
        cout << "      FPR: " << page_type_metrics.total_fallout << endl;
        cout << "      ACC: " << page_type_metrics.accuracy << endl;
        cout << "      TNR: " << page_type_metrics.specificity << endl;
        cout << "      PPV: " << page_type_metrics.total_precision << endl;
        cout << "      FDR: " << page_type_metrics.total_fdr << endl;
        cout << "      NPV: " << page_type_metrics.negative_predictive_val << endl;
        // TODO: Add oversegmentation/undersegmentation metrics
      }
    }
  }
}

void printAvgMetrics(const vector<vector<DatasetMetrics> >& average_metrics) {
  for(int i = 0; i < average_metrics.size(); ++i) {
    const vector<DatasetMetrics>& average_dataset_metrics = average_metrics[i];
    cout << "Average Metrics for Dataset " << i << ":\n";
    for(int j = 0; j < average_dataset_metrics.size(); ++j) {
      const DatasetMetrics& average_dataset_type_metrics = average_dataset_metrics[j];
      average_dataset_type_metrics.printMetrics(2);
    }
  }
}

void printOverallAvg(const vector<DatasetMetrics>& overall_avg) {
  const int num_res_types = overall_avg.size();
  cout << "The overall averages for all datasets evaluated:\n";
  for(int i = 0; i < num_res_types; ++i) {
    const DatasetMetrics& res_type_avg = overall_avg[i];
    res_type_avg.printMetrics(2);
  }
}
