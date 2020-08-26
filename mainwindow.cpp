#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QHBoxLayout>
#include <QMessageBox>
#include <QCheckBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mcad_widget = new MCadWidget(this);
    setCentralWidget(mcad_widget);
    connect(mcad_widget,SIGNAL(displayHint(QString)),this,SLOT(slot_hint(QString)));
    connect(mcad_widget,SIGNAL(displaySPF(QString)),this,SLOT(slot_spf(QString)));
    connect(mcad_widget,SIGNAL(refresh_combox()),this,SLOT(refresh_combox()));


    labelHint = new QLabel("选择");
    labelSPF = new QLabel;
    module_edit = new QToolButton();
    module_edit->setText("编辑");

    module_combo = new QComboBox();
    ui->mainToolBar->addWidget(module_combo);
    ui->mainToolBar->addWidget(module_edit);

    ui->statusBar->addWidget(labelHint);
    ui->statusBar->addPermanentWidget(new QLabel(""));
    ui->statusBar->addPermanentWidget(labelSPF);

    labelHint->setStyleSheet("color:red");

    QActionGroup* groupEngine = new QActionGroup(this);
    groupEngine->addAction(ui->actionUseStupid);
    groupEngine->addAction(ui->actionUseQtGui);
    groupEngine->addAction(ui->actionUseOpenGL);
    groupEngine->setExclusive(true);

    QActionGroup* groupPlace = new QActionGroup(this);
    groupPlace->addAction(ui->actionPlaceLine);
//    groupPlace->addAction(ui->actionPlaceCircle);
    groupPlace->addAction(ui->actionPlaceRect);
    groupPlace->setExclusive(true);
    connect(groupPlace,SIGNAL(triggered(QAction*)),mcad_widget,SLOT(startNewCommand(QAction*)));

    QActionGroup* groupOther = new QActionGroup(this);
    groupOther->addAction(ui->actionDelete);
    groupOther->addAction(ui->action_save_as);
//    groupOther->addAction(ui->actionKoch);
//    groupOther->addAction(ui->actionCone);
//    groupOther->addAction(ui->actionPodetium);
    groupOther->addAction(ui->actionSelect);
    groupOther->addAction(ui->action_save);
    groupOther->addAction(ui->action_read);

    connect(groupOther,SIGNAL(triggered(QAction*)),mcad_widget,SLOT(startNewCommand(QAction*)));

//    connect(ui->menuColor,SIGNAL(triggered(QAction*)),mcad_widget,SLOT(setColorOption(QAction*)));

    connect(module_edit,SIGNAL(clicked()),mcad_widget,SLOT(addModule()));

    connect(module_combo,SIGNAL(currentIndexChanged(int)),mcad_widget,SLOT(module_changed(int)));


    refresh_combox();

    setWindowTitle("Circuit");

//    label_img = new ClickedLabel(this);

//    QCheckBox* blankingBox = new QCheckBox("开启消隐",this);
//    ui->statusBar->addWidget(blankingBox);

//    connect(blankingBox,SIGNAL(toggled(bool)),mcad_widget,SLOT(setUseBlanking(bool)));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionUseStupid_triggered()
{
    mcad_widget->setEngineType(MCadWidget::Stupid);
}

void MainWindow::on_actionUseQtGui_triggered()
{
    mcad_widget->setEngineType(MCadWidget::QtGUI);
}

void MainWindow::on_actionUseOpenGL_triggered()
{
    mcad_widget->setEngineType(MCadWidget::OpenGL);
}

void MainWindow::slot_hint(QString s)
{
    labelHint->setText(s);
}

void MainWindow::slot_spf(QString s)
{
//    QString s = QString("%1").arg(n/1000, 5, 10, QChar(' '));
    labelSPF->setText(s);
}

void MainWindow::refresh_combox()
{
    module_combo->clear();
    QJsonObject cfg_json = mcad_widget->get_cfg_json();
//    qDebug()<<"cfg_json:"<<cfg_json<<endl;
    for(auto name: cfg_json.keys())
    {
        QJsonObject tmp = cfg_json.value(name).toObject();

        qDebug()<<"name:"<<name<<" "<<QString::number(tmp.value("id").toInt())<<endl;

        QIcon p(CircuitTable::img_dir+"/c"+QString::number(tmp.value("id").toInt())+".png");
        module_combo->addItem(p,tmp.value("name").toString());
    }
}

void MainWindow::on_actionAbout_triggered()
{


    QMessageBox::about(this,"","");

}

void MainWindow::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this);
}
