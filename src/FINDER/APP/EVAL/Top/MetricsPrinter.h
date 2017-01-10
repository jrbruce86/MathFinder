/*
 * MetricsPrinter.h
 *
 *  Created on: Jan 2, 2017
 *      Author: jake
 */

#ifndef METRICSPRINTER_H_
#define METRICSPRINTER_H_

namespace MetricsPrinter {

using namespace std;

void printDatasetMetrics(const vector<vector<HypothesisMetrics> >& dataset_metrics,
    ofstream& fout) {
  // iterate the pages
  for(int i = 0; i < dataset_metrics.size(); ++i) {
    fout << "  Page " << i << ":\n";
    const vector<HypothesisMetrics>& page_metrics = dataset_metrics[i];
    // iterate the expression type(s)
    for(int j = 0; j < page_metrics.size(); ++j) {
      const HypothesisMetrics& page_type_metrics = page_metrics[j];
      fout << "    Result type '" << page_type_metrics.res_type_name << "' metrics:\n";
      fout << "      TPR: " << page_type_metrics.total_recall << endl;
      fout << "      FPR: " << page_type_metrics.total_fallout << endl;
      fout << "      ACC: " << page_type_metrics.accuracy << endl;
      fout << "      TNR: " << page_type_metrics.specificity << endl;
      fout << "      PPV: " << page_type_metrics.total_precision << endl;
      fout << "      FDR: " << page_type_metrics.total_fdr << endl;
      fout << "      NPV: " << page_type_metrics.negative_predictive_val << endl;
      fout << "      Oversegmentations: " << page_type_metrics.oversegmentedcomponents << endl;
      fout << "      Oversegmentation Severity: " << page_type_metrics.avg_oversegmentations_perbox << endl;
      fout << "      Undersegmentations: " << page_type_metrics.undersegmentedcomponents << endl;
      fout << "      Undersegmentation Severity: " << page_type_metrics.avg_undersegmentations_perbox << endl;
      fout << "      Correctly Segmented Ratio: "
          << (double)(page_type_metrics.correctsegmentations) <<
          "/" << (double)(page_type_metrics.total_gt_regions) << endl;
      fout << "      Completely Missed Region Ratio: "
          << ((double)page_type_metrics.falsenegatives
              / (double)(page_type_metrics.total_gt_regions)) << endl;
      fout << "      Falsely Detected Region Count: " << page_type_metrics.falsepositives << endl;
    }
  }
}

void printAvgMetrics(const vector<DatasetMetrics>& average_metrics,
    ofstream& fout) {
  fout << "Average Metrics for Dataset:\n";
  for(int j = 0; j < average_metrics.size(); ++j) {
    const DatasetMetrics& average_dataset_type_metrics = average_metrics[j];
    average_dataset_type_metrics.printMetrics(2, fout);
  }
}


}




#endif /* METRICSPRINTER_H_ */
