#include "form.h"
#include "ui_form.h"

Form::Form(QString old,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    flag=1;
    ui->setupUi(this);
    setWindowTitle("重命名");
    this->name=old;
    ui->outnameEdit->setText(old);
    connect(ui->ok_btn,&QPushButton::clicked,this,[=](){
        this->name=ui->newnameEdit->text();
        emit rename_respond(this->name);
        this->close();
    });
    connect(ui->cancel_btn,&QPushButton::clicked,this,[=](){
        emit rename_respond(this->name);
        flag=0;
        this->close();
    });
}

QString Form::getNewName(){
//    while(flag);
    return this->name;
}

Form::~Form()
{
    delete ui;
}
