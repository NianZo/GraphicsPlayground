/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QApplication>
#include <QPlainTextEdit>
#include <QVulkanInstance>
#include <QLibraryInfo>
#include <QLoggingCategory>
#include <QPointer>
#include "hellovulkanwidget.h"
#include <QVulkanWindow>

#include "Header.hpp"
#include "VulkanApplication.hpp"

#include <iostream>

#include <thread>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//Q_LOGGING_CATEGORY(lcVk, "qt.vulkan")

//static QPointer<QPlainTextEdit> messageLogWidget;
//static QtMessageHandler oldMessageHandler = nullptr;

//static void messageHandler(QtMsgType msgType, const QMessageLogContext &logContext, const QString &text)
//{
//    if (!messageLogWidget.isNull())
//        messageLogWidget->appendPlainText(text);
//    if (oldMessageHandler)
//        oldMessageHandler(msgType, logContext, text);
//}

void renderLoop(VulkanApplication* appObj)
{
	while(!appObj->render()) {
		appObj->update();
	};
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    VulkanApplication* appObj = VulkanApplication::GetInstance();


    //messageLogWidget = new QPlainTextEdit(QLatin1String(QLibraryInfo::build()) + QLatin1Char('\n'));
    //messageLogWidget->setReadOnly(true);

    //oldMessageHandler = qInstallMessageHandler(messageHandler);

    //QLoggingCategory::setFilterRules(QStringLiteral("qt.vulkan=true"));

    QVulkanInstance inst;

    inst.setVkInstance(appObj->instanceObj.instance);

    if (!inst.create())
        qFatal("Failed to create Vulkan instance: %d", inst.errorCode());

    //VulkanWindow *vulkanWindow = new VulkanWindow;
    //vulkanWindow->setSurfaceType(QSurface::VulkanSurface);
    //vulkanWindow->setVulkanInstance(&inst);

    QWindow* widget = new QWindow;
    widget->setSurfaceType(QSurface::VulkanSurface);
    widget->setVulkanInstance(&inst);

    MainWindow mainWindow(widget);
    mainWindow.show();
    VkSurfaceKHR surface = QVulkanInstance::surfaceForWindow(widget);
    if (surface == VK_NULL_HANDLE)
    {
    	std::cout << "Got NULL surface from surfaceForWindow\n";
    }
    std::cout << "Got surface from widget\n";


    appObj->initialize(inst, widget, (uint32_t)widget->width(), (uint32_t)widget->height());
    appObj->prepare();
    std::thread t1(renderLoop, appObj);
    std::cout << "About to execute the app\n";

    return app.exec();
}
