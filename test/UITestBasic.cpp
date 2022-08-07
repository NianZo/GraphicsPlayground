/*
 * UITestBasic.cpp
 *
 *  Created on: Jul 29, 2022
 *      Author: nic
 */

#include "gtest/gtest.h"
#include "MainWindow.h"
#include <QApplication>

int argc = 0;
char* argv = "UIUnitTests";

TEST(UIBasic, GPUBoxGrabsStrings)
{
	//char* testName = "UITest";
	//int numArgs = 1;
	QApplication app(argc, &argv);
	QWindow* window = new QWindow; // Test fails if this isn't dynamically allocated
	MainWindow mainWindow(window);

}


