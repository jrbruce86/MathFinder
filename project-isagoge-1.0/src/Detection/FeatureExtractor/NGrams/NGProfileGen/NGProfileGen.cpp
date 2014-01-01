/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   NGramProfileGenerator.cpp
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Nov 27, 2013 6:32:31 PM
 * ------------------------------------------------------------------------
 * Description: Takes a set of input document images as input and generates
 *              a set of ranked uni-grams, bi-grams, and tri-grams that
 *              have been counted the most in mathematical regions of the
 *              images. Matching non-mathematical N-Grams are subtracted
 *              from the math ones so that the ranking best represents the
 *              mathematical regions. Also stop-words, digits, and punctuation
 *              are removed. Since math and nonmath words are seen in different
 *              amounts the subtraction weights each matching non-math n-gram
 *              by the overall math to non-math word ratio. The resulting math
 *              n-grams are referred to as the NGram Profile for the training
 *              data.
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

#include <NGProfileGen.h>
#include <TessInterface.h>

#include <GTParser.h>
#define DBG_SINGLE_SENTENCE

using namespace GT_Parser;

typedef GenericVector<NGramFrequency*> RankedNGramVec;
typedef GenericVector<RankedNGramVec> RankedNGramVecs;

NGramProfileGenerator::NGramProfileGenerator(const string& training_set_path_) :
    dbgfile_open(false), mathfile(NULL),
    nonmathfile(NULL), api(NULL) {
  r.readInStopWords(training_set_path_);
  stopwords = r.getStopWords();
  training_set_path = training_set_path_;
}

NGramProfileGenerator::~NGramProfileGenerator() {
}

RankedNGramVecs NGramProfileGenerator::generateMathNGrams(TessBaseAPI* api_,
    vector<string> tess_api_params, const string& groundtruth_path_,
    const string& training_set_path, const string& ext, bool make_new) {

  // Are the n-gram rankings already done? Is so they will be in files
  // called uni-grams-sub-ranked, bi-grams-sub-ranked, and tri-grams-sub-ranked.
  // If they are done and its not desired to make new ones then just use
  // the old ones to avoid unnecessary overhead (it takes a while to generate
  // all the n-grams from scratch each time, and is unnecessary when the same
  // one is being used each time anyway).
  bool files_exist = true;
  string ngramdir = training_set_path + (string)"../../N-Grams/";
  string ngrams_math_dir = ngramdir + "math/";
  for(int i = 0; i < 3; i++) {
    string ngramfile = ngrams_math_dir + r.getNGramFileName(i + 1)
         + (string)"-sub-ranked";
    if(!Basic_Utils::existsFile(ngramfile)) {
      cout << "file at " << ngramfile << ", doesn't exist\n";
      files_exist = false;
      break;
    }
  }

  RankedNGramVecs math_ngrams;
  if(!files_exist || make_new)
    math_ngrams = generateNewNGrams(api_, tess_api_params, groundtruth_path_,
        training_set_path, ngramdir, ext);
  else
    math_ngrams = readInOldNGrams(ngramdir);

  return math_ngrams;
}

RankedNGramVecs NGramProfileGenerator::readInOldNGrams(const string& ngramdir) {
  RankedNGramVecs math_ngrams;
  for(int i = 0; i < 3; i++) {
    int gram = i + 1;
    string ngramfile = ngramdir + (string)"math/"
        + r.getNGramFileName(gram) + (string)"-sub-ranked";
    ifstream ng_fs(ngramfile.c_str());
    if(!ng_fs.is_open()) {
      cout << "ERROR: Could not open " << ngramfile << " to get the NGrams.\n";
      exit(EXIT_FAILURE);
    }
    int maxlen = 150;
    char line[maxlen];
    RankedNGramVec ngramvec;
    while(!ng_fs.eof()) {
      ng_fs.getline(line, maxlen);
      if(strlen(line) == 0)
        continue;
      GenericVector<char*> words;
      vector<string> parsedline = Basic_Utils::stringSplit((string)line);
      int numgrams = parsedline.size() - 1;
      // last entry should be the count
      if(!Basic_Utils::isStrNumeric(parsedline[numgrams])
          || numgrams != gram) {
        cout << "ERROR: Invalid NGram file!\n";
        exit(EXIT_FAILURE);
      }
      double count = Basic_Utils::strToDouble(parsedline[numgrams]);
      for(int j = 0; j < numgrams; j++) {
        string s = parsedline[j];
        char* wrd = new char[s.length() + 1]; // + 1 NULL terminator
        wrd = Basic_Utils::strCopy(s.c_str());
        words.push_back(wrd);
      }
      assert(words.length() == gram);
      NGram* ng = new NGram;
      ng->words = words;
      NGramFrequency* ngf = new NGramFrequency;
      ngf->ngram = ng;
      ngf->frequency = count;
      ngramvec.push_back(ngf);
    }
    math_ngrams.push_back(ngramvec);
    ngramvec.clear();
    ng_fs.close();
  }
  return math_ngrams;
}

RankedNGramVecs NGramProfileGenerator::generateNewNGrams(TessBaseAPI* api_,
    vector<string> tess_api_params, const string& groundtruth_path_,
    const string& training_set_path, const string& ngramdir, const string& ext) {
  cout << "Generating Math N-Gram Profile.\n";
  api = api_;
  r.setTessAPI(api);
  groundtruth_path = groundtruth_path_;

  // initialize the filestreams used to write ngrams to files
  ofstream math_streams[3];
  ofstream nonmath_streams[3];

  // initialize the directories in which the n-grams will
  // be stored. if files already exists delete them so they
  // will be made anew.
  string math_ngramdir = ngramdir + "math/";
  string nonmath_ngramdir = ngramdir + "nonmath/";
  if(!Basic_Utils::existsDirectory(ngramdir))
    Basic_Utils::exec("mkdir " + ngramdir);
  if(!Basic_Utils::existsDirectory(math_ngramdir))
    Basic_Utils::exec("mkdir " + math_ngramdir);
  else {
    Basic_Utils::exec("rm -r " + math_ngramdir); // make anew
    Basic_Utils::exec("mkdir " + math_ngramdir);
  }
  if(!Basic_Utils::existsDirectory(nonmath_ngramdir))
    Basic_Utils::exec("mkdir " + nonmath_ngramdir);
  else {
    Basic_Utils::exec("rm -r " + nonmath_ngramdir); // make anew
    Basic_Utils::exec("mkdir " + nonmath_ngramdir);
  }

  // count the number of training images in the training_set_path
  int img_num = Basic_Utils::fileCount(training_set_path);

  // need to initialize the API
  api->Init(tess_api_params[0].c_str(), tess_api_params[1].c_str());
  char* page_seg_mode = (char*)"tessedit_pageseg_mode";
   if (!api->SetVariable(page_seg_mode, "3")) {
     cout << "ERROR: Could not set tesseract's api corectly during N-Gram Profile Generation!\n";
     exit(EXIT_FAILURE);
   }
   // make sure that we're in the right document layout analysis mode
   int psm = 0;
   api->GetIntVariable(page_seg_mode, &psm);
   assert(psm == tesseract::PSM_AUTO);
   // turn on equation detection
   if (!api->SetVariable("textord_equation_detect", "true")) {
     cout << "Could not turn on Tesseract's equation detection during N-Gram Profile Generation!\n";
     exit(EXIT_FAILURE);
   }

   // set up an interface to Tesseract (interface is through the findEquationParts method
   // of Tesseract's equation detector which is overridden by the interface. The interface
   // is where the BlobInfoGrid used for N-Gram Profile generation is created from within
   // Tesseract as layout analysis is being done on a page. The resulting BlobInfoGrid
   // is owned by the TessInterface so it is important that the reset() method be called
   // on TessInterface whenever all processing is completed for a given document image.
   EquationDetectBase* tess_interface = new TessInterface;

   // The BlobInfoGrid belonging to the interface needs to have a feshly allocated api
   // provided to it by the TessInterface which owns it. Here I provide the TessInterface
   // with a freshly allocated api which will be owned by the BlobInfoGrid.
   TessBaseAPI* newapi = new TessBaseAPI;

   // Assign the interface (i.e., equation detector) to the primary API
   api->setEquationDetector(tess_interface);

  // read in and process each image (using half of the training set currently)
  int mathsentence_cnt = 0;
  int nonmathsentence_cnt = 0;
  for(int i = 1; i <= img_num/2; i++) {
    ((TessInterface*)tess_interface)->setTessAPI(newapi);
    string img_name = Basic_Utils::intToString(i) + ext;
    string img_filepath = training_set_path + img_name;
    Pix* curimg = Basic_Utils::leptReadImg(img_filepath);
    api->SetImage(curimg); // SetImage SHOULD deallocate everything from the last page
    // including my MEDS module, the BlobInfoGrid, etc!!!!
    api->AnalyseLayout(); // Run Tesseract's layout analysis
    BlobInfoGrid* grid = ((TessInterface*)tess_interface)->getGrid();
#ifdef DBG_NGRAM_INIT
    bool showgrid = false;
    if(showgrid) {
      string winname = "(featext)BlobInfoGrid for Image " + Basic_Utils::intToString(i);
      ScrollView* gridviewer = grid->MakeWindow(100, 100, winname.c_str());
      grid->DisplayBoxes(gridviewer);
      Basic_Utils::waitForInput();
      delete gridviewer;
      gridviewer = NULL;
    }
#endif
    // Grab all the sentences from the grid
    GenericVector<Sentence*> sentences = grid->getSentences();

    // Based upon the bounding boxes for each sentence and the manually
    // generated groundtruth, assign each sentence to either math or
    // non-math. A sentence is a math sentence if it overlaps any region
    // of the groundtruth for the given page, otherwise it is a non-math
    // sentence.
    for(int j = 0; j < sentences.length(); j++)
      sentences[j]->ismath = isSentenceMath(sentences[j], i);

#ifdef DBG_NGRAM_INIT_SHOW_SENTENCE_LABELS
    dbgShowSentenceLabels(training_set_path, curimg, sentences, grid, img_name);
#endif

    // Separate the sentences out into two separate vectors
    // one for math and the other for non-math
    GenericVector<Sentence*> math_sentences;
    GenericVector<Sentence*> nonmath_sentences;
    for(int j = 0; j < sentences.length(); j++) {
      Sentence* s = sentences[j];
      if(s->ismath) {
        math_sentences.push_back(s);
        mathsentence_cnt++;
      }
      else {
        nonmath_sentences.push_back(s);
        nonmathsentence_cnt++;
      }
    }
    assert(math_sentences.length() + nonmath_sentences.length()
        == sentences.length());

#ifdef DBG_NGRAM_INIT
    dbgWriteMathNonMathFiles(training_set_path, math_sentences, nonmath_sentences);
#endif

    // Now write the uni, bi, and tri -grams each to their own file in the
    // n-grams directory. There's a subdir for math and a subdir for non-math
    // both contain 3 files named as follows: unigrams, bigrams, and trigrams.
    // It will save a lot of memory to write them to files and only load in
    // one file at a time as needed.
    r.writeNGramFiles(math_sentences, math_ngramdir, math_streams);
    r.writeNGramFiles(nonmath_sentences, nonmath_ngramdir, nonmath_streams);

    pixDestroy(&curimg); // destroy finished image
    // clear the memory used by the current MEDS module (deletes the grid)
    ((TessInterface*)tess_interface)->reset();
    newapi = new TessBaseAPI; // interface will need a new api for next iteration
    cout << "Finished processing image " << i << endl;
  }

  delete newapi;
  newapi = NULL;

  cout << "Total math sentences: " << mathsentence_cnt << endl;
  cout << "Total non-math sentences: " << nonmathsentence_cnt << endl;

  // Now rank all the n-grams based on frequency of occurrence
  ranked_math = r.rankNGrams(math_ngramdir);
  RankedNGramVecs ranked_nonmath = r.rankNGrams(nonmath_ngramdir);

  // Now subtract frequencies of nonmath n-grams from each frequency
  // and then re-sort based on updated counts
  double nonmath_weight = findMathNonMathRatio(math_ngramdir, nonmath_ngramdir);
  cout << "math to non-math ratio: " << nonmath_weight << endl;
  ranked_math = r.subtractAndReRankNGrams(ranked_math, ranked_nonmath,
      math_ngramdir, nonmath_weight);

  r.destroyNGramVecs(ranked_nonmath); // done with non-math vectors
  api->End(); // this api is owned
  return ranked_math;
}

double NGramProfileGenerator::findMathNonMathRatio(const string& mathdir, const string& nonmathdir) {
  string math_uni_fp = mathdir + r.getNGramFileName(1);
  string nonmath_uni_fp = nonmathdir + r.getNGramFileName(1);
  ifstream m_uni_fs(math_uni_fp.c_str());
  ifstream nm_uni_fs(nonmath_uni_fp.c_str());
  //count the lines in each
  double m_count = 0, nm_count = 0;
  int max = 55;
  char buf[max];
  while(!m_uni_fs.eof()) {
    m_uni_fs.getline(buf, max);
    m_count++;
  }
  m_uni_fs.close();
  while(!nm_uni_fs.eof()) {
    nm_uni_fs.getline(buf, max);
    nm_count++;
  }
  nm_uni_fs.close();
  return m_count / nm_count;
}

bool NGramProfileGenerator::isSentenceMath(Sentence* sentence, int imgnum) {
  Boxa* lineboxes = sentence->lineboxes;
  bool found = false;
  for(int i = 0; i < lineboxes->n; i++) {
    Box* line = boxaGetBox(lineboxes, i, L_CLONE);
    // see if any entry of the groundtruth for the given image
    // overlaps with the current line's box
    ifstream gtfile;
    string gtfilename = groundtruth_path;
    gtfile.open(gtfilename.c_str(), ifstream::in);
    if((gtfile.rdstate() & ifstream::failbit) != 0) {
      cout << "ERROR: Could not open Groundtruth.dat in " \
           << groundtruth_path << endl;
      exit(EXIT_FAILURE);
    }
    int max = 55;
    char* curline = new char[max];
    GroundTruthEntry* entry = NULL;
    while(!gtfile.eof()) {
      gtfile.getline(curline, max);
      if(curline == NULL)
        continue;
      string curlinestr = (string)curline;
      assert(curlinestr.length() < max);
      entry = parseGTLine(curlinestr);
      if(entry == NULL)
        continue;
      if(entry->image_index == imgnum) {
        // see if any part of the entry is contained within the current line
        int bb_intersects = 0; // this will be 1 if blob_bb intersects math
        boxIntersects(line, entry->rect, &bb_intersects);
        if(bb_intersects == 1) {
          // the sentence to which this line belongs is a math one!
          found = true;
          break;
        }
      }
    }
    boxDestroy(&line);
    if(found == true)
      break;
  }
  return found;
}

#ifdef DBG_NGRAM_INIT
// colors the foreground of all blobs belonging to a sentence
// (for debugging)
void NGramProfileGenerator::colorSentenceBlobs(Pix* im, int sentencenum,
    BlobInfoGrid* grid, LayoutEval::Color color) {
  Lept_Utils lu;
  M_Utils mu;
  BLOBINFO* b = NULL;
  BlobInfoGridSearch bigs(grid);
  bigs.StartFullSearch();
  while((b = bigs.NextFullSearch()) != NULL) {
    if(b->sentenceindex == sentencenum) {
      Box* bbox = mu.getBlobInfoBox(b, im);
      lu.fillBoxForeground(im, bbox, color);
      boxDestroy(&bbox);
    }
  }
}

void NGramProfileGenerator::dbgShowSentenceLabels(string path, Pix* curimg,
    const GenericVector<Sentence*>& sentences, BlobInfoGrid* grid,
    string img_name) {
  path = path + (string)"../";
  // for debugging display all the math sentences in red and non-math in blue
  bool show_math_nonmath = true;
  bool turnondisplay = false;
  if(show_math_nonmath) {
    Lept_Utils lu;
    M_Utils mu;
    Pix* dbgimg = pixCopy(NULL, curimg);
    dbgimg = pixConvertTo32(dbgimg);
    string dbgsavename = path + (string)"SentenceMathNonMath" + img_name;
    ofstream dbgfile((dbgsavename + ".txt").c_str());
    if(!dbgfile.is_open()) {
      cout << "ERROR: Could not open debug file for writing in " \
           << dbgfile << endl;
      exit(EXIT_FAILURE);
    }
    for(int j = 0; j < sentences.length(); j++) {
      Sentence* s = sentences[j];
      cout << "The following sentence is "
           << (s->ismath ? "math" : "nonmath") << endl;
      cout << s->sentence_txt << "\n----------\n";
      dbgfile << "The following sentence is "
           << (s->ismath ? "math" : "nonmath") << endl;
      dbgfile << s->sentence_txt << "\n----------\n";
      if(s->ismath)
        colorSentenceBlobs(dbgimg, j, grid, LayoutEval::RED);
      else
        colorSentenceBlobs(dbgimg, j, grid, LayoutEval::BLUE);
      if(turnondisplay) {
        pixDisplay(dbgimg, 100, 100);
        mu.waitForInput();
      }
    }
    pixWrite(dbgsavename.c_str(), dbgimg, IFF_PNG);
    pixDestroy(&dbgimg);
    dbgfile.close();
  }
}

void NGramProfileGenerator::dbgWriteMathNonMathFiles(const string& training_path,
    const GenericVector<Sentence*>& math_sentences,
    const GenericVector<Sentence*>& nonmath_sentences) {
  // for debugging write the math sentences and non-math sentences
  // each to their own seperate file
  string path = training_path + (string)"../";
  if(!dbgfile_open) {
    mathfile = new ofstream;
    nonmathfile = new ofstream;
    mathfile->open((path + (string)"DBG_Math_Sentences").c_str());
    nonmathfile->open((path + (string)"DBG_Nonmath_Sentences").c_str());
    if(!mathfile->is_open() || !nonmathfile->is_open()) {
      cout << "ERROR: Could not open debug file for writing in one of the following: " \
           << "DBG_Math_Sentences or DBG_Nonmath_Sentences" << endl;
      exit(EXIT_FAILURE);
    }
    dbgfile_open = true;
  }
  static int dbgwritemath_offset = 0;
  static int dbgwritenonmath_offset = 0;
  for(int j = 0; j < math_sentences.length(); j++)
    *mathfile << j+dbgwritemath_offset << ": " << math_sentences[j]->sentence_txt
             << "\n-------\n";
  for(int j = 0; j < nonmath_sentences.length(); j++)
    *nonmathfile << j+dbgwritenonmath_offset << ": " << nonmath_sentences[j]->sentence_txt
             << "\n-------\n";
  dbgwritemath_offset += math_sentences.length();
  dbgwritenonmath_offset += nonmath_sentences.length();
}

#endif
