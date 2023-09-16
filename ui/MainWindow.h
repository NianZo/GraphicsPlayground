//#include "VulkanApplication.hpp"
#include <QMainWindow>
#include <QVulkanInstance>
#include <QWidget>
#include <QWindow>
#include <QMenu>
#include <QToolBar>
#include <filesystem>
#include <memory>
#include "RendererBase.hpp"
#include "VulkanRenderer.hpp"
#include "Drawable.hpp"

//QT_BEGIN_NAMESPACE
class QTabWidget;
class QPlainTextEdit;
class QLCDNumber;
//QT_END_NAMESPACE

namespace Ui
{
	class Form;
} // namespace Ui

class MainWindow : public QMainWindow
{
//    Q_OBJECT

public:
    explicit MainWindow();
    ~MainWindow() override;
    MainWindow(const MainWindow&) = delete;
    MainWindow(MainWindow&&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;
    MainWindow& operator=(MainWindow&&) = delete;

	void resizeEvent(QResizeEvent* event) override;
	//static void renderLoop(VulkanApplication* appObj);

	//VulkanApplication* appObj;

	RendererBase rendererBase;
	std::unique_ptr<VulkanRenderer> renderer;

private:
	VkSurfaceKHR surface;
    QWidget *m_windowWrapper;
    QWidget formWidget;
    QVulkanInstance inst;
    std::unique_ptr<QWindow> m_window;
    //QWidget *wrapper;
    //QTabWidget *m_infoTab;
    //QPlainTextEdit *m_info;
    //QLCDNumber *m_number;
    QMenu viewMenu;
    QToolBar *toolBar;
    std::filesystem::path projectDirectory;
    Ui::Form* ui;

    GraphicsPipelineDescriptor pipelineDescriptor;

    void gpuComboBoxSelection(int index);
    void polygonModeComboBox(int index);
    void vertexShaderTextChanged();
    void fragmentShaderTextChanged();
    void graphicsDescriptorView();
    void setRenderView();
    void createRenderer();
};

class VulkanPhysicalDeviceFeatureWrapper
{
public:
	static constexpr std::array<const char*, 55> featureNames =
		{{"robustBufferAccess",
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
	      "inheritedQueries"}};
	std::array<VkBool32, 55> featureArray;

	VulkanPhysicalDeviceFeatureWrapper(const VkPhysicalDeviceFeatures& features)
	{
		featureArray[robustBufferAccessIndex()] = features.robustBufferAccess;
		featureArray[fullDrawIndexUint32Index()] = features.fullDrawIndexUint32;
		featureArray[imageCubeArrayIndex()] = features.imageCubeArray;
		featureArray[independentBlendIndex()] = features.independentBlend;
		featureArray[geometryShaderIndex()] = features.geometryShader;
		featureArray[tessellationShaderIndex()] = features.tessellationShader;
		featureArray[sampleRateShadingIndex()] = features.sampleRateShading;
		featureArray[dualSrcBlendIndex()] = features.dualSrcBlend;
		featureArray[logicOpIndex()] = features.logicOp;
		featureArray[multiDrawIndirectIndex()] = features.multiDrawIndirect;
		featureArray[drawIndirectFirstInstanceIndex()] = features.drawIndirectFirstInstance;
		featureArray[depthClampIndex()] = features.depthClamp;
		featureArray[depthBiasClampIndex()] = features.depthBiasClamp;
		featureArray[fillModeNonSolidIndex()] = features.fillModeNonSolid;
		featureArray[depthBoundsIndex()] = features.depthBounds;
		featureArray[wideLinesIndex()] = features.wideLines;
		featureArray[largePointsIndex()] = features.largePoints;
		featureArray[alphaToOneIndex()] = features.alphaToOne;
		featureArray[multiViewportIndex()] = features.multiViewport;
		featureArray[samplerAnisotropyIndex()] = features.samplerAnisotropy;
		featureArray[textureCompressionETC2Index()] = features.textureCompressionETC2;
		featureArray[textureCompressionASTC_LDRIndex()] = features.textureCompressionASTC_LDR;
		featureArray[textureCompressionBCIndex()] = features.textureCompressionBC;
		featureArray[occlusionQueryPreciseIndex()] = features.occlusionQueryPrecise;
		featureArray[pipelineStatisticsQueryIndex()] = features.pipelineStatisticsQuery;
		featureArray[vertexPipelineStoresAndAtomicsIndex()] = features.vertexPipelineStoresAndAtomics;
		featureArray[fragmentStoresAndAtomicsIndex()] = features.fragmentStoresAndAtomics;
		featureArray[shaderTessellationAndGeometryPointSizeIndex()] = features.shaderTessellationAndGeometryPointSize;
		featureArray[shaderImageGatherExtendedIndex()] = features.shaderImageGatherExtended;
		featureArray[shaderStorageImageExtendedFormatsIndex()] = features.shaderStorageImageExtendedFormats;
		featureArray[shaderStorageImageMultisampleIndex()] = features.shaderStorageImageMultisample;
		featureArray[shaderStorageImageReadWithoutFormatIndex()] = features.shaderStorageImageReadWithoutFormat;
		featureArray[shaderStorageImageWriteWithoutFormatIndex()] = features.shaderStorageImageWriteWithoutFormat;
		featureArray[shaderUniformBufferArrayDynamicIndexingIndex()] = features.shaderUniformBufferArrayDynamicIndexing;
		featureArray[shaderSampledImageArrayDynamicIndexingIndex()] = features.shaderSampledImageArrayDynamicIndexing;
		featureArray[shaderStorageBufferArrayDynamicIndexingIndex()] = features.shaderStorageBufferArrayDynamicIndexing;
		featureArray[shaderStorageImageArrayDynamicIndexingIndex()] = features.shaderStorageImageArrayDynamicIndexing;
		featureArray[shaderClipDistanceIndex()] = features.shaderClipDistance;
		featureArray[shaderCullDistanceIndex()] = features.shaderCullDistance;
		featureArray[shaderFloat64Index()] = features.shaderFloat64;
		featureArray[shaderInt64Index()] = features.shaderInt64;
		featureArray[shaderInt16Index()] = features.shaderInt16;
		featureArray[shaderResourceResidencyIndex()] = features.shaderResourceResidency;
		featureArray[shaderResourceMinLodIndex()] = features.shaderResourceMinLod;
		featureArray[sparseBindingIndex()] = features.sparseBinding;
		featureArray[sparseResidencyBufferIndex()] = features.sparseResidencyBuffer;
		featureArray[sparseResidencyImage2DIndex()] = features.sparseResidencyImage2D;
		featureArray[sparseResidencyImage3DIndex()] = features.sparseResidencyImage3D;
		featureArray[sparseResidency2SamplesIndex()] = features.sparseResidency2Samples;
		featureArray[sparseResidency4SamplesIndex()] = features.sparseResidency4Samples;
		featureArray[sparseResidency8SamplesIndex()] = features.sparseResidency8Samples;
		featureArray[sparseResidency16SamplesIndex()] = features.sparseResidency16Samples;
		featureArray[sparseResidencyAliasedIndex()] = features.sparseResidencyAliased;
		featureArray[variableMultisampleRateIndex()] = features.variableMultisampleRate;
		featureArray[inheritedQueriesIndex()] = features.inheritedQueries;
	}

	static uint32_t robustBufferAccessIndex()
	{
		return 0;
	}

	static uint32_t fullDrawIndexUint32Index()
	{
		return 1;
	}

	static uint32_t imageCubeArrayIndex()
	{
		return 2;
	}

	static uint32_t independentBlendIndex()
	{
		return 3;
	}

	static uint32_t geometryShaderIndex()
	{
		return 4;
	}

	static uint32_t tessellationShaderIndex()
	{
		return 5;
	}

	static uint32_t sampleRateShadingIndex()
	{
		return 6;
	}

	static uint32_t dualSrcBlendIndex()
	{
		return 7;
	}

	static uint32_t logicOpIndex()
	{
		return 8;
	}

	static uint32_t multiDrawIndirectIndex()
	{
		return 9;
	}

	static uint32_t drawIndirectFirstInstanceIndex()
	{
		return 10;
	}

	static uint32_t depthClampIndex()
	{
		return 11;
	}

	static uint32_t depthBiasClampIndex()
	{
		return 12;
	}

	static uint32_t fillModeNonSolidIndex()
	{
		return 13;
	}

	static uint32_t depthBoundsIndex()
	{
		return 14;
	}

	static uint32_t wideLinesIndex()
	{
		return 15;
	}

	static uint32_t largePointsIndex()
	{
		return 16;
	}

	static uint32_t alphaToOneIndex()
	{
		return 17;
	}

	static uint32_t multiViewportIndex()
	{
		return 18;
	}

	static uint32_t samplerAnisotropyIndex()
	{
		return 19;
	}

	static uint32_t textureCompressionETC2Index()
	{
		return 20;
	}

	static uint32_t textureCompressionASTC_LDRIndex()
	{
		return 21;
	}

	static uint32_t textureCompressionBCIndex()
	{
		return 22;
	}

	static uint32_t occlusionQueryPreciseIndex()
	{
		return 23;
	}

	static uint32_t pipelineStatisticsQueryIndex()
	{
		return 24;
	}

	static uint32_t vertexPipelineStoresAndAtomicsIndex()
	{
		return 25;
	}

	static uint32_t fragmentStoresAndAtomicsIndex()
	{
		return 26;
	}

	static uint32_t shaderTessellationAndGeometryPointSizeIndex()
	{
		return 27;
	}

	static uint32_t shaderImageGatherExtendedIndex()
	{
		return 28;
	}

	static uint32_t shaderStorageImageExtendedFormatsIndex()
	{
		return 29;
	}

	static uint32_t shaderStorageImageMultisampleIndex()
	{
		return 30;
	}

	static uint32_t shaderStorageImageReadWithoutFormatIndex()
	{
		return 31;
	}

	static uint32_t shaderStorageImageWriteWithoutFormatIndex()
	{
		return 32;
	}

	static uint32_t shaderUniformBufferArrayDynamicIndexingIndex()
	{
		return 33;
	}

	static uint32_t shaderSampledImageArrayDynamicIndexingIndex()
	{
		return 34;
	}

	static uint32_t shaderStorageBufferArrayDynamicIndexingIndex()
	{
		return 35;
	}

	static uint32_t shaderStorageImageArrayDynamicIndexingIndex()
	{
		return 36;
	}

	static uint32_t shaderClipDistanceIndex()
	{
		return 37;
	}

	static uint32_t shaderCullDistanceIndex()
	{
		return 38;
	}

	static uint32_t shaderFloat64Index()
	{
		return 39;
	}

	static uint32_t shaderInt64Index()
	{
		return 40;
	}

	static uint32_t shaderInt16Index()
	{
		return 41;
	}

	static uint32_t shaderResourceResidencyIndex()
	{
		return 42;
	}

	static uint32_t shaderResourceMinLodIndex()
	{
		return 43;
	}

	static uint32_t sparseBindingIndex()
	{
		return 44;
	}

	static uint32_t sparseResidencyBufferIndex()
	{
		return 45;
	}

	static uint32_t sparseResidencyImage2DIndex()
	{
		return 46;
	}

	static uint32_t sparseResidencyImage3DIndex()
	{
		return 47;
	}

	static uint32_t sparseResidency2SamplesIndex()
	{
		return 48;
	}

	static uint32_t sparseResidency4SamplesIndex()
	{
		return 49;
	}

	static uint32_t sparseResidency8SamplesIndex()
	{
		return 50;
	}

	static uint32_t sparseResidency16SamplesIndex()
	{
		return 51;
	}

	static uint32_t sparseResidencyAliasedIndex()
	{
		return 52;
	}

	static uint32_t variableMultisampleRateIndex()
	{
		return 53;
	}

	static uint32_t inheritedQueriesIndex()
	{
		return 54;
	}
};

//VulkanPhysicalDeviceFeatureWrapper::featureNames = //std::array<QString, 55>
//{{
//			"robustBufferAccess",
//			"fullDrawIndexUint32",
//			"imageCubeArray",
//			"independentBlend",
//			"geometryShader",
//			"tessellationShader",
//			"sampleRateShading",
//			"dualSrcBlend",
//			"logicOp",
//			"multiDrawIndirect",
//			"drawIndirectFirstInstance",
//			"depthClamp",
//			"depthBiasClamp",
//			"fillModeNonSolid",
//			"depthBounds",
//			"wideLines",
//			"largePoints",
//			"alphaToOne",
//			"multiViewport",
//			"samplerAnisotropy",
//			"textureCompressionETC2",
//			"textureCompressionASTC_LDR",
//			"textureCompressionBC",
//			"occlusionQueryPrecise",
//			"pipelineStatisticsQuery",
//			"vertexPipelineStoresAndAtomics",
//			"fragmentStoresAndAtomics",
//			"shaderTessellationAndGeometryPointSize",
//			"shaderImageGatherExtended",
//			"shaderStorageImageExtendedFormats",
//			"shaderStorageImageMultisample",
//			"shaderStorageImageReadWithoutFormat",
//			"shaderStorageImageWriteWithoutFormat",
//			"shaderUniformBufferArrayDynamicIndexing",
//			"shaderSampledImageArrayDynamicIndexing",
//			"shaderStorageBufferArrayDynamicIndexing",
//			"shaderStorageImageArrayDynamicIndexing",
//			"shaderClipDistance",
//			"shaderCullDistance",
//			"shaderFloat64",
//			"shaderInt64",
//			"shaderInt16",
//			"shaderResourceResidency",
//			"shaderResourceMinLod",
//			"sparseBinding",
//			"sparseResidencyBuffer",
//			"sparseResidencyImage2D",
//			"sparseResidencyImage3D",
//			"sparseResidency2Samples",
//			"sparseResidency4Samples",
//			"sparseResidency8Samples",
//			"sparseResidency16Samples",
//			"sparseResidencyAliased",
//			"variableMultisampleRate",
//			"inheritedQueries"
//}};





