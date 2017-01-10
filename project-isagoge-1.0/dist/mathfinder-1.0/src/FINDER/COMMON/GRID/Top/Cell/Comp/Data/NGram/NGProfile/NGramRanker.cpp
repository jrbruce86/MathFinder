/**************************************************************************
 * File name:   NGramRanker.cpp
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Nov 18, 2013 9:10:44 PM
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
 *              by the overall math to non-math word ratio
 ***************************************************************************/

#include <NGramRanker.h>

#include <baseapi.h>

NGramRanker::NGramRanker(StopwordFileReader* stopwordHelper) {
  this->stopwordHelper = stopwordHelper;
}

NGramRanker::~NGramRanker() {}

std::string NGramRanker::getNGramFileName(const int& gram) {
  std::string ngramfile = "";
  if(gram == 1)
    ngramfile = "uni-grams";
  else if(gram == 2)
    ngramfile = "bi-grams";
  else if(gram == 3)
    ngramfile = "tri-grams";
  else {
    std::cout << "ERROR: Attempt made to find n-grams other than the uni, bi, and tri"
         << " which are the only ones supported.\n";
    assert(false);
  }
  return ngramfile;
}

// This also writes the uni, bi, and tri -grams each to their own file in the
// a directory called Cur-Sentence-NGrams/ which is in the given directory
RankedNGramVecs NGramRanker::generateSentenceNGrams(
    TesseractSentenceData* sentence,
    const std::string& ngramdir) {
  RankedNGramVecs sentence_ngrams;
  // initialize the filestreams used to write ngrams to files
  std::ofstream ng_streams[3];
  // initialize filestructure
  std::string sentence_ngramdir = ngramdir + (std::string)"Cur-Sentence-NGrams/";
  if(!Utils::existsDirectory(sentence_ngramdir))
    Utils::exec("mkdir " + sentence_ngramdir);
  else {
    Utils::exec("rm -r " + sentence_ngramdir);
    Utils::exec("mkdir " + sentence_ngramdir);
  }
  GenericVector<TesseractSentenceData*> sentencevec;
  sentencevec.push_back(sentence);
  writeNGramFiles(sentencevec, sentence_ngramdir, ng_streams);
  sentence_ngrams = rankNGrams(sentence_ngramdir);
  return sentence_ngrams;
}

RankedNGramVecs NGramRanker::subtractAndReRankNGrams(RankedNGramVecs& mathngrams,
    const RankedNGramVecs& nonmathngrams, const std::string& mathdir,
    const double& nonmath_weight) {
  assert(mathngrams.length() == 3);
  for(int i = 0; i < mathngrams.length(); ++i)
    mathngrams[i] = subtractAndReRankNGram(i + 1,
        mathngrams[i], nonmathngrams[i], mathdir, nonmath_weight);
  return mathngrams;
}

RankedNGramVec NGramRanker::subtractAndReRankNGram(int gram, RankedNGramVec& mathngrams,
    const RankedNGramVec& nonmathngrams, const std::string& mathdir,
    const double& nonmath_weight) {
  // subtract matching (weighted) nonmath counts from math counts
  for(int i = 0; i < mathngrams.length(); ++i) {
    NGramFrequency* math_ngf = mathngrams[i];
    NGram* math_ngram = math_ngf->ngram;
    // see if there's a matching one in the nonmathngrams vector
    for(int j = 0; j < nonmathngrams.length(); ++j) {
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
  std::string ngramfilename = getNGramFileName(gram);
  std::string ngramfilepath = mathdir + ngramfilename + "-sub";
  writeNGramCounts(mathngrams, ngramfilepath);
  return mathngrams;
}

void NGramRanker::destroyNGramVecs(RankedNGramVecs& vecs) {
  for(int i = 0; i < vecs.length(); ++i)
    destroyNGramVec(vecs[i]);
  vecs.clear();
}

void NGramRanker::destroyNGramVec(GenericVector<NGramFrequency*> ngramcounts) {
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

RankedNGramVecs NGramRanker::rankNGrams(const std::string& path) {
  GenericVector<GenericVector<NGramFrequency*> > ngrams;
  for(int i = 1; i <= 3; ++i) {
    GenericVector<NGramFrequency*> ngram = rankNGram(i, path);
    ngrams.push_back(ngram);
  }
  return ngrams;
}

RankedNGramVec NGramRanker::rankNGram(int gram, const std::string& path) {
  // read in the ngrams into a vector
  std::string ngramfile = getNGramFileName(gram);
  std::string ngramfilepath = path + ngramfile;
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

void NGramRanker::writeNGramCounts(const GenericVector<NGramFrequency*> ngfs,
    const std::string& filepath) {
  std::string dbgfile = filepath + "-ranked";
  std::ofstream dbgfs(dbgfile.c_str());
  if(!dbgfs.is_open()) {
    std::cout << "ERROR Could not open file " << filepath << " for ngram debugging\n";
    assert(false);
  }
  for(int i = 0; i < ngfs.length(); i++) {
    NGramFrequency* ngf = ngfs[i];
    dbgfs << *(ngf->ngram) << " " << ngf->frequency << std::endl;
  }
  dbgfs.close();
}

RankedNGramVec NGramRanker::countNGramFrequencies(
    const GenericVector<NGram*>& ngrams) {
  RankedNGramVec ngramcounts;
  // first add shallow copies of each ngram to the vector, start
  // each count at one
  for(int i = 0; i < ngrams.length(); ++i) {
    NGramFrequency* ngf = new NGramFrequency;
    ngf->frequency = 1;
    ngf->ngram = ngrams[i];
    ngramcounts.push_back(ngf);
  }
  // now count each element while deleting all duplicates
  for(int i = 0; i < ngramcounts.length(); ++i) {
    NGramFrequency* cur_ngf = ngramcounts[i];
    NGram* cur_ngram = ngramcounts[i]->ngram;
    for(int j = i+1; j < ngramcounts.length(); ++j) {
      NGram* other_ngram = ngramcounts[j]->ngram;
      static int equalcount = 0;
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
        --j; // need to decrement j to avoid skipping an index (everything shifted left)
      }
    }
  }
  return ngramcounts;
}

GenericVector<NGram*> NGramRanker::
readNGramFile(const std::string& filepath, int gram) {
  std::ifstream ngram_str(filepath.c_str());
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
    for(int i = 0; i < strlen(ln); ++i) {
      if(i == 0 && (ln[i] == ' ' || ln[i] == '\n'))
        nGramReadError(filepath);
      if(ln[i] == ' ' || (i+1 == strlen(ln))) {
        // first grab the word
        int wrdlen = i - wrdstart; // this discards delimiters
        if(i+1 == strlen(ln)) // if we're at the end there's no delimiter
          wrdlen++;
        char* word = new char[wrdlen+1]; // +1 for null terminator
        for(int j = 0; j < wrdlen; ++j)
          word[j] = ln[wrdstart+j];
        word[wrdlen] = '\0';
        // now add the word to the current ngram
        ngram_words.push_back(word);
        if(ln[i] == '\n') {
          // done reading current ngram, make sure its the right length
          if(ngram_words.length() != gram || (ln[i+1] != '\0'))
            nGramReadError(filepath);
        }
        wrdstart = i+1; // + 1 to discard delimiter at beginning of new word!
      }
    }
    ngram->words = ngram_words;
    ngrams.push_back(ngram);
  }
  ngram_str.close();
  return ngrams;
}

void NGramRanker::writeNGramFiles(
    const GenericVector<TesseractSentenceData*>& sentences,
    const std::string& path, std::ofstream* streams) {
  for(int i = 1; i <= 3; ++i)
    writeNGramFile(i, sentences, path, (streams+(i-1)));
}

// TODO: Abstract n-gram detection functionality here away from this function so writing
//       to the file is optional, not required. For now I'm just always writing to a file
//       since it makes it easier to debug when debugging is necessary.
void NGramRanker::writeNGramFile(const int& gram,
    const GenericVector<TesseractSentenceData*>& sentences,
    const std::string& path, std::ofstream* stream) {
  std::string filename = getNGramFileName(gram);
  std::string filepath = path + filename;
  stream->open(filepath.c_str(), std::ios_base::app);
  if(!stream->is_open()) {
    std::cout << "ERROR: Could not open " << filepath << " for writing!\n";
    assert(false);
  }

  // Init a tesseract api for validating words
  tesseract::TessBaseAPI api;
  api.Init("/usr/local/share/", "eng");

  for(int i = 0; i < sentences.length(); ++i) {
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
    for(int j = 0; j < strlen(s_txt); ++j) {
      assert(s_txt[j] != '\0');
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
            ++wrdlen;
          char* word = new char[wrdlen + 1]; // +1 for null terminator
          for(int k = 0; k < wrdlen; ++k)
            word[k] = s_txt[wrdstart + k];
          word[wrdlen] = '\0';
          ngram.push_back(word);
          int numgrams = ngram.length();
          if(numgrams == gram) {
            // first check all the words on the n-gram to make sure they are valid
            // and also to convert all uppercase characters to lowercase
            for(int k = 0; k < numgrams; ++k) {
              word = ngram[k];
              wrdlen = strlen(word);
              // convert all uppercase letters in the word to lower-case!
              for(int l = 0; l < wrdlen; ++l) {
                char char_ = word[l];
                if(isupper((int)char_))
                  word[l] = (char)tolower((int)char_);
              }
              // discard any punctuation/numbers and if the word is nothing but
              // punctuation/numbers then discard the whole word
              char* original_wrd = Utils::strCopy(word);
              int chars_removed = 0;
              if(!(wrdlen == 1 && gram == 1 && (Utils::stringCompare(word, "=")
              || Utils::stringCompare(word, "+")
              || Utils::stringCompare(word, "-")
              || Utils::stringCompare(word, "*")
              || Utils::stringCompare(word, "/")))) {
                for(int l = 0; l < wrdlen; ++l) {
                  char char_ = original_wrd[l];
                  if(isalpha((int)char_) == 0) {
                    word = Utils::strRemoveChar(word, l - chars_removed);
                    ++chars_removed;
                  }
                }
              }
              // discard any invalid word or any word on the stop word list
              if(word != NULL) {
                if(((api.IsValidWord(word) == 0)
                    && !Utils::stringCompare(word, "=")
                    && !Utils::stringCompare(word, "+")
                    && !Utils::stringCompare(word, "-")
                    && !Utils::stringCompare(word, "*")
                    && !Utils::stringCompare(word, "/"))
                    || (gram == 1 && stopwordHelper->isStopWord(word))) {
                  Utils::destroyStr(word);
                }
              }
              Utils::destroyStr(original_wrd); // finished using temporary copy
              ngram[k] = word; // make sure the word in the ngram points at the right place!!
            }
            // make sure the ngram is still valid (invalid words would have been discarded)
            bool ngram_ok = true;
            for(int k = 0; k < numgrams; ++k) {
              if(ngram[k] == NULL) {
                ngram_ok = false;
                break;
              }
            }
            // ready to write the n-gram to the file (assuming it is valid)
            if(ngram_ok) {
              for(int k = 0; k < ngram.length(); ++k)
                *stream << ngram[k]
                        << ((k+1) != ngram.length() ? " " : "\n");
            }
            for(int k = 0; k < ngram.length(); ++k) {
              char* w = ngram[k];
              Utils::destroyStr(w);
            }
            ngram.clear();
            if(gram != 1) {
              assert(ngram_next_index > 0);
              j = ngram_next_index; // go back to start of next ngram
            }
          }
          else if(numgrams == 1)
            ngram_next_index = j; // will go back to this once ngram is done
          else if(numgrams > gram) {
            std::cout << "ERROR: Too many words were written to an n-gram!\n";
            assert(false);
          }
          word_found = false;
        }
      }
    }
    // get rid of any left overs (i.e. if sentence ended while looking for more
    // words for the n-gram which just get rid of the remainder).
    for(int j = 0; j < ngram.length(); ++j) {
      char* w = ngram[j];
      delete [] w;
      w = NULL;
    }
    ngram.clear();
  }
  stream->close();
}

