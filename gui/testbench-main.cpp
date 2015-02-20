/*       Testbench GUI Main Window

Here the data stream from the test microcontroller is received and saved to a
file

@date 18 February 2015
*/

/****************************************************************************
 *   Copyright (C) 2015 by Ken Sarkies                                      *
 *   ksarkies@internode.on.net                                              *
 *                                                                          *
 *   This file is part of Testbench                                         *
 *                                                                          *
 *   Testbench is free software; you can redistribute it and/or             *
 *   modify it under the terms of the GNU General Public License as         *
 *   published by the Free Software Foundation; either version 2 of the     *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 *   Testbench is distributed in the hope that it will be useful,           *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU General Public License for more details.                           *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with testbench.                                                  *
 *   If not, write to the Free Software Foundation, Inc.,                   *
 *   51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.              *
 ***************************************************************************/

#include "testbench.h"
#include "testbench-main.h"
#include "serialport.h"
#include <QApplication>
#include <QString>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include <QTextEdit>
#include <QCloseEvent>
#include <QFileDialog>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <cstdlib>
#include <iostream>
#include <unistd.h>

//-----------------------------------------------------------------------------
/** Testbench Main Window Constructor

@param[in] parent Parent widget.
*/

TestbenchGui::TestbenchGui(QString inPort, uint baudrate,
                           QWidget* parent) : QDialog(parent)
{
    TestbenchMainUi.setupUi(this);
    socket = new SerialPort(inPort);
    connect(socket, SIGNAL(readyRead()), this, SLOT(onDataAvailable()));
    if (socket->initPort(baudrate,100))
        synchronized = true;
    else
        errorMessage = QString("Unable to access the serial port\n"
                            "Check the connections and power.\n"
                            "You may need root privileges?");
    TestbenchMainUi.startPushButton->setText("Start");
    TestbenchMainUi.startPushButton->setStyleSheet("background-color:lightgreen;");
    recordingActive = false;
}

TestbenchGui::~TestbenchGui()
{
}

//-----------------------------------------------------------------------------
/** @brief Handle incoming serial data

This is called when data appears in the serial buffer. Data is pulled in until
a newline occurs, at which point the assembled data record QString is processed.

*/

void TestbenchGui::onDataAvailable()
{
    QByteArray data = socket->readAll();
    int n=0;
    while (n < data.size())
    {
        if ((data.at(n) != '\r') && (data.at(n) != '\n')) response += data.at(n);
        if (data.at(n) == '\n')
        {
// The current time is saved to ms precision followed by the data record.
            tick.restart();
            processResponse(response);
            response.clear();
        }
        n++;
    }
}

//-----------------------------------------------------------------------------
/** @brief Process the incoming serial data

Parse the line as csv and display each field.
*/

void TestbenchGui::processResponse(const QString response)
{
    QStringList breakdown = response.split(",");
    int size = breakdown.size();

// Extract time from ISO 8601 formatted date-time.
    QString timeStamp = breakdown[0].simplified();
//    TestbenchMainUi.timeDisplay->setText(timeStamp.mid(11,8));

/* Flow meter count and period. These are processed further to derive a
flow rate by selecting the most accurate measure. 
Datasheet for the FS200A specifies 450 counts per litre.
Count is number of pulses in 10ms, period is in ms. */
    int flowMeterCount = breakdown[1].simplified().toInt();
    long flowMeterPeriod = breakdown[2].simplified().toLong();
    float flowMeterRate;
    if (flowMeterCount < 10)
        flowMeterRate = 133.3/flowMeterPeriod;
    else
        flowMeterRate = 1.333*flowMeterCount;
    TestbenchMainUi.flowRate->setText(QString("%1").arg(flowMeterRate,0,'f',1));

/* Pressure scaled by datasheet specification 0.5V to 4.5V for 0 to 1.2 MPa.
Results given in ATM */
    int pressval = breakdown[3].simplified().toInt();
    float pressure = 3.0*(5.0*(float)pressval/1024.0-0.5);
    TestbenchMainUi.pressure->setText(QString("%1").arg(pressure,0,'f',3));

// Temperature scaled by rough calibration.
    int tempval = breakdown[4].simplified().toInt();
    float temperature = 26.0+((float)tempval-716.0)/15.07;
    TestbenchMainUi.temperature->setText(QString("%1").arg(temperature,0,'f',1));

/* Get local time in ISO 8601 format. */
    QString timeString = QDateTime::currentDateTime().toString(Qt::ISODate);
    TestbenchMainUi.timeDisplay->setText(timeString.mid(11,8));

/* Save the response with timestamp stripped and replaced wth local time. */
    QString line = timeString + ',' + response.mid(response.indexOf(",")+1);
    if ((! saveFile.isEmpty()) && recordingActive) saveLine(line);
}

//-----------------------------------------------------------------------------
/** @brief Activate the Start/Stop Recording button.

*/

void TestbenchGui::on_startPushButton_clicked()
{
    recordingActive = (TestbenchMainUi.startPushButton->text() == "Start");
    if (recordingActive)
    {
        TestbenchMainUi.startPushButton->setText("Stop");
        TestbenchMainUi.startPushButton->setStyleSheet("background-color:red;");
    }
    else
    {
        TestbenchMainUi.startPushButton->setText("Start");
        TestbenchMainUi.startPushButton->setStyleSheet("background-color:lightgreen;");
    }
}

//-----------------------------------------------------------------------------
/** @brief Obtain a save file name and path and attempt to open it.

The files are csv but the ending can be arbitrary to allow compatibility
with the data processing application.
*/

void TestbenchGui::on_saveFileButton_clicked()
{
//! Make sure there is no file already open.
    if (! saveFile.isEmpty())
    {
        displayErrorMessage("A file is already open - close first");
        return;
    }
    TestbenchMainUi.errorLabel->clear();
    QString filename = QFileDialog::getSaveFileName(this,
                        "Save Acquired Data",
                        QString(),
                        "Comma Separated Variables (*.csv *.txt)");
    if (filename.isEmpty()) return;
//    if (! filename.endsWith(".csv")) filename.append(".csv");
    QFileInfo fileInfo(filename);
    saveDirectory = fileInfo.absolutePath();
    saveFile = saveDirectory.filePath(filename);
    outFile = new QFile(saveFile);             // Open file for output
    if (! outFile->open(QIODevice::WriteOnly))
    {
        displayErrorMessage("Could not open the output file");
        return;
    }
}

//-----------------------------------------------------------------------------
/** @brief Save a line to the opened save file.

*/
void TestbenchGui::saveLine(QString line)
{
//! Check that the save file has been defined and is open.
    if (saveFile.isEmpty())
    {
        displayErrorMessage("Output File not defined");
        return;
    }
    if (! outFile->isOpen())
    {
        displayErrorMessage("Output File not open");
        return;
    }
/* If either of the last two characters are \n or \r, strip them out. */
    if (line.endsWith("\n") || line.endsWith("\r"))
        line = line.remove(1);
    if (line.endsWith("\n") || line.endsWith("\r"))
        line = line.remove(1);

    QTextStream out(outFile);
    out << line << "\n\r";
}

//-----------------------------------------------------------------------------
/** @brief Close the save file.

*/
void TestbenchGui::on_closeFileButton_clicked()
{
    TestbenchMainUi.errorLabel->clear();
    if (saveFile.isEmpty())
        displayErrorMessage("File already closed");
    else
    {
        outFile->close();
        delete outFile;
//! Save the name to prevent the same file being used.
        saveFile = QString();
    }
}
//-----------------------------------------------------------------------------
/** @brief Show an error condition in the Error label.

@param[in] message: Message to be displayed.
*/

void TestbenchGui::displayErrorMessage(const QString message)
{
    TestbenchMainUi.errorLabel->setText(message);
}

//-----------------------------------------------------------------------------
/** @brief Error Message

@returns a message when the device didn't respond.
*/
QString TestbenchGui::error()
{
    return errorMessage;
}

//-----------------------------------------------------------------------------
/** @brief Successful establishment of serial port setup

@returns TRUE if successful.
*/
bool TestbenchGui::success()
{
    return true;
}


