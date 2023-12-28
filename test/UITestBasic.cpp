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
#include <QPushButton>
#include <filesystem>


extern std::vector<char*> args;

TEST(UIBasic, CreateUIForm)
{
	std::cout << "Started CreateUIForm test\n";
	int numArgs = static_cast<int>(args.size());
	QApplication app(numArgs, args.data());
	{
		MainWindow mainWindow;
	}
/*
	QPushButton *quitButton = new QPushButton("Quit");
	QObject::connect(quitButton, &QPushButton::clicked, &app, &QCoreApplication::quit, Qt::QueuedConnection);
	//std::thread t1(MainWindow::renderLoop, mainWindow.appObj);
	quitButton->click();
	std::cout << "About to call app.exec()\n";

	app.exec(); // Ties up the calling thread
	std::cout << "Called app.exec()\n";

	//mainWindow.appObj->shouldClose = true;
	std::cout << "Waiting to join thread t1\n";
	//t1.join();
*/
}



