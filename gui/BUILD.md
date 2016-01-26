Watermeter Testbench GUI
========================

This has been designed for Linux only. Instructions are for a standard UBUNTU
installation.

1. Install qt4-designer and qt4-qmake to ensure that all libraries are present.
2. Download QExtSerialPort https://github.com/qextserialport/qextserialport
   (a zip file https://github.com/qextserialport/qextserialport/archive/master.zip
   is provided).
3. Unpack QExtSerialPort into any desired location and modify testbench.pro
   line 6 to replace ../auxiliary/ with the directory containing QExtSerialPort.
4. In a terminal, change to the Testbench GUI directory and issue the commands:

   $ qmake-qt4

   $ make

K. Sarkies, 26/1/2016

