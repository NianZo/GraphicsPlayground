#include "MainWindow.h"
// #include "VulkanApplication.hpp"
#include "ui_MainGUI.h"
#include <QString>
#include <iostream>
#include <span>
#include <string>
#include <thread>

// void MainWindow::renderLoop(VulkanApplication* appObj)
//{
//   while (!appObj->render())
//   {
//     appObj->update();
//   };
// }

MainWindow::MainWindow() :
    ui(new Ui::Form),
    rendererBase(RendererBase(std::filesystem::weakly_canonical(std::filesystem::path(QApplication::arguments()[0].toStdString())).parent_path(), "Graphics Playground")) // appObj(VulkanApplication::GetInstance())
{
    // menuBar()->setNativeMenuBar(false);
    // std::cout << "started mainWindow constructor\n";
    //  renderer = nullptr;
    QStringList arguments = QApplication::arguments();
    projectDirectory = std::filesystem::weakly_canonical(std::filesystem::path(arguments[0].toStdString())).parent_path();
    // std::cout << "centralWidget: " << this->centralWidget() << "\n";
    ui->setupUi(&formWidget);

    this->setCentralWidget(&formWidget);

    // appObj = VulkanApplication::GetInstance();
    //    QMenu *menu = new QMenu("First menu");
    //    menu->addAction("item 1");
    //    menu->addAction("item 2");
    //    m_menuBar->addAction(menu->menuAction());
    //    viewMenu = menuBar()->addMenu(tr("View"));
    toolBar = addToolBar(tr("File"));
    QAction* renderViewAction = new QAction("Render View");
    connect(renderViewAction, &QAction::triggered, this, &MainWindow::setRenderView);
    toolBar->addAction(renderViewAction);
    QAction* graphicsDescriptorViewAction = new QAction("Graphics Pipeline");
    connect(graphicsDescriptorViewAction, &QAction::triggered, this, &MainWindow::graphicsDescriptorView);
    toolBar->addAction(graphicsDescriptorViewAction);

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

    ui->comboBox_2->addItem("FILL");
    ui->comboBox_2->addItem("LINE");
    ui->comboBox_2->addItem("POINT");

    gpuComboBoxSelection(ui->comboBox->currentIndex());
    connect(ui->comboBox, &QComboBox::currentIndexChanged, [this](int index)
            { gpuComboBoxSelection(index); });
    // connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(gpuComboBoxSelection(int)));
    ui->comboBox->setCurrentIndex(1); // TODO(nic): this makes the tests pass. Device 0 on my machine is a NULL GPU that causes a seg fault on certain Vulkan calls

    connect(ui->textEdit_vertexShader, &QTextEdit::textChanged, [this]()
            { vertexShaderTextChanged(); });
    connect(ui->textEdit_fragmentShader, &QTextEdit::textChanged, [this]()
            { fragmentShaderTextChanged(); });
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
    connect(ui->comboBox_2, &QComboBox::currentIndexChanged, [this](int index)
            { polygonModeComboBox(index); });
    // ui->comboBox_2->setCurrentIndex(0);
    polygonModeComboBox(0); // Initialize

    // For now don't do this until the interface is more fleshed out
    // createRenderer();

    //    renderer = std::make_unique<VulkanRenderer>(rendererBase, surface, ui->comboBox->currentIndex(), ui->widget->size().width(), ui->widget->size().height());
    //    // renderer(rendererBase, surface, ui->comboBox->currentIndex());
    //    // TODO (nic) holy hell the separation of concerns is currently non-existent
    //    GraphicsPipelineDescriptor descriptor;
    //    //descriptor.rasterizer.polygonMode = pipelineDescriptor.rasterizer.polygonMode;
    //    const std::filesystem::path shaderDirectory = renderer->rendererBase.projectDirectory.parent_path() / "shaders";
    //    if (descriptor.vertexShader[0] == "")
    //    {
    //        std::cout << "Shader directory: " << shaderDirectory << "\n";
    //        descriptor.vertexShader = {std::string(shaderDirectory / "DrawTriangle-vert.spv"), "main"};
    //    }
    //
    //    if (descriptor.fragmentShader[0] == "")
    //    {
    //    	descriptor.fragmentShader = {std::string(shaderDirectory / "DrawTriangle-frag.spv"), "main"};
    //    }
    //
    //    descriptor.dynamicStates.emplace_back(VK_DYNAMIC_STATE_VIEWPORT);
    //    descriptor.dynamicStates.emplace_back(VK_DYNAMIC_STATE_SCISSOR);
    //
    //    descriptor.viewports[0].width = static_cast<float>(renderer->display->swapchainExtent.width);
    //    descriptor.viewports[0].height = static_cast<float>(renderer->display->swapchainExtent.height);
    //
    //    descriptor.scissors[0].extent = renderer->display->swapchainExtent;
    //    descriptor.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    //    descriptor.colorAttachment.format = renderer->display->swapchainImageFormat;
    //    renderer->scenes.emplace_back(*renderer.get());
    //    renderer->scenes[0].drawables.emplace_back(*renderer.get(), renderer->commandPool, descriptor);
    //    renderer->scenes[0].cameras.emplace_back(*renderer.get());
    // renderer->drawables.push_back(*renderer.get(), renderer->commandPool, descriptor);
    // renderer->Render(pipelineDescriptor);
    // renderer->scenes[0].render();
    // renderer->RenderTriangle();
    // std::cout << "Finished MainWindow::MainWindow()\n";
}

MainWindow::~MainWindow()
{
    // delete renderer;
    renderer.reset();
    close(); // needed or there will be a segfault
    // delete m_window;
    // std::cout << "Called delete m_window\n";
    delete ui;
    // std::cout << "Finished running MainWindow::~MainWindow()\n";
}

void MainWindow::resizeEvent([[maybe_unused]] QResizeEvent* event)
{
    // std::cout << "calling resize event\n";

    // ui->widget->setMinimumSize(this->size());

    if (renderer != nullptr)
    {
        surface = QVulkanInstance::surfaceForWindow(m_window.get());
        // renderer->Resize(surface, ui->widget->size().width(), ui->widget->size().height());
        m_windowWrapper->setMinimumSize(ui->widget->size());
        m_windowWrapper->setMaximumSize(ui->widget->size());
        renderer.reset();
        createRenderer();
        // renderer->Render(pipelineDescriptor);
        // renderer->scenes[0].render();
        //  renderer->RenderTriangle();
    }
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

void MainWindow::graphicsDescriptorView()
{
    // std::cout << "graphicsDescriptorView()\n";
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::setRenderView()
{
    ui->stackedWidget->setCurrentIndex(0);
    renderer->Resize(surface, ui->widget->size().width(), ui->widget->size().height());
    renderer->Render();
}

void MainWindow::polygonModeComboBox(int index)
{
    switch (index)
    {
    case 0:
        pipelineDescriptor.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        break;
    case 1:
        pipelineDescriptor.rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
        break;
    case 2:
        pipelineDescriptor.rasterizer.polygonMode = VK_POLYGON_MODE_POINT;
        break;
    }
    // renderer->Resize(surface, ui->widget->size().width(), ui->widget->size().height());
    // renderer->Render(pipelineDescriptor);
}

void MainWindow::vertexShaderTextChanged()
{
    // std::cout << "vertexShaderTextChanged\n";
    const std::filesystem::path shaderDirectory = projectDirectory.parent_path() / "shaders";
    pipelineDescriptor.vertexShader = { std::string(shaderDirectory / ui->textEdit_vertexShader->toPlainText().toStdString()), "main" };
    try
    {
        renderer->Render();
    }
    catch (std::exception& e)
    {
        std::cout << "Failed to render: " << pipelineDescriptor.vertexShader[0] << "\n";
    }
}

void MainWindow::fragmentShaderTextChanged()
{
    const std::filesystem::path shaderDirectory = projectDirectory.parent_path() / "shaders";
    pipelineDescriptor.fragmentShader = { std::string(shaderDirectory / ui->textEdit_fragmentShader->toPlainText().toStdString()), "main" };
    try
    {
        renderer->Render();
    }
    catch (std::exception& e)
    {
        std::cout << "Failed to render: " << pipelineDescriptor.fragmentShader[0] << "\n";
    }
}

void MainWindow::createRenderer()
{
    renderer = std::make_unique<VulkanRenderer>(rendererBase, surface, rendererBase.physicalDevices[ui->comboBox->currentIndex()], ui->widget->size().width(), ui->widget->size().height());
    // renderer(rendererBase, surface, ui->comboBox->currentIndex());
    // TODO (nic) holy hell the separation of concerns is currently non-existent
    GraphicsPipelineDescriptor descriptor;
    // descriptor.rasterizer.polygonMode = pipelineDescriptor.rasterizer.polygonMode;
    const std::filesystem::path shaderDirectory = renderer->rendererBase.projectDirectory.parent_path() / "shaders";
    if (descriptor.vertexShader[0] == "")
    {
        // std::cout << "Shader directory: " << shaderDirectory << "\n";
        descriptor.vertexShader = { std::string(shaderDirectory / "DrawTriangle-vert.spv"), "main" };
    }

    if (descriptor.fragmentShader[0] == "")
    {
        descriptor.fragmentShader = { std::string(shaderDirectory / "DrawTriangle-frag.spv"), "main" };
    }

    // TODO (nic) I don't want the user to have to set the extents (unless they need to) so for a fullscreen render these shouldn't be dynamic states set here
    descriptor.dynamicStates.emplace_back(VK_DYNAMIC_STATE_VIEWPORT);
    descriptor.dynamicStates.emplace_back(VK_DYNAMIC_STATE_SCISSOR);

    descriptor.viewports[0].width = static_cast<float>(renderer->display->swapchainExtent.width);
    descriptor.viewports[0].height = static_cast<float>(renderer->display->swapchainExtent.height);

    descriptor.scissors[0].extent = renderer->display->swapchainExtent;

    descriptor.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

    // TODO (nic) The user shouldn't need to set these (or be able to?) These depend on what is being rendered to, so should be populated based on the camera used
    descriptor.colorAttachment.format = renderer->display->swapchainImageFormat;
    descriptor.depthAttachment.format = VulkanImage::findDepthFormat(renderer->gpu.physicalDevice);

    //	const std::vector<Vertex> vertices = {
    //			{{-0.5F, -0.5F, 0.0F}, {0.0F, 0.0F, 0.0F}},
    //			{{0.5F, -0.5F, 0.0F}, {0.0F, 1.0F, 0.0F}},
    //			{{0.5F, 0.5F, 0.0F}, {1.0F, 1.0F, 0.0F}},
    //			{{-0.5F, 0.5F, 0.0F}, {1.0F, 0.0F, 0.0F}}
    //	};
    //
    //	descriptor.vertexData = vertices;
    //	const std::vector<uint16_t> indices = {
    //			0, 1, 2, 2, 3, 0
    //	};
    //	descriptor.indexData = indices;

    //	const std::vector<Vertex> vertices = {
    //			{{-0.5F, -0.5F, -0.5F}, {0.0F, 0.0F, 0.0F}},
    //			{{-0.5F, -0.5F, 0.5F}, {0.0F, 0.0F, 1.0F}},
    //			{{-0.5F, 0.5F, -0.5F}, {0.0F, 1.0F, 0.0F}},
    //			{{-0.5F, 0.5F, 0.5F}, {0.0F, 1.0F, 1.0F}},
    //			{{0.5F, -0.5F, -0.5F}, {1.0F, 0.0F, 0.0F}},
    //			{{0.5F, -0.5F, 0.5F}, {1.0F, 0.0F, 1.0F}},
    //			{{0.5F, 0.5F, -0.5F}, {1.0F, 1.0F, 0.0F}},
    //			{{0.5F, 0.5F, 0.5F}, {1.0F, 1.0F, 1.0F}},
    //	};
    //
    //	const std::vector<uint16_t> indices =  {
    //			0, 1, 2,
    //			3, 2, 1,
    //			1, 5, 3,
    //			7, 3, 5,
    //			5, 4, 7,
    //			6, 7, 4,
    //			4, 0, 6,
    //			2, 6, 0,
    //			0, 4, 1,
    //			5, 1, 4,
    //			2, 3, 6,
    //			7, 6, 3
    //	};

    const std::vector<Vertex> vertices = {
        { { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        { { 0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
        { { 0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
        { { -0.5f, 0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f } },

        { { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
        { { 0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
        { { 0.5f, 0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f } },
        { { -0.5f, 0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f } }
    };

    const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4
    };
    descriptor.vertexData = vertices;
    descriptor.indexData = indices;
    {
        // std::cout << "About to try taking renderMutex\n";
        std::scoped_lock lock(renderer->renderMutex);
        // std::cout << "Took renderMutex\n";
        renderer->scenes.emplace_back(*renderer.get(), 800, 600);
        // renderer->scenes[0].cameras.emplace_back(*renderer.get());
        renderer->scenes[0].drawables.emplace_back(*renderer.get(), renderer->commandPool, descriptor);

        //	    for (uint32_t imageIndex = 0; imageIndex < renderer->display->surfaceCapabilities.minImageCount; imageIndex++)
        //	    {
        //	    	std::ranges::for_each(renderer->scenes[0].drawables, [imageIndex](Drawable& drawable){drawable.Render(imageIndex);});
        //	    }

        // renderer->scenes[0].drawables[0].render()
        // std::cout << "Set up scene successfully\n";
    }
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
