#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
namespace Ui {
class Form;
}

class Form : public QWidget
{
    Q_OBJECT
signals:
    void rename_respond(QString name);
public:
    QString name;
    int flag;
    explicit Form(QString old,QWidget *parent = 0);
    ~Form();

    QString getNewName();
private:
    Ui::Form *ui;
};

#endif // FORM_H
