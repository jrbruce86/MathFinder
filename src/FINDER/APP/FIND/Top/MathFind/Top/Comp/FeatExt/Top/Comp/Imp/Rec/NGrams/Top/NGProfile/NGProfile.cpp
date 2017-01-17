/**************************************************************************
 * File name:   NGramProfileGenerator.cpp
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Nov 27, 2013 6:32:31 PM
 *              Modified Dec 18, 2016
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
 ***************************************************************************/

#include <NGProfile.h>

#include <FinderInfo.h>
#include <NGramRanker.h>
#include <NGram.h>
#include <Utils.h>
#include <BlobDataGrid.h>
#include <BlobDataGridFactory.h>
#include <M_Utils.h>
#include <SentenceData.h>
#include <GTParser.h>
#include <WordData.h>
#include <Lept_Utils.h>
#include <BlockData.h>

#include <baseapi.h>

#include <allheaders.h>

#include <string>
#include <iostream>
#include <assert.h>

//#define DBG_SINGLE_SENTENCE

NGramProfileGenerator::NGramProfileGenerator(
    FinderInfo* const finderInfo,
    NGramRanker* const ngramRanker,
    const std::string& ngramdir) {
  this->finderInfo = finderInfo;
  this->ngramRanker = ngramRanker;
  this->ngramdir = ngramdir;
  this->groundtruthFilePath = finderInfo->getGroundtruthFilePath();
}


NGramProfileGenerator::~NGramProfileGenerator() {}

RankedNGramVecs NGramProfileGenerator::generateMathNGrams() {

  // Are the n-gram rankings already done? Is so they will be in files
  // called uni-grams-sub-ranked, bi-grams-sub-ranked, and tri-grams-sub-ranked.
  // If they are done and it's not desired to make new ones then just use
  // the old ones to avoid unnecessary overhead (it takes a while to generate
  // all the n-grams from scratch each time, and is unnecessary when the same
  // one is being used each time anyway).
  bool files_exist = true;
  std::string ngrams_math_dir = ngramdir + "math/";
  for(int i = 0; i < 3; i++) {
    std::string ngramfile = ngrams_math_dir + NGramRanker::getNGramFileName(i + 1)
         + (std::string)"-sub-ranked";
    if(!Utils::existsFile(ngramfile)) {
      files_exist = false;
      break;
    }
  }

  RankedNGramVecs math_ngrams;

  // If the n-gram files were already created, prompt to see if ok
  // to just read them in rather than generating them again from scratch.
  bool generateNew = true;
  if(files_exist) {
    std::cout << "The training features for the n-gram extractor were already "
        "extracted and written to a file. Want to load in the file to reuse those "
        "same features for this training? ";
    generateNew = !Utils::promptYesNo();
  }

  if(generateNew)
    math_ngrams = generateNewNGrams();
  else
    math_ngrams = readInOldNGrams(ngramdir);

  return math_ngrams;
}

RankedNGramVecs NGramProfileGenerator::readInOldNGrams(const std::string& ngramdir) {
  RankedNGramVecs math_ngrams;
  for(int i = 0; i < 3; i++) {
    int gram = i + 1;
    std::string ngramfile = ngramdir + (std::string)"math/"
        + NGramRanker::getNGramFileName(gram) + (std::string)"-sub-ranked";
    std::ifstream ng_fs(ngramfile.c_str());
    if(!ng_fs.is_open()) {
      std::cout << "ERROR: Could not open " << ngramfile << " to get the NGrams.\n";
      assert(false);
    }
    int maxlen = 150;
    char line[maxlen];
    RankedNGramVec ngramvec;
    while(!ng_fs.eof()) {
      ng_fs.getline(line, maxlen);
      if(strlen(line) == 0)
        continue;
      GenericVector<char*> words;
      std::vector<std::string> parsedline = Utils::stringSplit((std::string)line);
      int numgrams = parsedline.size() - 1;
      // last entry should be the count
      if(!Utils::isStrNumeric(parsedline[numgrams])
          || numgrams != gram) {
        std::cout << "ERROR: Invalid NGram file!\n";
        assert(false);
      }
      double count = Utils::strToDouble(parsedline[numgrams]);
      for(int j = 0; j < numgrams; ++j) {
        std::string s = parsedline[j];
        char* wrd = Utils::strCopy(s.c_str());
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

RankedNGramVecs NGramProfileGenerator::generateNewNGrams() {
  std::cout << "Generating Math N-Gram Profile.\n";

  // initialize the filestreams used to write ngrams to files
  std::ofstream math_streams[3];
  std::ofstream nonmath_streams[3];

  // initialize the directories in which the n-grams will
  // be stored. if files already exists delete them so they
  // will be made anew.
  std::string math_ngramdir = ngramdir + "math/";
  std::string nonmath_ngramdir = ngramdir + "nonmath/";
  if(!Utils::existsDirectory(ngramdir))
    Utils::exec("mkdir " + ngramdir);
  if(!Utils::existsDirectory(math_ngramdir))
    Utils::exec("mkdir " + math_ngramdir);
  else {
    Utils::exec("rm -rf " + math_ngramdir); // make anew
    Utils::exec("mkdir " + math_ngramdir);
  }
  if(!Utils::existsDirectory(nonmath_ngramdir))
    Utils::exec("mkdir " + nonmath_ngramdir);
  else {
    Utils::exec("rm -rf " + nonmath_ngramdir); // make anew
    Utils::exec("mkdir " + nonmath_ngramdir);
  }

  // Get the number of training images
  int img_num = finderInfo->getGroundtruthImagePaths().size();

  // Iterate through half of the training images and generate the n-gram profile from their OCR results
  int mathsentence_cnt = 0;
  int nonmathsentence_cnt = 0;
  for(int i = 0; i < img_num/2; ++i) {
    tesseract::TessBaseAPI api;
    std::string trainingImagePath = finderInfo->getGroundtruthImagePaths()[i];
    Pix* trainingImage = Utils::leptReadImg(trainingImagePath);
    BlobDataGrid* blobDataGrid = BlobDataGridFactory().createBlobDataGrid(trainingImage, &api, Utils::getNameFromPath(trainingImagePath));

#ifdef DBG_NGRAM_INIT
    bool showgrid = true;
    if(showgrid) {
      std::string winname = "(ngrams)BlobInfoGrid for Image " + Utils::intToString(i);
      ScrollView* gridviewer = blobDataGrid->MakeWindow(100, 100, winname.c_str());
      blobDataGrid->DisplayBoxes(gridviewer);
      Utils::waitForInput();
      delete gridviewer;
      gridviewer = NULL;
    }
#endif
    // Grab all the sentences from the grid
    std::vector<TesseractSentenceData*> sentences = blobDataGrid->getAllRecognizedSentences();

    // Based upon the bounding boxes for each sentence and the manually
    // generated groundtruth, assign each sentence to either math or
    // non-math. A sentence is a math sentence if it overlaps any region
    // of the groundtruth for the given page, otherwise it is a non-math
    // sentence.
    for(int j = 0; j < sentences.size(); ++j) {
      sentences[j]->setIsMath(isSentenceMath(sentences[j], i));
    }
    std::cout << "Done checking sentences for math.\n";
#ifdef DBG_NGRAM_INIT_SHOW_SENTENCE_LABELS
    dbgShowSentenceLabels(trainingImage, blobDataGrid, trainingImagePath);
#endif

    // Separate the sentences out into two separate vectors
    // one for math and the other for non-math
    GenericVector<TesseractSentenceData*> math_sentences;
    GenericVector<TesseractSentenceData*> nonmath_sentences;
    for(int j = 0; j < sentences.size(); j++) {
      TesseractSentenceData* s = sentences[j];
      if(s->getIsMath()) {
        math_sentences.push_back(s);
        ++mathsentence_cnt;
      }
      else {
        nonmath_sentences.push_back(s);
        ++nonmathsentence_cnt;
      }
    }
    assert(math_sentences.length() + nonmath_sentences.length()
        == sentences.size());

#ifdef DBG_NGRAM_INIT
    dbgWriteMathNonMathFiles(math_sentences, nonmath_sentences);
#endif

    // Now write the uni, bi, and tri -grams each to their own file in the
    // n-grams directory. There's a subdir for math and a subdir for non-math
    // both contain 3 files named as follows: unigrams, bigrams, and trigrams.
    // It will save a lot of memory to write them to files and only load in
    // one file at a time as needed.
    ngramRanker->writeNGramFiles(math_sentences, math_ngramdir, math_streams);
    ngramRanker->writeNGramFiles(nonmath_sentences, nonmath_ngramdir, nonmath_streams);

    delete blobDataGrid;
    pixDestroy(&trainingImage);
    std::cout << "Finished processing image " << i << std::endl;
  }

  std::cout << "Total math sentences: " << mathsentence_cnt << std::endl;
  std::cout << "Total non-math sentences: " << nonmathsentence_cnt << std::endl;

  // Now rank all the n-grams based on frequency of occurrence
  RankedNGramVecs ranked_math = ngramRanker->rankNGrams(math_ngramdir);
  RankedNGramVecs ranked_nonmath = ngramRanker->rankNGrams(nonmath_ngramdir);

  // Now subtract frequencies of nonmath n-grams from each frequency
  // and then re-sort based on updated counts
  double nonmath_weight = findMathNonMathRatio(math_ngramdir, nonmath_ngramdir);
  std::cout << "math to non-math ratio: " << nonmath_weight << std::endl;
  ranked_math = ngramRanker->subtractAndReRankNGrams(ranked_math, ranked_nonmath,
      math_ngramdir, nonmath_weight);

  NGramRanker::destroyNGramVecs(ranked_nonmath); // done with non-math vectors
  return ranked_math;
}

double NGramProfileGenerator::findMathNonMathRatio(const std::string& mathdir, const std::string& nonmathdir) {
  std::string math_uni_fp = mathdir + NGramRanker::getNGramFileName(1);
  std::string nonmath_uni_fp = nonmathdir + NGramRanker::getNGramFileName(1);
  std::ifstream m_uni_fs(math_uni_fp.c_str());
  std::ifstream nm_uni_fs(nonmath_uni_fp.c_str());
  //count the lines in each
  double m_count = 0, nm_count = 0;
  int max = 55;
  char buf[max];
  while(!m_uni_fs.eof()) {
    m_uni_fs.getline(buf, max);
    ++m_count;
  }
  m_uni_fs.close();
  while(!nm_uni_fs.eof()) {
    nm_uni_fs.getline(buf, max);
    ++nm_count;
  }
  nm_uni_fs.close();
  return m_count / nm_count;
}

bool NGramProfileGenerator::isSentenceMath(TesseractSentenceData* sentence, int imgnum) {
  Boxa* lineboxes = sentence->getRowBoxes();
  bool found = false;
  for(int i = 0; i < lineboxes->n; i++) {
    Box* line = boxaGetBox(lineboxes, i, L_CLONE);
    // see if any entry of the groundtruth for the given image
    // overlaps with the current line's box
    std::ifstream gtfile;
    std::string gtfilename = groundtruthFilePath;
    gtfile.open(gtfilename.c_str(), std::ifstream::in);
    if((gtfile.rdstate() & std::ifstream::failbit) != 0) {
      std::cout << "ERROR: Could not open Groundtruth.dat in " \
           << groundtruthFilePath << std::endl;
      assert(false);
    }
    int max = 55;
    char* curline = new char[max];
    GroundTruthEntry* entry = NULL;
    while(!gtfile.eof()) {
      gtfile.getline(curline, max);
      if(curline == NULL)
        continue;
      std::string curlinestr = (std::string)curline;
      assert(curlinestr.length() < max);
      entry = GtParser::parseGTLine(curlinestr);
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
void NGramProfileGenerator::colorSentenceBlobs(Pix* const im, const int sentencenum,
    TesseractBlockData* const block, BlobDataGrid* const grid, const LayoutEval::Color color) {
  BlobData* b = NULL;
  BlobDataGridSearch bdgs(grid);
  bdgs.SetUniqueMode(true);
  BLOCK_RES* blockRes = block->getBlockRes();
  if(blockRes == NULL) {
    return;
  }
  TBOX blockBox = blockRes->block->bounding_box();
  blockBox.print();
  bdgs.StartRectSearch(blockBox);
  while((b = bdgs.NextRectSearch()) != NULL) {
    TesseractWordData* const word = b->getParentWord();
    if(word == NULL) {
      continue;
    }
    TesseractBlockData* parentWordBlock = word->getParentBlock();
    if(parentWordBlock != NULL) {
      BLOCK_RES* parentWordBlockRes = parentWordBlock->getBlockRes();
      if(parentWordBlockRes == NULL) {
        continue;
      }
      if(parentWordBlockRes->block == NULL) {
        continue;
      }
      if(!(parentWordBlockRes->block->bounding_box() == blockBox)) {
        continue;
      }
      //std::cout << "4\n";
    } else {
      continue;
    }
    if(word->getSentenceIndex() == sentencenum) {
      Box* bbox = M_Utils::getBlobDataBox(b, im);
      Lept_Utils::fillBoxForeground(im, bbox, color);
      boxDestroy(&bbox);
    }
  }
}

#include <DatasetMenu.h>
void NGramProfileGenerator::dbgShowSentenceLabels(Pix* const curimg,
    BlobDataGrid* const grid,
    std::string img_path) {
  std::string path = ngramdir;
  // for debugging display all the math sentences in red and non-math in blue
  bool show_math_nonmath = true;
  bool turnondisplay = false;
  if(show_math_nonmath) {
    Pix* dbgimg = pixCopy(NULL, curimg);
    dbgimg = pixConvertTo32(dbgimg);
    std::string dbgsavename = path + (std::string)"SentenceMathNonMath_" + Utils::intToString(DatasetSelectionMenu::getFileNumFromPath(img_path));
    std::ofstream dbgfile((dbgsavename + ".txt").c_str());
    if(!dbgfile.is_open()) {
      std::cout << "ERROR: Could not open debug file for writing in " \
          << dbgsavename << std::endl;
      assert(false);
    }
    for(int i = 0; i < grid->getTesseractBlocks().size(); ++i) {
      TesseractBlockData* const block = grid->getTesseractBlocks()[i];
      for(int j = 0; j < block->getRecognizedSentences().size(); ++j) {
        TesseractSentenceData* const sentence = block->getRecognizedSentences()[j];
        std::cout << "The following sentence is "
            << (sentence->getIsMath() ? "math" : "nonmath") << std::endl;
        std::cout << sentence->getSentenceText() << "\n----------\n";
        dbgfile << "The following sentence is "
            << (sentence->getIsMath() ? "math" : "nonmath") << std::endl;
        dbgfile << sentence->getSentenceText() << "\n----------\n";
        if(sentence->getIsMath())
          colorSentenceBlobs(dbgimg, j, block, grid, LayoutEval::RED);
        else
          colorSentenceBlobs(dbgimg, j, block, grid, LayoutEval::BLUE);
        if(turnondisplay) {
          pixDisplay(dbgimg, 100, 100);
          Utils::waitForInput();
        }
      }
    }
    pixWrite(dbgsavename.c_str(), dbgimg, IFF_PNG);
    pixDestroy(&dbgimg);
    dbgfile.close();
  }
}

void NGramProfileGenerator::dbgWriteMathNonMathFiles(
    const GenericVector<TesseractSentenceData*>& math_sentences,
    const GenericVector<TesseractSentenceData*>& nonmath_sentences) {
  // for debugging write the math sentences and non-math sentences
  // each to their own seperate file
  std::string path = ngramdir;
  std::ofstream mathfile;
  std::ofstream nonmathfile;
  mathfile.open((path + (std::string)"DBG_Math_Sentences").c_str());
  nonmathfile.open((path + (std::string)"DBG_Nonmath_Sentences").c_str());
  if(!mathfile.is_open() || !nonmathfile.is_open()) {
    std::cout << "ERROR: Could not open debug file for writing in one of the following: " \
        << "DBG_Math_Sentences or DBG_Nonmath_Sentences" << std::endl;
    exit(EXIT_FAILURE);
  }

  static int dbgwritemath_offset = 0;
  static int dbgwritenonmath_offset = 0;

  for(int j = 0; j < math_sentences.length(); j++)
    mathfile << j+dbgwritemath_offset << ": " << math_sentences[j]->sentence_txt
             << "\n-------\n";
  for(int j = 0; j < nonmath_sentences.length(); j++)
    nonmathfile << j+dbgwritenonmath_offset << ": " << nonmath_sentences[j]->sentence_txt
             << "\n-------\n";
  dbgwritemath_offset += math_sentences.length();
  dbgwritenonmath_offset += nonmath_sentences.length();
  mathfile.close();
  nonmathfile.close();
}

#endif
