/*
 * UITestMain.cpp
 *
 *  Created on: Jul 29, 2022
 *      Author: nic
 */

#include "gtest/gtest.h"
#include <QApplication>

int g_argc;
char** g_argv;

int main(int argc, char** argv)
{
	g_argc = argc;
	g_argv = argv;
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
