/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   DocumentLayoutTest.h
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Aug 5, 2013 7:51 PM
 * Description:
 * By definition a bipartite graph consists of two independent sets with edges
 * only being drawn between the the two sets (there are no edges allowed within
 * a set and of course this makes sense for our application since we are
 * comparing the hypothesis to the groundtruth, not to iteself). One set in the
 * bipartite graph will represent the hypothesis, the other represents the
 * groundtruth. Each element of the set is a rectangular portion of the image
 * and has both an area and a number of foreground pixels. A combination of
 * the area and foreground pixel count is used to measure the strength of an
 * edge between two vertices as well as penalty to be incurred by false
 * positives and false negatives.
 *-------------------------------------------------------------------------
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

#ifndef BIPARTITEGRAPH_H
#define BIPARTITEGRAPH_H

#include <vector>
#include <string>
#include <fstream>
using namespace std;
#include <allheaders.h> // leptonica api

#include <Lept_Utils.h>
#include <Basic_Utils.h>
using namespace Basic_Utils;

struct Edge; // forward declaration

// this struct holds all of the information necessary
// for creating a bipartite graph except for the type
// of block detection it is testing. The type of
// block detection specifies which rectangles in the
// box files are of interest
struct GraphInput {
  string hypboxfile; // text file holding the hypothesis rectangles
  string gtboxfile; // text file holding the groundtruth rectangles
  string imgname; // the name of the image being evaluated
  PIX* hypimg;
  PIX* gtimg;
  PIX* inimg;
};

struct Vertex {
  BOX* rect;
  int pix_foreground;
  int area;
  string whichset; // either hypothesis or groundtruth
  int setindex;
  vector<Edge> edges;
};

struct Edge {
  Vertex* vertexptr;
  int pixfg_intersecting;
  int overlap_area;
};


namespace Bipartite {
  enum GraphChoice {GroundTruth, Hypothesis};
}


struct GTBoxDescription {
  double fg_pix_ratio; // ratio of the box's foreground
                        // pixels to the total_seg_fg_pixels
  double area_ratio; // ratio of the box's area
                     // to the total_seg_area
};

struct GroundTruthMetrics {
  int segmentations; // total correct segmentations based on groundtruth
  int total_seg_fg_pixels; // total segmented foreground pixels
  int total_nonseg_fg_pixels; // total true negative foreground pixels
  int total_fg_pixels; // total foreground pixels
  double fg_pixel_ratio; // ratio of total segmented foreground
                         // pixels to total foreground pixels
  int total_seg_area; // total area of all segmented regions
  int total_area; // total area of the image
  double area_ratio; // ratio of total segmented rectangle area
                     // to total image area
  vector<GTBoxDescription> descriptions; // metrics useful in weighting
                                         // the importance of each box
};

// this describes a region which was detected for the hypothesis
// regions detected in the hypothesis can only have either true
// or false positive pixels (since if a region was detected then all the
// pixels within it are, by definition, positive detections). However,
// if a hypothesis box only partially overlaps a groundtruth one, then
// it is possible that the false negatives can be counted on the spot
// by counting the pixels in the corresponding groundtruth region
// which do not overlap the hypothesis one. For regions which were
// entirely missed by the hypothesis, however, it will be necessary
// to iterate the groundtruth vertices since there will be no edges
// from the hypothesis vertices to these ones.
struct RegionDescription {
  int num_fg_pixels;
  int area;
  BOX* box;
  Vertex* vertptr;
  double recall; // total true positive pixels detected in region
                 // divided by the total positives in the groundtruth
  double fallout; // total false positive pixels detected in the region
                  // divided by the total negative pixels in the groundtruth
  double precision; // total true positive pixels detected in region
                    // divided by the total positive pixels in hypothesis
                    // (including incorrect ones ie false postives)
  double false_discovery; // total false positive pixels detected in the
                          // region divided by all of the positive pixels
                          // in the hypothesis
  int true_positive_pix;
  int false_positive_pix;
  int false_negative_pix; // the number of pixels which are in the groundtruth
                          // but do not overlap with the hypothesis
  int num_gt_overlap; // the number of groundtruth regions which overlap this one
                      // this is needed to determine the overall number of
                      // regions in the groundtruth that were undersegmented
                      // as well as for getting an average number of groundtruth
                      // regions for each undersegmented hypothesis region
};

struct OverlappingGTRegion {
  Vertex* vertptr;
  BOX* box;
  int falsenegativepix;
  int numedges;
};

// True Positive Rate (Sensitivity, Hit Rate, Recall): TPR = TP/P
// False Positive Rate (Fallout): FPR = FP/N
// Accuracy: ACC = (TP+TN)/(P+N)
// True Negative Rate (Specificity): SPC = TN/N = 1-FPR
// Positive Predictive Value (Precision): PPV = TP/(TP+FP)
// Negative Predictive Value: NPV = TN/(TN+FN)
// False Discovery Rate: FDR = FP/(FP+TP)
struct HypothesisMetrics {
  int correctsegmentations; // total correctly segmented regions
                            // in order for a box to be considered
                            // correctly segmented it must overlap
                            // with all of the foreground pixels
                            // in the groundtruth box with which
                            // the region is overlapping
  double total_recall; // the summation of the recalls for each
                       // region detected in the hypothesis
  double total_fallout; // the summation of the fallouts for each
                        // region detected in the hypothesis
  double total_precision; // the summation of the precisions for
                          // each region detected in the hypothesis
  double total_fdr; // the summation of all the false_discoveries
                    // for each region detected in the hypothesis
  int oversegmentations; // whenever a box in the groundtruth has more
                         // than one intersection in the hypothesis then
                         // this counts as an oversegmentation
  double avg_oversegmentations_perbox; // avg number of hyptothesis boxes
                                       // greater than one corresponding to
                                       // a groundtruth box
  int undersegmentations; // whenever a box in the hypothesis has more than
                          // one intersection in the groundtruth then this
                          // counts as an undersegmentation
  double avg_undersegmentations_perbox; // avg number of groundtruth boxes
                                        // greater than one corresponding
                                        // to a box in the hypothesis
  int oversegmentedcomponents; // number of oversegmented rectangles
  int undersegmentedcomponents; // number of undersegmented (merged) rects
  int falsenegatives; // number of completely missed regions
  int falsepositives; // number of entirely falsely detected regions
  double negative_predictive_val; // total true negative foreground pixels
                                  // detected in the hypothesis divided by the
                                  // total negative pixels in the hypothesis
  double specificity; // should be equivalent to 1-total_fallout, this is
                      // the true negatives detected in the hypothesis
                      // divided by the total true negatives in the groundtruth
  double accuracy; // gives the percentage of correct negative and
                   // positive detections overall
  int total_false_negative_pix; // total number of wrongly missed pixels
  int total_false_positive_pix;
  int total_positive_fg_pix; // the total segmented foreground pixels
                             // by the hypothesis = (TP+FP)
  int total_true_positive_fg_pix; // total correctly segmented foreground pixels
  int total_true_negative_fg_pix; // total correctly unsegmented foreground pixels
  int total_fg_pix; // the total number of foreground pixels for the entire image
  int total_negative_fg_pix; // = total_fg_pix - total_positive_fg_pix
                             // this is the total number of pixels detected
                             // as negative in the hypothesis (TN+FN)
  vector<RegionDescription> boxes; // metrics on each hypothesis rectangle
  vector<OverlappingGTRegion> overlapgts;
};

/**********************************************************************
*   The bipartite graph class is used to evaluate the
* accuracy of document analysis on a single image. Thus if multiple
* images are to be tested then each one will use its own bipartite
* graph separately for evaluation. For the image being evaluated
* there will be a hypothesis segmentation which is to be evaluated
* against the groundtruth (correct) segmentation. The segmentation
* of both the groundtruth and the hypothesis are specified by their
* own respective image and box file pairs. The image contains all
* of the foreground regions colored based upon how they were segmented
* and the box file contains all of the rectangles representing the
* regions that were segmented.
*   The edges between the groundtruth and hypothesis represent the
* intersection of pixels between them. If a vertex is unmatched by
* the other image then it will have no edges. By analyzing the number
* of vertices and edges as well as their associated weights it is
* then possible to compute various metrics on the hypothesis accuracy:
* 1. Total Correct Segmentations - Total true positive detections.
*          A bounding box is labeled as a correct segmentation if enough
*          of its foreground pixels in the hypothesis image were colored
*          the same as in the groundtruth (in the colorblobs dir). If the
*          entire page is labeled as math (and there is math in the
*          groundtruth) then hypothesis will have all of the correct
*          segmentations but will also have a significant number of
*          false positives and undersegmentations (7).
* 2. Total Oversegmentations - If a rectangle in the groundtruth is
*          divided into more than one rectangle in the hypothesis, then
*          each of the hypothesis rectangles in that region are
*          oversegmentations of the groundtruth one. The total
*          number of oversegmentations is the sum of oversegmentations
*          for each such rectangle in the groundtruth.
* 3. Total Undersegmentations - If a rectangle in the hypothesis is
*          divided into more than one rectangle in the groundtruth, then
*          the all-encompassing hypothesis rectangle is labeled as an
*          undersegmentation component having the number of groundtruth
*          rectangles inside it as it's number of undersegmentations.
*          The sum of all of the undersegmentations for each such
*          rectangle is the total undersegmentations
* 4. Total Oversegmented Components - The total number of rectangles in
*          the groundtruth that are divided into more than one rectangle
*          in the hypothesis.
* 5. Total Undersegmented Components - The total number of rectangles in
*          the hypothesis that are divided into more than one rectangle
*          in the groundtruth.
* 6. Total Missed Components (False Negatives) - The total number of
*          rectangles (or part of a rectangle) that are in the groundtruth
*          but not in the hypothesis.
* 7. Total False Alarms (False Positives) - The total number of rectangles
*          (or part of a rectangle) that aren't in the hypothesis but not
*          the groundtruth.
**********************************************************************/
class BipartiteGraph {
public:
  // builds the bipartite graph the document analysis carried out
  // on the given type of block (i.e. for math equation detection
  // this could be either displayed regions, embedded regions,
  // or equation labels.
  BipartiteGraph(string type, GraphInput input);


  void getHypothesisMetrics();

  // Prints the image-wide metrics
  void printMetrics(FILE* stream);

  // Prints the metrics for each individual
  // region of the image
  void printMetricsVerbose(FILE* stream);

  // for debugging
  void printSet(Bipartite::GraphChoice graph);

  void clear();

  HypothesisMetrics hypmetrics;
  GroundTruthMetrics gtmetrics;

private:

  // creates all the vertices for the given set in the
  // bipartite graph (either hypothesis or groundtruth)
  // while appending them to their appropriate vector
  void makeVertices(Bipartite::GraphChoice graph);

  // creates all of the edges for the given set in the
  // bipartite graph (either hypothesis or groundtruth)
  // and appends them to their appropriate vertex
  void makeEdges(Bipartite::GraphChoice graph);

  void getGroundTruthMetrics();

  LayoutEval::Color getColorFromType(const string& type);

  string type; // the type of rectangle to look for in the txt file
  LayoutEval::Color color; // the color associated with the type

  vector<Vertex> GroundTruth; // the groudtruth set
  vector<Vertex> Hypothesis; // the hypothesis set
  Lept_Utils lu;

  ifstream gtfile;
  ifstream hypfile;
  int filenum;
  string filename;
  PIX* inimg;
  PIX* hypimg;
  PIX* gtimg;
};

#endif
