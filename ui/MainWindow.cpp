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

  for (const auto& gpu : renderer.physicalDevices)
  {
	  ui->comboBox->insertItem(ui->comboBox->count(), gpu.properties.deviceName);
  }

  gpuComboBoxSelection(ui->comboBox->currentIndex());
  connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index){gpuComboBoxSelection(index);});
//  sprintf(deviceProperties, "API Version: %d\nDriver Version: %d\nVendor ID: %d\nDevice ID: %d\nDevice Type: %d\nDevice Name: %s\nPipeline Cache UUID: %d\nLimits: \nSparse Properties: \n",
//		  renderer.physicalDevices[0].properties.apiVersion,
//		  renderer.physicalDevices[0].properties.driverVersion,
//		  renderer.physicalDevices[0].properties.vendorID,
//		  renderer.physicalDevices[0].properties.deviceID,
//		  renderer.physicalDevices[0].properties.deviceType,
//		  renderer.physicalDevices[0].properties.deviceName,
//		  renderer.physicalDevices[0].properties.pipelineCacheUUID);

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

void MainWindow::gpuComboBoxSelection(int index)
{
	  QString deviceProperties;
	  QTextStream dpStream(&deviceProperties);
	  dpStream << "API Version: " << renderer.physicalDevices[index].properties.apiVersion <<
			  "\nDriver Version: " << renderer.physicalDevices[index].properties.driverVersion <<
			  "\nVendor ID: " << Qt::showbase << Qt::uppercasedigits << Qt::hex << renderer.physicalDevices[index].properties.vendorID <<
			  "\nDevice ID: " << Qt::hex << renderer.physicalDevices[index].properties.deviceID <<
			  "\nDevice Type: " << renderer.physicalDevices[index].properties.deviceType <<
			  "\nDevice Name: " << renderer.physicalDevices[index].properties.deviceName <<
			  "\nPipeline Cache UUID: " << renderer.physicalDevices[index].properties.pipelineCacheUUID <<
			  "\nLimits: " <<
			  "\nSparse Properties: " << "\n";
	  ui->label_2->setText(deviceProperties);

	  QString deviceFeatures;
	  QTextStream dfStream(&deviceFeatures);
	  VulkanPhysicalDeviceFeatureWrapper featureWrapper(renderer.physicalDevices[index].features);
	  for (uint32_t i = 0; i < featureWrapper.featureArray.size(); i++)
	  {
		  if (!featureWrapper.featureArray[i])
		  {
			  dfStream << "XXX ";
		  }
		  dfStream << QString::fromStdString(VulkanPhysicalDeviceFeatureWrapper::featureNames[i]) << "\n";
	  }

	  ui->label_3->setText(deviceFeatures);

}

const std::array<std::string, 55> VulkanPhysicalDeviceFeatureWrapper::featureNames =
{{
				"robustBufferAccess",
				"fullDrawIndexUint32",
				"imageCubeArray",
				"independentBlend",
				"geometryShader",
				"tessellationShader",
				"sampleRateShading",
				"dualSrcBlend",
				"logicOp",
				"multiDrawIndirect",
				"drawIndirectFirstInstance",
				"depthClamp",
				"depthBiasClamp",
				"fillModeNonSolid",
				"depthBounds",
				"wideLines",
				"largePoints",
				"alphaToOne",
				"multiViewport",
				"samplerAnisotropy",
				"textureCompressionETC2",
				"textureCompressionASTC_LDR",
				"textureCompressionBC",
				"occlusionQueryPrecise",
				"pipelineStatisticsQuery",
				"vertexPipelineStoresAndAtomics",
				"fragmentStoresAndAtomics",
				"shaderTessellationAndGeometryPointSize",
				"shaderImageGatherExtended",
				"shaderStorageImageExtendedFormats",
				"shaderStorageImageMultisample",
				"shaderStorageImageReadWithoutFormat",
				"shaderStorageImageWriteWithoutFormat",
				"shaderUniformBufferArrayDynamicIndexing",
				"shaderSampledImageArrayDynamicIndexing",
				"shaderStorageBufferArrayDynamicIndexing",
				"shaderStorageImageArrayDynamicIndexing",
				"shaderClipDistance",
				"shaderCullDistance",
				"shaderFloat64",
				"shaderInt64",
				"shaderInt16",
				"shaderResourceResidency",
				"shaderResourceMinLod",
				"sparseBinding",
				"sparseResidencyBuffer",
				"sparseResidencyImage2D",
				"sparseResidencyImage3D",
				"sparseResidency2Samples",
				"sparseResidency4Samples",
				"sparseResidency8Samples",
				"sparseResidency16Samples",
				"sparseResidencyAliased",
				"variableMultisampleRate",
				"inheritedQueries"
}};







