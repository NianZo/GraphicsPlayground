#include "MainWindow.h"
// #include "VulkanApplication.hpp"
#include "ui_MainGUI.h"
#include <QString>
#include <iostream>
#include <span>
#include <thread>

// void MainWindow::renderLoop(VulkanApplication* appObj)
//{
//   while (!appObj->render())
//   {
//     appObj->update();
//   };
// }

MainWindow::MainWindow() : ui(new Ui::Form),
                           rendererBase(RendererBase(std::filesystem::weakly_canonical(std::filesystem::path(QApplication::arguments()[0].toStdString())).parent_path(), "Graphics Playground")) // appObj(VulkanApplication::GetInstance())
{
    std::cout << "started mainWindow constructor\n";
    // renderer = nullptr;
    QStringList arguments = QApplication::arguments();
    projectDirectory = std::filesystem::weakly_canonical(std::filesystem::path(arguments[0].toStdString())).parent_path();

    ui->setupUi(this);

    // appObj = VulkanApplication::GetInstance();

    if (rendererBase.instance == VK_NULL_HANDLE)
    // if (appObj->instanceObj.instance == nullptr)
    {
        std::cout << "Got invalid VkInstance from appObj\n";
    }

    inst.setVkInstance(rendererBase.instance);
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

    for (PhysicalDeviceDescriptor& gpu : rendererBase.physicalDevices)
    {
        const std::span<char> deviceNameSpan(gpu.properties.deviceName);
        ui->comboBox->insertItem(ui->comboBox->count(), QString(std::string(deviceNameSpan.begin(), deviceNameSpan.end()).c_str()));
    }

    gpuComboBoxSelection(ui->comboBox->currentIndex());
    connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index)
            { gpuComboBoxSelection(index); });
    // connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(gpuComboBoxSelection(int)));
    ui->comboBox->setCurrentIndex(1); // TODO(nic): this makes the tests pass. Device 0 on my machine is a NULL GPU that causes a seg fault on certain Vulkan calls
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
    // renderer = VulkanRenderer2("Graphics Playground");
    // renderer.chooseGPU(surface, ui->comboBox->currentIndex());
    renderer = std::make_unique<VulkanRenderer>(rendererBase, surface, ui->comboBox->currentIndex(), ui->widget->size().width(), ui->widget->size().height());
    // renderer(rendererBase, surface, ui->comboBox->currentIndex());
    renderer->Render();
    // renderer->RenderTriangle();
    std::cout << "Finished MainWindow::MainWindow()\n";
}

MainWindow::~MainWindow()
{
    // delete renderer;
    renderer.reset();
    close(); // needed or there will be a segfault
    // delete m_window;
    std::cout << "Called delete m_window\n";
    delete ui;
    std::cout << "Finished running MainWindow::~MainWindow()\n";
}

void MainWindow::resizeEvent([[maybe_unused]] QResizeEvent* event)
{
    std::cout << "calling resize event\n";

    if (renderer != nullptr)
    {
        surface = QVulkanInstance::surfaceForWindow(m_window.get());
        renderer->Resize(surface, ui->widget->size().width(), ui->widget->size().height());
        renderer->Render();
        // renderer->RenderTriangle();
    }
    m_windowWrapper->setMinimumSize(ui->widget->size());
    m_windowWrapper->setMaximumSize(ui->widget->size());
}

void MainWindow::gpuComboBoxSelection(int index)
{
    const auto deviceNameSpan = std::span<char>(rendererBase.physicalDevices[index].properties.deviceName);
    const auto pipelineCacheUUIDSpan = std::span<uint8_t>(rendererBase.physicalDevices[index].properties.pipelineCacheUUID);
    QString deviceProperties;
    QTextStream dpStream(&deviceProperties);
    dpStream << "API Version: " << rendererBase.physicalDevices[index].properties.apiVersion
             << "\nDriver Version: " << rendererBase.physicalDevices[index].properties.driverVersion
             << "\nVendor ID: " << Qt::showbase << Qt::uppercasedigits << Qt::hex << rendererBase.physicalDevices[index].properties.vendorID
             << "\nDevice ID: " << Qt::hex << rendererBase.physicalDevices[index].properties.deviceID
             << "\nDevice Type: " << rendererBase.physicalDevices[index].properties.deviceType
             << "\nDevice Name: " << QString(std::string(deviceNameSpan.begin(), deviceNameSpan.end()).c_str())
             << "\nPipeline Cache UUID: " << QString(std::string(pipelineCacheUUIDSpan.begin(), pipelineCacheUUIDSpan.end()).c_str())
             << "\nLimits: "
             << "\nSparse Properties: "
             << "\n";
    ui->label_2->setText(deviceProperties);

    QString deviceFeatures;
    QTextStream dfStream(&deviceFeatures);
    VulkanPhysicalDeviceFeatureWrapper featureWrapper(rendererBase.physicalDevices[index].features);
    for (uint32_t i = 0; i < featureWrapper.featureArray.size(); i++)
    {
        if (featureWrapper.featureArray[i] == 0U)
        {
            dfStream << "XXX ";
        }
        dfStream << QString::fromStdString(VulkanPhysicalDeviceFeatureWrapper::featureNames[i]) << "\n";
    }

    ui->label_3->setText(deviceFeatures);
}

// constexpr std::array<const char*, 55> VulkanPhysicalDeviceFeatureWrapper::featureNames =
//     {{"robustBufferAccess",
//       "fullDrawIndexUint32",
//       "imageCubeArray",
//       "independentBlend",
//       "geometryShader",
//       "tessellationShader",
//       "sampleRateShading",
//       "dualSrcBlend",
//       "logicOp",
//       "multiDrawIndirect",
//       "drawIndirectFirstInstance",
//       "depthClamp",
//       "depthBiasClamp",
//       "fillModeNonSolid",
//       "depthBounds",
//       "wideLines",
//       "largePoints",
//       "alphaToOne",
//       "multiViewport",
//       "samplerAnisotropy",
//       "textureCompressionETC2",
//       "textureCompressionASTC_LDR",
//       "textureCompressionBC",
//       "occlusionQueryPrecise",
//       "pipelineStatisticsQuery",
//       "vertexPipelineStoresAndAtomics",
//       "fragmentStoresAndAtomics",
//       "shaderTessellationAndGeometryPointSize",
//       "shaderImageGatherExtended",
//       "shaderStorageImageExtendedFormats",
//       "shaderStorageImageMultisample",
//       "shaderStorageImageReadWithoutFormat",
//       "shaderStorageImageWriteWithoutFormat",
//       "shaderUniformBufferArrayDynamicIndexing",
//       "shaderSampledImageArrayDynamicIndexing",
//       "shaderStorageBufferArrayDynamicIndexing",
//       "shaderStorageImageArrayDynamicIndexing",
//       "shaderClipDistance",
//       "shaderCullDistance",
//       "shaderFloat64",
//       "shaderInt64",
//       "shaderInt16",
//       "shaderResourceResidency",
//       "shaderResourceMinLod",
//       "sparseBinding",
//       "sparseResidencyBuffer",
//       "sparseResidencyImage2D",
//       "sparseResidencyImage3D",
//       "sparseResidency2Samples",
//       "sparseResidency4Samples",
//       "sparseResidency8Samples",
//       "sparseResidency16Samples",
//       "sparseResidencyAliased",
//       "variableMultisampleRate",
//       "inheritedQueries"}};
