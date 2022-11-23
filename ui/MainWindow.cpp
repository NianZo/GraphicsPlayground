#include "MainWindow.h"
//#include "VulkanApplication.hpp"
#include "ui_MainGUI.h"
#include <iostream>
#include <thread>
#include <QString>

// void MainWindow::renderLoop(VulkanApplication* appObj)
//{
//   while (!appObj->render())
//   {
//     appObj->update();
//   };
// }

MainWindow::MainWindow() : ui(new Ui::Form), renderer(VulkanRenderer2("Graphics Playground")) // appObj(VulkanApplication::GetInstance())
{
  ui->setupUi(this);

  // appObj = VulkanApplication::GetInstance();

  if (renderer.instance == VK_NULL_HANDLE)
  // if (appObj->instanceObj.instance == nullptr)
  {
    std::cout << "Got invalid VkInstance from appObj\n";
  }

  inst.setVkInstance(renderer.instance);
  // inst.setVkInstance(appObj->instanceObj.instance);

  if (!inst.create())
  {
    qFatal("Failed to create Vulkan instance: %d", inst.errorCode());
  }

  m_window = std::make_unique<QWindow>();
  m_window->setSurfaceType(QSurface::VulkanSurface);
  m_window->setVulkanInstance(&inst);

  m_windowWrapper = QWidget::createWindowContainer(m_window.get(), ui->widget);
  m_windowWrapper->setMinimumSize(ui->widget->size());

  show();
  surface = QVulkanInstance::surfaceForWindow(m_window.get());
  if (surface == VK_NULL_HANDLE)
  {
    std::cout << "Got NULL surface from surfaceForWindow\n";
  }

  QString deviceProperties;
  QTextStream dpStream(&deviceProperties);
  dpStream << "API Version: " << renderer.physicalDevices[1].properties.apiVersion <<
		  "\nDriver Version: " << renderer.physicalDevices[1].properties.driverVersion <<
		  "\nVendor ID: " << Qt::showbase << Qt::uppercasedigits << Qt::hex << renderer.physicalDevices[1].properties.vendorID <<
		  "\nDevice ID: " << Qt::hex << renderer.physicalDevices[1].properties.deviceID <<
		  "\nDevice Type: " << renderer.physicalDevices[1].properties.deviceType <<
		  "\nDevice NAme: " << renderer.physicalDevices[1].properties.deviceName <<
		  "\nPipeline Cache UUID: " << renderer.physicalDevices[1].properties.pipelineCacheUUID <<
		  "\nLimits: " <<
		  "\nSparse Properties: " << "\n";
//  sprintf(deviceProperties, "API Version: %d\nDriver Version: %d\nVendor ID: %d\nDevice ID: %d\nDevice Type: %d\nDevice Name: %s\nPipeline Cache UUID: %d\nLimits: \nSparse Properties: \n",
//		  renderer.physicalDevices[0].properties.apiVersion,
//		  renderer.physicalDevices[0].properties.driverVersion,
//		  renderer.physicalDevices[0].properties.vendorID,
//		  renderer.physicalDevices[0].properties.deviceID,
//		  renderer.physicalDevices[0].properties.deviceType,
//		  renderer.physicalDevices[0].properties.deviceName,
//		  renderer.physicalDevices[0].properties.pipelineCacheUUID);
  ui->label_2->setText(deviceProperties);
  //  appObj->initialize(&surface, static_cast<uint32_t>(m_window->width()), static_cast<uint32_t>(m_window->height()));
  //  appObj->prepare();
  std::cout << "Finished MainWindow::MainWindow()\n";
}

MainWindow::~MainWindow()
{

  close(); // needed or there will be a segfault
  // delete m_window;
  std::cout << "Called delete m_window\n";
  delete ui;
  std::cout << "Finished running MainWindow::~MainWindow()\n";
}

void MainWindow::resizeEvent([[maybe_unused]] QResizeEvent* event)
{
  m_windowWrapper->setMinimumSize(ui->widget->size());
}
