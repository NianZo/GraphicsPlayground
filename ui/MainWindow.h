#include <QWidget>
#include <QWindow>
#include "VulkanApplication.hpp"

//QT_BEGIN_NAMESPACE
class QTabWidget;
class QPlainTextEdit;
class QLCDNumber;
//QT_END_NAMESPACE

namespace Ui
{
	class Form;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow();

public:
//    void onVulkanInfoReceived(const QString &text);
//    void onFrameQueued(int colorValue);
//    void onGrabRequested();
	void resizeEvent(QResizeEvent* event);
	static void renderLoop(VulkanApplication* appObj);

	VulkanApplication* appObj;

private:
    QWidget *m_window;
    //QTabWidget *m_infoTab;
    //QPlainTextEdit *m_info;
    //QLCDNumber *m_number;

    Ui::Form* ui;


};






