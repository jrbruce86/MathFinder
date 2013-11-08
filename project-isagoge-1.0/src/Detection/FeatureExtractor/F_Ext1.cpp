/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   F_Ext1.cpp
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Oct 21, 2013 9:16:32 PM
 * ------------------------------------------------------------------------
 * Description: Implements the feature extraction interface. TODO: More details
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


#include <F_Ext1.h>

#include <MEDS.h>
#include <GTParser.h>

typedef GenericVector<NGramFrequency*> RankedNGramVec;
typedef GenericVector<RankedNGramVec> RankedNGramVecs;


using namespace GT_Parser;

F_Ext1::F_Ext1() : dbgfile_open(false), mathfile(NULL),
    nonmathfile(NULL), unigram_filename("uni-grams"),
    bigram_filename("bi-grams"), trigram_filename("tri-grams") {}

void F_Ext1::initFeatExtFull(TessBaseAPI& api, const string& groundtruth_path_,
    const string& training_set_path, const string& ext) {
  groundtruth_path = groundtruth_path_;
  // Set up N-Gram here if not done yet
  // TODO: look for files to see if its been done yet

  // initialize the filestreams used to write ngrams to files
  ofstream math_streams[3];
  ofstream nonmath_streams[3];

  // initialize the directories in which the n-grams will
  // be stored. if files already exists delete them so they
  // will be made anew.
  string ngramdir = "N-Grams/";
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

  // grab the MEDS module being used by the api
  MEDS* detector_segmentor = (MEDS*)(api.getEquationDetector());

  // read in and process each image
  for(int i = 1; i <= img_num; i++) {
    string img_name = Basic_Utils::intToString(i) + ext;
    string img_filepath = training_set_path + img_name;
    Pix* curimg = Basic_Utils::leptReadImg(img_filepath);
    api.SetImage(curimg); // SetImage SHOULD deallocate everything from the last page
    // including my MEDS module, the BlobInfoGrid, etc!!!!
    api.AnalyseLayout(); // Run Tesseract's layout analysis
    BlobInfoGrid* grid = detector_segmentor->getGrid();

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
    dbgShowSentenceLabels(curimg, sentences, grid, img_name);
#endif

    // Separate the sentences out into two separate vectors
    // one for math and the other for non-math
    GenericVector<Sentence*> math_sentences;
    GenericVector<Sentence*> nonmath_sentences;
    for(int j = 0; j < sentences.length(); j++) {
      Sentence* s = sentences[j];
      if(s->ismath)
        math_sentences.push_back(s);
      else
        nonmath_sentences.push_back(s);
    }
    assert(math_sentences.length() + nonmath_sentences.length()
        == sentences.length());

#ifdef DBG_NGRAM_INIT
    dbgWriteMathNonMathFiles(math_sentences, nonmath_sentences);
#endif

    // Now write the uni, bi, and tri -grams each to their own file in the
    // n-grams directory. There's a subdir for math and a subdir for non-math
    // both contain 3 files named as follows: unigrams, bigrams, and trigrams.
    // It will save a lot of memory to write them to files and only load in
    // one file at a time as needed.
    writeNGramFiles(math_sentences, math_ngramdir, math_streams);
    writeNGramFiles(nonmath_sentences, nonmath_ngramdir, nonmath_streams);

    pixDestroy(&curimg); // destroy finished image
    // clear the memory used by the current MEDS module (deletes the grid)
    detector_segmentor->reset();
    cout << "Finished processing image " << i << endl;

  }

  // Now rank all the n-grams based on frequency of occurrence
  RankedNGramVecs ranked_math = rankNGrams(math_ngramdir);
  RankedNGramVecs ranked_nonmath = rankNGrams(nonmath_ngramdir);

  // Now subtract frequencies of nonmath n-grams from each frequency
  // and then re-sort based on updated counts
  double nonmath_weight = findMathNonMathRatio(math_ngramdir, nonmath_ngramdir);
  cout << "math to non-math ratio: " << nonmath_weight << endl;
  ranked_math = subtractAndReRankNGrams(ranked_math, ranked_nonmath,
      math_ngramdir, nonmath_weight);

  // Look into Garrain work to decide on threshold for counts

  for(int i = 0; i < ranked_math.length(); i++)
    destroyNGramVec(ranked_math[i]);
  for(int i = 0; i < ranked_nonmath.length(); i++)
    destroyNGramVec(ranked_nonmath[i]);


  exit(EXIT_FAILURE);
}

double F_Ext1::findMathNonMathRatio(const string& mathdir, const string& nonmathdir) {
  string math_uni_fp = mathdir + getNGramFileName(1);
  string nonmath_uni_fp = nonmathdir + getNGramFileName(1);
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

RankedNGramVecs F_Ext1::subtractAndReRankNGrams(RankedNGramVecs& mathngrams,
    const RankedNGramVecs& nonmathngrams, const string& mathdir,
    const double& nonmath_weight) {
  assert(mathngrams.length() == 3);
  for(int i = 0; i < mathngrams.length(); i++)
    mathngrams[i] = subtractAndReRankNGram(i + 1,
        mathngrams[i], nonmathngrams[i], mathdir, nonmath_weight);
  return mathngrams;
}

RankedNGramVec F_Ext1::subtractAndReRankNGram(int gram, RankedNGramVec& mathngrams,
    const RankedNGramVec& nonmathngrams, const string& mathdir,
    const double& nonmath_weight) {
  // subtract matching (weighted) nonmath counts from math counts
  for(int i = 0; i < mathngrams.length(); i++) {
    NGramFrequency* math_ngf = mathngrams[i];
    NGram* math_ngram = math_ngf->ngram;
    // see if there's a matching one in the nonmathngrams vector
    for(int j = 0; j < nonmathngrams.length(); j++) {
      NGramFrequency* nonmath_ngf = nonmathngrams[j];
      NGram* nonmath_ngram = nonmath_ngf->ngram;
      assert(math_ngram->words.length() == gram
          && nonmath_ngram->words.length() == gram); // just to be safe
      if(*math_ngram == *nonmath_ngram) {
        // found match! decrement math frequency
        math_ngf->frequency -= (nonmath_ngf->frequency * nonmath_weight);
        if(math_ngf->frequency < 0)
          math_ngf->frequency = 0;
        break; // move on to next math ngram
      }
    }
  }
  // rerank math based on updated counts
  mathngrams.sort(&sortcmp);
  // write to file for analysis
  string ngramfilename = getNGramFileName(gram);
  string ngramfilepath = mathdir + ngramfilename + "-sub";
  writeNGramCounts(mathngrams, ngramfilepath);
  return mathngrams;
}


void F_Ext1::destroyNGramVec(GenericVector<NGramFrequency*> ngramcounts) {
  for(int i = 0; i < ngramcounts.length(); i++) {
    NGramFrequency* ngf = ngramcounts[i];
    if(ngf != NULL) {
      NGram* ngram = ngf->ngram;
      if(ngram != NULL) {
        delete ngram;
        ngram = NULL;
      }
      delete ngf;
      ngf = NULL;
    }
  }
  ngramcounts.clear();
}

RankedNGramVecs F_Ext1::rankNGrams(const string& path) {
  GenericVector<GenericVector<NGramFrequency*> > ngrams;
  for(int i = 1; i <= 3; i++) {
    GenericVector<NGramFrequency*> ngram = rankNGram(i, path);
    ngrams.push_back(ngram);
  }
  return ngrams;
}

RankedNGramVec F_Ext1::rankNGram(int gram, const string& path) {
  // read in the ngrams into a vector
  string ngramfile = getNGramFileName(gram);
  string ngramfilepath = path + ngramfile;
  GenericVector<NGram*> ngrams =
      readNGramFile(ngramfilepath, gram);
  // count the frequency of each
  RankedNGramVec ngramcounts = countNGramFrequencies(ngrams);

  // sort the ngrams by increasing frequency
  ngramcounts.sort(&sortcmp);

  // write ngrams and their counts to file
  writeNGramCounts(ngramcounts, ngramfilepath);

  ngrams.clear();
  return ngramcounts;
}

void F_Ext1::writeNGramCounts(const GenericVector<NGramFrequency*> ngfs,
    const string& filepath) {
  string dbgfile = filepath + "-ranked";
  ofstream dbgfs(dbgfile.c_str());
  if(!dbgfs.is_open()) {
    cout << "ERROR Could not open file " << filepath << " for ngram debugging\n";
    exit(EXIT_FAILURE);
  }
  for(int i = 0; i < ngfs.length(); i++) {
    NGramFrequency* ngf = ngfs[i];
    dbgfs << "ngram: " <<  "(" << *(ngf->ngram) << ")"
          << "\tcount: " << ngf->frequency << endl;
  }
  dbgfs.close();
}

RankedNGramVec F_Ext1::countNGramFrequencies(
    const GenericVector<NGram*>& ngrams) {
  RankedNGramVec ngramcounts;
  // first add shallow copies of each ngram to the vector, start
  // each count at zero
  for(int i = 0; i < ngrams.length(); i++) {
    NGramFrequency* ngf = new NGramFrequency;
    ngf->frequency = 1; // duh.. it has to occur once if it's there :P
    ngf->ngram = ngrams[i];
    ngramcounts.push_back(ngf);
  }
  // now count each element while deleting all duplicates
  for(int i = 0; i < ngramcounts.length(); i++) {
    NGramFrequency* cur_ngf = ngramcounts[i];
    NGram* cur_ngram = ngramcounts[i]->ngram;
    for(int j = i+1; j < ngramcounts.length(); j++) {
      NGram* other_ngram = ngramcounts[j]->ngram;
      if(*cur_ngram == *other_ngram) {
        ++(cur_ngf->frequency); // increment count
        // remove the duplicate from the vector
        NGramFrequency* other_ngf = ngramcounts[j];
        if(other_ngf != NULL) {
          if(other_ngf->ngram != NULL) {
            delete other_ngf->ngram;
            other_ngf->ngram = NULL;
          }
          delete other_ngf;
          other_ngf = NULL;
        }
        ngramcounts.remove(j);
      }
    }
  }
  return ngramcounts;
}

GenericVector<NGram*> F_Ext1::
readNGramFile(const string& filepath, int gram) {
  ifstream ngram_str(filepath.c_str());
  if(!ngram_str.is_open())
    nGramReadError(filepath);
  int maxlinelen = 55;
  char ln[maxlinelen];
  //initialize the vector of ngrams
  GenericVector<NGram*> ngrams;
  while(!ngram_str.eof()) {
    ngram_str.getline(ln, maxlinelen);
    if(strlen(ln) == 0)
      continue;
    NGram* ngram = new NGram;
    GenericVector<char*> ngram_words;
    int wrdstart = 0;
    // now get all the words on the line
    for(int i = 0; i < strlen(ln); i++) {
      if(i == 0 && (ln[i] == ' ' || ln[i] == '\n'))
        nGramReadError(filepath);
      if(ln[i] == ' ' || (i+1 == strlen(ln))) {
        // first grab the word
        int wrdlen = i - wrdstart; // this discards delimiters
        if(i+1 == strlen(ln)) // if we're at the end there's no delimiter
          wrdlen++;
        char* word = new char[wrdlen+1]; // +1 for null terminator
        for(int j = 0; j < wrdlen; j++)
          word[j] = ln[wrdstart+j];
        word[wrdlen] = '\0';
        // now add the word to the current ngram
        ngram_words.push_back(word);
        if(ln[i] == '\n') {
          // done reading current ngram, make sure its the right length
          if(ngram_words.length() != gram || (ln[i+1] != '\0'))
            nGramReadError(filepath);
        }
        wrdstart = i;
      }
    }
    ngram->words = ngram_words;
    ngrams.push_back(ngram);
  }
  ngram_str.close();
  return ngrams;
}

void F_Ext1::writeNGramFiles(const GenericVector<Sentence*>& sentences,
    const string& path, ofstream* streams) {
  for(int i = 1; i <= 3; i++)
    writeNGramFile(i, sentences, path, (streams+(i-1)));
}

void F_Ext1::writeNGramFile(int gram, const GenericVector<Sentence*>& sentences,
    const string& path, ofstream* stream) {
  string filename = getNGramFileName(gram);
  string filepath = path + filename;
  stream->open(filepath.c_str(), ios_base::app);
  if(!stream->is_open()) {
    cout << "ERROR: Could not open " << filepath << " for writing!\n";
    exit(EXIT_FAILURE);
  }
  for(int i = 0; i < sentences.length(); i++) {
    char* s_txt = sentences[i]->sentence_txt;
    GenericVector<char*> ngram; // holds 1, 2, or 3 strings
    int wrdstart = 0;
    int ngram_next_index = 0; // index of the end of an ngram's first word
                              // the next ngram will start on the current
                              // ngram's second word. So for instance, if
                              // the sentence is "The boy went to school",
                              // trigram 1 is: "The boy went", trigram 2
                              // is: "boy went to", etc.
    bool word_found = false;
    for(int j = 0; j < strlen(s_txt); j++) {
      if(s_txt[j] == '\0') {
        cout << "ERROR: NULL terminator found before the end of a sentence!\n";
        exit(EXIT_FAILURE);
      }
      if(!word_found) { // looking for a word start
        if(s_txt[j] == ' ' || s_txt[j] == '\n')
          continue; // keep looking
        else {
          word_found = true;
          wrdstart = j;
          continue;
        }
      }
      else { // looking for a word end
        if(s_txt[j] == ' ' || s_txt[j] == '\n' || (j+1 == strlen(s_txt))) {
          // found it! (either space, newline, or last character of sentence)
          int wrdlen = j - wrdstart; // this excludes the last character (space or newline)
          // include last character if on the last in the string and its not space or newline
          if(j+1 == strlen(s_txt) && s_txt[j] != ' ' && s_txt[j] != '\n')
            wrdlen++;
          char* word = new char[wrdlen + 1]; // +1 for null terminator
          for(int k = 0; k < wrdlen; k++)
            word[k] = s_txt[wrdstart + k];
          word[wrdlen] = '\0';
          ngram.push_back(word);
          int numgrams = ngram.length();
          if(numgrams == gram) {
            // ready to write the n-gram to the file
            for(int k = 0; k < ngram.length(); k++)
              *stream << ngram[k]
                        << ((k+1) != ngram.length() ? " " : "\n");
            for(int k = 0; k < ngram.length(); k++) {
              char* w = ngram[k];
              delete [] w;
              w = NULL;
            }
            ngram.clear();
            if(ngram_next_index > 0)
              j = ngram_next_index; // go back to start of next ngram
                                    // (if we're doing unigrams this isn't applicable)
          }
          else if(numgrams == 1)
            ngram_next_index = j; // will go back to this once ngram is done
          else if(numgrams > gram) {
            cout << "ERROR: Too many words were written to an n-gram!\n";
            exit(EXIT_FAILURE);
          }
          word_found = false;
        }
      }
    }
    // get rid of any left overs (i.e. if sentence ended while looking for more
    // words for the n-gram which just get rid of the remainder).
    for(int j = 0; j < ngram.length(); j++) {
      char* w = ngram[j];
      delete [] w;
      w = NULL;
    }
    ngram.clear();
  }
  stream->close();
}

bool F_Ext1::isSentenceMath(Sentence* sentence, int imgnum) {
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
void F_Ext1::colorSentenceBlobs(Pix* im, int sentencenum,
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

void F_Ext1::dbgShowSentenceLabels(Pix* curimg,
    const GenericVector<Sentence*>& sentences, BlobInfoGrid* grid,
    string img_name) {
  // for debugging display all the math sentences in red and non-math in blue
  bool show_math_nonmath = true;
  bool turnondisplay = false;
  if(show_math_nonmath) {
    Lept_Utils lu;
    M_Utils mu;
    Pix* dbgimg = pixCopy(NULL, curimg);
    dbgimg = pixConvertTo32(dbgimg);
    string dbgsavename = "SentenceMathNonMath" + img_name;
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

void F_Ext1::dbgWriteMathNonMathFiles(
    const GenericVector<Sentence*>& math_sentences,
    const GenericVector<Sentence*>& nonmath_sentences) {
  // for debugging write the math sentences and non-math sentences
  // each to their own seperate file
  if(!dbgfile_open) {
    mathfile = new ofstream;
    nonmathfile = new ofstream;
    mathfile->open("DBG_Math_Sentences");
    nonmathfile->open("DBG_Nonmath_Sentences");
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


void F_Ext1::initFeatExtSinglePage() {

}

vector<double> F_Ext1::extractFeatures(tesseract::BLOBINFO* blob,
    tesseract::BlobInfoGrid* big_) {
  vector<double> ret;
  return ret;
}
