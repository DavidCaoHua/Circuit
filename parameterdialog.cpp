#include "parameterdialog.h"
#include "ui_parameterdialog.h"
#include <QJsonObject>
#include<QDebug>
#include <qdir.h>
#include <qfiledialog.h>
#include <xy_dialog.h>
#include <qmessagebox.h>
#include <geometry.h>
ParameterDialog::ParameterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ParameterDialog)
{
    ui->setupUi(this);
//    connect(ui->info,SIGNAL(update_cfg()),this,SLOT(update_cfg()));
}

ParameterDialog::~ParameterDialog()
{
    delete ui;
}


void ParameterDialog::update_cfg()
{
    int id = last_id+1;

    if(id>0)
    {
//        auto tmp = cfg_json[QString::number(id)].toObject();
        QJsonObject tmp;

        tmp.insert("id",id);
        tmp.insert("name",ui->info->name);
        tmp.insert("r",ui->info->color.red());
        tmp.insert("g",ui->info->color.green());
        tmp.insert("b",ui->info->color.blue());

         qDebug()<<ui->info->link_dots.size()<<endl;
        if(ui->info->link_dots.size()>0)
        {
            tmp.insert("link_dot_size",ui->info->link_dots.size());
            for(int n=0;n<ui->info->link_dots.size();++n)
            {
                QString q= ui->info->link_dots[n];
                auto split_tmp = q.split(";");

                tmp.insert(("link_dot_"+QString::number(n)+"x"),split_tmp[0].replace("x=","").toDouble());
                tmp.insert(("link_dot_"+QString::number(n)+"y"),split_tmp[1].replace("y=","").toDouble());
            }

        }
        else
        {
            auto old_cfg = cfg_json[QString::number(id)].toObject();
            tmp.insert("link_dot_size",old_cfg["link_dot_size"]);
            for(int n=0;n<old_cfg["link_dot_size"].toInt();++n)
            {

                tmp.insert(("link_dot_"+QString::number(n)+"x"),old_cfg["link_dot_"+QString::number(n)+"x"].toDouble());
                tmp.insert(("link_dot_"+QString::number(n)+"y"),old_cfg["link_dot_"+QString::number(n)+"y"].toDouble());
            }

        }

        cfg_json[QString::number(id)] =tmp;
        ui->info->pixmap.save(CircuitTable::img_dir+"/c"+QString::number(ui->listWidget_circuit->currentRow()+1)+".png","PNG");

//        qDebug()<<"tst:"<<cfg_json<<endl;
    }


}

QJsonObject ParameterDialog::get_cfg_json()
{
    return cfg_json;
}

void ParameterDialog::refresh_cfg_list(int select_index)
{

    ui->listWidget_circuit->disconnect(SIGNAL(currentRowChanged(int)));
    ui->listWidget_circuit->clear();

    QStringList str_list;
    for(auto name:cfg_json.keys())
    {

        QJsonObject tmp = cfg_json.value(name).toObject();

        int class_id = cfg_json.value("id").toInt();
        QIcon Icon(CircuitTable::img_dir+"/c"+QString::number(class_id)+".png");
        QListWidgetItem * IconItem = new QListWidgetItem(Icon,tmp.value("name").toString(),ui->listWidget_circuit);//通过 QListWidgetItem添加文本以及Icon数据的
        ui->listWidget_circuit->addItem(IconItem);

    }

    connect(ui->listWidget_circuit, SIGNAL(currentRowChanged(int)), this, SLOT(on_listWidget_circuit_currentRowChanged(int)));//自定义区域添加点击事件的信号槽；连接

    if(select_index == -1)
    {
        if (ui->listWidget_circuit->count()>0)
            ui->listWidget_circuit->setCurrentRow(ui->listWidget_circuit->count()-1);
    }
    else
    {
        if(select_index>=0 && select_index<ui->listWidget_circuit->count())
            ui->listWidget_circuit->setCurrentRow(select_index);

    }

}


bool ParameterDialog::initial_ui(QJsonObject cfg_json)
{
    this->cfg_json = cfg_json;

//    ui->info->refresh_info(cfg_json.value(cfg_json.keys()[0]).toObject());
    refresh_cfg_list(0);

    return true;
}



void ParameterDialog::on_listWidget_circuit_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{


}



void ParameterDialog::on_listWidget_circuit_currentRowChanged(int currentRow)
{

    if(last_id!=currentRow)
    {
        update_cfg();

        last_id =currentRow;

////        qDebug()<<"change"<<endl;

        ui->info->refresh_info(cfg_json.value(cfg_json.keys()[currentRow]).toObject());
        refresh_cfg_list(currentRow);
    }
}




void ParameterDialog::on_listWidget_circuit_itemDoubleClicked(QListWidgetItem *item)
{
    if(ui->listWidget_circuit->count()==1)
    {
        QMessageBox msgBox;
        msgBox.setText("提示");
        msgBox.setInformativeText("元件至少需要一个连接点");
        msgBox.setStandardButtons(QMessageBox::Ok );
        msgBox.setDefaultButton(QMessageBox::Ok);
        int ret = msgBox.exec();

    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText("提示");
        msgBox.setInformativeText("确定要删除吗?");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();
        if(ret == QMessageBox::Ok){
            int select_last = ui->listWidget_circuit->currentRow();
            int n =ui->listWidget_circuit->row(item);
            cfg_json.remove(cfg_json.keys()[n]);
            refresh_cfg_list(select_last-1);
        }
    }

}


void ParameterDialog::on_pushButton_add_commad_clicked()
{
    QDialog *new_module_dialog = new QDialog();
    Info * new_infow = new Info(new_module_dialog);
    new_infow->setGeometry(20,20,380,350);
    QPushButton *btn_accept = new QPushButton(new_module_dialog);
    QPushButton *btn_reject = new QPushButton(new_module_dialog);


    btn_accept->setText("确认");
    btn_reject->setText("取消");
    btn_accept->setGeometry(260,330,40,23);
    btn_reject->setGeometry(310,330,40,23);

    connect(btn_reject,&QPushButton::clicked,new_module_dialog,&QDialog::reject);
    connect(btn_accept,&QPushButton::clicked,new_module_dialog,&QDialog::accept);

    if(new_module_dialog->exec())
    {
        QJsonObject new_module;
        QString name = new_infow->name;

        new_module.insert("name",name);

        int class_id=0;
        for(auto n:cfg_json.keys())
        {
            class_id = qMax(class_id,n.toInt());
        }

        class_id = class_id+1;
        new_module.insert("id",class_id);
        new_module.insert("r",new_infow->color.red());
        new_module.insert("g",new_infow->color.green());
        new_module.insert("b",new_infow->color.blue());

        QDir dir;
        dir.mkpath(CircuitTable::img_dir);//创建多级目录，如果已存在则会返回去true
//        qDebug()<<"path:"<<CircuitTable::img_dir<<endl;

        new_infow->pixmap.save(CircuitTable::img_dir+"/c"+QString::number(class_id)+".png","PNG");

        new_module.insert("link_dot_size",new_infow->link_dots.size());

        for(int n=0;n<new_infow->link_dots.count();++n)
        {
            QString q= new_infow->link_dots[n];
            auto split_tmp = q.split(";");

            new_module.insert(("link_dot_"+QString::number(n)+"x"),split_tmp[0].replace("x=","").toDouble());
            new_module.insert(("link_dot_"+QString::number(n)+"y"),split_tmp[1].replace("y=","").toDouble());
        }

        cfg_json.insert(QString::number(class_id),new_module);
        refresh_cfg_list(-1);
    }
    else
    {

    }
}
