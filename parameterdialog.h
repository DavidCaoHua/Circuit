#ifndef PARAMETERDIALOG_H
#define PARAMETERDIALOG_H

#include <QDialog>
#include <QStringListModel>
#include <qjsonobject.h>
#include <qlistwidget.h>
#include<clickedlabel.h>
namespace Ui {
class ParameterDialog;
}

class ParameterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ParameterDialog(QWidget *parent = 0);
    ~ParameterDialog();
     bool initial_ui(QJsonObject cfg_json);
     void refresh_info(QJsonObject module_json);
     void refresh_cfg_list(int select_index);

    float value();
    QJsonObject get_cfg_json();

public slots:
    void update_cfg();
private slots:
    void on_listWidget_circuit_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_listWidget_circuit_currentRowChanged(int currentRow);
    void on_listWidget_circuit_itemDoubleClicked(QListWidgetItem *item);


    void on_pushButton_add_commad_clicked();

private:
    Ui::ParameterDialog *ui;
    QJsonObject cfg_json;
    int last_id=-10;
};

#endif // PARAMETERDIALOG_H
