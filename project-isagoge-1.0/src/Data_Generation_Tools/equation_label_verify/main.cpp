/*
 * main.cpp
 *
 *  Created on: Mar 27, 2013
 *      Author: Jake Bruce
 *  Purpose:
 *  	When supplied with a unix-based directory consisting of
 *  	one or more .png images whose expression regions have been
 *    manually labeled and appended to a file called "GroundTruth.dat",
 *    create new copies of each image in a subdirectory, with the 
 *    labeled regions drawn on top with colored rectangles (red if
 *    equation has been labeled as "displayed", and blue otherwise).
 */

#include <QFile>
#include <QTextStream>
#include <QString>
#include <QStringList>
#include <QRect>
#include <QPoint>
#include <QColor>
#include <QImage>
#include <QPainter>
#include <QDebug>

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
using namespace std;

string exec(const string cmd) {
  FILE* stdout = popen(cmd.c_str(), "r");
  const int bytenum = 32; // bytes per chunk
  if(!stdout) {
    cout << "ERROR executing subprocess\n";
    exit(EXIT_FAILURE);
  }
  string output = "";
  char chunk[bytenum];
  while(!feof(stdout)) {
    if(fgets(chunk, bytenum, stdout))
      output += chunk;
    else if(!feof(stdout)) {
      cout << "ERROR reading from stdout\n";
      exit(EXIT_FAILURE);
    }
  }
  pclose(stdout);
  return output;
}

void exec_display(string cmd) {
  string output = exec(cmd);
  cout << output << endl;
  return;
}

int main() {
  const string subdirname = "Labeled";
  exec("mkdir " + subdirname);
  QFile dataFile("GroundTruth.dat");
  if(!dataFile.open(QIODevice::ReadOnly))
    qDebug() << "ERROR: could not open data file!";
  QTextStream dataStream(&dataFile);
  QString curData, curFile, prevFile, br, tl;
  string subdirfile;
  QImage img;
  int curFileNum = -1, prevFileNum = -1;

  int num_expressions = QString::fromStdString(exec("cat GroundTruth.dat | wc -l")).toInt();
  int curlinenum = 0;
  cout << "Total of " << num_expressions << " to be processed.\n";

  while(!dataStream.atEnd()) {
    curData = dataStream.readLine();
    curFile = curData.mid(5, curData.indexOf(":") - 5);
    curFileNum = curFile.toInt();

    // load new and save previous image
    if(curFileNum != prevFileNum) {
      if(prevFileNum != -1)
        img.save(QString::fromStdString(subdirfile));
      subdirfile = subdirname + "/" + curFile.toStdString(); 
      exec("cp " + curFile.toStdString() + " " + subdirname);
      if(!img.load(QString::fromStdString(subdirfile))) {
        qDebug() << "ERROR: could not load " << curFile;
        exit(EXIT_FAILURE);
      }
      img = img.convertToFormat(QImage::Format_RGB32);
      prevFileNum = curFileNum;
    }
    
    // get the important indexes
    const int colon1 = curData.indexOf(":") + 1;
    const int colon2 = curData.indexOf(":", colon1) + 1;
    
    // get the expression type
    const QString exptype = curData.mid(colon1 + 1, colon2 - colon1 - 3);
    
    // get the points of the rectangle for the current line
    const int left = (curData.mid(colon2 + 1, curData.indexOf(",", colon2) - colon2 - 1)).toInt();
    int from = curData.indexOf(",", colon2) + 1;
    const int right = (curData.mid(curData.indexOf("(tl)", from) + 5, curData.indexOf(",", from + 1) 
                                   - curData.indexOf("(tl)", from) - 5)).toInt();
    int to = curData.indexOf(")", colon2);
    const int top = (curData.mid(from, to - from)).toInt();
    from = curData.indexOf(",", to) + 1;
    to = curData.indexOf(")", from);
    const int bottom = (curData.mid(from, to - from)).toInt(); 
    
    // draw the right colored rectangle on the image (red for displayed, blue for embedded) 
    QPainter paint(&img);
    QPen pen;
    pen.setWidth(8);
    if(exptype[0] == 'd')
      pen.setColor(QColor(255,0,0));
    else
      pen.setColor(QColor(0,0,255));
    paint.setPen(pen);
    for(int i = 0; i < 3; ++i) {
      paint.drawLine(left + i, top + i, right + i, top +i); 
      paint.drawLine(right +i, top + i, right + i, bottom + i);
      paint.drawLine(right + i, bottom + i, left + i, bottom + i);
      paint.drawLine(left + i, bottom + i, left + i, top + i);
    }
    curlinenum++;
    cout << "--- Processed " << curlinenum << " out of " << num_expressions << " ---\r";
  }
  img.save(QString::fromStdString(subdirfile));
  cout << "\nComplete!\n";
  return 0;
}

