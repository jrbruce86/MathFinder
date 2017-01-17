/*
 * BlobDataGridFactory.cpp
 *
 *  Created on: Dec 4, 2016
 *      Author: jake
 */

#include <BlobDataGridFactory.h>

#include <baseapi.h>
#include <BlobDataGrid.h>
#include <TessParamManager.h>
#include <BlobDataGrid.h>
#include <BlockData.h>
#include <CharData.h>
#include <RowData.h>
#include <WordData.h>

#include <string>

#include <M_Utils.h>
#include <Utils.h>

#include <stdio.h> // for NULL
#include <assert.h>

//#define DBG_ROW_CHARACTERISTICS
#define DBG_INFO_GRID
//#define DBG_VERBOSE
//#define DBG_SHOW_ALL_BLOB_RESULT

BlobDataGrid* BlobDataGridFactory::createBlobDataGrid(Pix* image,
    tesseract::TessBaseAPI* tessBaseApi, const std::string imageName) {

  std::cout << "Creating grid with image name of: " << imageName << std::endl;
  Utils::waitForInput();

  // Initialize the tesseract api
  tessBaseApi->Init("/usr/local/share/", "eng");

  // Choose the page segmentation mode as PSM_AUTO
  // Fully automatic page segmentation, but no OSD
  // (Orientation and Script Detection).
  tessBaseApi->SetPageSegMode(tesseract::PSM_AUTO);

  TesseractParamManager tesseractParamManager = TesseractParamManager(tessBaseApi); // in case...
  tesseractParamManager.activateBoolParam("save_blob_choices"); // Tell Tesseract to save blob choices

  /**
   * ---------------
   *    Stage 1:
   * ---------------
   * Here I leverage Tesseract's auto page segmentation. While their provided
   * math finding logic is disabled at this stage, the table detection
   * and segmentation logic is kept among various other features. I also
   * leverage their recognition results without any math equation detection in
   * place.
   */
  // Run Tesseract's layout analysis and recognition
  tessBaseApi->SetImage(image); // set the image
  tessBaseApi->Recognize(NULL); // Run Tesseract's layout analysis and recognition without equation detection


  /**
   * ---------------
   *    Stage 2:
   * ---------------
   * Take all of the raw connected components which Tesseract extracted from the image
   * during its early preprocessing stage and place them on a 2D search-able grid.
   * The grid entries added include, at this stage, just the connected component image
   * and its bounding box coordinates. More information will be added for each connected
   * component at later stages.
   */
  // Grab the connected components (as images)

  Pixa* blobImages = pixaCreate(0);
  Boxa* blobCoords = tessBaseApi->GetConnectedComponents(&blobImages);
  assert(blobImages->n == blobCoords->n); // should be the same.. don't see why not...

  // Create a grid containing an entry for each connected component which includes
  // its image and coordinates
  BlobDataGrid* blobDataGrid = new BlobDataGrid(1,
      ICOORD(0, 0), ICOORD(image->w, image->h), tessBaseApi, image, imageName);

  // Load all of the connected components and their images onto the grid
  // along with the recognition results
#ifdef DBG_INFO_GRID
  int total_blobs_grid = 0; // for debugging, count the total number of blobs in the original BlobGrid
#endif
  for(int i = 0; i < blobImages->n; ++i) {
    Box* box = blobCoords->box[i];
    Pix* blobImage = blobImages->pix[i];
    BlobData* blobData =
        new BlobData(M_Utils::LeptBoxToTessBox(box, image),
            blobImage, blobDataGrid);
    blobDataGrid->InsertBBox(true, true, blobData);
    ++total_blobs_grid;
  }
#ifdef DBG_INFO_GRID
  {
    std::cout << "total blobs in grid: " << total_blobs_grid << std::endl; // debug
    ScrollView* sv = blobDataGrid->MakeWindow(image->w, image->h,
        "Grid");
    blobDataGrid->DisplayBoxes(sv);
    M_Utils::waitForInput();
    delete sv;
  }
#endif


  /**
   * ---------------
   *    Stage 3:
   * ---------------
   * Iterating the results of Tesseract's layout analysis, insert each symbol-level
   * result into the grid created in Stage 2 at its appropriate connected component
   * entry.
   */
  // Note: Take into account all of the features available, remember the inheritance hierarchy
  // Note: While I'm using the mutable iterator, I'm still only changing the pointer through the higher level api.
  //       Only reason I'm using the mutable iterator is so I have public access to everything for readonly purposes

  // Initialize the grid search
  BlobDataGridSearch blobDataGridSearch(blobDataGrid);

  // TODO: Once compiles, see about iterating through the symbols in the word to
  //       try and get the isSuperscript feature or other features that might be helpful......

  // TODO: Add in row_res and other stuff I may need later (this will come in handy
  //       for n-grams and stuff).

  // Iterate the blocks, and within the blocks the rows to get the sentences.
  // I'll then map the individual blobs to the sentences to which they belong to later (if they belong to a sentence)
  std::vector<TesseractBlockData*> blocks;
  // The Page_Res is sort of like a Russian doll. There are many layers:
  // BLOCK_RES -> ROW_RES -> WERD_RES -> WERD_CHOICE -> BLOB_CHOICE
  //                         WERD_RES -> WERD        -> C_BLOB

  // My resulting data structure is similar but slightly different:
  // Block -> Row -> Word -> Character -> Blob

  // Iterate through the block(s) of text inside the page
  const BLOCK_RES_LIST* block_results = &(tessBaseApi->extGetPageResults()->block_res_list);
  BLOCK_RES_IT bres_it((BLOCK_RES_LIST*)block_results);
  bres_it.move_to_first();
  for(int i = 0; i < block_results->length(); ++i) { // start iterating blocks on page

    TesseractBlockData* tesseractBlockData = new TesseractBlockData(bres_it.data(), blobDataGrid);

    // Go ahead and add this block data to the grid, so I can get quick top-down access
    // to the blocks, and the sentences or other contents within them that are of interest
    blobDataGrid->getTesseractBlocks().push_back(tesseractBlockData);

    // Iterate through the row(s) of text inside the text block
    ROW_RES_LIST& rowResList = tesseractBlockData->getBlockRes()->row_res_list;
    ROW_RES_IT rowresit(&rowResList);
    rowresit.move_to_first();
    for(int j = 0; j < rowResList.length(); ++j) { // start iterating rows on block
      TesseractRowData* tesseractRowData = new TesseractRowData(rowresit.data(), tesseractBlockData);
      tesseractBlockData->getTesseractRows().push_back(tesseractRowData);
      tesseractRowData->rowIndex = j;
      char* firstvalidword = getRowValidTessWord(tesseractRowData, tessBaseApi);
      if(firstvalidword != NULL) {
        tesseractRowData->setHasValidTessWord(true);
        tesseractRowData->aValidWordFound = firstvalidword;
      }


      // Iterate the words within the row
      WERD_RES_IT wordresit(tesseractRowData->getWordResList());
      wordresit.move_to_first();
      for(int k = 0; k < tesseractRowData->getWordResList()->length(); ++k) { // start iterating words on row on block
        WERD_RES* wordResultData = wordresit.data();
        if(!wordResultData) {
          assert(false); // asserting false because I don't think this is possible but we'll see
          continue; // Only care if has results (don't think it should get here.. but if it did this is what I'd do)
        }
        WERD_CHOICE* const bestWordChoice = wordResultData->best_choice;
        if(!bestWordChoice) {
          // assert(false); // Shouldn't happen? We'll see.... (did happen.. whatever)
          continue; // Only care if has results
        }

        // Create a word wrapper for this word that has access to its parent row among other things
        // Also add it to its parent's row wrapper to allow top-down access
        TesseractWordData* tesseractWordData =
            new TesseractWordData(wordResultData->word->bounding_box(), wordResultData, tesseractRowData);
        tesseractRowData->getTesseractWords().push_back(tesseractWordData);

        // Go ahead and find out if Tesseract api sees the word as valid or not
        if(tesseractWordData->wordstr() != NULL) {
          tesseractWordData->setIsValidTessWord(tessBaseApi->IsValidWord(tesseractWordData->wordstr()));
        }

        // Iterate the characters in the word, adding all of each character's blobs to the grid
        // Bounding boxes and results for the recognized characters in the word
        const char* const unicodeWordResult = bestWordChoice->unichar_string().string();
        const char* const unicodeCharLengths = bestWordChoice->unichar_lengths().string();
        const int numCharactersInWord = strlen(unicodeCharLengths);
        int charBytePosition = 0;
        for (int i = 0; i < numCharactersInWord; i++) { // start iterating chars in word in row in block

          // get the substring representing the unicode result for the current character
          const int unicodeCharBytes = unicodeCharLengths[i]; // num bytes in this character's unicode result
          std::string unicodeCharResult(std::string(unicodeWordResult),
              charBytePosition, unicodeCharBytes);
          charBytePosition += unicodeCharBytes; // advance the position

          // get the bounding box of the current recognized character within the word
          TBOX charResultBox = tesseractWordData->getBoundingBox().intersection(
              wordResultData->box_word->BlobBox(i));

          // get the recognition data for this character (should be at the head of the choice list for this character)
          // iterator over the characters (each entry has a list of choices
          // for the corresponding character)
          BLOB_CHOICE_LIST_C_IT characterChoiceListIt(bestWordChoice->blob_choices());
          for(int j = 0; j < i; ++j) { // advance to the choice list to that of the current character
            characterChoiceListIt.forward();
          }

          if(characterChoiceListIt.empty()) {
            continue; // only care if there's actually a result
          }

          BLOB_CHOICE* const bestChoice =
              BLOB_CHOICE_IT(characterChoiceListIt.data()).data(); // should be at the head of the choice list for this character


          // the below shouldn't happen, every char recognized
          // by Tesseract should correspond to at least
          // one blob in the image.... as it turns out this does happen....
          // in this case Tesseract must be confused. I will just ignore the character in this case
          if(blobDataGrid->RectangleEmpty(charResultBox)) {
            continue;
          }

          // Get blobs that overlap this character in my grid and update them to have this data
          TesseractCharData* tesseractCharData =
              (new TesseractCharData(charResultBox, tesseractWordData))
              ->setCharResultInfo(bestChoice)
              ->setRecognitionResultUnicode(unicodeCharResult);
          tesseractWordData->getTesseractChars().push_back(tesseractCharData);
          blobDataGridSearch.SetUniqueMode(true);
          blobDataGridSearch.StartRectSearch(charResultBox);
          BlobData* curBlobData = blobDataGridSearch.NextRectSearch();
          while(curBlobData != NULL) { // start iterating blobs in char in word in row in block
            // only care about blobs completely inside the current character box
            if(tesseractCharData->getBoundingBox()->contains(curBlobData->getBoundingBox())) {

              if(curBlobData->getParentChar() == NULL) {
                // Above if rules out the possibility of a blob mapping to more than one character.
                // Shouldn't be possible. But as it turns out it is when Tesseract gets confused which happens VERY OFTEN with the images I'm giving it...
                // Below else statement contains debug for when I want to investigate.
                // But for now, I'm just taking whatever I'm given first, and aside from printing out
                // a warning, I'm just ignoring the rest and moving on with my life.
                curBlobData->setCharacterRecognitionData(tesseractCharData);
                tesseractCharData->getBlobs().push_back(curBlobData);
#ifdef DBG_SHOW_ALL_BLOB_RESULT
                std::cout << "The shown blob was recognized as belonging to the character " << tesseractCharData->getUnicode() << " and word " << tesseractCharData->getParentWord()->wordstr() << std::endl;
                std::cout << "The character boundingbox to which the blob is currently being assigned is: ";
                tesseractCharData->getBoundingBox()->print();
                std::cout << "The blob boundingbox is: ";
                curBlobData->getBoundingBox().print();
                pixDisplayWithTitle(curBlobData->getBlobImage(), 100, 100, "Blob", 1);
                M_Utils::dispRegion(M_Utils::tessTBoxToImBox(tesseractCharData->getBoundingBox(), image), image);
                Utils::waitForInput();
#endif
              } else {
                std::cout << "WARNING: blob was assigned by Tesseract a parent character more than once!!... "
                    << "was previously assigned to char at memory " << curBlobData->getParentChar() << ", "
                    << "now being assigned to " << tesseractCharData << ".. They the same? I'm taking the previous assignment and ignoring this one" << std::endl;
#ifdef DBG_MULTI_PARENT_ISSUE
                std::cout << "The blob was recognized as belonging to the character " << tesseractCharData->getUnicode() << std::endl;
                std::cout << "The character boundingbox to which the blob is currently being assigned is: ";
                tesseractCharData->getBoundingBox()->print();
                std::cout << "The blob boundingbox is: ";
                curBlobData->getBoundingBox().print();
                std::cout << "Showing the hierarchy for the current parent the blob is being assigned to:\n";
                dbgDisplayHierarchy(tesseractBlockData,
                    tesseractRowData,
                    tesseractWordData,
                    tesseractCharData,
                    curBlobData, image);
                std::cout << "Showing the hierarchy for the parent the blob was already assigned to previously:\n";
                std::cout << "The character boundingbox this blob was previously assigned to: ";
                curBlobData->getParentChar()->getBoundingBox()->print();
                dbgDisplayHierarchy(curBlobData->getParentBlock(),
                    curBlobData->getParentRow(),
                    curBlobData->getParentWord(),
                    curBlobData->getParentChar(),
                    curBlobData, image);
#endif
              }
            }
            curBlobData = blobDataGridSearch.NextRectSearch();
          } // done iterating blobs in char in word in row in block
        } // done iterating chars in word in row in block
        wordresit.forward();
      } // done iterating words in row in block
      rowresit.forward();
    } // done iterating rows in block
    bres_it.forward();
  } // done iterating blocks on the page
#ifdef DBG_INFO_GRID
  {
    ScrollView* sv = blobDataGrid->MakeWindow(100, 100,
        "The BlobDataGrid");
    blobDataGrid->DisplayBoxes(sv);
    M_Utils::waitForInput();
    delete sv;
  }
#endif
  for(int i = 0; i < blobDataGrid->getTesseractBlocks().size(); ++i) {
    blobDataGrid->getTesseractBlocks()[i]->findRecognizedSentences(
        tessBaseApi, blobDataGrid);
  }

  findAllRowCharacteristics(blobDataGrid);

  // Put all of the recognized sentences and rows directly on list stored within the grid for convenience
  for(int i = 0; i < blobDataGrid->getTesseractBlocks().size(); ++i) {
    std::vector<TesseractSentenceData*> blockSentences = blobDataGrid->getTesseractBlocks()[i]->getRecognizedSentences();
    for(int j = 0; j < blockSentences.size(); ++j) {
      blobDataGrid->getAllRecognizedSentences().push_back(blockSentences[j]);
    }
    std::vector<TesseractRowData*> blockRows = blobDataGrid->getTesseractBlocks()[i]->getTesseractRows();
    for(int j = 0; j < blockRows.size(); ++j) {
      blobDataGrid->getAllTessRows().push_back(blockRows[j]);
    }
  }

  return blobDataGrid;
}

/**
 * Determines if the given row has a valid word on it according to Tesseract
 */
char* BlobDataGridFactory::getRowValidTessWord(
    TesseractRowData* const rowData,
    tesseract::TessBaseAPI* const api) {
  WERD_RES_LIST* wordreslist = rowData->getWordResList();
  WERD_RES_IT wordresit1(wordreslist);
  wordresit1.move_to_first();
  bool row_has_valid = false;
  for(int k = 0; k < wordreslist->length(); k++) {
    WERD_RES* wordres = wordresit1.data();
    WERD_CHOICE* wordchoice = wordres->best_choice;
    if(wordchoice != NULL) {
      char* wrd = (char*)wordchoice->unichar_string().string();
      if(api->IsValidWord(wrd)) {
        return wrd;
      }
    }
    wordresit1.forward();
  }
  return NULL;
}

//TODO: Modify so that first row with valid words is considered the top row
//      and discarded, rather than simply the top row. Sometimes there is noise
//      on the top row.
void BlobDataGridFactory::findAllRowCharacteristics(BlobDataGrid* const blobDataGrid) {

  // Put all of the rows from the blocks on the page onto a single vector
  std::vector<TesseractBlockData*>& tesseractBlocks = blobDataGrid->getTesseractBlocks();
  std::vector<TesseractRowData*> rows;
  for(int i = 0; i < tesseractBlocks.size(); ++i) {
    for(int j = 0; j < (tesseractBlocks[i])->getTesseractRows().size(); ++j) {
      rows.push_back(tesseractBlocks[i]->getTesseractRows()[j]);
    }
  }

#ifdef DBG_ROW_CHARACTERISTICS
  std::cout << "here are the rows in their vector ordering:\n";
  for(int i = 0; i < rows.size(); ++i) {
    std::cout << rows[i]->getRowText() << std::endl;
  }
  M_Utils::waitForInput();
#endif
  // find the average # of valid words per row
  double avg_valid_words = 0;
  double num_rows_with_valid = 0;
  for(int i = 0; i < rows.size(); ++i) {
    TesseractRowData* const row = rows[i];
    if(!row->getHasValidTessWord()) {
      row->setValidWordCount(0);
      continue;
    }
    GenericVector<TesseractWordData*> words = row->getTesseractWords();
    int valid_words_cur_row = 0;
    std::cout << "Checking " << words.size() << " words on the current row...\n";
    for(int j = 0; j < words.length(); ++j) {
      if(!words[j]->wordstr())
        continue;
      if(blobDataGrid->getTessBaseAPI()->IsValidWord(words[j]->wordstr()))
        ++valid_words_cur_row;
      else {
        std::cout << words[j]->wordstr() << " found not to be valid...\n";
      }
    }
    if(words.empty()) {
      std::cout << "No words on the row......\n";
    }
    std::cout << "valid word that was found previously: " << row->aValidWordFound << std::endl;
    if(valid_words_cur_row > 0) { // shouldn't be 0... but sometimes is when the engine is confused so...
      ++num_rows_with_valid;
    }
    row->setValidWordCount(valid_words_cur_row);
    avg_valid_words += (double)valid_words_cur_row;
  }
  avg_valid_words /= num_rows_with_valid;
#ifdef DBG_ROW_CHARACTERISTICS
  std::cout << "average number of valid words per row: " << avg_valid_words << std::endl;
#endif
  // find the standard deviation of the # of valid words per row that fall short of
  // the average
  double std_dev_valid_words = 0;
  for(int i = 0; i < rows.size(); ++i) {
    if(!rows[i]->getHasValidTessWord()) {
      assert(rows[i]->numValidWords() == 0);
      continue;
    }
    double cur_deviation = avg_valid_words - (double)(rows[i]->numValidWords());
    cur_deviation = pow(cur_deviation, (double)2);
    std_dev_valid_words += cur_deviation;
    ++num_rows_with_valid;
  }
  std_dev_valid_words /= num_rows_with_valid;
  std_dev_valid_words = sqrt(std_dev_valid_words);
#ifdef DBG_ROW_CHARACTERISTICS
  std::cout << "standard deviation of valid words per row: " << std_dev_valid_words << std::endl;
#endif
  // for low standard deviation it is expected that much of the text is normal
  // although it could also indicate that most of the text is abnormal and with
  // very little variance. in the latter case it would be difficult to come up
  // with any indication of whether or not a row is normal without comparing the
  // page to some preconceived model, which would be outside of the scope of
  // the current work. thus the former case is what is assuemd by this design.
  // for a row to be considered as "normal" in the first pass it must have
  // a number of valid words greater than a threshold determined as follows:
  // if the row has a count of valid words which negatively deviates from the average by
  // more than twice the standard deviation, then the row is considered abnormal
  // initially, otherwise it is labeled as normal. only rows that have at least one
  // valid word are used to calculate average and standard deviation.

  // now get average vertical spacing between rows and standard devation
  double avg_vertical_space = 0;
  for(int i = 0; i < rows.size(); ++i) {
    // discard top row since it is usually a heading
    if(i == 0)
      continue;
    if(!rows[i]->hasValidTessWord)
      continue;
    if(i != (rows.size() - 1)) {
      if(!rows[i+1]->hasValidTessWord)
        continue;
      double dist_below = rows[i]->getBoundingBox().bottom() - rows[i+1]->getBoundingBox().top();
      avg_vertical_space += dist_below;
    }
  }
  avg_vertical_space /= num_rows_with_valid;
#ifdef DBG_ROW_CHARACTERISTICS
  std::cout << "average vertical space between rows with at least one valid word: " << avg_vertical_space << std::endl;
#endif
  double vert_space_std_dev = 0;
  for(int i = 0; i < rows.size(); ++i) {
    if(i == 0)
      continue;
    if(!rows[i]->getHasValidTessWord())
      continue;
    if(i != (rows.size() - 1)) {
      if(!rows[i+1]->getHasValidTessWord())
        continue;
      double dist_below = rows[i]->getBoundingBox().bottom() - rows[i+1]->getBoundingBox().top();
      double cur_vert_space_dev = avg_vertical_space - dist_below;
      cur_vert_space_dev = pow(cur_vert_space_dev, (double)2);
      vert_space_std_dev += cur_vert_space_dev;
    }
  }
  vert_space_std_dev /= num_rows_with_valid;
  vert_space_std_dev = sqrt(vert_space_std_dev);
#ifdef DBG_ROW_CHARACTERISTICS
  std::cout << "vertical space standard deviation for rows with at least 1 valid word: " << vert_space_std_dev << std::endl;
#endif
  // pass 1: assign initial abnormal rows based on valid word counts
  for(int i = 0; i < rows.size(); ++i) {
    if(i == 0) // assume top row is heading
      continue;
    TesseractRowData* const row = rows[i];
    double valid_word_deviation = (double)(rows[i]->numValidWords() - avg_valid_words);
    if(valid_word_deviation < 0) {
      valid_word_deviation = -valid_word_deviation;
      if(valid_word_deviation > (1.0*std_dev_valid_words)) {
#ifdef DBG_ROW_CHARACTERISTICS
        std::cout << "pass 1: assigning the following row to being abnormal:\n";
        std::cout << row->getRowText() << std::endl;
        if(row->getRowText().empty())
          std::cout << "NULL\n";
        M_Utils::dispHlTBoxRegion(row->row()->bounding_box(), blobDataGrid->getImage());
        M_Utils::waitForInput();
#endif
        row->setIsConsideredNormal(false);
      }
    }
    else
      assert(rows[i]->numValidWords() > 0);
  }

  // pass 2: use vertical space standard deviation to label rows at
  // paragraph endings which may have a small number of valid words as normal
  for(int i = 0; i < rows.size(); ++i) {
    TesseractRowData* const row = rows[i];
    if(i == 0 || i == 1) { // here I just assume the top row is a heading
      continue;
    }
    double above_v_space = 0;
    double above_deviation = 0;
    above_v_space = rows[i-1]->getBoundingBox().bottom() - rows[i]->getBoundingBox().top();
    above_deviation = -(avg_vertical_space - above_v_space);
    if(above_deviation < ((double)1.0)*vert_space_std_dev) {
      if(rows[i-1]->isConsideredNormal && !row->getIsConsideredNormal()) {
#ifdef DBG_ROW_CHARACTERISTICS
        std::cout << "pass 2: assigning the following row back to normal:\n";
        std::cout << row->getRowText() << std::endl;
        std::cout << "above deviation: " << above_deviation << std::endl;
        std::cout << "vert space standard deviation: " << vert_space_std_dev << std::endl;
        M_Utils::dispHlTBoxRegion(row->row()->bounding_box(), blobDataGrid->getImage());
        M_Utils::waitForInput();
#endif
        row->setIsConsideredNormal(true);
      }
    }
  }
#ifdef DBG_ROW_CHARACTERISTICS
  std::cout << "Finished getting row characteristics.\n";
  M_Utils::waitForInput();
#endif
}

void BlobDataGridFactory::dbgDisplayHierarchy(
    TesseractBlockData* tesseractBlockData,
    TesseractRowData* tesseractRowData,
    TesseractWordData* tesseractWordData,
    TesseractCharData* tesseractCharData,
    BlobData* blob,
    Pix* image) {
  TBOX blockbox = tesseractBlockData->getBlockRes()->block->bounding_box();
  std::cout << "Showing the block.\n";
  M_Utils::dispRegion(M_Utils::tessTBoxToImBox(&blockbox, image), image);
  M_Utils::waitForInput();
  TBOX rowbox = (tesseractRowData->rowRes->row->bounding_box());
  std::cout << "Showing the row.\n";
  M_Utils::dispRegion(M_Utils::tessTBoxToImBox(&rowbox, image), image);
  M_Utils::waitForInput();
  std::cout << "Showing the word.\n";
  TBOX wordbox = tesseractWordData->getBoundingBox();
  M_Utils::dispRegion(M_Utils::tessTBoxToImBox(&wordbox, image), image);
  M_Utils::waitForInput();
  std::cout << "Showing the character.\n";
  M_Utils::dispRegion(M_Utils::tessTBoxToImBox(tesseractCharData->getBoundingBox(), image), image);
  M_Utils::waitForInput();
  std::cout << "Showing the blob.\n";
  pixDisplay(blob->getBlobImage(), 100, 100);
  M_Utils::waitForInput();
}

