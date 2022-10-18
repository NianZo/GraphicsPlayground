#include <QApplication>
#include <QLibraryInfo>
#include <QLoggingCategory>
#include <QPlainTextEdit>
#include <QPointer>
#include <QVulkanInstance>
#include <QVulkanWindow>

#include "Header.hpp"
#include "VulkanApplication.hpp"

#include <iostream>

#include "MainWindow.h"
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

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  MainWindow mainWindow;
  std::thread t1(MainWindow::renderLoop, mainWindow.appObj);
  // mainWindow.show();

  return app.exec();
}
