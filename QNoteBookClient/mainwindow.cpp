#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    text=new QTextEdit(this);
    this->encoding="UTF-8";
    this->setWindowTitle("QNoteBook");
    this->setWindowIcon(QIcon(":/images/logo.jpg"));
    setCentralWidget(text);
    QMenu *menu=ui->menuBar->addMenu("文件");
    QAction *act=menu->addAction("save");
    QAction *act_open=menu->addAction("open");
    connect(act,&QAction::triggered,this,&MainWindow::saveFile);
    connect(act_open,&QAction::triggered,this,&MainWindow::openFile);
    QMenu *menu2=ui->menuBar->addMenu("编码");
    QAction *act_encode1=menu2->addAction("UTF-8");
    QAction *act_encode2=menu2->addAction("GBK");
    QAction *act_encode3=menu2->addAction("ASCII");
    connect(act_encode1,&QAction::triggered,[=](){
        file->seek(0);
        this->encoding="UTF-8";
        in->setCodec(this->encoding);
        text->setText(in->readAll());
    }
    );
    connect(act_encode2,&QAction::triggered,[=](){
        file->seek(0);
        this->encoding="GBK";
        in->setCodec(this->encoding);
        text->setText(in->readAll());
    }
    );
    connect(act_encode3,&QAction::triggered,[=](){
        file->seek(0);
        this->encoding="Latin1";
        in->setCodec(this->encoding);
        text->setText(in->readAll());
    }
    );
    QMenu *menu3=ui->menuBar->addMenu("云文件");
    QAction *act_save2=menu3->addAction("save");
    QAction *act_open2=menu3->addAction("open");
    connect(act_save2,&QAction::triggered,[=](){
        qDebug()<<"cloud save\n";
        this->cfd=new QTcpSocket(this);
        cfd->connectToHost(QHostAddress("192.168.127.2"),8086);
        connect(this->cfd,&QTcpSocket::connected,this,[=](){

            qDebug()<<"sucess conect to server";
            //请求头
            QString filename=this->label_filename->text();
            QString headstr=tr("save %1 100B cloud \n").arg(filename);

            char *head=headstr.toLocal8Bit().data();

            cfd->write(head);
            cfd->write("\n");
            //请求体
            char buf[4096] = {0};
            QString text_str=this->text->toPlainText();
            char *text_data=text_str.toLocal8Bit().data();
            cfd->write(text_data,sizeof(buf));
            cfd->write("\n");
            qDebug()<<"file1保存成功\n";
            cfd->close();
        });

    });
    connect(act_open2,&QAction::triggered,[=](){
        qDebug()<<"cloud get\n";
        this->cfd=new QTcpSocket(this);
        cfd->connectToHost(QHostAddress("192.168.127.2"),8086);
        connect(this->cfd,&QTcpSocket::connected,this,[=](){

            qDebug()<<"sucess conect to server";
            //请求头
            QString filename=this->label_filename->text();
            QString headstr=tr("get %1 100B cloud \n").arg(filename);

            char *head=headstr.toLocal8Bit().data();

            cfd->write(head);
            cfd->write("\n");
            //响应头
            connect(cfd,&QTcpSocket::readyRead,this,[=](){
                char buf[4096] = {0};
                char pro[16], no[256], desc[16];
                memset(buf,'0',sizeof(buf));
                int ret=cfd->readLine(buf,sizeof(buf));
                // "Cloud/1.1 %d %s\r\n"
                sscanf(buf, "%[^ ] %[^ ] %[^ ]",pro, no, desc);
                qDebug()<<"protocol:"<<pro<<" num:"<<no<<" desc:"<<desc<<endl;
                while(cfd->readLine(buf,sizeof(buf))>0){
                    qDebug()<<"buf:"<<QString(buf)<<endl;
//                    qDebug()<<tr("cmp:%1").arg(strcmp(buf,"\r\n"));
                    if(strcmp(buf,"\r\n")==0)   break;
                    memset(buf,0,sizeof(buf));
                }
                this->text->setText("");
                memset(buf,0,sizeof(buf));
                while(cfd->read(buf,sizeof(buf))>0){
                     qDebug()<<"buf_text:"<<QString(buf)<<endl;
                    this->text->append(QString(buf));
                }
                cfd->close();

            });

//            //分割标志
//            while (1) {
//                 char buf[4096] = {0};
//                 int len=cfd->readLine(buf,sizeof(buf));
//                 if (buf[0] == '\n') break;
//                 else if (len == -1) break;
//            }


//            //响应体
//            this->text->setText("");
//            while(1){
//                memset(buf,'0',sizeof(buf));
//                int n=cfd->read(buf,sizeof(buf));
//                if(n==0){
//                    qDebug()<<"read from cloud finished\n";
//                    cfd->close();
//                    break;
//                }else if(n==-1){
//                    perror("read from cloud error\n");
//                }else{
//                    this->text->append(QString(buf));
//                }
//            }
        });
    });
    label_filename=new QLabel(QString("untitle.txt"),this);
    ui->mainToolBar->addWidget(label_filename);
    QLabel *label=new QLabel(tr("endcode:%1").arg(this->encoding));
    ui->statusBar->addWidget(label);

    QMenu *menu4=ui->menuBar->addMenu("编辑");
    QAction *act_rename=menu4->addAction("rename");

    connect(act_rename,&QAction::triggered,this,[=](){
        w=new Form(this->label_filename->text());
        connect(w,&Form::rename_respond,this,[=](QString newName){
            this->label_filename->setText(newName);
        });
        w->show();
//        emit rename_request();
//        this->label_filename->setText();


    });

    QPushButton *btn_big=new QPushButton(QIcon(":/images/big.jpg"),"放大",this);
    QPushButton *btn_s=new QPushButton(QIcon(":/images/small.jpg"),"缩小",this);
    ui->statusBar->addWidget(btn_big);
    ui->statusBar->addWidget(btn_s);
//    text->fontMetrics();
    connect(btn_big,&QPushButton::clicked,this,[=](){
        qDebug()<<"big"<<endl;
        QString family=this->text->font().family();
        int size=this->text->font().pointSize()+1;
        this->text->setFont(QFont(family,size));
    });

    connect(btn_s,&QPushButton::clicked,this,[=](){
        qDebug()<<"small"<<endl;
        QString family=this->text->font().family();
        int size=this->text->font().pointSize()-1;
        this->text->setFont(QFont(family,size));
    });

}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::saveFile(){
    QString filename=QFileDialog::getSaveFileName(this,tr("saveFile"),"./untitle.txt",tr("Text files (*.txt)"));
    qDebug()<<tr("save filename:%1").arg(filename);
    this->file->close();
    file=new QFile(filename);
    file->open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(file);
    out.setCodec(this->encoding);
    out<<this->text->toPlainText();
    file->close();

}
void MainWindow::openFile(){
    QString filename=QFileDialog::getOpenFileName(this,tr("openFile"),".",tr("Text files (*.txt)"));
    int index_r=filename.lastIndexOf('/');
    this->label_filename->setText(filename.mid(index_r+1));
    qDebug()<<tr("open filename:%1").arg(filename);
    this->file=new QFile(filename);
    this->file->open(QIODevice::ReadWrite|QIODevice::Text);
    this->in=new QTextStream(file);
    in->setCodec("UTF-8");
    text->setText(in->readAll());
//    file->close();
}
