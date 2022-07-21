

#include "hellovulkanwidget.h"
#include <QVulkanFunctions>
#include <QApplication>
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLCDNumber>
#include <QFileDialog>
#include <QMessageBox>
#include <QTabWidget>
#include "ui_MainGUI.h"

MainWindow::MainWindow(QWindow *w) :
    //: m_window(w)
	ui(new Ui::Form)
{
	ui->setupUi(this);

    QWidget* wrapper = QWidget::createWindowContainer(w, ui->widget);
    wrapper->setMinimumSize(QSize(960, 540));
    //ui->verticalLayout->addWidget(wrapper);
    //ui->retranslateUi(this);

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

//void MainWindow::onVulkanInfoReceived(const QString &text)
//{
//    m_info->setPlainText(text);
//}
//
//void MainWindow::onFrameQueued(int colorValue)
//{
//    m_number->display(colorValue);
//}

//void MainWindow::onGrabRequested()
//{
////    if (!m_window->supportsGrab()) {
////        QMessageBox::warning(this, tr("Cannot grab"), tr("This swapchain does not support readbacks."));
////        return;
////    }
////
////    QImage img = m_window->grab();
////
////    // Our startNextFrame() implementation is synchronous so img is ready to be
////    // used right here.
////
////    QFileDialog fd(this);
////    fd.setAcceptMode(QFileDialog::AcceptSave);
////    fd.setDefaultSuffix("png");
////    fd.selectFile("test.png");
////    if (fd.exec() == QDialog::Accepted)
////        img.save(fd.selectedFiles().first());
//}
