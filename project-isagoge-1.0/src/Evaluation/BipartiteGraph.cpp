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
 * ----------------------------------------------------------------------
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

#include "BipartiteGraph.h"

#include <assert.h>

#include "Basic_Utils.h"
using namespace Basic_Utils;

#define debug 0
//#define SHOW_HYP_VERTICES
//#define SHOW_VERTICES
//#define SHOW_TRUE_NEGATIVES
// saves the tracker to a file, optionally displays as well if DISPLAY_ON is turned on
#define SHOW_HYP_TRACKER_FINAL
//#define DISPLAY_ON

BipartiteGraph::BipartiteGraph(string type_, GraphInput input)
: type(type_), typemode(true) {
  if(type == "all")
    typemode = false;
  color = getColorFromType(type);
  if(!Lept_Utils::isColorSignificant(color)) {
    cout << "ERROR: No evaluation color-code associated with specified type\n";
    exit(EXIT_FAILURE);
  }
  tracker_dir = input.dbgdir;
  // extract all the info from the GraphInput struct
  string hypboxfilename = input.hypboxfile;
  string gtboxfilename  = input.gtboxfile;
  string imgname    = input.imgname;
  vector<string> tmp = stringSplit(imgname, '.');
  filenum = atoi(tmp[0].c_str());
  filename = imgname;
  hypimg       = input.hypimg;
  gtimg        = input.gtimg;
  inimg        = input.inimg;
  assert(hypimg->w == gtimg->w && hypimg->w == inimg->w
      && hypimg->h == gtimg->h && hypimg->h == inimg->h);
  gt_tracker = pixCreate(gtimg->w, gtimg->h, 32); // TODO: Make the background white!!
  hyp_tracker = pixCreate(gtimg->w, gtimg->h, 32);

  // open both of the files for reading
  gtfile.open(gtboxfilename.c_str(), ifstream::in);
  if((gtfile.rdstate() & ifstream::failbit) != 0) {
    cout << "ERROR: Could not open " << gtboxfilename << endl;
    exit(EXIT_FAILURE);
  }
  hypfile.open(hypboxfilename.c_str(), ifstream::in);
  if((hypfile.rdstate() & std::ifstream::failbit) != 0 ) {
    cout << "ERROR: Could not open " << hypboxfilename << endl;
    exit(EXIT_FAILURE);
  }

  // build and append all the vertices to the groundtruth set and
  // then the hypothesis set
  makeVertices(Bipartite::GroundTruth);
  makeVertices(Bipartite::Hypothesis);

#ifdef SHOW_VERTICES
  string gt_vertex_file = tracker_dir + "gt_vertices.png";
  cout << "Showing the foreground regions of the vertices in the groundtruth image "
       << "and saving them to " << gt_vertex_file << endl;
  pixDisplay(gt_tracker, 100, 100);
  pixWrite(gt_vertex_file.c_str(), gt_tracker, IFF_PNG);
  waitForInput();
  string hyp_vertex_file = tracker_dir + "hyp_vertices.png";
  cout << "Showing the foreground regions of the vertices in the hypothesis image "
       << "and saving them to " << hyp_vertex_file << endl;
  pixDisplay(hyp_tracker, 100, 100);
  pixWrite(hyp_vertex_file.c_str(), hyp_tracker, IFF_PNG);
  waitForInput();
#endif
  // build and append all of the edges to the vertices for the
  // groundtruth set and then the hypothesis set
  makeEdges(Bipartite::GroundTruth);
  makeEdges(Bipartite::Hypothesis);

  if(debug) {
    cout << "-----------------\nPrinting the Groundtruth\n---------------\n";
    printSet(Bipartite::GroundTruth);
    cout << "-----------------\nPrinting the Hypothesis\n---------------\n";
    cout << "hypothesis vertices: " << Hypothesis.size() << endl;
    printSet(Bipartite::Hypothesis);
  }
}

//TODO: Run valgrind to make sure I'm not missing anything...
BipartiteGraph::~BipartiteGraph() {
  if(gt_tracker != NULL)
    pixDestroy(&gt_tracker);
  if(hyp_tracker != NULL)
    pixDestroy(&hyp_tracker);
  destroyVerticesAndEdges(Bipartite::GroundTruth);
  destroyVerticesAndEdges(Bipartite::Hypothesis);
  destroyMetrics();
}

void BipartiteGraph::destroyVerticesAndEdges(Bipartite::GraphChoice graphchoice) {
  vector<Vertex>* graph;
  if(graphchoice == Bipartite::GroundTruth)
    graph = &GroundTruth;
  else
    graph = &Hypothesis;
  for(int i = 0; i < graph->size(); ++i) {
    Vertex v = (*graph)[i];
    BOX* b = v.rect;
    boxDestroy(&b);
    vector<Edge> edges = v.edges;
    for(int j = 0; j < edges.size(); ++j) {
      Edge e = edges[j];
      e.vertexptr = NULL; // avoid dangling pointer
    }
  }
}

void BipartiteGraph::destroyMetrics() {
  vector<RegionDescription> pbox_metrics = hypmetrics.boxes;
  for(int i = 0; i < pbox_metrics.size(); ++i) {
    RegionDescription rd = pbox_metrics[i];
    rd.box = NULL;
    rd.vertptr = NULL;
  }
  vector<OverlappingGTRegion> fnbox_metrics = hypmetrics.overlapgts;
  for(int i = 0; i < fnbox_metrics.size(); ++i) {
    OverlappingGTRegion gtr = fnbox_metrics[i];
    gtr.box = NULL;
    gtr.vertptr = NULL;
  }
}

void BipartiteGraph::makeVertices(Bipartite::GraphChoice graph) {
  ifstream* file;
  PIX* img;
  vector<Vertex>* set;
  PIX* tracker;

  if(graph == Bipartite::GroundTruth) {
    file = &gtfile;
    img = gtimg;
    set = &GroundTruth;
    tracker = gt_tracker;
  } else {
    file = &hypfile;
    img = hypimg;
    set = &Hypothesis;
    tracker = hyp_tracker;
  }
  int idx = 0;
  int max = 55;
  char* curline = new char[max];
  while(!file->eof()) {
    file->getline(curline, max);
    string curlinestr = (string)curline;
  //  if(graph == Bipartite::Hypothesis)
   //   cout << "hypline: " << curlinestr << endl;
    assert(curlinestr.length() < max);
    // parse the line
    if(curlinestr.empty())
      continue;
    vector<string> splitline = stringSplit(curlinestr);
    string filename   = splitline[0];
    string recttype   = splitline[1];
    int rectleft   = atoi(splitline[2].c_str());
    int recttop    = atoi(splitline[3].c_str());
    int rectright  = atoi(splitline[4].c_str());
    int rectbottom = atoi(splitline[5].c_str());
    vector<string> tmp = stringSplit(filename, '.');
    int curfilenum = atoi(tmp[0].c_str());

    // .dat file is very specific format where data from
    // the same file numbers are clustered together
    // (i.e. all the 1's go before the 2's etc)
    if(curfilenum < filenum)
      continue;
    if(curfilenum > filenum)
      break;
    if(recttype != type && typemode)
      continue;

    // now we know we have a vertex so build and append
    // it to the appropriate vector
    if(graph == Bipartite::GroundTruth &&
        (rectleft == -1 || recttop == -1  || rectright == -1 || rectbottom == -1))
      continue; // empty groundtruth images have an entry like this, just skip it
    BOX* box = boxCreate(rectleft, recttop, rectright-rectleft,
        rectbottom-recttop); // create the box
    Vertex vert; // create the vertex
    vert.rect = box; // put the box in the vertex
    // count the foreground pixels and put them in the vertex
    int duplicate_cnt = 0;
    vert.pix_foreground = countColorPixels(box, img, color, tracker,
        !typemode, duplicate_cnt);
    vert.pix_foreground_duplicate = duplicate_cnt;
    vert.setindex = idx;
    vert.area = (rectright-rectleft)*(rectbottom-recttop);
    vert.whichset = (graph == Bipartite::Hypothesis) ?\
        (string)"Hypothesis" : (string)"GroundTruth";
#ifdef SHOW_HYP_VERTICES
    if(graph == Bipartite::Hypothesis) {
      cout << "Creating hypothesis vertex for displayed highlighted region\n";
      Lept_Utils::dispHLBoxRegion(box, inimg);
      Lept_Utils::dispRegion(box, hypimg);
      cout << "Region has " << vert.pix_foreground << " foreground pixels.\n";
      cout << vert.pix_foreground_duplicate
           << " of them were already counted by a different vertex.\n";
      waitForInput();
    }
#endif
    set->push_back(vert);
    idx++;
  }
}

void BipartiteGraph::makeEdges(Bipartite::GraphChoice graph) {
  vector<Vertex>* make_edges_for; // set to make the edges for
  vector<Vertex>* make_edges_from; // the set the edges will point to
  PIX* edge_for_pix;
  PIX* edge_from_pix;
  PIX* edge_for_pix_tracker;
  PIX* edge_from_pix_tracker;
  if(graph == Bipartite::GroundTruth) {
    make_edges_for = &GroundTruth;
    make_edges_from = &Hypothesis;
    edge_for_pix = gtimg;
    edge_from_pix = hypimg;
    edge_for_pix_tracker = gt_tracker;
    edge_from_pix_tracker = hyp_tracker;
  } else {
    make_edges_for = &Hypothesis;
    make_edges_from = &GroundTruth;
    edge_for_pix = hypimg;
    edge_from_pix = gtimg;
    edge_for_pix_tracker = hyp_tracker;
    edge_from_pix_tracker = gt_tracker;
  }
  for(vector<Vertex>::iterator vert = make_edges_for->begin();
      vert != make_edges_for->end(); vert++) {
    // look for intersections of pixels and add an edge for each
    // one that is found
    BOX* vertbox = vert->rect;
    for(vector<Vertex>::iterator edgevert = make_edges_from->begin();
        edgevert != make_edges_from->end(); edgevert++) {
      BOX* edgebox = edgevert->rect; // potential edge
      l_int32 intersects = 0;
      boxIntersects(vertbox, edgebox, &intersects);
      if(!intersects)
        continue;
      // edge found! create the edge and append it to the current
      // vertex's edge list. first need to find the number of pixels
      // that intersect as well as the area that intersects
      BOX* overlap = boxOverlapRegion(vertbox, edgebox);
      int intersect1 = 0;
      int notcounted1 = countColorPixels(overlap, edge_for_pix, color,
          edge_for_pix_tracker, !typemode, intersect1);
      int intersect2 = 0;
      int notcounted2 = countColorPixels(overlap, edge_from_pix, color,
          edge_from_pix_tracker, !typemode, intersect2);
      if(debug) {
        if(intersect1 != intersect2) {
          cout << "edge from " << (make_edges_from == &Hypothesis ? "hypthesis" : "groundtruth")
               << " to " << (make_edges_for == &Hypothesis ? "hypthesis" : "groundtruth")
               << " has an unexpected number of color pixels at region of intersection!\n";
          Lept_Utils::dispRegion(overlap, edge_from_pix);
          cout << "showing vertex from which edge is measured. "
               << intersect2 << " pixels found\n";
          waitForInput();
          Lept_Utils::dispRegion(overlap, edge_for_pix);
          cout << "showing vertex to which edge is measured. "
               << intersect1 << " pixels found\n";
          waitForInput();
          if(edge_from_pix->w == edge_for_pix->w
              && edge_from_pix->h == edge_for_pix->h) {
            cout << "The images have the same dimensions.\n";
          }
          else
            cout << "The images have different dimensions!!!\n";
          cout << "showing the entire image from which edge is measured\n";
          pixDisplay(edge_from_pix, 100, 100);
          waitForInput();
          cout << "showing the entire image to whic hthe edge is measured\n";
          pixDisplay(edge_for_pix, 100, 100);
        }
      }
      // the region of overlap should have the same number of color-coded pixels in each image
      assert(intersect1 == intersect2);
      // the foreground region of all vertices should have already been counted
      assert(notcounted1 == 0 && notcounted2 == 0);
      Edge edge;
      edge.vertexptr = &(*edgevert);
      edge.pixfg_intersecting = intersect1;
      edge.overlap_area = (int)(overlap->w) * (int)(overlap->h);
      vert->edges.push_back(edge);
      boxDestroy(&overlap);
    }
  }
}

void BipartiteGraph::getHypothesisMetrics() {
  getGroundTruthMetrics();
  // total segmented foreground pixels in the groundtruth
  const int gt_positive_fg_pix = gtmetrics.total_seg_fg_pixels;
  // make sure it matches up to what was found when making the vertices
  int check_gt_positives = 0;
  for(int i = 0; i < GroundTruth.size(); ++i)
    check_gt_positives += GroundTruth[i].pix_foreground;
  assert(check_gt_positives == gt_positive_fg_pix);


  // initialize everything
  hypmetrics.total_fg_pix = gtmetrics.total_fg_pixels;
  hypmetrics.correctsegmentations = 0;
  hypmetrics.total_recall = 0;
  hypmetrics.total_fallout = 0;
  hypmetrics.total_precision = 0;
  hypmetrics.total_fdr = 0;
  hypmetrics.oversegmentations = 0;
  hypmetrics.avg_oversegmentations_perbox = 0;
  hypmetrics.undersegmentations = 0;
  hypmetrics.avg_undersegmentations_perbox = 0;
  hypmetrics.oversegmentedcomponents = 0;
  hypmetrics.undersegmentedcomponents = 0;
  hypmetrics.falsenegatives = 0;
  hypmetrics.falsepositives = 0;
  hypmetrics.negative_predictive_val = 0;
  hypmetrics.specificity = 0;
  hypmetrics.total_positive_fg_pix = 0;
  hypmetrics.total_false_negative_pix = 0;
  hypmetrics.total_false_positive_pix = 0;
  hypmetrics.total_true_positive_fg_pix = 0;
  const int gt_negative_fg_pix = gtmetrics.total_nonseg_fg_pixels;

  // need to sum up all the positive pixels detected in the entire
  // hypothesis set so we can make the precision and false discovery
  // rate calculations later
  int hyp_positive_fg_pix_tmp = 0;
  for(vector<Vertex>::iterator hyp_it = Hypothesis.begin(); \
      hyp_it != Hypothesis.end(); hyp_it++)
    hyp_positive_fg_pix_tmp += hyp_it->pix_foreground;
  const int hyp_positive_fg_pix = hyp_positive_fg_pix_tmp;
  // make sure that the number of positive pixels counted here is the same as the
  // number of positive ones (color-coded white) in the hyp_tracker image.
  int hyp_positives = 0;
  for(int i = 0; i < hypimg->h; ++i) {
    for(int j = 0; j < hypimg->w; ++j) {
      l_uint32* curpix = Lept_Utils::getPixelAtXY(hyp_tracker, j, i);
      if(Lept_Utils::getPixelColor(curpix) == LayoutEval::WHITE)
        ++hyp_positives;
    }
  }
  assert(hyp_positives == hyp_positive_fg_pix);

  // if there aren't any segmented regions in the groundtruth
  // then there can't be any correct segmentations and
  // we already know that all of the segemented pixels in the
  // hypothesis are false positives! If that's the case we'll
  // go ahead and set a flag to avoid any confusion later
  bool all_false_positives = false;
  if(gt_positive_fg_pix <= 0) {
    all_false_positives = true;
    // if there are boxes and none of them have foreground pixels
    // then we already know off the bat that something is seriously
    // wrong here.
    if(GroundTruth.size() > 0) {
      cout << "ERROR: None of the foreground pixels in the "
           << "GroundTruth.dat rectangles were detected!!\n";
      exit(EXIT_FAILURE);
    }
  }

  // go ahead and count and track all the true negative
  // pixels in the hypothesis
  int counted_truenegatives = countTrueNegatives();

#ifdef SHOW_TRUE_NEGATIVES
  string dbgTNfile = tracker_dir + "TrueNegatives.png";
  cout << "Displaying the hypothesis image with the true negatives in orange and "
       << "the positively recognized pixels (tp+fp) in white. The image is also "
       << "saved to " << dbgTNfile << ".\n";
  pixDisplay(hyp_tracker, 100, 100);
  pixWrite(dbgTNfile.c_str(), hyp_tracker, IFF_PNG);
  waitForInput();
#endif

  // iterate through the hypothesis boxes (vertices) to get
  // metrics on each individual one
  for(vector<Vertex>::iterator hyp_it = Hypothesis.begin();
      hyp_it != Hypothesis.end(); hyp_it++) {
    // initialize some parameters for the hypothesis box
    // to be used in determining the metrics for the box
    l_int32 h_x, h_y, h_w, h_h; // hypothesis box top left,
                                // width, and height
    BOX* hyp_box = hyp_it->rect;
    boxGetGeometry(hyp_box, &h_x, &h_y, &h_w, &h_h);
    int hyp_box_fg_pix = hyp_it->pix_foreground;
    int hyp_box_fg_pix_duplicate = hyp_it->pix_foreground_duplicate;
    vector<Edge> edges = hyp_it->edges;

    // initialize the current box's description
    // before we calculate it
    RegionDescription hyp_box_desc; // description of the current box
    hyp_box_desc.num_fg_pixels = hyp_box_fg_pix;
    hyp_box_desc.num_fg_pixels_duplicate = hyp_box_fg_pix_duplicate;
    hyp_box_desc.area = hyp_it->area;
    hyp_box_desc.box = hyp_it->rect;
    hyp_box_desc.vertptr = &(*hyp_it);
    hyp_box_desc.recall = 0;
    hyp_box_desc.fallout = 0;
    hyp_box_desc.precision = 0;
    hyp_box_desc.false_discovery = 0;
    hyp_box_desc.false_negative_pix = 0;
    hyp_box_desc.true_positive_pix = 0;
    hyp_box_desc.false_positive_pix = 0;
    const int num_edges = edges.size();
    hyp_box_desc.num_gt_overlap = num_edges;


    if(num_edges == 0) {
      // no intersections with the groundtruth
      // all the pixels in this region are false positives
      vector<BOX*> bv;
      BOX* b = boxCreate(0,0,0,0);
      bv.push_back(b);
      int duplicate_fp = 0;
      const int falsepositive_pix = countFalsePositives(hyp_box,
          bv, duplicate_fp);
      double fallout = ((double)falsepositive_pix + (double)duplicate_fp) /
          (double)gt_negative_fg_pix;
      double fallout_duplicate = (double)duplicate_fp / (double)gt_negative_fg_pix;
      hyp_box_desc.fallout = fallout;
      hyp_box_desc.fallout_duplicate = fallout_duplicate;
      hypmetrics.falsepositives++;
      hyp_box_desc.false_positive_pix = falsepositive_pix;
      hyp_box_desc.false_positive_pix_duplicate = duplicate_fp;
    }
    else if(num_edges >= 1) {
      // there is at least one region in the groundtruth
      // that intersects with this one.
      // each overlapping groundtruth region is either a
      // correct segmentation or is only partially correct.
      if(num_edges > 1) {
        hypmetrics.undersegmentations += num_edges;
        hypmetrics.undersegmentedcomponents++;
      }

      // first count all the false positive pixels
      // in the hypothesis box and also count the number
      // of false negatives which could spring up in the
      // case that the hypothesis box does not completely
      // overlap the groundtruth one, then count the
      // true positives (pixels in both the groundtruth
      // region and the hypothesis region)
      vector<Box*> edgeboxes; // first put all of the boxes
                              // into a vector
      for(vector<Edge>::iterator edgeit = edges.begin();
          edgeit != edges.end(); edgeit++)
        edgeboxes.push_back(edgeit->vertexptr->rect);
      int fp_duplicates = 0;
      const int falsepositives = countFalsePositives(hyp_box,
          edgeboxes, fp_duplicates);
      int tp_duplicates = 0;
      const int truepositives = countTruePositives(hyp_box,
          edgeboxes, tp_duplicates);
      if((truepositives + tp_duplicates + falsepositives + fp_duplicates)
          != (hyp_box_fg_pix + hyp_box_fg_pix_duplicate)) {
        cout << "ERROR: The sum of the true and false positives found in "
             << "a rectangle is not equal to the number of foreground "
             << "pixels in that region!\n";
        cout << "This occurred in image: " << filename << endl;
        cout << "Here is the sum of true and false positives: "
             << truepositives+falsepositives << endl;
        cout << "here's the sum of the duplicate true and false positives: "
             << tp_duplicates + fp_duplicates << endl;
        cout << "Here is the number of foreground pixels in the region: "
             << hyp_box_fg_pix << endl;
        cout << "here is the number of duplicate fg pixels in the region: "
             << hyp_box_fg_pix_duplicate << endl;
        cout << "Here is the number of true positives in the region: "
             << truepositives << endl;
        cout << "Here is the number of true positives that werealready counted: "
             << tp_duplicates << endl;
        cout << "Here is the number of false positives in the region: "
             << falsepositives << endl;
        cout << "Here is the number of false positives that were already counted: "
             << fp_duplicates << endl;
        cout << "Here are the dimensions of the hypothesis region "
             << "(l,t,w,h): " << h_x << ", " << h_y << ", " << h_w
             << ", " << h_h << endl;
        cout << "Here are the dimensions of the overlapping groundtruth region(s):\n";
        for(int i = 0; i < edgeboxes.size(); i++) {
          cout << "(l,t,w,h): " << edgeboxes[i]->x << ", " << edgeboxes[i]->y
               << ", " << edgeboxes[i]->w << ", " << edgeboxes[i]->h << endl;
          Lept_Utils::dispRegion(edgeboxes[i], gtimg);
        }
        Lept_Utils::dispRegion(hyp_box, hypimg);
        pixDisplay(hyp_tracker, 100, 100);
        exit(EXIT_FAILURE);
      }

      // only true and false positives can be detected from counting pixels in the
      // hypothesis boxes and comparing them to the groundtruth. When counting pixels in
      // the groundtruth boxes and comparing them to the hypothesis the true and false
      // negative rates can then be counted. For now only the true positive rate,
      // false positive rate, precision and false discovery rate are calculated
      // for each region.
      hyp_box_desc.recall = (double)truepositives/(double)gt_positive_fg_pix;
      hyp_box_desc.fallout = (double)falsepositives/(double)gt_negative_fg_pix;
      hyp_box_desc.precision = (double)truepositives/(double)hyp_positive_fg_pix;
      hyp_box_desc.false_discovery = (double)falsepositives/(double)hyp_positive_fg_pix;
      hyp_box_desc.true_positive_pix = truepositives;
      hyp_box_desc.false_positive_pix = falsepositives;
    }
    else {
      cout << "ERROR: Box with negative # of edges!\n";
      exit(EXIT_FAILURE);
    }
    hypmetrics.boxes.push_back(hyp_box_desc);
  }
  if(hypmetrics.undersegmentations > 0) {
    hypmetrics.avg_undersegmentations_perbox = (double)hypmetrics.undersegmentations /
        (double)hypmetrics.undersegmentedcomponents;
  }

  // now iterate the groundtruth to count the false negative
  // regions in the hypothesis and go ahead and add them onto the
  // vector of RegionDescriptions
  // also find the regions that were oversegmented by the hypothesis
  // and include that information in the hypothesis metrics as well
  vector<OverlappingGTRegion> overlappingregions;
  for(vector<Vertex>::iterator gt_it = GroundTruth.begin();
      gt_it != GroundTruth.end(); gt_it++) {
    BOX* gtbox = gt_it->rect;
    vector<Edge> edges = gt_it->edges;
    const int numedges = edges.size();
    vector<Box*> hypboxes;
    for(vector<Edge>::iterator edgeit = edges.begin();
        edgeit != edges.end(); edgeit++)
      hypboxes.push_back(edgeit->vertexptr->rect);
    const int gt_fg_pix = gt_it->pix_foreground;
    if(numedges == 0) {
      // there are no edges thus this entire region was
      // missed completely by the hypothesis
      RegionDescription missedregion;
      missedregion.num_fg_pixels = gt_fg_pix;
      missedregion.area = gt_it->area;
      missedregion.box = gt_it->rect;
      missedregion.vertptr = &(*gt_it);
      missedregion.recall = 0;
      missedregion.fallout = 0;
      missedregion.precision = 0;
      missedregion.false_discovery = 0;
      missedregion.true_positive_pix = 0;
      missedregion.false_positive_pix = 0;
      missedregion.num_gt_overlap = 0;
      missedregion.false_negative_pix = gt_fg_pix;
      Lept_Utils::fillBoxForeground(hyp_tracker, gt_it->rect, LayoutEval::GREEN, inimg);
      hypmetrics.boxes.push_back(missedregion);
      hypmetrics.falsenegatives++;
    }
    else {
      OverlappingGTRegion overlapgt;
      // check for false negatives
      int fn_duplicates = 0;
      int falsenegatives = countFalseNegatives(gtbox, hypboxes,
          fn_duplicates);
      if(falsenegatives == 0 && fn_duplicates == 0)
        hypmetrics.correctsegmentations++;
      overlapgt.falsenegativepix = falsenegatives;
      overlapgt.falsenegativepix_duplicates = fn_duplicates;
      overlapgt.box = gtbox;
      overlapgt.vertptr = &(*gt_it);
      overlapgt.numedges = numedges;
      if(numedges > 1){
        hypmetrics.oversegmentations += numedges;
        hypmetrics.oversegmentedcomponents++;
      }
      overlappingregions.push_back(overlapgt);
    }
  }
  hypmetrics.overlapgts = overlappingregions;
  if(hypmetrics.oversegmentations > 0) {
    hypmetrics.avg_oversegmentations_perbox = (double)hypmetrics.oversegmentations /
        (double)hypmetrics.oversegmentedcomponents;
  }
  // now that we have all the metrics for each rectangle it is
  // time to combine all that information in order to get the
  // total metrics for the entire image
  vector<RegionDescription> regions = hypmetrics.boxes;
  for(vector<RegionDescription>::iterator region = regions.begin();
      region != regions.end(); region++) {
    hypmetrics.total_recall += region->recall;
    hypmetrics.total_fallout += region->fallout;
    hypmetrics.total_precision += region->precision;
    hypmetrics.total_fdr += region->false_discovery;
    int tp = region->true_positive_pix;
    int fp = region->false_positive_pix;
    int tp_fp = tp + fp;
    hypmetrics.total_false_positive_pix += fp;
    hypmetrics.total_positive_fg_pix += tp_fp;
    int fn = region->false_negative_pix;
    hypmetrics.total_false_negative_pix += fn;
    hypmetrics.total_true_positive_fg_pix += tp;
  }
  if(debug) {
    if(hyp_positive_fg_pix != hypmetrics.total_positive_fg_pix) {
      cout << "the correct positives: " << hyp_positive_fg_pix << endl;
      cout << "the one found from combining total true and false positives: "
           << hypmetrics.total_positive_fg_pix << endl;
    }
  }
  assert(hyp_positive_fg_pix == hypmetrics.total_positive_fg_pix);

  // add false negatives for overlapping groundtruth boxes (already counted
  // the false negatives for non-overlapping groundtruth boxes, i.e. ones
  // that are completely missing in the hypothesis)
  for(vector<OverlappingGTRegion>::iterator ogt_it =
      overlappingregions.begin(); ogt_it != overlappingregions.end();
      ogt_it++) {
    hypmetrics.total_false_negative_pix += ogt_it->falsenegativepix;
  }

  // now calculate the total negatively detected pixels by the
  // hypothesis (this is the same as TN+FN, both true negatives
  // and false negatives)
  const int total_fg = hypmetrics.total_fg_pix;
  const int total_positive_fg = hypmetrics.total_positive_fg_pix;
  const int total_hyp_negative = total_fg - total_positive_fg;
  hypmetrics.total_negative_fg_pix = total_hyp_negative;

  // the total actual negatives are known from the groundtruth
  // metrics and the true negatives found in the hypothesis can
  // be either a subset of the actual negatives or their entirety
  // if the specificity (TNR) was perfect
  const int total_false_neg_pix = hypmetrics.total_false_negative_pix;
  const int hyp_true_negatives = total_hyp_negative - total_false_neg_pix;
  if(hyp_true_negatives != counted_truenegatives) {
    cout << "ERROR: the total false negatives and true negatives "
         << "counted don't add up to the total negatives "
         << "(i.e. total_foreground - total_positive)!\n";
    cout << "hyp_true_negatives: " << hyp_true_negatives << endl;
    cout << "counted true negatives: " << counted_truenegatives << endl;
    cout << "false negatives: " << total_false_neg_pix << endl;
    cout << "false positives: " << hypmetrics.total_false_positive_pix << endl;
    cout << "total_fg - false negatives: " << total_hyp_negative << endl;
    cout << "negatives in the groundtruth: "
         << gtmetrics.total_nonseg_fg_pixels << endl;
    cout << "positives in the groundtruth: "
         << gtmetrics.total_seg_fg_pixels << endl;
    pixDisplay(hyp_tracker, 100, 100);
    exit(EXIT_FAILURE);
  }
  hypmetrics.total_true_negative_fg_pix = hyp_true_negatives;
  const int gt_true_negatives = gtmetrics.total_nonseg_fg_pixels;

  // now to calculate the specificity (TN/N) and make sure it is
  // the same as 1-FPR
  // FPR = FP/N
  // SPC = TN/N
  // N/N - FP/N = TN/N -> N-FP=TN
  double specificity = (double)hyp_true_negatives /
      (double)gt_true_negatives;

  //assert(specificity == (1.-hypmetrics.total_fallout));
  double oneminusfpr = (double)1- (double)hypmetrics.total_fallout;
  if(specificity != oneminusfpr) {
    if((gt_true_negatives - hypmetrics.total_false_positive_pix)
        == hyp_true_negatives) {
      cout << "WARNING: The specificity and 1-FPR are not equal, however ";
      cout << "the false positive and true negative pixels add up to the ";
      cout << "total negatives in the groundtruth.\n";
      cout.precision(20);
      cout << "The specificity and 1-FPR are only off by "
           << specificity-oneminusfpr << endl;
      // something weird happened with division... if the false positives
      // and true negatives add up then there should be nothing wrong!!
      goto noerror;
    }
    cout << "ERROR:: for some reason the specificity does not equal 1-FPR!\n";
    cout << "image: " << filename << endl;
    cout.precision(20);
    cout << "specificity: " << specificity << endl;
    cout << "1-FPR      : " << oneminusfpr << endl;
    cout << "specificity is " << (double)hyp_true_negatives
         << " / " << (double)gt_true_negatives << endl;
    cout << "1-FPR is " << (double)1 << " - "
         << (double)hypmetrics.total_fallout << endl;
    cout << "FPR         : " << hypmetrics.total_fallout << endl;
    cout << "Expected FPR: " << (double)hypmetrics.total_false_positive_pix /
        (double)gt_true_negatives << endl;
    cout << "False positives in hypothesis: " << hypmetrics.total_false_positive_pix << endl;
    cout << "True positives in hypothesis: " << hypmetrics.total_true_positive_fg_pix << endl;
    cout << "true negatives in the hypothesis: " << hyp_true_negatives << endl;
    cout << "true negatives in the groundtruth: " << gt_true_negatives << endl;
    cout << "gt_negative_fg_pix: " << gt_negative_fg_pix << endl;
    cout << "false negatives in the hypothesis: " << total_false_neg_pix << endl;
    cout << "positives in the hypothesis: " << total_positive_fg << endl;
    cout << "positives in the groundtruth: " << gtmetrics.total_seg_fg_pixels << endl;
    cout << "negatives in the hypothesis: " << total_hyp_negative << endl;
    cout << "negatives in the groundtruth: " << gtmetrics.total_nonseg_fg_pixels << endl;
    cout << "total foreground pixels in the hypothesis: " << total_fg << endl;
    cout << "total foreground pixels in the groundtruth: " << gtmetrics.total_fg_pixels << endl;
    cout << "here is the sum of the false positives and the true negatives in hypothesis: "
         << hyp_true_negatives + hypmetrics.total_false_positive_pix << endl;
    int diff = hyp_true_negatives + hypmetrics.total_false_positive_pix - gt_true_negatives;
    cout << "there are " << diff << " more true negatives than there are supposed to be in the hyp!!\n";
    specificity = (1.-hypmetrics.total_fallout);
    exit(EXIT_FAILURE); // (the above solution is a no more than a band-aid... need to figure it out..)
  }
  noerror:
  hypmetrics.specificity = specificity;

  // now calculate the negative predictive value (TN/TN+FN)
  double npv = (double)hyp_true_negatives/(double)total_hyp_negative;
  hypmetrics.negative_predictive_val = npv;

  // now calculate the accuracy
  const int tp = hypmetrics.total_true_positive_fg_pix;
  const int tn = hyp_true_negatives;
  const int p = gtmetrics.total_seg_fg_pixels;
  const int n = gt_true_negatives;
  assert(p+n == hypmetrics.total_fg_pix);
  double accuracy = ((double)tp+(double)tn)/((double)p+(double)n);
  hypmetrics.accuracy = accuracy;
#ifdef SHOW_HYP_TRACKER_FINAL
  string final_tracker_im = tracker_dir + (string)"hyp_tracker_final.png";
  cout << "Displaying the final hypothesis tracker image and saving it to "
       << final_tracker_im << endl;
  pixWrite(final_tracker_im.c_str(), hyp_tracker, IFF_PNG);
#ifdef DISPLAY_ON
  pixDisplay(hyp_tracker, 100, 100);
  waitForInput();
#endif
#endif
}

int BipartiteGraph::countTrueNegatives() {
  int counted_truenegatives = 0;
  for(l_int32 i = 0; i < gtimg->h; i++) {
    for(l_int32 j = 0; j < gtimg->w; j++) {
      l_uint32* cur_gt_pixel = Lept_Utils::getPixelAtXY(gtimg, j, i);
      l_uint32* cur_hyp_pixel = Lept_Utils::getPixelAtXY(hypimg, j, i);
      l_uint32* cur_tracker_pixel = Lept_Utils::getPixelAtXY(hyp_tracker, j, i);
      rgbtype gt_pix_rgb[3];
      rgbtype hyp_pix_rgb[3];
      Lept_Utils::getPixelRGB(cur_gt_pixel, gt_pix_rgb);
      Lept_Utils::getPixelRGB(cur_hyp_pixel, hyp_pix_rgb);
      LayoutEval::Color colorgt = Lept_Utils::getPixelColor(cur_gt_pixel);
      if((!Lept_Utils::isColorSignificant(colorgt) && Lept_Utils::isDark(gt_pix_rgb))
          // ^^checks for an actual negative in the groundtruth that is
          // simply a foreground pixel. below we check
          // for a pixel that may be significant but isn't what we are
          // looking at now (i.e. an embedded pixel when we are evaluating
          // for displayed expressions <- this doesn't apply when typemode is disabled)
          || ((Lept_Utils::isColorSignificant(colorgt) && (colorgt != color)) && typemode)) {
        // negative detected from the groundtruth
        // if the hypothesis pixel here is also negative then we have
        // a true negative!
        LayoutEval::Color colorhyp = Lept_Utils::getPixelColor(cur_hyp_pixel);
        if((colorgt != color && colorhyp != color && typemode)
            || (!Lept_Utils::isColorSignificant(colorgt)
             && !Lept_Utils::isColorSignificant(colorhyp) && !typemode)) {
          // found a true negative!!!
          // set the true negative to orange in tracker image and increment if
          // it hasn't already been counted
          int duplicates = 0;
          countAndTrackPixel(j, i, counted_truenegatives,
              hyp_tracker, LayoutEval::ORANGE, duplicates);
          assert(duplicates == 0); // there's no reason why anything would be double counted here...
        }
      }
    }
  }
  return counted_truenegatives;
}

int BipartiteGraph::countFalsePositives(BOX* hypbox, vector<BOX*> gtboxes,
    int& duplicates) {
  assert(duplicates == 0);
  bool insidegt = false;
  int falsepositives = 0;
  l_int32 x1, y1, w1, h1;
  boxGetGeometry(hypbox, &x1, &y1, &w1, &h1);
  for(l_int32 i = y1; i < y1+h1; i++) {
    for(l_int32 j = x1; j < x1+w1; j++) {
      // if inside one of the groundtruth rectangles then move on
      insidegt = false;
      for(vector<Box*>::iterator boxit = gtboxes.begin();
         boxit != gtboxes.end(); boxit++) {
        Box* box = *boxit;
        l_int32 x2, y2, w2, h2;
        boxGetGeometry(box, &x2, &y2, &w2, &h2);
        if(j >= x2 && i >= y2 && j < x2+w2 && i < y2+h2) {
          insidegt = true;
          break;
        }
      }
      if(insidegt)
        continue;
      // otherwise, if we are at a foreground pixel that hasn't
      // already been counted then increment the counter
      l_uint32* curpixel = Lept_Utils::getPixelAtXY(hypimg, j, i);
      if(Lept_Utils::getPixelColor(curpixel) == color
          || (!typemode &&
              Lept_Utils::isColorSignificant(Lept_Utils::getPixelColor(curpixel)))) {
        countAndTrackPixel(j, i, falsepositives, hyp_tracker,
            LayoutEval::BLUE, duplicates);
      }
    }
  }
  return falsepositives;
}

int BipartiteGraph::countTruePositives(BOX* hypbox, vector<BOX*> gtboxes,
    int& duplicates) {
  assert(duplicates == 0);
  bool insidegt = false;
  int truepositives = 0;
  l_int32 x1, y1, w1, h1;
  boxGetGeometry(hypbox, &x1, &y1, &w1, &h1);
  for(l_int32 i = y1; i < y1+h1; i++) {
    for(l_int32 j = x1; j < x1+w1; j++) {
      // if not inside any of the gtboxes then continue
      insidegt = false;
      for(vector<Box*>::iterator gtboxit = gtboxes.begin();
          gtboxit != gtboxes.end(); gtboxit++) {
        Box* gtbox = *gtboxit;
        l_int32 x2, y2, w2, h2;
        boxGetGeometry(gtbox, &x2, &y2, &w2, &h2);
        if(x2 <= j && y2 <= i && x2+w2 > j && y2+h2 > i) {
          insidegt = true;
          break;
        }
      }
      if(!insidegt)
        continue;
      l_uint32* curpixel = Lept_Utils::getPixelAtXY(hypimg, j, i);
      if(Lept_Utils::getPixelColor(curpixel) == color
          || (!typemode
              && Lept_Utils::isColorSignificant(Lept_Utils::getPixelColor(curpixel)))) {
        countAndTrackPixel(j, i, truepositives, hyp_tracker,
            LayoutEval::RED, duplicates);
      }
    }
  }
  return truepositives;
}

int BipartiteGraph::countFalseNegatives(BOX* gtbox, vector<BOX*> hypboxes,
    int& duplicates) {
  bool in_hypbox = false;
  int falseneg = 0;
  l_int32 x1, y1, w1, h1;
  boxGetGeometry(gtbox, &x1, &y1, &w1, &h1);
  for(l_int32 i = y1; i < (y1+h1); i++) {
    for(l_int32 j = x1; j < (x1+w1); j++) {
      // if the current pixel is inside of any of the groundtruth
      // boxes then we ignore it (continue)
      in_hypbox = false;
      for(vector<BOX*>::iterator hypboxit = hypboxes.begin();
          hypboxit != hypboxes.end(); hypboxit++) {
        BOX* hypbox = *hypboxit;
        l_int32 x2, y2, w2, h2;
        boxGetGeometry(hypbox, &x2, &y2, &w2, &h2);
        if(j >= x2 && i >= y2 && j < (x2+w2) && i < (y2+h2)) {
          in_hypbox = true;
          break;
        }
      }
      if(in_hypbox)
        continue;
      // otherwise we've detected a false negative if there is
      // a foreground pixel here!
      l_uint32* cur_gt_pix = Lept_Utils::getPixelAtXY(gtimg, j, i);
      if(Lept_Utils::getPixelColor(cur_gt_pix) == color ||
          (!typemode
           && Lept_Utils::isColorSignificant(Lept_Utils::getPixelColor(cur_gt_pix)))) {
        l_uint32* cur_hyp_pix = Lept_Utils::getPixelAtXY(hypimg, j, i);
        rgbtype hyppixelcolor[3];
        Lept_Utils::getPixelRGB(cur_hyp_pix, hyppixelcolor);
        assert(Lept_Utils::isNonWhite(hyppixelcolor));
        countAndTrackPixel(j, i, falseneg, hyp_tracker,
            LayoutEval::GREEN, duplicates);
      }
    }
  }
  return falseneg;
}

// TODO (someday...): Handle counting foreground pixels regardless
//       of the background color
int BipartiteGraph::countColorPixels(BOX* box, PIX* pix,
    LayoutEval::Color color, PIX* tracker, bool countall_nonwhite,
    int& duplicate_cnt) {
  l_int32 x, y, w, h;
  boxGetGeometry(box, &x, &y, &w, &h);
  l_uint32* curpixel;
  rgbtype rgb[3];
  int foreground_count = 0;
  assert(duplicate_cnt == 0);
  for(l_int32 i = y; i < (y+h); i++) {
    for(l_int32 j = x; j < (x+w); j++) {
      curpixel = Lept_Utils::getPixelAtXY(pix, j, i);
      if(!countall_nonwhite) {
        if(Lept_Utils::getPixelColor(curpixel) == color)
          countAndTrackPixel(j, i, foreground_count, tracker,
              LayoutEval::WHITE, duplicate_cnt);
      } else { // otherwise count any non-white color
               // (here we assume the background to be white)
        Lept_Utils::getPixelRGB(curpixel, rgb);
        if(Lept_Utils::isNonWhite(rgb))
          countAndTrackPixel(j, i, foreground_count, tracker,
              LayoutEval::WHITE, duplicate_cnt);
      }
    }
  }
  return foreground_count;
}

void BipartiteGraph::countAndTrackPixel(l_int32 x, l_int32 y, int& count,
    PIX* tracker, LayoutEval::Color colorcode, int& duplicate_cnt) {
  l_uint32* curpixel = Lept_Utils::getPixelAtXY(tracker, x, y);
  if(Lept_Utils::getPixelColor(curpixel) != colorcode) {
    Lept_Utils::setPixelRGB(tracker, curpixel, x, y, colorcode);
    ++count;
  }
  else
    ++duplicate_cnt;
}

void BipartiteGraph::getGroundTruthMetrics() {
  // first initialize everything
  gtmetrics.segmentations = 0;
  gtmetrics.total_seg_fg_pixels = 0;
  gtmetrics.fg_pixel_ratio = 0;
  gtmetrics.total_seg_area = 0;
  gtmetrics.total_area = (l_int32)gtimg->w * (l_int32)gtimg->h;
  gtmetrics.area_ratio = 0;
  // get all the metrics by iterating through the groundtruth
  // vertices. first need to get the totals
  for(vector<Vertex>::iterator gt_it = GroundTruth.begin();
      gt_it != GroundTruth.end(); gt_it++) {
    BOX* gtbox = gt_it->rect;
    int vertex_fg_pixels = 0;
    int not_counted = countColorPixels(gtbox, gtimg, color, gt_tracker,
        !typemode, vertex_fg_pixels);
    assert(not_counted == 0); // should have already been counted while creating vertices
    gtmetrics.total_seg_fg_pixels += vertex_fg_pixels;
    gtmetrics.total_seg_area += (int)gtbox->w * (int)gtbox->h;
    gtmetrics.segmentations++;
  }
  // get the ratio of the total segmented foreground pixels
  // to the total foreground pixels
  BOX* fullimgbox = boxCreate(0, 0, (l_int32)gtimg->w, (l_int32)gtimg->h);
  int already_counted = 0;
  gtmetrics.total_nonseg_fg_pixels = countColorPixels(fullimgbox, gtimg, color,
      gt_tracker, true, already_counted);
  boxDestroy(&fullimgbox);
  assert(already_counted == gtmetrics.total_seg_fg_pixels);
  gtmetrics.total_fg_pixels = gtmetrics.total_nonseg_fg_pixels
      + gtmetrics.total_seg_fg_pixels;
  gtmetrics.fg_pixel_ratio = (double)gtmetrics.total_seg_fg_pixels /
      (double)gtmetrics.total_fg_pixels;
  // get the ratio of the total segmented area to the
  // image's total area
  gtmetrics.area_ratio = (double)gtmetrics.total_seg_area /
      (double)gtmetrics.total_area;
  // now get the ratios for the area and foreground pixels of each
  // individual box to that of the summation for all the regions
  // that were segmented.. TODO: incorporate this for weighting purposes
  // right now it isn't even being used for anything..
  for(vector<Vertex>::iterator gt_it = GroundTruth.begin();
      gt_it != GroundTruth.end(); gt_it++) {
    GTBoxDescription boxdesc;
    int area = gt_it->area;
    int total_seg_area = gtmetrics.total_seg_area;
    boxdesc.area_ratio = (double)area / (double)total_seg_area;
    int fgpixels = gt_it->pix_foreground;
    int total_seg_fg = gtmetrics.total_seg_fg_pixels;
    boxdesc.fg_pix_ratio = (double)fgpixels / (double)total_seg_fg;
    gtmetrics.descriptions.push_back(boxdesc);
  }
}

// this prints the image-wide statistics (non-verbose)
// verbose print method pritns the statistics for each individual
// region of the image in turn
void BipartiteGraph::printMetrics(FILE* stream){
  // *****This is how a normal, non-verbose metric file is formated:
  // -----region-wide statistics:
  // [# correctly segemented regions] / [total # regions]
  fprintf(stream, "%d/%d\n", hypmetrics.correctsegmentations,
      gtmetrics.segmentations);
  // [# regions completely missed (fn)]
  fprintf(stream, "%d\n", hypmetrics.falsenegatives);
  // [# regions completely wrongly detected (fp)]
  fprintf(stream, "%d\n\n", hypmetrics.falsepositives);
  // -----stats on oversegmentations and undersegmentations:
  // [# oversegmented regions]
  fprintf(stream, "%d\n", hypmetrics.oversegmentedcomponents);
  // [# total oversegmentations for all regions]
  fprintf(stream, "%d\n", hypmetrics.oversegmentations);
  // [# avg oversegmentations per oversegmented groundtruth region]
  fprintf(stream, "%f\n", hypmetrics.avg_oversegmentations_perbox);
  // [# undersegmented regions]
  fprintf(stream, "%d\n", hypmetrics.undersegmentedcomponents);
  // [# total undersegmentations for all regions]
  fprintf(stream, "%d\n", hypmetrics.undersegmentations);
  // [# avg undersegmentations for undersegmented hypothesis region]
  fprintf(stream, "%f\n\n", hypmetrics.avg_undersegmentations_perbox);
  // -----pixel counts:
  // [# total foreground pix (tp+fp+tn+fn)]
  fprintf(stream, "%d\n", hypmetrics.total_fg_pix);
  // [# total positively detected pix (tp+fp)]
  fprintf(stream, "%d\n", hypmetrics.total_positive_fg_pix);
  // [# total negatively detected pix (tn+fn)]
  fprintf(stream, "%d\n", hypmetrics.total_negative_fg_pix);
  // [# total true positive pix (tp)]
  fprintf(stream, "%d\n", hypmetrics.total_true_positive_fg_pix);
  // [# total false negative pix (fn)]
  fprintf(stream, "%d\n", hypmetrics.total_false_negative_pix);
  // [# total true negative pix (tn)]
  fprintf(stream, "%d\n", hypmetrics.total_true_negative_fg_pix);
  // [# total false positive pix (fp)]
  fprintf(stream, "%d\n\n", hypmetrics.total_false_positive_pix);
  // -----metrics based on pixel counts (all between 0 and 1)
  // [TPR/Recall/Sensitivity/Hit_Rate = tp/(tp+fn)]
  fprintf(stream, "%f\n", hypmetrics.total_recall);
  // [Precision/Positive_Predictive_Value = tp/(tp+fp)]
  fprintf(stream, "%f\n", hypmetrics.total_precision);
  // [Accuracy = (tp+tn)/(tp+fn+tn+fp)]
  fprintf(stream, "%f\n", hypmetrics.accuracy);
  // [FPR/Fallout = fp/(fp+tn)]
  fprintf(stream, "%f\n", hypmetrics.total_fallout);
  // [False_Discovery_Rate = fp/(fp+tp)]
  fprintf(stream, "%f\n", hypmetrics.total_fdr);
  // [TNR/Specificity = tn/(fp+tn)]
  fprintf(stream, "%f\n", hypmetrics.specificity);
  // [Negative_Predictive_Value = tn/(tn+fn)]
  fprintf(stream, "%f\n", hypmetrics.negative_predictive_val);

  if(debug) {
    cout << "----\n" << filename << ":\n";
    cout << "False negatives found in overlapping groundtruth boxes:\n";
    vector<OverlappingGTRegion> ogt = hypmetrics.overlapgts;
    for(vector<OverlappingGTRegion>::iterator ogt_it = ogt.begin();
        ogt_it != ogt.end(); ogt_it++) {
      Vertex* vertptr = ogt_it->vertptr;
      int idx = vertptr->setindex;
      int falseneg = ogt_it->falsenegativepix;
      cout << "GroundTruth region at index " << idx << " has "
           << falseneg << " false negative pixels\n";
    }
  }
}

void BipartiteGraph::printMetricsVerbose(FILE* stream) {
  // ******Verbose metric files give pixel accurate metrics on each individual region:
  vector<RegionDescription> boxes = hypmetrics.boxes;
  for(vector<RegionDescription>::iterator region = boxes.begin();
      region != boxes.end(); region++) {
    Vertex* vertptr = region->vertptr;
    string whichset = vertptr->whichset;
    int setindex = vertptr->setindex;
    int setnum;
    if(whichset == (string)"Hypothesis")
      setnum = 0;
    else if(whichset == (string)"GroundTruth")
      setnum = 1;
    else {
      cout << "ERROR: Invalid set type in bipartite graph!\n";
      exit(EXIT_FAILURE);
    }
    // -----info on which box we are on!
    // [Box #] [Set #] // box is the region #, set is either 0 or 1 (0 for hypothesis, 1 for groundtruth)
    fprintf(stream, "\n%d %d\n", setindex, setnum);
    // -----region dimensions
    // [Box area]
    fprintf(stream, "\t%d\n", region->area);
    l_int32 x,y,w,h;
    boxGetGeometry(region->box, &x, &y, &w, &h);
    // [Box width]
    fprintf(stream, "\t%d\n", w);
    // [Box height]
    fprintf(stream, "\t%d\n\n", h);
    // -----number of overlapping groundtruth boxes corresponding to this hypothesis box
    // [# overlapping groundtruth boxes]
    fprintf(stream, "\t%d\n\n", region->num_gt_overlap);
    // -----pixel counts
    // [# total foreground pix (tp+fp+tn+fn)]
    fprintf(stream, "\t%d\n", region->num_fg_pixels);
    // [# total positively detected pix (tp+fp)]
    int p = region->true_positive_pix + region->false_positive_pix;
    fprintf(stream, "\t%d\n", p);
    // [# total true positive pix (tp)]
    fprintf(stream, "\t%d\n", region->true_positive_pix);
    // [# total false negative pix (fn)]
    fprintf(stream, "\t%d\n", region->false_negative_pix);
    // [# total false positive pix (fp)]
    fprintf(stream, "\t%d\n\n", region->false_positive_pix);

    if(debug) {
      if(region->num_gt_overlap >= 1) {
        cout << filename << ", region # " << setindex
             << " of " << whichset << " set\n";
        cout << "\tOverlaps Groundtruth Boxes at index(es) ";
        vector<Edge> overlapping = vertptr->edges;
        for(vector<Edge>::iterator edge = overlapping.begin();
            edge != overlapping.end(); edge++) {
          Vertex* edgevert = edge->vertexptr;
          int setindex_ = edgevert->setindex;
          if((edge + 2) == overlapping.end())
            cout << setindex_ << " and ";
          else if((edge + 1) == overlapping.end())
            cout << setindex_ << "\n";
          else
            cout << setindex_ << ", ";
        }
      }
    }
    // -----metrics based on the pixel counts
    // [TPR/Recall/Sensitivity/Hit_Rate = tp/(tp+fn)]
    fprintf(stream, "\t%f\n", region->recall);
    // [Precision/Positive_Predictive_Value = tp/(tp+fp)]
    fprintf(stream, "\t%f\n", region->precision);
    // [FPR/Fallout = fp/(fp+tn)]
    fprintf(stream, "\t%f\n", region->fallout);
    // [False_Discovery_Rate = fp/(fp+tp)]
    fprintf(stream, "\t%f\n", region->false_discovery);
  }
}

void BipartiteGraph::printSet(Bipartite::GraphChoice graph) {
  vector<Vertex>* set;
  if(graph == Bipartite::GroundTruth)
    set = &GroundTruth;
  else
    set = &Hypothesis;

  int i = 0, j = 0;
  for(vector<Vertex>::iterator vert = set->begin();
      vert != set->end(); vert++) {
    cout << "Vertex " << i << ":\n";
    l_int32 t,l,w,h;
    boxGetGeometry(vert->rect, &t, &l, &w, &h);
    cout << "\tBox (t,l,w,h): " << "(" << t << ", " << l
         << ", " << w << ", " << h << ")\n";
    cout << "\tBox Area: " << w*h << endl;
    cout << "\tNumber of Foreground Pixels: "
         << vert->pix_foreground << endl;
    cout << "\tForeground Pixels Ratio: "
         << (double)(vert->pix_foreground) /
         (double(w) * double(h)) << endl;
    cout << "\tEdges:\n";
    vector<Edge> edges = vert->edges;

    for(vector<Edge>::iterator edge = edges.begin();
        edge != edges.end(); edge++) {
      cout << "\t\tEdge " << j << ":\n";
      Vertex* vptr = edge->vertexptr;
      boxGetGeometry(vptr->rect, &t, &l, &w, &h);
      cout << "\t\t\tEdge points at vertex " << vptr->setindex
           << " from other set\n";
      cout << "\t\t\tArea of box edge points at: "
           << vptr->area << endl;
      cout << "\t\t\tEdge points to box at (t,l,w,h): "
           << "(" << t << ", " << l << ", " << w
           << ", " << h << ")\n";
      cout << "\t\t\tTotal foreground pixels edge points at: "
           << vptr->pix_foreground << endl;
      cout << "\t\t\tIntersecting pixels: "
           << edge->pixfg_intersecting << endl;
      cout << "\t\t\tOverlap area: "
           << edge->overlap_area << endl;
      j++;
    }
    i++;
    j = 0;
  }
}

void BipartiteGraph::clear() {
  Hypothesis.erase(Hypothesis.begin(), Hypothesis.end());
  GroundTruth.erase(GroundTruth.begin(), GroundTruth.end());
  pixDestroy(&inimg);
  pixDestroy(&hypimg);
  pixDestroy(&gtimg);
}

LayoutEval::Color BipartiteGraph::getColorFromType(const string& type) {
  if(type == "displayed" || type == "all")
    return LayoutEval::RED;
  else if(type == "embedded")
    return LayoutEval::BLUE;
  else if(type == "label")
    return LayoutEval::GREEN;
  else
    return LayoutEval::NONE;
}

