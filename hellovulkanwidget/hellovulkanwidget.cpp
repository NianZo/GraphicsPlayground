#include "hellovulkanwidget.h"
#include "ui_MainGUI.h"

MainWindow::MainWindow(QWindow *w) :
    //: m_window(w)
	ui(new Ui::Form)
{
	ui->setupUi(this);

    QWidget* wrapper = QWidget::createWindowContainer(w, ui->widget);
    wrapper->setMinimumSize(ui->widget->size());

    m_window = wrapper;
	//ui->widget = QWidget::createWindowContainer(w);
//
//    m_info = new QPlainTextEdit;
//    m_info->setReadOnly(true);
//
//    m_number = new QLCDNumber(3);
//    m_number->setSegmentStyle(QLCDNumber::Filled);
//
//    QPushButton *grabButton = new QPushButton(tr("&Grab"));
//    grabButton->setFocusPolicy(Qt::NoFocus);
//
//    connect(grabButton, &QPushButton::clicked, this, &MainWindow::onGrabRequested);
//
//    QPushButton *quitButton = new QPushButton(tr("&Quit"));
//    quitButton->setFocusPolicy(Qt::NoFocus);
//
//    connect(quitButton, &QPushButton::clicked, qApp, &QCoreApplication::quit);
//
//    QVBoxLayout *layout = new QVBoxLayout;
//    m_infoTab = new QTabWidget(this);
//    m_infoTab->addTab(m_info, tr("Vulkan Info"));
//    m_infoTab->addTab(logWidget, tr("Debug Log"));
//    layout->addWidget(m_infoTab, 2);
//    layout->addWidget(m_number, 1);
//    layout->addWidget(wrapper, 5);
//    layout->addWidget(grabButton, 1);
//    layout->addWidget(quitButton, 1);
//    setLayout(layout);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
	m_window->setMinimumSize(ui->widget->size());
}


