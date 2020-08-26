#include <QPaintEvent>
#include <QApplication>
#include <QDebug>
#include <QAction>
#include <QTimer>
#include <QMatrix4x4>
#include <QColorDialog>
#include <QFileDialog>
#include <parameterdialog.h>

#include "mainwidget.h"
#include "mcad_utils.h"

MCadWidget::MCadWidget(QWidget *parent) :
    QWidget(parent),
    m_glwidget(NULL),
    m_use_opengl(false),
    m_scale(1.0),
    m_x_rotation(0),
    m_y_rotation(0),
    m_view_vector(0,0,-1),
//    m_use_blanking(false),
    m_offset(0,0),
    m_selected(0)
{
    gene_img_dir();
    UserCommand::envInit(new CircuitList); //建立真实图形表，初始化用户命令环境

    QPixmap cursor = QPixmap(":/icons/Resource/cross.png");
    cursor.setDevicePixelRatio(5); //指针的图案太大，设置缩小五倍
    setCursor(QCursor(cursor));//设置鼠标指针

    m_paint_engines[Stupid] = new StupidPaintEngine(this);//初始化三个引擎，实时切换
    m_paint_engines[QtGUI]  = new QtPaintEngine(this);
    m_paint_engines[OpenGL] = new QtPaintEngine(this);

    setEngineType(Stupid);//默认引擎为Stupid
    setMouseTracking(true);//鼠标追踪

    m_command = NULL;
    m_command_action = NULL;
    m_animation_timer = new QTimer(this);
    connect(m_animation_timer,SIGNAL(timeout()),this,SLOT(rotating_animation()));

    m_background_color = Qt::white;
    m_selected_color   = Qt::yellow;
    m_line_color       = Qt::green;


//    emit refresh_combox();//显示命令提示
//    setWindowTitle(current_path);

}

bool copyDirectoryFiles(const QDir &fromDir, const QDir &toDir, bool coverFileIfExist)
{
  QDir sourceDir = fromDir;
  QDir targetDir = toDir;
  if(!targetDir.exists()){
    if(!targetDir.mkdir(toDir.absolutePath()))
      return false;
  }

  QFileInfoList fileInfoList = sourceDir.entryInfoList();
  foreach(QFileInfo fileInfo, fileInfoList){
    if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
      continue;

    if(fileInfo.isDir()){
          if(!copyDirectoryFiles(fileInfo.filePath(),
                 targetDir.filePath(fileInfo.fileName()),
                 coverFileIfExist))
            return false;
    }
    else{
//        /**< 当允许覆盖操作时，将旧文件进行删除操作 */
          if(coverFileIfExist && targetDir.exists(fileInfo.fileName()))
          {
                targetDir.remove(fileInfo.fileName());
          }


          //进行文件copy
          if(!targetDir.exists(fileInfo.fileName()))
          {
              if(!QFile::copy(fileInfo.filePath(),
                              targetDir.filePath(fileInfo.fileName())))
                return false;
          }
    }
  }

  fileInfoList.clear();

  return true;
}


void MCadWidget::gene_img_dir()
{
    QDir dir;
    dir.mkpath(CircuitTable::img_dir);//创建多级目录，如果已存在则会返回去true
    if(copyDirectoryFiles(QDir(":/cad/"),QDir(CircuitTable::img_dir),false))
    {  //将文件复制到新的文件路径下
//        qDebug()<<"img_dir:"<<CircuitTable::img_dir<<endl;
    }
    else
    {
        qDebug()<<"程序初始化失败"<<endl;
    }


}
//绘制事件
void MCadWidget::paintEvent(QPaintEvent *e)
{
    MCadUtil::StopWatch watch;
    watch.start();
    QPainter p;
    if(m_use_opengl)//根据所选引擎决定画布是否OpenGL画布
        p.begin(m_glwidget);
    else
        p.begin(this);

    p.fillRect(e->rect(),m_background_color);//填充背景

    if(m_command)
        m_command->paint(e,p,m_current_engine);//绘制当前命令

    if(m_use_opengl)
    {
        p.translate(m_scale_center);
        p.scale(m_scale,m_scale);
        p.translate(-m_scale_center);
        p.translate(m_offset);
    }

    CircuitList& buff = m_use_opengl ? *UserCommand::geoTab() : m_buff;
    buff.draw_module(m_current_engine,p);
    buff.draw_lines(m_current_engine,p);



//    emit displaySPF(watch.tell());//计算绘制单帧时间
}

//鼠标移动时间
void MCadWidget::mouseMoveEvent(QMouseEvent *e)
{
    qDebug()<<e->x()<<" "<<e->y()<<endl;
    CircuitList& buff = m_use_opengl ? *UserCommand::geoTab() : m_buff;

    if(m_command)
    {
        m_command->move(e);//通知命令移动的坐标
    }

    if(e->buttons() & Qt::LeftButton)
    {

    }
    if(e->buttons() & Qt::MiddleButton)
    {
        QPoint delta = e->pos() - m_rotate_center;
        if(!m_command || (m_command && m_command->state()==UserCommand::Finished))
        {
            m_x_rotation += (qreal)delta.x()/10;
            m_y_rotation -= (qreal)delta.y()/10;

            if(m_x_rotation > 90)m_x_rotation=90;
            if(m_y_rotation > 90)m_y_rotation=90;
            if(m_x_rotation < -90)m_x_rotation=-90;
            if(m_x_rotation < -90)m_x_rotation=-90;
            m_rotate_center = e->pos();
            refresh_buff();
        }
    }

    if(e->buttons() & Qt::RightButton)
    {
        QPoint delta = e->pos() - m_offset_center;
        if(!m_command || (m_command && m_command->state()==UserCommand::Finished))
        {
            m_offset += delta;
            m_offset_center = e->pos();
            refresh_buff();
        }

    }

    refresh_buff();
    update();

}

//鼠标按下事件
void MCadWidget::mousePressEvent(QMouseEvent *e)
{

    if(e->button() == Qt::LeftButton)//左键
    {
        if(m_command && m_command->state() != UserCommand::Finished)//如果当前命令还未完成
        {
            if(m_command->proceed(e) == UserCommand::AskForSelection)//步进命令
            {

                toggle_selection(e->pos(),false);
            }

            if(m_command->state() == UserCommand::Finished)//如果完成
            {
                qDebug()<<"LeftButton"<<endl;
                raise_new_command(m_command_action);//产生同样的命令
            }
            emit displayHint(m_command->hint());//显示命令提示
        }
        else
        {

            toggle_selection(e->pos());
//                qDebug()<<"toggle_selection"<<endl;
        }
    }
    else if(e->button() == Qt::RightButton)//右键
    {
//        UserCommand::geoTab()->lines.delete_lines_one(QUuid u1, QUuid u2, int dot1, int dot2)
//                        qDebug()<<"RightButton"<<endl;
//        quit_current_command(m_command_action);//退出命令
//        quit_selection();//清楚选择
//        m_offset_center =  e->pos();
    }
    else if(e->button() == Qt::MiddleButton)
    {
//        m_rotate_center = e->pos();
    }

    update();
    QWidget::mousePressEvent(e);
}


//鼠标弹起事件
void MCadWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if(m_command)
    {
        m_command->release_mouse(e);//通知命令移动的坐标
        if(m_command->state() == UserCommand::Finished)//如果完成
        {
         qDebug()<<"LeftButton"<<endl;
            raise_new_command(m_command_action);//产生同样的命令
            m_command_action->setChecked(true);
        }
        emit displayHint(m_command->hint());//显示命令提示
    }
    update();
    QWidget::mouseReleaseEvent(e);
}

void MCadWidget::resizeEvent(QResizeEvent *e)
{
    if(m_use_opengl && m_glwidget)
        m_glwidget->setGeometry(0,0,e->size().width()-1,e->size().height()-1);

    QWidget::resizeEvent(e);
}

void MCadWidget::closeEvent(QCloseEvent *event)
{
//    qDebug()<<"close"<<endl;
}
//滚轮事件
void MCadWidget::wheelEvent(QWheelEvent *e)
{
    if(m_command && m_command->state()!=UserCommand::Finished)return;//命令执行过程中禁止缩放
    qreal cur_scale = m_scale;
    cur_scale += e->delta()/1000.0;

    if(cur_scale<0 || cur_scale >5)return;
    m_scale = cur_scale;
    m_scale_center = e->pos();
    if(!m_use_opengl)
    {
        refresh_buff();
    }
    update();
}

//创建新的命令
void MCadWidget::raise_new_command(QAction *action)
{


    reset_view();
    //如果还未完成，退出当前的命令
    if(m_command && m_command->state()!=UserCommand::Finished)
    {
        quit_current_command(m_command_action);
    }


//    if(m_command && action->objectName()!=m_command_action->objectName())
//    {
//        if(m_command_action)
//            m_command_action->setChecked(false);
//    }

    QString name = action->objectName();



    //除了需要选择的命令外，消除选择
    if(name != "actionDelete" && name != "actionSelect")
    {
       quit_selection();

    }

    if(name == "actionPlaceLine")
    {
        if(m_command && m_command->state() != UserCommand::Finished)
        {
            delete m_command;
        }

        m_command = new MCadCommand::PlaceLine(this);
        emit displayHint(m_command->hint());
        m_selected = 1;
    }
    else if(name == "actionPlaceRect")
    {
        if(m_command && m_command->state() != UserCommand::Finished)
        {
            delete m_command;
        }

        m_command = new MCadCommand::PlaceRect(this);
        emit displayHint(m_command->hint());
        m_selected = 2;
    }
    else if(name == "actionDelete")
    {

        if(m_command && m_command->state() != UserCommand::Finished)
        {
            delete m_command;
        }


         m_command = new MCadCommand::Delete(this);
         m_selected = 3;
    }
    else if(name == "actionSelect")
    {
        if(m_command && m_command->state() != UserCommand::Finished)
        {
            delete m_command;
        }

        m_command = new MCadCommand::SelectMove(this);
        m_selected = 0;
        emit displayHint("选择控件");
    }
    else if(name == "action_save")
    {
        QString file_name;
        if(current_path=="")
        {
            file_name = QFileDialog::getSaveFileName(this,
                    tr("保存文件"),
                    "",
                    tr("Json Files (*.json)"),
                    0);
            current_path = file_name;
        }
        else
        {
            file_name = current_path;
        }


         if(!file_name.isNull())
         {
            UserCommand::geoTab()->save(file_name.toStdString().c_str());
            qDebug()<<"save："<<file_name<<endl;
            emit displayHint("保存成功");
            emit displaySPF(current_path);
         }
         else
         {
            emit displayHint("保存失败");
         }

    }
    else if(name == "action_save_as")
    {
        QString file_name = QFileDialog::getSaveFileName(this,
                tr("保存文件"),
                "",
                tr("Json Files (*.json)"),
                0);

         if(!file_name.isNull())
         {
            UserCommand::geoTab()->save(file_name.toStdString().c_str());
            current_path = file_name;
            emit displayHint("保存成功");
            emit displaySPF(current_path);

         }
         else
         {
            emit displayHint("保存失败");
         }

    }
    else if(name == "action_read")
    {
        QString file_name = QFileDialog::getOpenFileName(this,
                tr("打开文件"),
                "",
                tr("Json Files (*.json)"),
                0);

        if(!file_name.isNull())
        {
           UserCommand::geoTab()->load(file_name.toStdString().c_str());
           current_path = file_name;
           emit displayHint("读取成功");
           emit displaySPF(current_path);
        }
        else
        {
            emit displayHint("读取失败");
        }

//        setWindowTitle(current_path);

//
//        qDebug()<<UserCommand::geoTab()->lines.data<<endl;
//        qDebug()<<"read："<<file_name<<endl;
    }

    if(m_command_action )
    {
        m_command_action->setChecked(false);
    }

    action->setChecked(true);
    m_command_action = action;

    refresh_buff();
    update();
}

//退出当前命令
void MCadWidget::quit_current_command(QAction *action)
{
    if(m_command_action)
        m_command_action->setChecked(false);


    if(m_command)
        delete m_command;
    if(m_animation_timer->isActive())
    {
        m_animation_timer->stop();
    }

    emit displayHint("        ");
    m_command = NULL;
}

//消除选择
void MCadWidget::quit_selection()
{
    for(int i=0; i<m_buff.modules.count(); ++i)
    {
        m_buff.modules[i].select(false);
        (UserCommand::geoTab()->modules)[i].select(false);
    }
    m_buff.modules.select_index=-1;
    m_buff.select_type=-1;
    m_buff.lines.select_line.clear();

    UserCommand::geoTab()->modules.select_index=-1;
    UserCommand::geoTab()->select_type=-1;
    UserCommand::geoTab()->lines.select_line.clear();

    m_selected = 0;
}

void MCadWidget::toggle_selection(QPoint p, bool mulity)
{
//    if(UserCommand::geoTab()->isEmpty())return;

//    for(int i=0; i<m_buff.count(); ++i)
//    {
////        if(m_buff[i].in_box(p))
////        {
////            m_buff.select_index=i;
////            (*UserCommand::geoTab()).select_index = i;
////            break;
////        }
////        else
////        {
////            m_buff.select_index=-1;
////            (*UserCommand::geoTab()).select_index = -1;
////        }


//    }

//    qDebug()<<"m_buff.select_index:"<<m_buff.select_index<<" "<<m_buff.count()<<endl;

}

//更新显示表
void MCadWidget::refresh_buff()
{
    m_buff = *UserCommand::geoTab();

    for(auto it=m_buff.modules.begin() ; it!=m_buff.modules.end() ; ++it)
    {
//        for(Entity::iterator e = it->begin() ; e!=it->end() ; ++e)
//        {
//            for(Base_Circuit::iterator p = e->begin() ; p!=e->end() ; ++p)
//            {
//                Line3d& line = *p;
//                QMatrix4x4 T;
//                T.translate(QVector3D(-m_scale_center));
//                line = line*T;
//                T = QMatrix4x4();
//                T.scale(m_scale);
//                line = line*T;
//                T = QMatrix4x4();
//                T.translate(QVector3D(m_scale_center));
//                T.translate(QVector3D(m_offset));
//                T.rotate(m_x_rotation,QVector3D(0,1,0));
//                T.rotate(m_y_rotation,QVector3D(1,0,0));
//                line = line*T;
//            }
//        }
    }
}

void MCadWidget::reset_view()
{
    m_x_rotation = 0;
    m_y_rotation = 0;
    m_scale = 1;
    m_offset = QPointF(0,0);
    refresh_buff();
}

//设置绘图引擎
void MCadWidget::setEngineType(MCadWidget::EngineType type)
{
    if(type == OpenGL)
    {
        if(!m_glwidget)
        {
            m_glwidget = new QGLWidget(this);
            m_glwidget->setAutoFillBackground(false);
            m_glwidget->setMouseTracking(true);
            m_glwidget->setAttribute(Qt::WA_TransparentForMouseEvents,true);
            QApplication::postEvent(this, new QResizeEvent(size(), size()));
        }
        m_use_opengl = true;
        m_glwidget->show();
    }   
    else
    {
        m_use_opengl = false;
        if(m_glwidget)
            m_glwidget->hide();
        refresh_buff();
    }

    m_current_engine = m_paint_engines[type];
}

//创建新的命令
void MCadWidget::startNewCommand(QAction *action)
{
    raise_new_command(action);
}

void MCadWidget::rotating_animation()
{
    for(auto it = UserCommand::geoTab()->modules.begin() ; it!=UserCommand::geoTab()->modules.end() ; ++it)
    {

//        if(it->isSelected())
//        {
//            qDebug()<<"hhselct_index:"<<it->select_index<<endl;
////            it->rotate(Entity::Y_Axis,5);
////            it->rotate(Entity::X_Axis,5);
//        }
    }

    refresh_buff();
    update();
}

//void MCadWidget::setUseBlanking(bool b)
//{
//    m_use_blanking = b;
//    update();
//}

void MCadWidget::setColorOption(QAction *a)
{
    QColorDialog dlg;
    if(a->objectName() == "actionLineColor")
    {
        dlg.setWindowTitle("选择线条颜色");
        dlg.setCurrentColor(m_line_color);
        if(dlg.exec())
        {
            m_line_color = dlg.currentColor();
        }
    }
    else if(a->objectName() == "actionBackgroundColor")
    {
        dlg.setWindowTitle("选择背景颜色");
        dlg.setCurrentColor(m_background_color);
        if(dlg.exec())
        {
            m_background_color = dlg.currentColor();
        }
    }
    else if(a->objectName() == "actionSelectedColor")
    {
        dlg.setWindowTitle("选择选中线条颜色");
        dlg.setCurrentColor(m_selected_color);
        if(dlg.exec())
        {
            m_selected_color = dlg.currentColor();
        }
    }

    update();
}


void MCadWidget::addModule()
{
    ParameterDialog module_dialog(this);

    module_dialog.initial_ui((UserCommand::geoTab()->modules.cfg_json));
    auto s=module_dialog.exec();

    if(s==1)
    {
        module_dialog.update_cfg();
        qDebug()<<UserCommand::geoTab()->modules.cfg_json <<endl;
        UserCommand::geoTab()->modules.cfg_json = module_dialog.get_cfg_json();
        m_buff.modules.cfg_json = module_dialog.get_cfg_json();
        emit refresh_combox();

    }
//    qDebug()<<"addModule:"<<UserCommand::geoTab()->modules.cfg_json<<endl;

    UserCommand::geoTab()->modules.save_cfg(CircuitTable::img_dir,QString("cfg.json"));

}

void MCadWidget::module_changed(int index)
{
    CircuitTable::current_class_id = index+1;
    qDebug()<<"index:"<<index<<endl;
}


QJsonObject MCadWidget::get_cfg_json()
{
    return UserCommand::geoTab()->modules.cfg_json;
}
