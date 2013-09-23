// OSR - with Linux OCR Capabilities
//
// Author - Derek White
// Edited - Jake Bruce, Gia Ngo, Kelly Kiang
// MyWidget.cpp - definition file for MyWidget object (Main GUI page)
//
// Created: 07 April 2008
// Final Version Date: 01 May 2008 (Spring 2008 Semester): 

#include "mainWindow.h"

mainWindow::mainWindow() :
    highlightdone(false), inZoomMode(false), inSelectMode(false), selecting(
        false), zoomcount(0), scaleFactor(1.0) {
  /*
   mathCoordFile = new QFile("GroundTruth.dat");
   if(!mathCoordFile->open(QIODevice::Append))
   qDebug() << "ERROR: Couldn't open GroundTruth.dat..";
   gtWrite.setDevice(mathCoordFile);*/

  mathCoordFile = 0;

  // now start up the OSR GUI
  this->setWindowTitle(tr("Open Scan and Read (OSR)"));
  createViews();
  createActions();
  createMenus();
  createToolbars();
  createStatusBar();
  setCentralWidget(tabLayout);
  filter = new MouseEventEater(this);
  connect(tabLayout, SIGNAL(currentChanged(int)), this, SLOT(tab_changed()));
}

//SLOTS:
void mainWindow::tab_changed() {
  //find tab in tabList
  list<pageTab*>::iterator iter = tabList.begin();
  if (tabLayout->currentIndex() > -1) {
    while ((*iter)->getIndex() != tabLayout->currentIndex()) {
      if ((*iter)->getIndex() < tabLayout->currentIndex())
        iter++;
      else
        iter--;
    }
    currentTab = *iter;
    //update doc menu selection
    if ((*iter)->getState() == "img") {
      rect = QRect(QPoint(0, 0),
          QPoint(currentTab->scanImage->width(),
              currentTab->scanImage->height()));
      currentTab->image = new QImage(
          (QString) (currentTab->getImagePath().c_str()));
      currentTab->imgView->installEventFilter(filter);
    }
  }
}

void mainWindow::closeTab(int index) {
  tabLayout->setCurrentIndex(index);
  closePage();
}

void mainWindow::file_new() {
  list<pageTab*>::iterator iter = tabList.begin();
  pageTab *tempTab;

  tempTab = new pageTab((int) tabList.size(), ("New" + (int) tabList.size()),
      ("New" + (int) tabList.size()));
  tabList.push_back(tempTab);

  while ((*iter)->getIndex() != ((int) tabList.size() - 1))
    iter++;

  //(*iter)->viewDual();
  tabLayout->addTab((*iter), tr((*iter)->getImageFilename().c_str()));
  tabLayout->setCurrentIndex((*iter)->getIndex());
}

void mainWindow::openGroundTruth() {
  cout << "opening the groundtruth!!!\n";
  QString path = QFileDialog::getOpenFileName(this, "Open file",
      "Text file");
  mathCoordFile = new QFile(path);
  if (!mathCoordFile->open(QIODevice::Append)) {
    qDebug() << "ERROR: Couldn't open the GroundTruth file!";
    exit(EXIT_FAILURE);
  }
  gtWrite.setDevice(mathCoordFile);
  cout << "groundtruth opened successfully!!\n";
}

void mainWindow::file_open() {
  QString path;
  pageTab *tempTab;
  list<pageTab*>::iterator iter = tabList.begin();
  path = QFileDialog::getOpenFileName(this, "Open file",
      "Images or Text (*.png *.pnm *.tif *.txt *.html *.xml *.gif)");
  if (path.contains(".tif", Qt::CaseInsensitive)
      || path.contains(".png", Qt::CaseInsensitive)
      || path.contains(".pnm", Qt::CaseInsensitive)
      || path.contains(".gif", Qt::CaseInsensitive)) {
    path.replace(path.lastIndexOf((QString) (".")),
        path.size() - path.lastIndexOf((QString) (".")), ".png"); //png should be the output name
    QFile file(path);
    if (file.size() < 10500179) { // only try to open it if its less than 10 MB
      //load and show image in new tab
      statusBar()->showMessage(tr("Image loaded. Ready to OCR."));
      isImageLoaded = true;
      tempTab = new pageTab((int) tabList.size(), (string) path.toStdString(),
          "");
      tabList.push_back(tempTab);

      //find newest tab in list
      while ((*iter)->getIndex() != ((int) tabList.size() - 1))
        iter++;
      currentTab = *iter;
      //update tabs and menus
      tabLayout->addTab((tempTab), tr(tempTab->getImageFilename().c_str()));
      tabLayout->setCurrentIndex(tempTab->getIndex());
    } else { // if its greater than 10 MB show a message indicating the image is too large to be opened
      QMessageBox errorBox(tr("File Open Error"),
          tr(
              "File Open: Files cannot be larger than 10 MB. Try a smaller file, or \
try compressing this one.\n"),
          QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton,
          QMessageBox::NoButton, this);
      errorBox.exec();
    }
  } else if (path.contains(".txt", Qt::CaseInsensitive)
      || path.contains(".html", Qt::CaseInsensitive)) {
    tempTab = new pageTab((int) tabList.size(), (string) path.toStdString(),
        "");
    tabList.push_back(tempTab);
    //load and show text in new tab
    statusBar()->showMessage(tr("Text loaded."));
    tempTab = new pageTab((int) tabList.size(), "",
        (string) path.toStdString());
    //tempTab->viewText();
    tabList.push_back(tempTab);

    //find newest tab in list
    while ((*iter)->getIndex() != ((int) tabList.size() - 1))
      iter++;

    //update tabs and menus
    //(*iter)->viewText();
    tabLayout->addTab((*iter), tr((*iter)->getTextFilename().c_str()));
    tabLayout->setCurrentIndex((*iter)->getIndex());
  }
}

void mainWindow::import_image() {
  file_open();
}

void mainWindow::writecoordsdisp() {
  if (!mathCoordFile) {
    qDebug() << "ERROR: Please select a GroundTruth file to write to!!";
    exit(EXIT_FAILURE);
  }
  qreal resizefactor = (qreal) currentTab->resizeFactor;
  scaledRect = QRect(QPoint(rect.topLeft() / resizefactor),
      QPoint(rect.bottomRight() / resizefactor));
  int tl_x = scaledRect.topLeft().x();
  int tl_y = scaledRect.topLeft().y();
  int br_x = scaledRect.bottomRight().x();
  int br_y = scaledRect.bottomRight().y();
  gtWrite << currentTab->getImageFilename().c_str() << " " << "displayed" << " "
      << tl_x << " " << tl_y << " " << br_x << " " << br_y << "\n";
  gtWrite.flush();
  qDebug() << currentTab->getImageFilename().c_str() << " " << "displayed"
      << " " << tl_x << " " << tl_y << " " << br_x << " " << br_y;
}

void mainWindow::writecoordsemb() {
  if (!mathCoordFile) {
    qDebug() << "ERROR: Please select a GroundTruth file to write to!!";
    exit(EXIT_FAILURE);
  }
  qreal resizefactor = (qreal) currentTab->resizeFactor;
  scaledRect = QRect(QPoint(rect.topLeft() / resizefactor),
      QPoint(rect.bottomRight() / resizefactor));
  int tl_x = scaledRect.topLeft().x();
  int tl_y = scaledRect.topLeft().y();
  int br_x = scaledRect.bottomRight().x();
  int br_y = scaledRect.bottomRight().y();
  gtWrite << currentTab->getImageFilename().c_str() << " " << "embedded" << " "
      << tl_x << " " << tl_y << " " << br_x << " " << br_y << "\n";
  gtWrite.flush();
  qDebug() << currentTab->getImageFilename().c_str() << " " << "embedded" << " "
      << tl_x << " " << tl_y << " " << br_x << " " << br_y;
}

void mainWindow::writecoordslabel() {
  if (!mathCoordFile) {
    qDebug() << "ERROR: Please select a GroundTruth file to write to!!";
    exit(EXIT_FAILURE);
  }
  qreal resizefactor = (qreal) currentTab->resizeFactor;
  scaledRect = QRect(QPoint(rect.topLeft() / resizefactor),
      QPoint(rect.bottomRight() / resizefactor));
  int tl_x = scaledRect.topLeft().x();
  int tl_y = scaledRect.topLeft().y();
  int br_x = scaledRect.bottomRight().x();
  int br_y = scaledRect.bottomRight().y();
  gtWrite << currentTab->getImageFilename().c_str() << " " << "label" << " "
      << tl_x << " " << tl_y << " " << br_x << " " << br_y << "\n";
  gtWrite.flush();
  qDebug() << currentTab->getImageFilename().c_str() << " " << "label" << " "
      << tl_x << " " << tl_y << " " << br_x << " " << br_y;
}

void mainWindow::file_exit() {
  //clear memory
  list<pageTab*>::iterator iter;
  for (iter = tabList.begin(); iter != tabList.end(); iter++)
    delete (*iter);

  tabList.clear();
  close();
}

void mainWindow::view_tool_file() {
  if (action_view_tool_file->isChecked() == true)
    file_toolbar->show();
  else
    file_toolbar->hide();
}

void mainWindow::zoomMode() {
  qDebug() << "zoom mode!";
  QPixmap img(QString(":/icons/zoom.png"));
  QCursor zoomcursor(img);
  currentTab->imgView->setCursor(zoomcursor);
  inZoomMode = true;
  inSelectMode = false;
}

void mainWindow::zoomIn() {
  if (zoomcount < 6) {
    scaleImage(1.25);
    ++zoomcount;
  }
}

void mainWindow::zoomOut() {
  if (zoomcount > -3) {
    scaleImage(0.8);
    --zoomcount;
  }
}

void mainWindow::selectMode() {
  currentTab->imgView->setCursor(Qt::CrossCursor);
  inZoomMode = false;
  inSelectMode = true;
}

void mainWindow::normalMode() {
  currentTab->imgView->unsetCursor();
  inZoomMode = false;
  inSelectMode = false;
}

void mainWindow::doc_img_only() {
  list<pageTab*>::iterator iter = tabList.begin();
  while (((*iter)->getIndex() != tabLayout->currentIndex())
      && (iter != tabList.end()))
    iter++;

  (*iter)->viewImage();
  tabLayout->setTabText(tabLayout->currentIndex(),
      tr(((*iter)->getImageFilename()).c_str()));
}

void mainWindow::closePage() {
  qDebug() << "in closePage!";
  list<pageTab*>::iterator iter1;
  list<pageTab*>::iterator iter2;

  //find tab to close
  iter1 = tabList.begin();
  while ((*iter1)->getIndex() != tabLayout->currentIndex())
    iter1++;

  //decrement index of subsequent tabs
  iter2 = iter1;
  for (iter2 = iter1; iter2 != tabList.end(); iter2++)
    (*iter2)->setIndex((*iter2)->getIndex() - 1);

  //remove tab
  tabLayout->removeTab(tabLayout->currentIndex());
  tabList.remove(*iter1);
}

// GUI CREATION FUNCTIONS:
void mainWindow::createViews() {
  tabLayout = new QTabWidget;
  tabLayout->setTabsClosable(true);
  tabLayout->setMovable(true);
  connect(tabLayout, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
}

void mainWindow::createActions() {
  //File Menu
  action_file_new = new QAction(tr("New"), this);
  action_file_new->setToolTip(tr("Create a new tab."));
  connect(action_file_new, SIGNAL(triggered()), this, SLOT(file_new()));
  action_file_new->setEnabled(false);

  action_file_open = new QAction(tr("Open"), this);
  action_file_open->setToolTip(tr("Open an image or text file in a new tab."));
  connect(action_file_open, SIGNAL(triggered()), this, SLOT(file_open()));

  action_GT_open = new QAction(tr("Open GroundTruth"), this);
  action_GT_open->setToolTip(tr("Open the GroundTruth file we'll be writing to\n"));
  connect(action_GT_open, SIGNAL(triggered()), this, SLOT(openGroundTruth()));

  action_file_import_img = new QAction(tr("Import image"), this);
  action_file_import_img->setToolTip(
      tr("Import an image into the current tab."));
  connect(action_file_import_img, SIGNAL(triggered()), this,
      SLOT(import_image()));

  action_file_exit = new QAction(tr("Exit"), this);
  action_file_exit->setToolTip(tr("Exit OSR."));
  connect(action_file_exit, SIGNAL(triggered()), this, SLOT(file_exit()));

  //File Toolbar
  action_fileT_open = new QAction(QIcon(":/icons/open_icon.png"), tr("Open"),
      this);
  action_fileT_open->setToolTip(action_file_open->toolTip());
  connect(action_fileT_open, SIGNAL(triggered()), this, SLOT(file_open()));

  action_writecoordsdisp = new QAction(QIcon(":/icons/ptmd.png"),
      tr("write disp coordinates"), this);
  action_writecoordsdisp->setShortcut(tr("s"));
  connect(action_writecoordsdisp, SIGNAL(triggered()), this,
      SLOT(writecoordsdisp()));

  action_writecoordsemb = new QAction(QIcon(":/icons/ptm.png"),
      tr("write emb coordinates"), this);
  action_writecoordsemb->setShortcut(tr("d"));
  connect(action_writecoordsemb, SIGNAL(triggered()), this,
      SLOT(writecoordsemb()));

  action_writecoordslabel = new QAction(QIcon(":/icons/label.png"),
      tr("write label coordinates"), this);
  action_writecoordslabel->setShortcut(tr("f"));
  connect(action_writecoordslabel, SIGNAL(triggered()), this,
      SLOT(writecoordslabel()));

  //View Menu
  action_view_tool_file = new QAction(tr("File Toolbar"), this);
  action_view_tool_file->setCheckable(true);
  action_view_tool_file->setChecked(true);
  connect(action_view_tool_file, SIGNAL(triggered()), this,
      SLOT(view_tool_file()));

  setZoomMode = new QAction(tr("Zoom Mode"), this);
  setZoomMode->setShortcut(tr("Ctrl+Z"));
  connect(setZoomMode, SIGNAL(triggered()), this, SLOT(zoomMode()));

  zoomin = new QAction(tr("Zoom In"), this);
  zoomin->setShortcut(tr("Ctrl+="));
  connect(zoomin, SIGNAL(triggered()), this, SLOT(zoomIn()));

  zoomout = new QAction(tr("Zoom Out"), this);
  zoomout->setShortcut(tr("Ctrl+-"));
  connect(zoomout, SIGNAL(triggered()), this, SLOT(zoomOut()));

  setSelectMode = new QAction(tr("Select Mode"), this);
  setSelectMode->setShortcut(tr("Ctrl+X"));
  connect(setSelectMode, SIGNAL(triggered()), this, SLOT(selectMode()));

  setNormalMode = new QAction(tr("Normal Mode"), this);
  setNormalMode->setShortcut(tr("Ctrl+C"));
  connect(setNormalMode, SIGNAL(triggered()), this, SLOT(normalMode()));

  // View Toolbar
  action_viewT_zoom = new QAction(QIcon(":/icons/zoom_icon.png"),
      tr("Zoom Mode (CTRL+Z)"), this);
  connect(action_viewT_zoom, SIGNAL(triggered()), this, SLOT(zoomMode()));

  action_viewT_select =
      new QAction(QIcon(":/icons/select.png"),
          tr(
              "Select Mode (CTRL+X):\nSelect a math equation with this tool.\nThen click Image2Math to the left to run\nOCR on the math equation."),
          this);
  connect(action_viewT_select, SIGNAL(triggered()), this, SLOT(selectMode()));

  //Window Menu
  action_win_close_page = new QAction(tr("Close Page"), this);
  connect(action_win_close_page, SIGNAL(triggered()), this, SLOT(closePage()));

}

void mainWindow::createMenus() {
  //File Menu
  file_menu = menuBar()->addMenu(tr("&File"));
  file_menu->addAction(action_file_new);
  file_menu->addAction(action_file_open);
  file_menu->addAction(action_GT_open);
  file_menu->addSeparator();
  file_menu->addAction(action_file_import_img);
  file_menu->addSeparator();
  file_menu->addAction(action_file_exit);

  menuBar()->addSeparator();

  //View menu
  view_toolbar_menu = new QMenu(tr("Toolbars"));
  view_toolbar_menu->addAction(action_view_tool_file);
  //view_toolbar_menu->addAction(file_toolbar->toggleViewAction());

  view_menu = menuBar()->addMenu(tr("&View"));
  view_menu->addMenu(view_toolbar_menu);
  view_menu->addAction(setZoomMode);
  view_menu->addAction(zoomin);
  view_menu->addAction(zoomout);
  view_menu->addAction(setSelectMode);
  view_menu->addAction(setNormalMode);

  menuBar()->addSeparator();

  //Window Menu
  window_menu = menuBar()->addMenu(tr("&Window"));
  window_menu->addAction(action_win_close_page);
}

void mainWindow::createToolbars() {
  QSize iconSize(50, 30);
  //File Toolbar
  file_toolbar = addToolBar(tr("File Toolbar"));
  file_toolbar->addAction(action_fileT_open);
  file_toolbar->addAction(action_writecoordsdisp);
  file_toolbar->addAction(action_writecoordsemb);
  file_toolbar->addAction(action_writecoordslabel);
  file_toolbar->setIconSize(iconSize);

  //View Toolbar
  view_toolbar = addToolBar(tr("View Toolbar"));
  view_toolbar->addAction(action_viewT_zoom);
  view_toolbar->addAction(action_viewT_select);
  view_toolbar->setIconSize(iconSize);

}

void mainWindow::createStatusBar() {
  statusBar()->showMessage(tr("Ready."));
}

void mainWindow::scaleImage(double factor) {
  scaleFactor *= factor;
  currentTab->imgView->resize(
      scaleFactor * currentTab->imgView->pixmap()->size());
  adjustScrollBar(currentTab->scrollArea->horizontalScrollBar(), factor);
  adjustScrollBar(currentTab->scrollArea->verticalScrollBar(), factor);
}

void mainWindow::adjustScrollBar(QScrollBar *scrollBar, double factor) {
  scrollBar->setValue(
      int(
          factor * scrollBar->value()
              + ((factor - 1) * scrollBar->pageStep() / 2)));
}

void mainWindow::paintEvent(QPaintEvent *) {
  if (selecting) {
    if (currentTab->imgView->pixmap() != 0) {
      currentTab->imgView->setPixmap(*currentTab->scanImage);
      QPixmap* pix = (QPixmap*) (currentTab->scanImage);
      QImage img = pix->toImage();
      QPainter painter;
      painter.begin(&img);
      painter.setPen(QPen(QColor(0, 0, 0)));
      ;
      painter.drawRect(rect);
      currentTab->imgView->setPixmap(pix->fromImage(img));
      painter.end();
    }
  }
}

MouseEventEater::MouseEventEater(mainWindow* parent) :
    myparent(parent) {
}

bool MouseEventEater::eventFilter(QObject* obj, QEvent* event) {
  if (((event->type() == QEvent::MouseButtonPress)
      || (event->type() == QEvent::Wheel)) && (myparent->inZoomMode)) {
    static bool lastwheel = true; // true if last wheel turn was positive, false otherwise
    if (event->type() == QEvent::Wheel) { // if wheel turned
      QWheelEvent *wheelevent = static_cast<QWheelEvent*>(event);
      if (wheelevent->delta() > 0) {
        myparent->zoomIn();
        lastwheel = true;
      } else {
        myparent->zoomOut();
        lastwheel = false;
      }
    } else { // if mouse clicked
      if (lastwheel) // make the clicks follow last wheel direction
        myparent->zoomIn();
      else
        myparent->zoomOut();
    }
    return true;
  } else if ((event->type() == QEvent::MouseButtonPress)
      && (myparent->inSelectMode)) {
    QMouseEvent *mouseevent = static_cast<QMouseEvent*>(event);
    QPoint pos_calc = mouseevent->pos();
    pos_calc.rx() /= pow((double) 1.25, myparent->zoomcount);
    pos_calc.ry() /= pow((double) 1.25, myparent->zoomcount);
    myparent->initRect = pos_calc;
    QPixmap original;
    myparent->currentTab->imgView->setPixmap(
        *(myparent->currentTab->scanImage));
    myparent->selecting = true;
    return true;
  } else if ((event->type() == QEvent::MouseMove) && (myparent->inSelectMode)
      && (myparent->selecting)) {
    QMouseEvent *mouseevent = static_cast<QMouseEvent*>(event);
    QPoint pos_calc = mouseevent->pos();
    pos_calc.rx() /= pow((double) 1.25, myparent->zoomcount);
    pos_calc.ry() /= pow((double) 1.25, myparent->zoomcount);
    myparent->endRect = pos_calc;

    if ((myparent->initRect.x() < myparent->endRect.x())
        && (myparent->initRect.y() < myparent->endRect.y())) { //left-right downwards
      myparent->rect.setTopLeft(myparent->initRect);
      myparent->rect.setBottomRight(myparent->endRect);
    } else if ((myparent->initRect.x() < myparent->endRect.x())
        && (myparent->initRect.y() > myparent->endRect.y())) { //left-right upwards
      myparent->rect.setBottomLeft(myparent->initRect);
      myparent->rect.setTopRight(myparent->endRect);
    } else if ((myparent->initRect.x() > myparent->endRect.x())
        && (myparent->initRect.y() < myparent->endRect.y())) { //right-left downwards
      myparent->rect.setTopRight(myparent->initRect);
      myparent->rect.setBottomLeft(myparent->endRect);
    } else if ((myparent->initRect.x() > myparent->endRect.x())
        && (myparent->initRect.y() > myparent->endRect.y())) { //right-left upwards
      myparent->rect.setBottomRight(myparent->initRect);
      myparent->rect.setTopLeft(myparent->endRect);
    }
    myparent->update();
    return true;
  } else
    return QObject::eventFilter(obj, event);
}

