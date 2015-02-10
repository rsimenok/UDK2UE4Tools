#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

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
    // Add test params
    //pParams.push_back(Params("FracturedStaticMesh'ce_two_asteroids.Geometry.aster_fr'", "StaticMesh'/Game/EP_03/Geometry/Asteroid/EP_03_asteroid_01.EP_03_asteroid_01'", "StaticMeshDerivedDataKey=\"STATICMESH_46A8778361B442A9523C54440EA1E9D_0db5412b27ab480f844cc7f0be5abaff_E761B3D7462866F64840EC9FFBB6EBA100000000010000000100000000000000010000004000000000000000010000000000803F0000803F0000803F0000803F000000000000803F00000000000000000000344203030300000000\""));

    this->refreshList();
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
            QString str = actors[i].trimmed();
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
                str = str.replace("         End Object", "\r\n         End Object");
                str = str.replace("\r\n\r\n", "\r\n");
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
