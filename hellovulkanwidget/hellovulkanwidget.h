
#include <QWidget>
#include <QWindow>

//class VulkanWindow;
//class

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
    explicit MainWindow(QWindow *w = nullptr);

public slots:
//    void onVulkanInfoReceived(const QString &text);
//    void onFrameQueued(int colorValue);
//    void onGrabRequested();
	void resizeEvent(QResizeEvent* event);

private:
    QWidget *m_window;
    //QTabWidget *m_infoTab;
    //QPlainTextEdit *m_info;
    //QLCDNumber *m_number;

    Ui::Form* ui;


};






