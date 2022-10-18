#include "MainWindow.h"
#include "VulkanApplication.hpp"
#include "ui_MainGUI.h"
#include <iostream>
#include <thread>

void MainWindow::renderLoop(VulkanApplication *appObj)
{
  while (!appObj->render())
    {
      appObj->update();
    };
}

MainWindow::MainWindow() : ui(new Ui::Form), appObj(VulkanApplication::GetInstance())
{
  ui->setupUi(this);

  //appObj = VulkanApplication::GetInstance();

  if (appObj->instanceObj.instance == nullptr)
    {
      std::cout << "Got invalid VkInstance from appObj\n";
    }

  inst.setVkInstance(appObj->instanceObj.instance);

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

  appObj->initialize(&surface, static_cast<uint32_t>(m_window->width()), static_cast<uint32_t>(m_window->height()));
  appObj->prepare();
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
