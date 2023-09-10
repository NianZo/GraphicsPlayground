// #include "Header.hpp"
#include "MainWindow.h"
#include "VulkanRenderer.hpp"
#include <QApplication>
#include <QLibraryInfo>
#include <QLoggingCategory>
#include <QPlainTextEdit>
#include <QPointer>
#include <QVulkanInstance>
#include <QVulkanWindow>
#include <iostream>
#include <span>
#include <thread>

// Q_LOGGING_CATEGORY(lcVk, "qt.vulkan")

// static QPointer<QPlainTextEdit> messageLogWidget;
// static QtMessageHandler oldMessageHandler = nullptr;

// static void messageHandler(QtMsgType msgType, const QMessageLogContext &logContext, const QString &text)
//{
//     if (!messageLogWidget.isNull())
//         messageLogWidget->appendPlainText(text);
//     if (oldMessageHandler)
//         oldMessageHandler(msgType, logContext, text);
// }

int main(int argc, char* argv[])
{
	const auto args = std::span(argv, static_cast<size_t>(argc));
    const QApplication app(argc, argv);

    const MainWindow mainWindow;
    // std::thread thread1(MainWindow::renderLoop, mainWindow.appObj);
    //  mainWindow.show();

    return QApplication::exec();
}
