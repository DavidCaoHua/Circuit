#include "info.h"
#include "info.h"
#include "ui_info.h"

#include <qfiledialog.h>
#include <qjsonobject.h>
#include <xy_dialog.h>
#include<geometry.h>
#include <qcolordialog.h>
Info::Info(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Info)
{
    ui->setupUi(this);
    ui->label_img->setScaledContents(true);
    connect(ui->label_img,SIGNAL(clicked(ClickedLabel*)),this,SLOT(label_clicked(ClickedLabel *)));
    name = "Please enter the component name";
//    ui->textEdit_info_name->setText(name);
    ui->textEdit_info_name->setText(name);
    pixmap = QPixmap(CircuitTable::img_dir+"/defaut.png");
    ui->label_img->setPixmap(pixmap);
//    color = QColor(255,255,255);

}

Info::~Info()
{
    delete ui;
}

void Info::label_clicked(ClickedLabel * label)
{
    QString file_name = QFileDialog::getOpenFileName(this,
            tr("选择图片"),
            "",
            tr("Json Files (*.png)"),
            0);

    if(!file_name.isNull())
    {
        QPixmap img(file_name);

        ui->label_img->setPixmap(img);

        pixmap =img;


    }
    else
    {
//        emit displayHint("选择失败");
    }
}

void Info::setColor(QColor c)
{
    color = c;
    ui->pushButton->setStyleSheet("background: rgb("
                                  +QString::number(c.red())+","\
                                  +QString::number(c.green())+","\
                                  +QString::number(c.blue())+")"
                                                   );

}

void Info::update_link_dots()
{
    link_dots.clear();
    for(int n=0;n<ui->listWidget_xy->count();++n)
    {
        QString q= ui->listWidget_xy->item(n)->text();
        link_dots.append(q);
    }
}
void Info::refresh_info(QJsonObject module_json)
{

    int class_id = module_json.value("id").toInt();
    QString name = module_json.value("name").toString();
    QPixmap p(CircuitTable::img_dir+"/c"+QString::number(class_id)+".png");

    ui->listWidget_xy->clear();
    int link_dot_size = module_json.value("link_dot_size").toInt();
    for(int n=0;n<link_dot_size;++n)
    {
        double x_p = module_json[("link_dot_"+std::to_string(n)+"x").c_str()].toDouble();
        double y_p = module_json[("link_dot_"+std::to_string(n)+"y").c_str()].toDouble();
        ui->listWidget_xy->addItem("x="
                                   +QString::number(x_p)+";y="
                                   +QString::number(y_p));
    }

    color = QColor(module_json["r"].toInt(),module_json["g"].toInt(),module_json["b"].toInt());

    ui->label_img->setPixmap(p);
    ui->textEdit_info_name->setText(name);
    setColor(color);
}


//void Info::on_pushButton_add_new_module_clicked()
//{
//    qDebug()
//}

void Info::on_pushButton_add_xy_clicked()
{
    double x = ui->doubleSpinBox_x->value();
    double y = ui->doubleSpinBox_y->value();
    ui->listWidget_xy->addItem("x="
                               +QString::number(x)+";y="
                               +QString::number(y));
    update_link_dots();
}

void Info::on_listWidget_xy_itemDoubleClicked(QListWidgetItem *item)
{
    XY_Dialog *myDlg = new XY_Dialog();
    auto split_tmp = item->text().split(";");
    myDlg->set_xy(split_tmp[0].replace("x=","").toDouble(),
                  split_tmp[1].replace("y=","").toDouble());
    int res = myDlg->exec();
    if(res==1)
    {
        double x = myDlg->get_x();
        double y = myDlg->get_y();

        item->setText("x="
                      +QString::number(x)+";y="
                      +QString::number(y));

    }
    else
    {

        ui->listWidget_xy->removeItemWidget(item);
        delete item;
    }

    update_link_dots();
    myDlg->close();
    delete myDlg;
}

void Info::on_textEdit_info_name_textChanged()
{
   name = ui->textEdit_info_name->toPlainText();
//   if(name.length()>0)
//    emit update_cfg();
}

void Info::on_pushButton_clicked()
{
    QColorDialog dlg;

    dlg.setWindowTitle("选择元件背景颜色");
    dlg.setCurrentColor(QColor(255,255,255));
    if(dlg.exec())
    {
        setColor(dlg.currentColor());
    }

//       emit update_cfg();

}
