#include "MainWindow.h"

#include "VulkanApplication.hpp"
#include "ui_MainGUI.h"
#include <QVulkanInstance>
#include <iostream>
#include <thread>

void MainWindow::renderLoop(VulkanApplication *appObj) {
  while (!appObj->render()) {
    appObj->update();
  };
}

MainWindow::MainWindow() : //: m_window(w)
                           ui(new Ui::Form) {
  ui->setupUi(this);

  appObj = VulkanApplication::GetInstance();

  // messageLogWidget = new QPlainTextEdit(QLatin1String(QLibraryInfo::build()) + QLatin1Char('\n'));
  // messageLogWidget->setReadOnly(true);

  // oldMessageHandler = qInstallMessageHandler(messageHandler);

  // QLoggingCategory::setFilterRules(QStringLiteral("qt.vulkan=true"));

  QVulkanInstance inst;

  inst.setVkInstance(appObj->instanceObj.instance);

  if (!inst.create())
    qFatal("Failed to create Vulkan instance: %d", inst.errorCode());

  QWindow *widget = new QWindow;
  widget->setSurfaceType(QSurface::VulkanSurface);
  widget->setVulkanInstance(&inst);

  QWidget *wrapper = QWidget::createWindowContainer(widget, ui->widget);
  wrapper->setMinimumSize(ui->widget->size());

  m_window = wrapper;
  show();
  VkSurfaceKHR surface = QVulkanInstance::surfaceForWindow(widget);
  if (surface == VK_NULL_HANDLE) {
    std::cout << "Got NULL surface from surfaceForWindow\n";
  }
  std::cout << "Got surface from widget\n";

  appObj->initialize(&surface, (uint32_t)widget->width(), (uint32_t)widget->height());
  appObj->prepare();
  // std::thread t1(renderLoop, appObj);
  // ui->widget = QWidget::createWindowContainer(w);
  //
  //    m_info = new QPlainTextEdit;
  //    m_info->setReadOnly(true);
  //
  //    m_number = new QLCDNumber(3);
  //    m_number->setSegmentStyle(QLCDNumber::Filled);
  //
  //    QPushButton *grabButton = new QPushButton(tr("&Grab"));
  //    grabButton->setFocusPolicy(Qt::NoFocus);
  //
  //    connect(grabButton, &QPushButton::clicked, this, &MainWindow::onGrabRequested);
  //
  //    QPushButton *quitButton = new QPushButton(tr("&Quit"));
  //    quitButton->setFocusPolicy(Qt::NoFocus);
  //
  //    connect(quitButton, &QPushButton::clicked, qApp, &QCoreApplication::quit);
  //
  //    QVBoxLayout *layout = new QVBoxLayout;
  //    m_infoTab = new QTabWidget(this);
  //    m_infoTab->addTab(m_info, tr("Vulkan Info"));
  //    m_infoTab->addTab(logWidget, tr("Debug Log"));
  //    layout->addWidget(m_infoTab, 2);
  //    layout->addWidget(m_number, 1);
  //    layout->addWidget(wrapper, 5);
  //    layout->addWidget(grabButton, 1);
  //    layout->addWidget(quitButton, 1);
  //    setLayout(layout);
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  m_window->setMinimumSize(ui->widget->size());
}
