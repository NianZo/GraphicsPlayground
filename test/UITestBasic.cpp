/*
 * UITestBasic.cpp
 *
 *  Created on: Jul 29, 2022
 *      Author: nic
 */

#include "gtest/gtest.h"
#include "MainWindow.h"
#include <QApplication>
#include <thread>

int argc = 0;
char* argv = "UIUnitTests";

TEST(UIBasic, CreateUIForm)
{
	//char* testName = "UITest";
	//int numArgs = 1;
	QApplication app(argc, &argv);
	MainWindow mainWindow;
	std::thread t1(MainWindow::renderLoop, mainWindow.appObj);
	app.exec();
}


