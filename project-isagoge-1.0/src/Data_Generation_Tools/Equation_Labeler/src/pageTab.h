#ifndef PAGETAB_H
#define PAGETAB_H
// OSR - with Linux OCR Capabilities
//
// Author - whitedg
// 
// pageTab.h - declaration file for pageTab object (tab layout for each opened page)
//
// Created: 18 April 2008

#include <iostream>
#include <string>
#include <QWidget>
#include <QPixmap>
#include <QGridLayout>
#include <QTextEdit>
#include <QString>
#include <QLabel>
#include <QtGui>

using namespace std;

class pageTab: public QWidget
{
	Q_OBJECT
public:
	//===============================================================================================================================
	//	<pre><B> Purpose: </B> Creates a Widget to be placed on the QTabWidget for OSR.
	//	This widget will hold the image and/or text that was opened by the user.
	//	<B> Parameters: </B> The total number of tabs opened by the user (int), the path for an image (string), 
	//	and the path for text (string). If the user has opened an image, then the path for the text will be an empty string.
	//	Likewise, if a user opens text, the path for the image will be an empty string. 
	//	<B> Preconditions: </B> The user has chosen the file to open for the current tab. It must be one of the types specified in
	//	mainWindow.cpp's file_open.
	//	<B> Postconditions: </B> The tab will be added to the mainWindow and displayed there when the user selects it. 
	//===============================================================================================================================
	pageTab(int, string, string);

	//===============================================================================================================================
	//	<B> Return: </B> The image's entire path </pre>
	//===============================================================================================================================	
	string getImagePath();

	//===============================================================================================================================
	//	<B> Return: </B> The image's file name without extension or path </pre>
	//===============================================================================================================================	
	string getImageFilename();

	//===============================================================================================================================
	//	<B> Return: </B> The image's path without the image's name at the end </pre>
	//===============================================================================================================================	
	string getImageFileBase();

	//===============================================================================================================================
	//	<B> Return: </B> The text's entire path </pre>
	//===============================================================================================================================	
	string getTextPath();

	//===============================================================================================================================
	//	<B> Return: </B> The text's file name without extension or path </pre>
	//===============================================================================================================================	
	string getTextFilename();

	//===============================================================================================================================
	//	<B> Return: </B> The text's path without its name at the end </pre>
	//===============================================================================================================================	
	string getTextFileBase();

	//===============================================================================================================================
	//	<B> Return: </B> "text" if the tab is a text file and "img" if the tab is an image file </pre>
	//===============================================================================================================================	
	string getState();

	//===============================================================================================================================
	//	<B> Return: </B> The tab's index in reference to all other tabs opened by the user </pre>
	//===============================================================================================================================	
	int getIndex();

	//===============================================================================================================================
	//	<B> Purpose: </B> Change the index of the current tab (NOT RECOMMENDED) </pre>
	//===============================================================================================================================	
	void setIndex(int);

	/*************************
	// Windows/Views
	*************************/
	QPixmap *scanImage;
	QGridLayout *gLayout;
	QTextBrowser *textEdit;
	QScrollArea *scrollArea;
	QLabel *imgView;
	QImage* image;
	
	float resizeFactor;
	int scaled_height;
	int scaled_width;

public slots:
	void setImage(string);
	void setText(string);
	void viewImage();
	void viewText();
	void viewDual();

private:
	/*************************
	// Data
	*************************/
	string textPath; // for text files
	string txtFile;
	string txtFileBase;
	string txtFileExt;

	// for image files
	string imagePath; // file path
	string imgFile; // file name (with extension)
	string imgFileBase; // file name (without extension)
	string imgFileExt; // file extension

	string curState;
	
	int index;	
	
	/*************************
	// Functions
	*************************/
	void parseFilename();
	
	/**************************
	// Action Objects
	**************************/
};


#endif

