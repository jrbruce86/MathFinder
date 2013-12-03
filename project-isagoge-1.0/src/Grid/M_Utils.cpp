/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   M_Utils.cpp
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Oct 4, 2013 12:55:26 PM
 * ------------------------------------------------------------------------
 * Description: This is a static class containing assorted utility methods
 *              useful for the MEDS module.
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
#include "M_Utils.h"
#include <BlobInfoGrid.h>

namespace tesseract {

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

BOX* M_Utils::getColPartImCoords(ColPartition* cp, PIX* im) {
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

TBOX M_Utils::getColPartTBox(ColPartition* cp, PIX* im) {
  BOX* cpbox = getColPartImCoords(cp, im);
  TBOX tbox = imToCBlobGridCoords(cpbox, im);
  boxDestroy(&cpbox);
  return tbox;
}

BOX* M_Utils::getCBlobImCoords(C_BLOB* blob, PIX* im) {
  l_int32 height = (l_int32)im->h;
  l_int32 left = (l_int32)blob->bounding_box().left();
  l_int32 top = height - (l_int32)blob->bounding_box().top();
  l_int32 right = (l_int32)blob->bounding_box().right();
  l_int32 bottom = height - (l_int32)blob->bounding_box().bottom();
  return boxCreate(left, top, right-left, bottom-top);
}

BOX* M_Utils::tessTBoxToImBox(TBOX* box, PIX* im) {
  l_int32 height = (l_int32)im->h;
  l_int32 left = (l_int32)box->left();
  l_int32 top = height - (l_int32)box->top();
  l_int32 right = (l_int32)box->right();
  l_int32 bottom = height - (l_int32)box->bottom();
  return boxCreate(left, top, right-left, bottom-top);
}

BOX* M_Utils::getBlobInfoBox(BLOBINFO* b, PIX* im) {
  TBOX t = b->bounding_box();
  return tessTBoxToImBox(&t, im);
}

BLOB_CHOICE* M_Utils::runBlobOCR(BLOBNBOX* blob, Tesseract* ocrengine) {
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
    Tesseract* ocrengine) {
  return ocrengine->unicharset.id_to_unichar_ext(bc->unichar_id());
}

void M_Utils::dispBlobOCRRes(BLOBNBOX* blob, PIX* im,
    Tesseract* ocrengine) {
  BOX* box_ = getBlobBoxImCoords(blob, im);
  PIX* bboxim = pixClipRectangle(im, box_, NULL);
  pixDisplay(bboxim, 100, 100);
  BLOB_CHOICE* ocr_res = runBlobOCR(blob, ocrengine);
  const char* const unicode_res = getBlobChoiceUnicode(ocr_res, ocrengine);
  cout << "OCR result: " << unicode_res << endl;
  cout << "certainty: " << ocr_res->certainty() << endl;
  waitForInput();
}

void M_Utils::waitForInput() {
  char c = 'a';
  char pc = 'b';
  while(c != '\n') {
    if(c == 'q')
      exit(EXIT_SUCCESS);
    if(pc != c && c != '\n')
      printf("Press enter to continue! or 'q' to quit\n");
    pc = c;
    scanf("%c", &c);
  }
}

void M_Utils::dispBoxCoords(BOX* box) {
  cout << "(left, top, right, bottom): " << box->x << ", "
       << box->y << ", " << box->w+box->x << ", "
       << box->y+box->h << endl;
}

void M_Utils::dispRegion(BOX* box, PIX* im) {
  PIX* bboxim = pixClipRectangle(im, box, NULL);
  pixDisplay(bboxim, 100, 100);
}

// convenience function for debugging, display a blobinfo boundingbox
void M_Utils::dispBlobInfoRegion(BLOBINFO* bb, PIX* im) {
  TBOX t = bb->bounding_box();
  BOX* box = tessTBoxToImBox(&t, im);
  dispRegion(box, im);
  boxDestroy(&box);
}

void M_Utils::dispHlBlobInfoRegion(BLOBINFO* bb, PIX* im) {
  TBOX t = bb->bounding_box();
  BOX* box = tessTBoxToImBox(&t, im);
  PIX* imcpy = pixCopy(NULL, im);
  imcpy = pixConvertTo32(imcpy);
  Lept_Utils lu;
  lu.fillBoxForeground(imcpy, box, LayoutEval::RED);
  pixDisplay(imcpy, 100, 100);
  pixDestroy(&imcpy);
  boxDestroy(&box);
}

void M_Utils::drawHlBlobInfoRegion(BLOBINFO* bb, PIX* im, SimpleColor color) {
  TBOX t = bb->bounding_box();
  BOX* box = tessTBoxToImBox(&t, im);
  Lept_Utils lu;
  lu.fillBoxForeground(im, box, (LayoutEval::Color)color);
  boxDestroy(&box);
}

void M_Utils::dispTBoxCoords(TBOX* box) {
  cout << "int left=" << box->left() << ", top=" << box->top()
       << ", right=" << box->right() << ", bottom="
       << box->bottom() << ";\n";
}

// TODO: Move this to basic utils.. that's really we're this belongs o_o
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


} // end namespace tesseract
