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

  //appObj = VulkanApplication::GetInstance();
//
//  inst.setVkInstance(appObj->instanceObj.instance);
//
//  if (!inst.create())
//    qFatal("Failed to create Vulkan instance: %d", inst.errorCode());
//
//  m_window = new QWindow;
//  m_window->setSurfaceType(QSurface::VulkanSurface);
//  m_window->setVulkanInstance(&inst);
//
//  m_windowWrapper = QWidget::createWindowContainer(m_window, ui->widget);
//  m_windowWrapper->setMinimumSize(ui->widget->size());

//  show();
//  surface = QVulkanInstance::surfaceForWindow(m_window);
//  if (surface == VK_NULL_HANDLE) {
//    std::cout << "Got NULL surface from surfaceForWindow\n";
//  }

  //appObj->initialize(&surface, (uint32_t)m_window->width(), (uint32_t)m_window->height());
  //appObj->prepare();
  std::cout << "Finished MainWindow::MainWindow()\n";

}

MainWindow::~MainWindow()
{

	//close(); // needed or there will be a segfault
	//delete m_window;
	//delete m_window;
	std::cout << "Called delete m_window\n";
	delete ui;
	std::cout << "Finished running MainWindow::~MainWindow()\n";
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  m_windowWrapper->setMinimumSize(ui->widget->size());
}
