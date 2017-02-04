/**************************************************************************
 * File name:   M_Utils.cpp
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Oct 4, 2013 12:55:26 PM
 *              Modified Dec 28, 2016
 * ------------------------------------------------------------------------
 * Description: This is a static class containing assorted utility methods
 *              useful for the MEDS module.
 ***************************************************************************/
#include <M_Utils.h>
#include <Lept_Utils.h>
#include <BlobDataGrid.h>

#include <Utils.h>

//using namespace tesseract;


// TODO: Migrate everything here that belongs in basic_utils and lept_utils to there
//       and make sure everything using those functions uses the function from the right
//       file.

BOX* M_Utils::getBlobBoxImCoords(BLOBNBOX* blob, PIX* im) {
  l_int32 height = (l_int32)im->h;
  l_int32 left = (l_int32)blob->cblob()->bounding_box().left();
  l_int32 top = height - (l_int32)blob->cblob()->bounding_box().top();
  l_int32 right = (l_int32)blob->cblob()->bounding_box().right();
  l_int32 bottom = height - (l_int32)blob->cblob()->bounding_box().bottom();
  return boxCreate(left, top, right-left, bottom-top);
}

TBOX M_Utils::imToCBlobGridCoords(BOX* box, PIX* im) {
  inT16 height = (inT16)im->h;
  inT16 left = (inT16)(box->x);
  inT16 top = height - (inT16)(box->y);
  inT16 right = left + (inT16)(box->w);
  inT16 bottom = height - (box->y+(inT16)(box->h));
  return TBOX(left,bottom,right,top);
}

BOX* M_Utils::getColPartImCoords(tesseract::ColPartition* cp, PIX* im) {
  BLOBNBOX_CLIST* blobnboxes = cp->boxes();
  CLIST_ITERATOR bbox_it(blobnboxes);
  l_int32 height = (l_int32)im->h;
  l_int32 left = INT_MAX;
  l_int32 right = INT_MIN;
  l_int32 top = INT_MAX;
  l_int32 bottom = INT_MIN;
  l_int32 l, r, t, b;
  for (bbox_it.mark_cycle_pt (); !bbox_it.cycled_list();
      bbox_it.forward()) {
    BLOBNBOX* blobnbox = (BLOBNBOX*)bbox_it.data();
    l = (l_int32)blobnbox->cblob()->bounding_box().left();
    r = (l_int32)blobnbox->cblob()->bounding_box().right();
    t = height - (l_int32)blobnbox->cblob()->bounding_box().top();
    b = height - (l_int32)blobnbox->cblob()->bounding_box().bottom();
    if(l < left)
      left = l;
    if(r > right)
      right = r;
    if(t < top)
      top = t;
    if(b > bottom)
      bottom = b;
  }
  BOX* boxret = boxCreate(left, top, right-left, bottom-top);
  return boxret;
}

TBOX M_Utils::getColPartTBox(tesseract::ColPartition* cp, PIX* im) {
  BOX* cpbox = getColPartImCoords(cp, im);
  TBOX tbox = imToCBlobGridCoords(cpbox, im);
  boxDestroy(&cpbox);
  return tbox;
}

/*ColPartition* M_Utils::getTBoxColPart(ColPartitionGrid* cpgrid,
    TBOX t, PIX* img) {
  ColPartitionGridSearch colsearch(cpgrid);
  colsearch.StartFullSearch();
   ColPartition* curpart = NULL;
   while ((curpart = colsearch.NextFullSearch()) != NULL) {
     BOX* partbox;
     if(curpart->boxes_count() > 0)
       partbox = getColPartImCoords(curpart, img);
     else {
       TBOX b = curpart->bounding_box();
       partbox = tessTBoxToImBox(&b, img);
     }
     TBOX rtbox = t;
     BOX* box = tessTBoxToImBox(&rtbox, img);
     int intersects;
     boxIntersects(partbox, box, &intersects);
     if(intersects)
       return curpart;
     boxDestroy(&partbox);
     boxDestroy(&box);
   }
   return NULL;
}*/

BOX* M_Utils::getCBlobImCoords(C_BLOB* blob, PIX* im) {
  l_int32 height = (l_int32)im->h;
  l_int32 left = (l_int32)blob->bounding_box().left();
  l_int32 top = height - (l_int32)blob->bounding_box().top();
  l_int32 right = (l_int32)blob->bounding_box().right();
  l_int32 bottom = height - (l_int32)blob->bounding_box().bottom();
  return boxCreate(left, top, right-left, bottom-top);
}

/**
 * Leptonica uses a coordinate system with origin at the top-left
 * and with y increasing downwards. Tesseract uses a coordinate
 * system with the bottom left as the origina and y and increasing
 * upwards. Both increase x from left to right.
 */
BOX* M_Utils::tessTBoxToImBox(TBOX* box, PIX* im) {
  l_int32 height = (l_int32)im->h;
  l_int32 left = (l_int32)box->left();
  l_int32 top = height - (l_int32)box->top();
  l_int32 right = (l_int32)box->right();
  l_int32 bottom = height - (l_int32)box->bottom();
  return boxCreate(left, top, right-left, bottom-top);
}

/**
 * Leptonica uses a coordinate system with origin at the top-left
 * and with y increasing downwards. Tesseract uses a coordinate
 * system with the bottom left as the origina and y and increasing
 * upwards. Both increase x from left to right.
 */
TBOX M_Utils::LeptBoxToTessBox(BOX* box, PIX* im) {
  int height = im->h;
  int left = box->x;
  int right = box->x + box->w;
  int top = height - box->y;
  int bottom = height - (box->y + box->h);
  return TBOX(left, bottom, right, top);
}

BOX* M_Utils::getBlobDataBox(BlobData* b, PIX* im) {
  TBOX t = b->getBoundingBox();
  return tessTBoxToImBox(&t, im);
}

bool M_Utils::almostContains(TBOX box1, TBOX box2) {
  int adjustment = 1;//(int)((double)(box1.area()) * (double).02);
//  if(adjustment == 0) {
//    adjustment = 1;
//  }
  TBOX biggerBox1(
      box1.left() - adjustment,
      box1.bottom() - adjustment,
      box1.right() + adjustment,
      box1.top() + adjustment);
  return biggerBox1.contains(box2);
}

void M_Utils::dispTBoxAsCoords(TBOX box) {
  std::cout << "bottom left: (" << box.botleft().x() <<
      ", " << box.botleft().y() << "), top right " << "(" <<
      box.topright().x() << ", " <<
      box.topright().y() << ")\n";
}

BLOB_CHOICE* M_Utils::runBlobOCR(BLOBNBOX* blob, tesseract::Tesseract* ocrengine) {
  // * Normalize blob height to x-height (current OSD):
  // SetupNormalization(NULL, NULL, &rotation, NULL, NULL, 0,
  //                    box.rotational_x_middle(rotation),
  //                    box.rotational_y_middle(rotation),
  //                    kBlnXHeight / box.rotational_height(rotation),
  //                    kBlnXHeight / box.rotational_height(rotation),
  //                    0, kBlnBaselineOffset);
  BLOB_CHOICE_LIST ratings_lang;
  C_BLOB* cblob = blob->cblob();
  TBLOB* tblob = TBLOB::PolygonalCopy(cblob);
  const TBOX& box = tblob->bounding_box();

  // Normalize the blob. Set the origin to the place we want to be the
  // bottom-middle, and scaling is to mpx, box_, NULL);
  float scaling = static_cast<float>(kBlnXHeight) / box.height();
  DENORM denorm;
  float x_orig = (box.left() + box.right()) / 2.0f, y_orig = box.bottom();
  denorm.SetupNormalization(NULL, NULL, NULL, NULL, NULL, 0,
                            x_orig, y_orig, scaling, scaling,
                            0.0f, static_cast<float>(kBlnBaselineOffset));
  TBLOB* normed_blob = new TBLOB(*tblob);
  normed_blob->Normalize(denorm);
  ocrengine->AdaptiveClassifier(normed_blob, denorm, &ratings_lang, NULL);
  delete normed_blob;
  delete tblob;

  // Get the best choice from ratings_lang and rating_equ. As the choice in the
  // list has already been sorted by the certainty, we simply use the first
  // choice.
  BLOB_CHOICE *lang_choice = NULL;
  if (ratings_lang.length() > 0) {
    BLOB_CHOICE_IT choice_it(&ratings_lang);
    lang_choice = choice_it.data();
  }
  return lang_choice;
}

const char* const M_Utils::getBlobChoiceUnicode(BLOB_CHOICE* bc,
    tesseract::Tesseract* ocrengine) {
  return ocrengine->unicharset.id_to_unichar_ext(bc->unichar_id());
}

void M_Utils::dispBlobOCRRes(BLOBNBOX* blob, PIX* im,
    tesseract::Tesseract* ocrengine) {
  BOX* box_ = getBlobBoxImCoords(blob, im);
  PIX* bboxim = pixClipRectangle(im, box_, NULL);
  pixDisplay(bboxim, 100, 100);
  BLOB_CHOICE* ocr_res = runBlobOCR(blob, ocrengine);
  const char* const unicode_res = getBlobChoiceUnicode(ocr_res, ocrengine);
  std::cout << "OCR result: " << unicode_res << std::endl;
  std::cout << "certainty: " << ocr_res->certainty() << std::endl;
  waitForInput();
}

void M_Utils::waitForInput() {
  Utils::waitForInput();
}

void M_Utils::dispBoxCoords(BOX* box) {
  std::cout << "(left, top, right, bottom): " << box->x << ", "
       << box->y << ", " << box->w+box->x << ", "
       << box->y+box->h << std::endl;
}

void M_Utils::dispRegion(BOX* box, PIX* im) {
  PIX* bboxim = pixClipRectangle(im, box, NULL);
  pixDisplay(bboxim, 100, 100);
}

void M_Utils::dbgDisplayBlob(BlobData* blob) {
  Pix* im = blob->getParentGrid()->getBinaryImage();
  dispHlBlobDataRegion(blob, im);
  dispBlobDataRegion(blob, im);
  waitForInput();
}


// convenience function for debugging, display a blobinfo boundingbox
void M_Utils::dispBlobDataRegion(BlobData* bb, PIX* im) {
  TBOX t = bb->getBoundingBox();
  BOX* box = tessTBoxToImBox(&t, im);
  dispRegion(box, im);
  boxDestroy(&box);
}

void M_Utils::dispHlBlobDataRegion(BlobData* bb, PIX* im) {
  TBOX t = bb->getBoundingBox();
  dispHlTBoxRegion(t, im);
}

void M_Utils::dispHlTBoxRegion(TBOX tbox, PIX* im) {
  BOX* box = tessTBoxToImBox(&tbox, im);
  PIX* imcpy = pixCopy(NULL, im);
  imcpy = pixConvertTo32(imcpy);
  Lept_Utils::fillBoxForeground(imcpy, box, LayoutEval::RED);
  pixDisplay(imcpy, 100, 100);
  pixDestroy(&imcpy);
  boxDestroy(&box);
}

void M_Utils::drawHlBlobDataRegion(BlobData* bb, PIX* im, LayoutEval::Color color) {
  TBOX t = bb->getBoundingBox();
  BOX* box = tessTBoxToImBox(&t, im);
  Lept_Utils::fillBoxForeground(im, box, color);
  boxDestroy(&box);
}

void M_Utils::drawHlBoxRegion(BOX* box, PIX* im, LayoutEval::Color color) {
  Lept_Utils::fillBoxForeground(im, box, color);
}

void M_Utils::dispTBoxCoords(TBOX* box) {
  std::cout << "int left=" << box->left() << ", top=" << box->top()
       << ", right=" << box->right() << ", bottom="
       << box->bottom() << ";\n";
}

void M_Utils::dispTBoxCoords(TBOX box) {
  std::cout << "int left=" << box.left() << ", top=" << box.top()
       << ", right=" << box.right() << ", bottom="
       << box.bottom() << ";\n";
}

void M_Utils::dispTBoxLeptCoords(TBOX box, Pix* im) {
  BOX* lBox = tessTBoxToImBox(&box, im);
  dispBoxCoords(lBox);
  boxDestroy(&lBox);
}

void M_Utils::dispTBoxLeptCoords(TBOX* box, Pix* im) {
  BOX* lBox = tessTBoxToImBox(box, im);
  dispBoxCoords(lBox);
  boxDestroy(&lBox);
}

// TODO: Move this to basic utils.. that's really where this belongs o_o
char* M_Utils::strDeepCpy(const char* const str) {
  if(str == NULL)
    return NULL;
  char* cpy = new char[strlen(str)+1];
  for(int i = 0; i < strlen(str); i++)
    cpy[i] = str[i];
  cpy[strlen(str)] = '\0';
  return cpy;
}

GenericVector<char*> M_Utils::lineSplit(const char* txt) {
  int txtlen = (int)strlen(txt);
  // pass 1: find split points
  GenericVector<int> splitpoints;
  for(int i = 0; i < txtlen; i++) {
    if(txt[i] == '\n' && (i < (txtlen-1)))
      splitpoints.push_back(i);
  }
  // pass 2: iterate split points to do all the splitting
  int prevsplit = 0;
  GenericVector<char*> res;
  if(splitpoints.empty()) {
    // deep copy the string
    char* newstr = strDeepCpy(txt);
    res.push_back(newstr);
    return res;
  }
  for(int i = 0; i < splitpoints.length(); i++) {
    int split = splitpoints[i];
    int newstrsize = split-prevsplit;
    char* ln = new char[newstrsize+2]; // +1 for null terminator and +1 for newline
    for(int i = 0; i < newstrsize; i++)
      ln[i] = txt[prevsplit+i];
    ln[newstrsize] = '\n';
    ln[newstrsize+1] = '\0'; // null terminator
    res.push_back(ln);
    splitpoints.clear();
    prevsplit = split;
  }
  // now just need to add the last line
  int lastsplit = prevsplit;
  int newstrsize = txtlen - prevsplit;
  char* ln = new char[newstrsize+1];
  for(int i = 0; i < newstrsize; i++)
    ln[i] = txt[prevsplit+i];
  ln[newstrsize] = '\0';
  res.push_back(ln);
  return res;
}

/**
 * Normalizes the given value to 1 - e^-x where the given
 * value is x.
 */
double M_Utils::expNormalize(double f) {
  return (1 - exp(-f));
}

/**
 * Calculates and returns the center x coordinate of the given box
 */
inT16 M_Utils::centerx(const TBOX& box){
  inT16 w = box.width();
  inT16 l = box.left();
  return l + (w / 2);
}

/**
 * Calculates and returns the center y coordinate of the given box
 */
inT16 M_Utils::centery(const TBOX& box){
  inT16 h = box.height();
  inT16 b = box.bottom();
  return b + (h / 2);
}

//} // end namespace tesseract
