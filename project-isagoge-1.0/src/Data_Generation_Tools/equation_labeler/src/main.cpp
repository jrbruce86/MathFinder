// OSR - with Linux OCR Capabilities
//
// Author - Derek White
//
// main.cpp - creation of main GUI application and window
//
// Created: 07 April 2008
// Final Version Date: 01 May 2008 (Spring 2008 Semester):

#include <QSplashScreen>
#include "mainWindow.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setQuitOnLastWindowClosed(true);

  app.processEvents();

	mainWindow mainGUI;
	mainGUI.resize(1200, 600);

	mainGUI.show();
	return app.exec();
}

