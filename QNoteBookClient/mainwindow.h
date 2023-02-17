#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QMenu>
#include <QAction>
#include <QDebug>
#include <QFileDialog>
#include <QFile>
#include <QVector>
#include <QLabel>
#include <QTcpSocket>
#include <QHostAddress>
#include <QPushButton>
#include "form.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
//signals:
//    void rename_request();

public:
    QTextEdit *text;
    QLabel *label_filename;
    QFile *file;
    const char *encoding;
    QTextStream *in;
    QTcpSocket *cfd;

    Form *w;
    explicit MainWindow(QWidget *parent = 0);

    void saveFile();
    void openFile();
    void saveFileOnCloud();
    QVector<QString> getCloudFileList();
    void getCloudFile();
    ~MainWindow();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
