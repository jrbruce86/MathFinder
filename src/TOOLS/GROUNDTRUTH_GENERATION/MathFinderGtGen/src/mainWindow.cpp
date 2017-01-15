// OSR - with Linux OCR Capabilities
//
// Author - Derek White
// Edited - Jake Bruce, Gia Ngo, Kelly Kiang
// MyWidget.cpp - definition file for MyWidget object (Main GUI page)
//
// Created: 07 April 2008
// Final Version Date: 01 May 2008 (Spring 2008 Semester): 
// Updated Dec 2016 by Jake Bruce for different purpose

#include "mainWindow.h"

mainWindow::mainWindow() :
    highlightdone(false), inZoomMode(false), inSelectMode(false),
    selecting(false), zoomcount(0), scaleFactor(1.0), mathCoordFilePath(QString("")),
    curIndex(0) {

  // now start up the OSR GUI
  this->setWindowTitle(tr("Math Finder Groundtruth Generator"));
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
      currentTab->imgView->installEventFilter(filter);
    }
  }
  refresh();
}

void mainWindow::closeTab(int index) {
  tabLayout->setCurrentIndex(index);
  closePage();
}

void mainWindow::displayHelp() {
  readOnlyPromptQStr(QString("This user interface was designed for creating a groundtruth used for training/evaluating a Math Finder.\n\n")
      + QString("A groundtruth consists of a directory containing one or more images and a *.rect file.\n\n")
      + QString("The *.rect file contains the bounding box coordinates of the math regions in each of the images.\n\n")
      + QString("Types of math regions include 'displayed', 'embedded', or 'label'.\n\n")
      + QString("These types represent math expressions on their own isolated row (displayed), embedded in paragraph with other text (embedded), or labels to math expressions (label).\n\n")
      + QString("To create a groundtruth, select the 'Create groundtruth' option from the file menu. You will be prompted to create a new folder that will contain the groundtruth.\n\n")
      + QString("You will then be prompted to select one or more images to build the groundtruth from.\n\n")
      + QString("Once the images are selected, they will be renamed as 0,1,2, etc. and copied to the groundtruth directory.\n\n")
      + QString("These images will each be opened in the gui along with a .rect file which is created.\n\n")
      + QString("To generate the groundtruth, select the math regions in the opened images and press the 'displayed', 'embedded', or 'label' toolbar buttons.\n\n")
      + QString("The *.rect file will then be updated to contain the selected coordinates for the images.\n\n")
      + QString("The coordinates in the .rect file are visually displayed by this gui such that displayed boxes are red, embedded are blue, and label are green.\n\n")
      + QString("To erase any bounding box will require manually modifying the .rect file in a separate editor.\n\n"));
}

void mainWindow::createGroundTruth() {

  // Check that groundtruth wasn't already opened
  if(!mathCoordFilePath.isEmpty()) {
    readOnlyPrompt("A groundtruth directory was already opened. Restart the application to open/create a different one.");
    return;
  }

  // Create a fresh directory
  QDir tmpDir;
  {
    readOnlyPrompt("Please select or create an empty directory. All groundtruth contents will be placed in that directory.");
    QString gtDirPath = QFileDialog::getExistingDirectory(
        this,
        "Create a new directory or select an empty one which will contain the groundtruth contents.",
        "");
    tmpDir = QDir(gtDirPath);
    int fileCount = tmpDir.count() - 2; // subtract for . and ..
    if(fileCount != 0) {
      readOnlyPrompt("Either no directory was selected or the selected directory already has one or more files. Please try again with a new directory.");
      return;
    }
  }
  gtDirPath = tmpDir.absolutePath().append(QDir::separator());

  // Create and open the .rect file
  const QString rectFileName = gtDirPath + QString("groundtruth.rect");
  mathCoordFilePath = rectFileName;
  if (!QFile(mathCoordFilePath).open(QIODevice::Append)) {
    readOnlyPrompt("ERROR: Couldn't create the GroundTruth file!");
    return;
  }

  // Select the images for the groundtruth
  readOnlyPrompt("Select one or more images from which to create the groundtruth.");
  QStringList imagePaths = QFileDialog::getOpenFileNames(
        this,
        "Select one or more images from which to create the groundtruth.",
        "",
        tr("*.png (*.png);; *.tif (*.tif);; *.jpg (*.jpg *.jpeg);; all (*)"));

  // Copy the selected images into the groundtruth dir while renaming them
  for(int i = 0; i < imagePaths.size(); ++i) {
    QFile imFile(imagePaths[i]);
    if(QFileInfo(imFile).fileName().count(".") != 1) {
      readOnlyPromptQStr(QString("The image at ") + imagePaths[i] + QString(" has an unexpected file name. Should only have one '.' character to represent the extension."));
      return;
    }
    imagePaths[i] = gtDirPath + QString::number(i) + QString(".") + QFileInfo(imFile).fileName().split(".")[1];
    imFile.copy(imagePaths[i]); // copy to the new location
  }

  // Open the images
  for(int i = 0; i < imagePaths.size(); ++i) {
    openFile(imagePaths[i]);
  }

  // Open the .rect file in both the text window and for writing
  if(!openRectFile(rectFileName)) {
    return;
  }

  // Indicate success
  readOnlyPromptQStr("Groundtruth successfully created at " + gtDirPath);
}

bool mainWindow::openRectFile(const QString rectFilePath) {
  mathCoordFilePath = rectFilePath;
  openFile(mathCoordFilePath);
  mathCoordFile.setFileName(mathCoordFilePath);
  if(!mathCoordFile.open(QIODevice::Append)) {
    readOnlyPrompt("Error: Could not open file at " + mathCoordFilePath.toStdString());
    return false;
  }
  mathCoordFileStream.setDevice(&mathCoordFile);
  {
    // Get the current file index by reading in the end of the file
    QFile file(mathCoordFilePath);
    file.open(QIODevice::ReadOnly);
    QTextStream fileStream;
    fileStream.setDevice(&file);
    QString line = "";
    while(!fileStream.atEnd()) {
      line = fileStream.readLine();
    }
    curIndex = line.split(" ")[0].split(".")[0].toInt();
  }
  return true;
}

void mainWindow::openGroundTruth() {

  // Check that groundtruth wasn't already opened
  if(!mathCoordFilePath.isEmpty()) {
    readOnlyPrompt("A groundtruth directory was already opened. Restart the application to open/create a different one.");
    return;
  }

  // Print instructions
  readOnlyPrompt("Select an existing groundtruth directory to open. This directory should consist of a .rect file and one or more image files.");

  // Grab the directory
  QDir gtDir = QDir(QFileDialog::getExistingDirectory(
      this,
      "Open a groundtruth directory previously created by this application"));
  gtDirPath = gtDir.absolutePath().append(QDir::separator());
  if(gtDir == QDir::current()) {
    return;
  }

  // Grab the directory contents
  QStringList filePaths = gtDir.entryList();
  filePaths.removeAll(QString("."));
  filePaths.removeAll(QString(".."));
  for(int i = 0; i < filePaths.size(); ++i) {
    filePaths[i] = gtDirPath + filePaths[i];
  }

  // Verify the contents are valid (should contain one .rect file, one or more images, and could have one or more subdirs)
  // Also grab the .rect and image file paths
  QString rectFilePathTmp;
  QStringList imFilePathsTmp;
  {
    bool foundRectFile = false;
    for(int i = 0; i < filePaths.size(); ++i) {
      if(filePaths[i].endsWith(QString(".rect"))) {
        if(foundRectFile) {
          readOnlyPrompt("Error: The given directory has more than one .rect file. It should only have one. Please fix this groundtruth dir or create a new one.");
          return;
        }
        foundRectFile = true;
        rectFilePathTmp = filePaths[i];
        continue;
      }
      if(QFileInfo(filePaths[i]).isDir()) {
        continue; // don't care about subdirs
      }
      QImage im; // should otherwise be an image, make sure it can be read in
      if(!im.load(filePaths[i])) {
        readOnlyPromptQStr("Error: The given directory has an unexpected file at " + filePaths[i] + ". The directory should consist of a .rect file and one or more valid images.");
        return;
      }
      imFilePathsTmp.append(filePaths[i]);
    }
    if(!foundRectFile) {
      readOnlyPrompt("Error: The given directory has no .rect file. A groundtruth dir needs to have one or more images and a .rect file.");
      return;
    }
  }
  const QString rectFilePath = rectFilePathTmp;
  const QStringList imFilePaths = imFilePathsTmp;

  // Open the image files
  for(int i = 0; i < imFilePaths.size(); ++i) {
    openFile(imFilePaths[i]);
  }

  // Open the .rect file
  openRectFile(rectFilePath);

  // Indicate success
  readOnlyPrompt("Successfully opened a groundtruth directory");
}

void mainWindow::refresh() {

  // Check that there is something to refresh
  if(mathCoordFilePath.isEmpty()) {
    return;
  }

  QFile file(mathCoordFilePath);
  if(!file.open(QIODevice::ReadOnly)) {
    readOnlyPromptQStr(QString("Error: Cannot open the .rect file. Is the directory containing ")
        + mathCoordFilePath + QString(" corrupted? The directory may need to be fixed somehow manually."));
    return;
  }
  QTextStream fileStream;
  fileStream.setDevice(&file);

  // Refresh the groundtruth file display if it's opened
  if(currentTab->isRectFile()) {
    currentTab->textEdit->setText(fileStream.readAll());
  }

  // Refresh the currently opened image display
  else {
    QPixmap* image = currentTab->scanImage;
    //*image = image->convertToFormat(QImage::Format_RGB32);
    while(!fileStream.atEnd()) {
      // Read the line and start parsing
      QString line = fileStream.readLine().simplified();
      QStringList splitLine = line.split(" ");
      if(splitLine.size() != 6) continue; // just ignore ones we know are invalid

      // Move on if we're not there yet, or break if we're done
      const int imIndex = splitLine[0].split(".")[0].toInt();
      const int curTabIndex =
          QString(currentTab->getImageFilename().c_str()).split(".")[0].toInt();
      if(imIndex < curTabIndex) {
        continue;
      }
      if(imIndex > curTabIndex) {
        break;
      }
      // Grab the entry info
      const QString exptype = splitLine[1];
      qreal resizefactor = (qreal) currentTab->resizeFactor;
      const int left = (splitLine[2].toInt() /*/ pow((double) 1.25, zoomcount)*/) * resizefactor;
      const int top = (splitLine[3].toInt() /*/ pow((double) 1.25, zoomcount)*/) * resizefactor;
      const int right = (splitLine[4].toInt() /*/ pow((double) 1.25, zoomcount)*/) * resizefactor;
      const int bottom = (splitLine[5].toInt() /*/ pow((double) 1.25, zoomcount)*/) * resizefactor;

      // draw the right colored rectangle on the image (red for displayed,
      // green for expression labels, blue for embedded)
      const int thickness = 4;
      QPainter paint(image);
      QPen pen;
      pen.setWidth(thickness);
      if(exptype[0] == 'd')
        pen.setColor(QColor(255,0,0));
      else if(exptype[0] == 'e')
        pen.setColor(QColor(0,0,255));
      else
        pen.setColor(QColor(0,255,0));
      paint.setPen(pen);
      paint.drawLine(left, top, right, top);
      paint.drawLine(right, top, right, bottom);
      paint.drawLine(right, bottom, left, bottom);
      paint.drawLine(left, bottom, left, top);
    }
    currentTab->imgView->setPixmap(
        *(currentTab->scanImage));
  }
}

void mainWindow::openFile(QString path) {
  pageTab *tempTab;
  list<pageTab*>::iterator iter = tabList.begin();
  if (path.contains(".tif", Qt::CaseInsensitive)
      || path.contains(".png", Qt::CaseInsensitive)
      || path.contains(".pnm", Qt::CaseInsensitive)
      || path.contains(".gif", Qt::CaseInsensitive)) {
    path.replace(path.lastIndexOf((QString) (".")),
        path.size() - path.lastIndexOf((QString) (".")), ".png"); //png should be the output name
    QFile file(path);
    if (file.size() < 10500179) { // only try to open it if its less than 10 MB
      //load and show image in new tab
      statusBar()->showMessage(tr("Image loaded. Ready to generate groundtruth."));
      isImageLoaded = true;
      tempTab = new pageTab((int) tabList.size(), (string) path.toStdString(),
          "");
      tabList.push_back(tempTab);

      //find newest tab in list
      while ((*iter)->getIndex() != ((int) tabList.size() - 1))
        ++iter;
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
  } else {
    tempTab = new pageTab((int) tabList.size(), "", (string) path.toStdString());
    tabList.push_back(tempTab);
    //load and show text in new tab
    statusBar()->showMessage(tr("Text loaded."));

    //find newest tab in list
    while ((*iter)->getIndex() != ((int) tabList.size() - 1))
      iter++;

    //update tabs and menus
    //(*iter)->viewText();
    tabLayout->addTab((*iter), tr((*iter)->getTextFilename().c_str()));
    tabLayout->setCurrentIndex((*iter)->getIndex());
  }
}

void mainWindow::writecoordsdisp() {
  if (mathCoordFilePath.isEmpty()) {
    readOnlyPrompt("ERROR: Please select a GroundTruth file to write to!!");
    return;
  }
  if(!checkImageOrder()) {
    return;
  }
  qreal resizefactor = (qreal) currentTab->resizeFactor;
  scaledRect = QRect(QPoint(rect.topLeft() / resizefactor),
      QPoint(rect.bottomRight() / resizefactor));
  int tl_x = scaledRect.topLeft().x();
  int tl_y = scaledRect.topLeft().y();
  int br_x = scaledRect.bottomRight().x();
  int br_y = scaledRect.bottomRight().y();
  mathCoordFileStream << currentTab->getImageFilename().c_str() << " " << "displayed" << " "
      << tl_x << " " << tl_y << " " << br_x << " " << br_y << "\n";
  mathCoordFileStream.flush();

  refresh();
}

bool mainWindow::rectFileEmpty() {
  QFile file(mathCoordFilePath);
  file.open( QIODevice::WriteOnly|QIODevice::Append );
  if (file.pos() == 0) {
    return true;
  } else {
    return false;
  }
}

bool mainWindow::checkImageOrder() {

  // Grab the current tab's image index
  const int newIndex = QFileInfo(QFile(currentTab->getImageFilename().c_str())).fileName().split(".")[0].toInt();

  // check for image file being skipped, make sure its ok to skip (things should be in sequential order)
  // if nothings been written yet (rect file is empty) then ask if ok to skip the first file
  if(newIndex > (curIndex + 1) || (newIndex > curIndex && rectFileEmpty())) {
    if(!yesNoPromptQStr(QString("The groundtruth data for each image needs to be processed sequentially in the order that the images are named (i.e., 0.png, 1.png, 2.png, etc.). You have skipped from ")
        + QString("image ") + QString::number(curIndex) + QString(" to ") + QString::number(newIndex) + QString(". Once you start adding data for this new image you won't be able to go back to the ones ")
        + QString("in between ") + QString::number(curIndex) + QString(" and ") + QString::number(newIndex) + QString(". If you are sure there is no math in the images in between select 'yes' to continue. ")
        + QString("Otherwise select 'no'."))) {
      return false;
    }
  }

  // otherwise we're ok as long as we're on either the image with the current or next index from the last one thats been written
  else if(!(curIndex == newIndex || (newIndex == (curIndex + 1)))) {
    readOnlyPromptQStr(QString("This application does not support adding groundtruth data to previous images. All groundtruth data must be added to the images in sequential order based on name ")
        + QString("(i.e., to file 0.png, 1.png, 2.png, etc.)). Please either selected a file greater than " + QString::number(curIndex) + " or manually correct the ordering in the files."));
    return false;
  }

  // update the index
  curIndex = newIndex;

  // success
  return true;
}

void mainWindow::writecoordsemb() {
  if (mathCoordFilePath.isEmpty()) {
    readOnlyPrompt("ERROR: Please select a GroundTruth file to write to!!");
    return;
  }
  if(!checkImageOrder()) {
    return;
  }
  qreal resizefactor = (qreal) currentTab->resizeFactor;
  scaledRect = QRect(QPoint(rect.topLeft() / resizefactor),
      QPoint(rect.bottomRight() / resizefactor));
  int tl_x = scaledRect.topLeft().x();
  int tl_y = scaledRect.topLeft().y();
  int br_x = scaledRect.bottomRight().x();
  int br_y = scaledRect.bottomRight().y();
  mathCoordFileStream << currentTab->getImageFilename().c_str() << " " << "embedded" << " "
      << tl_x << " " << tl_y << " " << br_x << " " << br_y << "\n";
  mathCoordFileStream.flush();

  refresh();
}

void mainWindow::writecoordslabel() {
  if (mathCoordFilePath.isEmpty()) {
    readOnlyPrompt("ERROR: Please select a GroundTruth file to write to!!");
    return;
  }
  if(!checkImageOrder()) {
    return;
  }
  qreal resizefactor = (qreal) currentTab->resizeFactor;
  scaledRect = QRect(QPoint(rect.topLeft() / resizefactor),
      QPoint(rect.bottomRight() / resizefactor));
  int tl_x = scaledRect.topLeft().x();
  int tl_y = scaledRect.topLeft().y();
  int br_x = scaledRect.bottomRight().x();
  int br_y = scaledRect.bottomRight().y();
  mathCoordFileStream << currentTab->getImageFilename().c_str() << " " << "label" << " "
      << tl_x << " " << tl_y << " " << br_x << " " << br_y << "\n";
  mathCoordFileStream.flush();

  refresh();
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

void mainWindow::closePage() {
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
  tabLayout->setTabsClosable(false);
  tabLayout->setMovable(true);
  connect(tabLayout, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
}

void mainWindow::createActions() {
  //File Menu
  action_GT_create = new QAction(tr("Create GroundTruth"), this);
  action_GT_create->setToolTip(tr("Create the GroundTruth file we'll be writing to\n"));
  connect(action_GT_create, SIGNAL(triggered()), this, SLOT(createGroundTruth()));

  action_GT_open = new QAction(tr("Open GroundTruth"), this);
  action_GT_open->setToolTip(tr("Open the GroundTruth file we'll be writing to\n"));
  connect(action_GT_open, SIGNAL(triggered()), this, SLOT(openGroundTruth()));

  action_file_exit = new QAction(tr("Exit"), this);
  action_file_exit->setToolTip(tr("Exit OSR."));
  connect(action_file_exit, SIGNAL(triggered()), this, SLOT(file_exit()));

  //File Toolbar
  action_writecoordsdisp = new QAction(QIcon(":/icons/ptmd.png"),
      tr("write disp coordinates (shortcut 'd')"), this);
  action_writecoordsdisp->setShortcut(tr("d"));
  connect(action_writecoordsdisp, SIGNAL(triggered()), this,
      SLOT(writecoordsdisp()));

  action_writecoordsemb = new QAction(QIcon(":/icons/ptm.png"),
      tr("write emb coordinates (shortcut 'e')"), this);
  action_writecoordsemb->setShortcut(tr("e"));
  connect(action_writecoordsemb, SIGNAL(triggered()), this,
      SLOT(writecoordsemb()));

  action_writecoordslabel = new QAction(QIcon(":/icons/label.png"),
      tr("write label coordinates (shortcut 'l')"), this);
  action_writecoordslabel->setShortcut(tr("l"));
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

  action_viewT_select = new QAction(QIcon(":/icons/select.png"),
          tr("Select Mode (CTRL+X)"),
          this);
  connect(action_viewT_select, SIGNAL(triggered()), this, SLOT(selectMode()));

  //Window Menu
  action_win_close_page = new QAction(tr("Close Page"), this);
  connect(action_win_close_page, SIGNAL(triggered()), this, SLOT(closePage()));

  // Help menu
  action_display_help = new QAction(tr("Display Help"), this);
  connect(action_display_help, SIGNAL(triggered()), this, SLOT(displayHelp()));
}

void mainWindow::createMenus() {
  //File Menu
  file_menu = menuBar()->addMenu(tr("&File"));
  file_menu->addAction(action_GT_create);
  file_menu->addAction(action_GT_open);
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

  // Help menu
  help_menu = menuBar()->addMenu(tr("&Help"));
  help_menu->addAction(action_display_help);
}

void mainWindow::createToolbars() {
  QSize iconSize(50, 30);
  //File Toolbar
  file_toolbar = addToolBar(tr("File Toolbar"));
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
  } else {
    return QObject::eventFilter(obj, event);
  }
}

void mainWindow::readOnlyPrompt(const std::string& text) {
  readOnlyPromptQStr(QString(text.c_str()));
}

void mainWindow::readOnlyPromptQStr(const QString& text) {
  QMessageBox box(this);
  box.setText(text);
  box.exec();
}

bool mainWindow::yesNoPrompt(const std::string& text) {
  return yesNoPromptQStr(QString(text.c_str()));
}

bool mainWindow::yesNoPromptQStr(const QString& text) {
  QMessageBox::StandardButton buttonPressed =
      (QMessageBox::StandardButton)QMessageBox::question(this, QString(""), text,
          QMessageBox::Yes, QMessageBox::No);
  if(buttonPressed == QMessageBox::Yes) {
    return true;
  } else if(buttonPressed == QMessageBox::No) {
    return false;
  }
  readOnlyPrompt("Error: Unknown answer. No is being assumed.");
  return false;
}

