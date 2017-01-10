#ifndef MAINWIDGET_H
#define MAINWIDGET_H
// OSR - with Linux OCR Capabilities
//
// Author - whitedg 
// 
// Edited - Jake Bruce, Kelly Kiang, Gia Ngo
//
// mainWidget.h - declaration file for mainWidget object (Main GUI page)
//
// Created: 04 April 2008

#define OCR_VERSION 0 // 0 comparison, 1 key_gen
#include <QtGui>
#include <QApplication>
#include <QMainWindow>
#include <QTextEdit>
#include <QGridLayout>
#include <QPixmap>
#include <QLabel>
#include <QTabWidget>
#include <QThread>
#include <QTextCursor>
#include <iostream>
#include <string>
#include <list>
#include <math.h>
#include "pageTab.h"
#include <QSignalMapper>

using namespace std;

class MouseEventEater;
// class prototype

class mainWindow: public QMainWindow {
  Q_OBJECT
public:
  //=================================================================
  //	<pre>Constructor for GUI's main window. Calls several
  //	functions to set up the layout of the main window (menus,
  //	toolbars, statusbar) and initializes the tab layout as the
  //	central widget.</pre>
  //=================================================================
  mainWindow();

  bool highlightdone;

  // public data
  //= inZoomMode: flag to tell the event handler when the user is in zoom mode
  bool inZoomMode;

  //= inSelectMode: flag to tell event handler when the user is in select mode
  bool inSelectMode;

  //=selecting: When the user is in select mode and has clicked the mouse but not yet released it
  bool selecting;

  //=initRect: The starting QPoint for a user-selected rectangle
  QPoint initRect;

  //=endRect: The ending QPoint for a user-selected rectangle
  QPoint endRect;

  //=rect: The QRect used to represent the user-selected rectangle
  QRect rect;

  //=scaledRect: The QRect that delineates where in the actual image (if scaled down) that the rectangle will be located
  QRect scaledRect;

  //=zoomcount: The number of times a user has zoomed in (+) or out (-) for the image
  int zoomcount;

  //=scaleFactor: The factor by which the image is scaled during the zoom process
  double scaleFactor;

  //=currentTab: The tab that the user is currently viewing
  pageTab* currentTab;

  //=filter: The event filter that looks for various events from the mouse if in zoom or select mode
  MouseEventEater* filter;

  //== paintEvent: virtual function from Qt, redefined here so that it draws the current rectangle onto the image.
  // when a new rectangle is being drawn, it redraws the original pixmap onto the qlabel, so there is
  // only one rectangle at a time.
  void paintEvent(QPaintEvent *);

public slots:
//TAB SLOTS//
//=================================================================
// 	<pre>This slot is activated and run whenever the user
//	switches to a new tab.  The function checks the type of file(s)
//	included in the current tab, and enables/checks the layout
//	views that appropriate/currently shown.</pre>
//=================================================================
void tab_changed();

//=================================================================
// <pre>This slot responds to the signal sent out whenever
// a tab is closed by the user, thus closing that tab.</pre>
//=================================================================
void closeTab(int index);

//FILE SLOTS//

void displayHelp();

void refresh();

void openGroundTruth();

void createGroundTruth();

//=================================================================
// 	<pre>Clears all the pagetabs from memory and exits the
//	main window.</pre>
//=================================================================
void file_exit();

void writecoordsdisp();

void writecoordsemb();

void writecoordslabel();

//VIEW MENU SLOTS//
//=================================================================
//	<pre>Check box that allows the user the option to
//	view or not to view the file toolbar</pre>
//=================================================================
void view_tool_file();

//================================================================
// 	<pre>Allows the user to enter into zoom mode. In this
//	mode, the cursor appears as a magnifying glass. The user can
//	zoom in and out by scrolling the mouse forward and backward
//	respectively.</pre>
//=================================================================
void zoomMode();

//=================================================================
// 	<pre>Allows the user to enter into rectangular select mode.
//	In this mode, the cursor appears as a cross bar. When the user
//	clicks the mouse a rectangle will start being drawn on the screen.
//	The user can resize the rectangle by moving it right left up or
//	down while holding the right mouse button.</pre>
//=================================================================
void selectMode();

//=================================================================
// <pre>Allows the user to go back to the normal, default
//	mode. The cursor appears as normal and scrolling the mouse will
//	move the scroll bar up or down rather than zooming in or out.
//	The select feature is turned off in this mode.</pre>
//=================================================================
void normalMode();

//=================================================================
// <pre>This function is called whenever the user zooms in
//	by either clicking the mouse or moving the mouse scroll bar forward
//	while in zoom mode.
//	TODO: Implement hot key for zooming in </pre>
//=================================================================
void zoomIn();

//=================================================================
// 	<pre>This function is called whenever the user zooms out
//	by either clicking the mouse or moving the mouse scroll bar back
//	while in zoom mode.
//	TODO: Implement hot key for zooming out </pre>
//=================================================================
void zoomOut();


//WINDOW MENU SLOTS//
//=================================================================
// <pre>Closes the current tab. This function, however, is
// obsolete now since the tabs are set as closable, much like the
// tabs in most web browsers.</pre>
//=================================================================
void closePage();

private:
// Data
bool isImageLoaded;
string ocrProg;
list<pageTab*> tabList;

// Functions
//=================================================================
//	<pre>Initializes the tab layout (central view) to a new
//	QTabWidget.</pre>
//=================================================================
void createViews();

//=================================================================
// 	<pre>Initializes the QAction pointers for this class to
//	new objects.  These actions are used for the menu options and
//	toolbar buttons.  Each action is also connected to its
//	associated function/slot.  They are also enabled/disabled as
//	appropriate (i.e. functionalities not currently implemented are
//	disabled).</pre>
//=================================================================
void createActions();

//=================================================================
// 	<pre>Creates the menu objects and adds them to the
//	window's menu bar.  Each menu's associated actions are
//	inserted into the menu.</pre>
//=================================================================
void createMenus();

//=================================================================
// 	<pre>Adds the file and read toolbars to the window and
//	inserts the actions associated with each into the appropriate
//	toolbar.  These actions appear as buttons in the toolbar
//	(most have icons associated with them)</pre>
//=================================================================
void createToolbars();

//=================================================================
// <pre>Initialize the text in the window's status bar to "Ready".</pre>
//=================================================================
void createStatusBar();

//=================================================================
// 	<pre>Whenever the user zooms in or out this function is
//	called to scale the QLabel appropriately.</pre>
//=================================================================
void scaleImage(double factor);

//=================================================================
// 	<pre>Whenever the user zooms in or out this function is
//	called to adjust the scroll bar appropriately</pre>
//=================================================================
void adjustScrollBar(QScrollBar *scrollBar, double factor);

void openFile(QString path);

void readOnlyPrompt(const std::string& text);
void readOnlyPromptQStr(const QString& text);

bool yesNoPrompt(const std::string& test);
bool yesNoPromptQStr(const QString& text);

bool checkImageOrder();

bool openRectFile(const QString rectFilePath);

bool rectFileEmpty();

//========================
// Menu Objects
//========================
QMenu *file_menu;
QMenu *edit_menu;
QMenu *view_menu;
QMenu *view_toolbar_menu;
QMenu *document_menu;
QMenu *read_menu;
QMenu *read_highlight_menu;
QMenu *window_menu;
QMenu *help_menu;

//========================
// Toolbar Objects
//=========================
QToolBar *file_toolbar;
QToolBar *view_toolbar;
QToolBar *read_toolbar;

// Action Objects
// File Menu
QAction *action_GT_create;
QAction *action_GT_open;
QAction *action_file_exit;
// File Toolbar (with icons)
QAction *action_writecoordsdisp; // displayed expressions
QAction *action_writecoordsemb; // embedded expressions
QAction *action_writecoordslabel; // displayed expression label

// Edit Menu
QAction *action_edit_font;

// Read Menu
QAction *action_read_highlight_word;
QAction *action_read_highlight_sent;
QAction *action_read_highlight_para;
QAction *action_read_start;
QAction *action_read_pause;
QAction *action_read_prev;
QAction *action_read_next;
// Read Toolbar (with icons)
QAction *action_readT_highlight;
QAction *action_readT_start;
QAction *action_readT_pause;
QAction *action_readT_prev;
QAction *action_readT_next;

// View Menu
QAction *action_view_tool_file;
QAction *action_view_tool_read;
QAction *setZoomMode;
QAction *zoomin;
QAction *zoomout;
QAction *setSelectMode;
QAction *setNormalMode;
QAction *action_changeColor; //Change Color (added in by Gia)

// View Toolbar
QAction* action_viewT_zoom;
QAction* action_viewT_select;
QAction* action_viewT_normal;

// Document Menu
QAction *action_doc_img_only;
QAction *action_doc_text_only;
QAction *action_doc_dual_view;
QAction *action_doc_ocr_ocropus;
QAction *action_doc_ocr_tesseract;
QAction *action_doc_ocr_gocr;
QAction *action_doc_ocr_ocrad;

// Window Menu
QAction *action_win_close_page;

// Help Menu
QAction *action_display_help;

//========================
// windows/views
//========================
QTabWidget *tabLayout;

QString mathCoordFilePath;
QString gtDirPath;
QFile mathCoordFile;
QTextStream mathCoordFileStream;

int curIndex;
};

//===========================================================================================================
// <pre><B>MouseEventEater </B>
//  Control OSR's response to certain mouse events.Currently implemented mouse events include:
//  While in Zoom Mode:
//  - The Zoom in and out slots are activated in response to scrolling in or out respectively with
//    the mouse. If a user scrolls in then clicks the mouse the the zoom in slot will be activated. If the user
//    scrolls back then clicks the mouse, the zoom out slots will be activated.
//  While in Select Mode:
//  - If the user has pressed down and held the mouse key inside the QLabel (the QLabel is what this filter is
//    installed for) then a rectangle will be drawn in the area that the user is selecting.</pre>
//=============================================================================================================
class MouseEventEater: public QObject {
  Q_OBJECT
public:
  MouseEventEater(mainWindow* parent);
  mainWindow* myparent;
protected:
  bool eventFilter(QObject* obj, QEvent* event);
};

#endif
