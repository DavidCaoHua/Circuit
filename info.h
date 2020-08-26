#ifndef INFO_H
#define INFO_H

#include <QWidget>
#include <clickedlabel.h>
#include <qlistwidget.h>

namespace Ui {
class Info;
}

class Info : public QWidget
{
    Q_OBJECT

public:
    explicit Info(QWidget *parent = nullptr);
    ~Info();
    void refresh_info(QJsonObject module_json);
    void  update_link_dots();
    void setColor(QColor c);
    QString name;
    QPixmap pixmap;
    QList<QString>  link_dots;
    QColor color;

private:
    Ui::Info *ui;

signals:
    void update_cfg();
private slots:

    void label_clicked(ClickedLabel * label);
//    void on_pushButton_add_new_module_clicked();
    void on_pushButton_add_xy_clicked();
    void on_listWidget_xy_itemDoubleClicked(QListWidgetItem *item);
    void on_textEdit_info_name_textChanged();
    void on_pushButton_clicked();
};

#endif // INFO_H
