//#include "VulkanApplication.hpp"
#include "VulkanRenderer.hpp"
#include <QVulkanInstance>
#include <QWidget>
#include <QWindow>
#include <memory>

//QT_BEGIN_NAMESPACE
class QTabWidget;
class QPlainTextEdit;
class QLCDNumber;
//QT_END_NAMESPACE

namespace Ui
{
	class Form;
} // namespace Ui

class MainWindow : public QWidget
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
	VulkanRenderer2 renderer;

private:
	VkSurfaceKHR surface;
    QWidget *m_windowWrapper;
    QVulkanInstance inst;
    std::unique_ptr<QWindow> m_window;
    //QWidget *wrapper;
    //QTabWidget *m_infoTab;
    //QPlainTextEdit *m_info;
    //QLCDNumber *m_number;

    Ui::Form* ui;


};






