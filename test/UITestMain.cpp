/*
 * UITestMain.cpp
 *
 *  Created on: Jul 29, 2022
 *      Author: nic
 */

#include "gtest/gtest.h"
#include <QApplication>

std::vector<char*> args;

int main(int argc, char** argv)
{
    args = std::vector<char*>(argv, std::next(argv, static_cast<std::ptrdiff_t>(argc)));

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
