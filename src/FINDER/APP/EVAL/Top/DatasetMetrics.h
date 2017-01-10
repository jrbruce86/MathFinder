/*
 * DatasetMetrics.h
 *
 *  Created on: Jan 2, 2017
 *      Author: jake
 */

#ifndef DEATASETMETRICS_H_
#define DEATASETMETRICS_H_

#include <fstream>
#include <assert.h>
using namespace std;

#include <BipartiteGraph.h>

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

  inline void printMetrics(const int& numspaces, ofstream& fout) const {
    const int typenamespaces = numspaces;
    const int metricspaces = numspaces + 2;
    printSpaces(typenamespaces, fout); fout << "Result Type: " << res_type_name << endl;
    printSpaces(metricspaces, fout); fout << "TPR: " << TPR << endl;
    printSpaces(metricspaces, fout); fout << "FPR: " << FPR << endl;
    printSpaces(metricspaces, fout); fout << "ACC: " << ACC << endl;
    printSpaces(metricspaces, fout); fout << "TNR: " << TNR << endl;
    printSpaces(metricspaces, fout); fout << "PPV: " << PPV << endl;
    printSpaces(metricspaces, fout); fout << "FDR: " << FDR << endl;
    printSpaces(metricspaces, fout); fout << "NPV: " << NPV << endl;
    printSpaces(metricspaces, fout);
    fout << "Average Oversegmentations: " << avg_overseg << endl;
    printSpaces(metricspaces, fout);
    fout << "Average Oversegmentation Severity: " << overseg_severity << endl;
    printSpaces(metricspaces, fout);
    fout << "Average Undersegmentations: " << avg_underseg << endl;
    printSpaces(metricspaces, fout);
    fout << "Average Undersegmentation Severity: " << underseg_severity << endl;
    printSpaces(metricspaces, fout);
    fout << "Average Correctly Segmented Ratio: " << avg_correct_ratio << endl;
    printSpaces(metricspaces, fout);
    fout << "Average Completely Missed Region Ratio: " << avg_missed_ratio << endl;
    printSpaces(metricspaces, fout);
    fout << "Average Falsely Detected Region Count: " << avg_false_regions << endl;
  }

  inline void printSpaces(const int& numspaces, ofstream& fout) const {
    for(int i = 0; i < numspaces; ++i)
      fout << " ";
  }
};



#endif /* DEATASETMETRICS_H_ */


