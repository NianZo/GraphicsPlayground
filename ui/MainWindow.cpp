#include "MainWindow.h"

#include "VulkanApplication.hpp"
#include "ui_MainGUI.h"

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


  inst.setVkInstance(appObj->instanceObj.instance);

  if (!inst.create())
    qFatal("Failed to create Vulkan instance: %d", inst.errorCode());

  m_window = new QWindow;
  m_window->setSurfaceType(QSurface::VulkanSurface);
  m_window->setVulkanInstance(&inst);

  m_windowWrapper = QWidget::createWindowContainer(m_window, ui->widget);
  m_windowWrapper->setMinimumSize(ui->widget->size());

  show();
  surface = QVulkanInstance::surfaceForWindow(m_window); // causing seg fault on destruction
  //surface = inst.surfaceForWindow(m_window);
  if (surface == VK_NULL_HANDLE) {
    std::cout << "Got NULL surface from surfaceForWindow\n";
  }

  appObj->initialize(&surface, (uint32_t)m_window->width(), (uint32_t)m_window->height());
  appObj->prepare();
  std::cout << "Finished MainWindow::MainWindow()\n";
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

MainWindow::~MainWindow()
{
	//hide();
	//vkDestroySurfaceKHR(appObj->instanceObj.instance, surface, nullptr);
	//appObj->deInitialize();
	//delete wrapper;
	std::cout << "Called vkDestroySurfaceKHR\n";
	//widget->destroy(); // Causing segfault
	//delete widget;
	close(); // needed or there will be a segfault
	//delete widget;
	//m_window->destroy();
	std::cout << "Called widget->destroy()\n";
	//delete m_window;
	std::cout << "Called delete m_window\n";
	//delete m_windowWrapper;
	//hide();
	delete ui;
	//appObj->~VulkanApplication();
	std::cout << "Finished running MainWindow::~MainWindow()\n";
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  m_windowWrapper->setMinimumSize(ui->widget->size());
}
