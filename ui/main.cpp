#include <QApplication>
#include <QPlainTextEdit>
#include <QVulkanInstance>
#include <QLibraryInfo>
#include <QLoggingCategory>
#include <QPointer>
#include <QVulkanWindow>

#include "Header.hpp"
#include "VulkanApplication.hpp"

#include <iostream>

#include <thread>
#include "MainWindow.h"

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

    return app.exec();
}
