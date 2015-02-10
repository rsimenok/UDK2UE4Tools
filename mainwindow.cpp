#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <qdebug.h>
#include <QClipboard>
#include <qsettings.h>

#define FileToSave qApp->applicationDirPath()+"/saved.txt"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    pParams()
{
    // Ініціалізація UI
    ui->setupUi(this);
    try{// Пробуємо зчитати збережені параметри
        // Відкриваємо файл для читання
        QFile loadfile(FileToSave);
        loadfile.open(QIODevice::ReadOnly|QIODevice::Text);
        QTextStream in(&loadfile);
        // Зчитуємо все ...
        QString line = in.readAll();
        QStringList loadlist = line.split(QRegExp("\n"), QString::SkipEmptyParts);
        // ... розбираємо поелементно
        for(QString one : loadlist){
            Params Param(one);
            if(Param.valid())
                // Додаємо в список
                pParams.push_back(Param);
        }
        loadfile.close();
    }catch(...){ }

    this->refreshList();
    QSettings settings;
    ui->stayInTop->setChecked(settings.value("stayInTop").toBool());
    if  (ui->stayInTop->isChecked()) {
         this->setWindowFlags(Qt::WindowStaysOnTopHint);
    }
}

MainWindow::~MainWindow()
{
    try{// Пробуємо зберегти параметри
        // Відкриваємо файл для запису і очищаємо його
        QFile loadfile(FileToSave);
        loadfile.open(QIODevice::ReadWrite|QIODevice::Truncate|QIODevice::Text);
        QTextStream out(&loadfile);
        // Записуємо по елементно
        for(Params Param : pParams){
            out << Param.ToString();
        }
        // закриваємо файл
        loadfile.close();
    }catch(...){ }

    delete ui;
}

void MainWindow::on_convertBut_clicked()
{
    if (pParams.size()<1) {
        QMessageBox msgBox;
         msgBox.setText("Add params before convertation.");
         msgBox.exec();
    }else{
        QString pTxt = ui->pasteText->toPlainText();
        pTxt = pTxt.replace(QRegExp("Begin Map.+Begin Level"), "").replace(QRegExp("End Level.+Begin Surface.+End Surface.+End Map"), "").replace(QRegExp("End Actor"), "").trimmed();

        for (std::vector<Params>::iterator i = pParams.begin(); i<pParams.end(); i++) {
            pTxt = pTxt.replace(i->oldAddr, i->newAddr+"\r\n            "+i->key);
        }

        pTxt = pTxt.replace("Location", "RelativeLocation");
        pTxt = pTxt.replace("Rotation", "RelativeRotation");
        pTxt = pTxt.replace("Tag", "Tags(0)");
        pTxt = pTxt.replace("Layer", "Tags(1)");
        pTxt = pTxt.replace(QRegExp("\\s*ObjectArchetype[^\r\n]+[\r\n]*"), "\r\n");
        pTxt = pTxt.replace(QRegExp("\\s*CollisionComponent[^\r\n]+[\r\n]*"), "\r\n");
        pTxt = pTxt.replace(QRegExp("\\s*bNoEncroachCheck[^\r\n]+[\r\n]*"), "\r\n");
        pTxt = pTxt.replace(QRegExp("\\s*CollisionType[^\r\n]+[\r\n]*"), "\r\n");
        pTxt = pTxt.replace(QRegExp("(\\s{2,})Name\\s*=\\s*([^\r\n]+)"), "\\1ActorLabel=\\2");

        QStringList actors = pTxt.split("Begin Actor");
        QString resultStr = "Begin Map\r\n   Begin Level";
        int uNumb = 1;
        for (int i = 1; i<actors.count(); i++) {
            QString str = actors[i].replace("\r\n", "\n").replace("\r", "\n").replace("\n\n", "\n").replace("\n", "\r\n").trimmed();
            QString type = str.mid(6, str.indexOf(" ")-6);
            if (type == "StaticMeshActor") {
                str = str.replace("Name=StaticMeshComponent0", "Name=\"StaticMeshComponent0\"");
                str = str.replace(QRegExp("Name=[^\"\\s]+(\\s)"), QString("Name=%1_SMActor_%2\\1").arg(ui->namePrefixText->toPlainText()).arg(uNumb));
                str = str.replace("Engine.Default__StaticMeshActor:StaticMeshComponent0", "/Script/Engine.Default__StaticMeshActor:StaticMeshComponent0");
                str = str.replace(QRegExp("\\sObjName=\\S+"), "");
                str = str.replace(QRegExp("\\s*(DerivedDataKey[^\r\n]+).+End Object(.+)([\r\n]+\\s+RelativeLocation.+DrawScale[^\r\n]+[\r\n]*)"), "\\1\\3         End Object\\2\r\n");
                str = str.replace(QRegExp("(Begin Object.+ Archetype=\\S+\\s)"), "\\1         End Object\r\n         Begin Object Name=\"StaticMeshComponent0\"\r\n");
                str = str.replace(QRegExp("\\s*StaticMeshComponent=StaticMeshComponent[^\r\n]+[\r\n]*"), "\r\n         StaticMeshComponent=StaticMeshComponent0");
                str = str.replace(QRegExp("\\s*Components[^\r\n]+[\r\n]*"), "\r\n         RootComponent=StaticMeshComponent0\r\n");

                // Переводимо поворот в градуси
                if  (str.contains("RelativeRotation=")) {
                     QRegExp matcher("RelativeRotation=\\(Pitch=([+-\\d.]+),\\s*Yaw=([+-\\d.]+),\\s*Roll=([+-\\d.]+)\\)");
                     int i = matcher.indexIn(str);
                     float Rotation[3];
                     if (i) {
                         QStringList list = matcher.capturedTexts();
                         QStringList::iterator it = list.begin();
                         int q = 0;
                         for ( ++it ;it!=list.end(); it++) {
                             Rotation [q++] = it->toFloat() * 0.00549316540360483;
                         }
                     }
                     str = str.replace(QRegExp("RelativeRotation=\\(Pitch=([+-\\d.]+),\\s*Yaw=([+-\\d.]+),\\s*Roll=([+-\\d.]+)\\)"), QString("RelativeRotation=(Pith=%1,Yaw=%2,Roll=%3)").arg(Rotation[0]).arg(Rotation[1]).arg(Rotation[2]));
                }

                float fDrawScale = 1.0;
                float d3Scale [3]={1,1,1};
                if  (str.contains("DrawScale=")) {
                     QRegExp matcher("DrawScale=([^\r\n]+)");
                     int i =matcher.indexIn(str);
                     if (i) {
                         QStringList list = matcher.capturedTexts();
                         QStringList::iterator it = list.begin();
                         for ( ++it ;it!=list.end(); it++) {
                            fDrawScale = it->toFloat();
                         }
                     }
                     str = str.replace(QRegExp("\\s*DrawScale=[^\r\n]+[\r\n]*"), "");
                }

                if  (str.contains("DrawScale3D=")) {
                     QRegExp matcher("DrawScale3D=\\(X=([+-\\d.]+),Y=([+-\\d.]+),Z=([+-\\d.]+)\\)");
                     int i =matcher.indexIn(str);
                     if (i) {
                         QStringList list = matcher.capturedTexts();
                         QStringList::iterator it = list.begin();
                         int q = 0;
                         for ( ++it ;it!=list.end(); it++) {
                             d3Scale [q++] = it->toFloat();
                         }
                     }
                     str = str.replace(QRegExp("\\s*DrawScale3D=[^\r\n]+[\r\n]*"), "");
                }

                for (int i = 0; i<3; i++) {
                    d3Scale[i] *= fDrawScale;
                }

                QString scalePattern = "(X=%1,Y=%2,Z=%3)";
                scalePattern = scalePattern.arg(d3Scale[0]).arg(d3Scale[1]).arg(d3Scale[2]);

                str = str.replace(QRegExp("(\\s*DerivedDataKey=[^\r\n]+[\r\n]*)"), "\\1         BodyInstance=(Scale3D="+scalePattern+",CollisionProfileName=\"Custom\",CollisionResponses=(ResponseArray=((Channel=\"Pawn\",Response=ECR_Ignore),(Channel=\"PhysicsBody\",Response=ECR_Ignore))))\r\n         RelativeScale3D="+scalePattern+"\r\n");
                str = str.replace("         Relative", "              Relative");
                str = str.replace("         End Object", "\r\n         End Object");
                // o_O
                str = str.replace("\r\n\r\n", "\r\n").replace("\n\n", "\n").replace("\r\r", "\r").replace("\r\n\r\n", "\r\n");
                resultStr += "\r\n      Begin Actor "+str+"\r\n      End Actor";
            }
        }
        ui->newText->setPlainText(resultStr+"\r\n   End Level\r\nBegin Surface\r\nEnd Surface\r\nEnd Map");
    }
}

void MainWindow::on_addParams_clicked()
{
    if (ui->oldAddrText->toPlainText().isEmpty() || ui->newAddrText->toPlainText().isEmpty() || ui->keyText->toPlainText().isEmpty()) {
        return;
    }
    pParams.push_back(Params(ui->oldAddrText->toPlainText(), ui->newAddrText->toPlainText(), ui->keyText->toPlainText()));
    this->clearTextFields(false);
    this->refreshList();
}

void MainWindow::refreshList()
{
    ui->listWidget->clear();
    for (std::vector<Params>::iterator i = pParams.begin(); i<pParams.end(); i++){
        ui->listWidget->addItem(i->oldAddr+" | "+i->newAddr+" | "+i->key);
    }
}

void MainWindow::on_editBut_clicked()
{
    QList<QListWidgetItem*> items=ui->listWidget->selectedItems();
    if(items.size()!=1){
        return;
    }
    int id=ui->listWidget->row(items[0]);
    pParams[id].oldAddr = ui->editOldAddr->toPlainText();
    pParams[id].newAddr = ui->editNewAddr->toPlainText();
    pParams[id].key = ui->editKey->toPlainText();

    this->clearTextFields(true);
    this->refreshList();
}

void MainWindow::on_listWidget_itemClicked(QListWidgetItem *item)
{
    int id=ui->listWidget->row(item);
    ui->editOldAddr->setPlainText(pParams[id].oldAddr);
    ui->editNewAddr->setPlainText(pParams[id].newAddr);
    ui->editKey->setPlainText(pParams[id].key);
}

void MainWindow::on_delBut_clicked()
{
    QList<QListWidgetItem*> items=ui->listWidget->selectedItems();
    if(items.size()!=1){
        return;
    }
    int id=ui->listWidget->row(items[0]);
    pParams.erase(pParams.begin()+id);
    this->clearTextFields(true);
    this->refreshList();
}

void MainWindow::clearTextFields(bool isEditable) {
    if (isEditable) {
        ui->editOldAddr->setPlainText("");
        ui->editNewAddr->setPlainText("");
        ui->editKey->setPlainText("");
    }else{
        ui->oldAddrText->setPlainText("");
        ui->newAddrText->setPlainText("");
        ui->keyText->setPlainText("");
    }
}
void MainWindow::on_rotatitonBut_clicked() {
    if  (ui->oldRotationText->toPlainText().contains("Object.Movement.Rotation ")) {
        // почему-то матчер не работет пришлось по говняцки написать
//          QRegExp matcher("Object.Movement.Rotation \\(Pitch=([+-\\d.]+),\\s*Yaw=([+-\\d.]+),\\s*Roll=([+-\\d.]+)\\)");
//          int i = matcher.indexIn(ui->oldRotationText->toPlainText());
//          float Rotation[3];
//          if (i) {
//              QStringList list = matcher.capturedTexts();
//              QStringList::iterator it = list.begin();
//              int q = 0;
//              for ( ++it ;it!=list.end(); it++) {
//                  Rotation [q++] = it->toFloat() * 0.00549316540360483;
//                  qDebug() << Rotation;
//              }
//          }
        QString data = ui->oldRotationText->toPlainText();
        float pitch = (data.mid(data.indexOf("Pitch=")+6, data.indexOf(",")-data.indexOf("Pitch=")-6)).toFloat()*0.00549316540360483;
        float yaw =  (data.mid(data.indexOf("Yaw=")+4, data.indexOf("Roll=")-data.indexOf("Yaw=")-5)).toFloat()*0.00549316540360483;;
        float roll =  (data.mid(data.indexOf("Roll=")+5, data.indexOf(")")-data.indexOf("Roll=")-5)).toFloat()*0.00549316540360483;
        QString tmpStr  = ui->oldRotationText->toPlainText().replace(QRegExp("Object.Movement.Rotation \\(Pitch=([+-\\d.]+),\\s*Yaw=([+-\\d.]+),\\s*Roll=([+-\\d.]+)\\)"), QString("(Pitch=%1,Yaw=%2,Roll=%3)").arg(pitch).arg(yaw).arg(roll));
        ui->newRotationText->setPlainText(tmpStr);
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(tmpStr);
    }
}

void MainWindow::on_stayInTop_clicked()
{
    QSettings settings;
    if (ui->stayInTop->isChecked()) {
        settings.setValue("stayInTop", ui->stayInTop->isChecked());
    }else{
        settings.setValue("stayInTop", ui->stayInTop->isChecked());
    }
}
