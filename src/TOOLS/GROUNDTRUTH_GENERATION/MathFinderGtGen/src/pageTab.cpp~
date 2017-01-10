// OSR - with Linux OCR Capabilities
//
// Author - whitedg
//
// pageTab.cpp - definition file for pageTab object (tab layout for each opened page)
//
// Created: 18 April 2008

#include <QtGui>
#include <QFile>
#include <QString>
#include <QByteArray>
#include "pageTab.h"

pageTab::pageTab(int count, string imgPath = "", string txtPath = "") : resizeFactor((float)1), scaled_width(1000)
{
	textPath = txtPath;
	imagePath = imgPath;
	index = count;
	
	textEdit = new QTextBrowser;
	imgView = new QLabel;
	imgView->setScaledContents(true);
	scanImage = new QPixmap;
	gLayout = new QGridLayout(this);
	
	if (imagePath != "")
	{
		curState = "img";
		setImage(imagePath);
		scrollArea = new QScrollArea;
		scrollArea->setWidget(imgView);
		gLayout->addWidget(scrollArea, 0, 0, 1, 1);
	}
	
	if (textPath != "")
	{
		curState = "text";
		setText(textPath);
		gLayout->addWidget(textEdit, 0, 0, 1, 1);
	}
}

void pageTab::setImage(string path)
{
	imagePath = path;
	parseFilename();
	scanImage->load(tr(imagePath.c_str()));

	int* before = new int; // holds the width before scaling
	int* after = new int; // holds the width after scaling

	QImage resizeImg = scanImage->toImage();
	*before = resizeImg.width();
		
	resizeImg = resizeImg.scaledToWidth(scaled_width);
	*after = resizeImg.width();
		
	resizeFactor = ((float)(*after)) / ((float)(*before));
	qDebug() << "Resize factor: " << resizeFactor;

	*scanImage = QPixmap::fromImage(resizeImg);
	delete [] before;
	delete [] after; 

	imgView->setPixmap(*scanImage);
}

void pageTab::setText(string path)
{
	textPath = path;
	parseFilename();
	QFile* file = new QFile((QString)textPath.c_str());
	file->open(QIODevice::ReadOnly);
	QByteArray* temp = new QByteArray;
	*temp = file->readAll();
	QString* text_to_set = new QString(*temp);
	textEdit->setText(*text_to_set);
	delete text_to_set;
	delete temp;
	delete file;
}

void pageTab::viewImage()
{
	if (curState != "img")
	{
		delete(textEdit);
		textEdit = new QTextBrowser;
		if (textPath != "")
			setText(textPath);

		gLayout->addWidget(imgView, 0, 0 , 1, 1);
		curState = "img";
	}
}

void pageTab::viewText()
{
	if (curState != "text")
	{
		delete(imgView);
		imgView = new QLabel;
		if (imagePath != "")
			setImage(imagePath);
		
		gLayout->addWidget(textEdit, 0, 0, 1, 1);
		curState = "text";
	}
}

void pageTab::viewDual()
{
	if (curState != "dual")
	{
		setText(textPath);
		
		gLayout->addWidget(scrollArea, 1, 1, 1, 1);
		gLayout->addWidget(textEdit, 1, 2, 1, 1);
		curState = "dual";
	}
}

void pageTab::parseFilename()
{
	int lastSlash, lastPt;
	
	if (textPath.substr(0, 3) == "New")
	{
		txtFile = textPath;
		txtFileBase = textPath;
		txtFileExt = "";
		
		imgFile = imagePath;
		imgFileBase = imagePath;
		imgFileExt = "";
		
		return;
	}
	
	if (textPath != "")
	{	
		
		lastSlash = textPath.find_last_of("/", textPath.length());
		lastPt = textPath.find_last_of(".", textPath.length());
		txtFile = textPath.substr(lastSlash + 1);
		txtFileBase = textPath.substr(lastSlash + 1, lastPt - lastSlash - 1);
		txtFileExt = textPath.substr(lastPt + 1);
	}
	else
	{
		txtFile = "";
		txtFileBase = "";
		txtFileExt = "";
	}
	
	if (imagePath != "")
	{
		lastSlash = imagePath.find_last_of("/", imagePath.length());
		lastPt = imagePath.find_last_of(".", imagePath.length());
		imgFile = imagePath.substr(lastSlash + 1);
		imgFileBase = imagePath.substr(lastSlash + 1, lastPt - lastSlash - 1);
		imgFileExt = imagePath.substr(lastPt + 1);
	}
	else
	{
		imgFile = "";
		imgFileBase = "";
		imgFileExt = "";
	}
	
	return;
}

int pageTab::getIndex()
{
	return index;
}

void pageTab::setIndex(int i)
{
	this->index = i;
}

string pageTab::getImagePath()
{
	return imagePath;
}

string pageTab::getImageFilename()
{
	return imgFile;
}

string pageTab::getImageFileBase()
{
	return imgFileBase;
}

string pageTab::getTextPath()
{
	return textPath;
}

string pageTab::getTextFilename()
{
	return txtFile;
}

string pageTab::getTextFileBase()
{
	return txtFileBase;
}

string pageTab::getState()
{
	return curState;
}
