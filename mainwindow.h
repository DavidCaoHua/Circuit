#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include<QComboBox>
#include<QToolButton>
#include "mainwidget.h"
#include "clickedlabel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionUseStupid_triggered();
    void on_actionUseQtGui_triggered();
    void on_actionUseOpenGL_triggered();
    void on_actionAbout_triggered();
    void on_actionAboutQt_triggered();

    void slot_hint(QString s);
    void slot_spf(QString s);
    void refresh_combox();

private:
    Ui::MainWindow *ui;
    MCadWidget* mcad_widget;
    QLabel* labelHint;
    QLabel* labelSPF;
    QToolButton* module_edit;
    QComboBox *module_combo;
//    ClickedLabel * label_img;

};

#endif // MAINWINDOW_H
