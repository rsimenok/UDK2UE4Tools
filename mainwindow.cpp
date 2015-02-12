#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <qdebug.h>
#include <QClipboard>
#include <qsettings.h>

#define FileToSave qApp->applicationDirPath()+"/saved.txt"

QString& QReplace(QString& Str, QRegExp What, QString To){
    Str = Str.replace(What, To);
    return Str;
}
QString& QReplace(QString& Str, QList<QRegExp>& What, QStringList& To, bool empty = true){
    int num = (std::min)(What.size(), To.size());
    for(int i =0; i<num; i++){
        Str = Str.replace(What[i], To[i]);
    }
    if(empty){
        What.clear();
        To.clear();
    }
    return Str;
}

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
          QRegExp matcher("\\(Pitch=([+-\\d.]+),\\s*Yaw=([+-\\d.]+),\\s*Roll=([+-\\d.]+)\\)");
          QString Str(ui->oldRotationText->toPlainText());
          QString Cleared("");
          qDebug() << Str;
          for(int i = matcher.indexIn(Str);i!=-1;i = matcher.indexIn(Str, i+5)){
              float Rotation[3];
              QStringList list = matcher.capturedTexts();
              QStringList::iterator it = list.begin();
              int q = 0;
              for ( ++it ;it!=list.end(); it++) {
                  Rotation [q++] = it->toFloat() * 0.00549316540360483;
                  qDebug() << Rotation[q];
              }
//              Str = Str.replace(i, list[0].count(), QString("(Pitch=%1,Yaw=%2,Roll=%3)").arg(Rotation[0]).arg(Rotation[1]).arg(Rotation[2]));
              Cleared = Cleared + QString("(Pitch=%1,Yaw=%2,Roll=%3)\n").arg(Rotation[0]).arg(Rotation[1]).arg(Rotation[2]);
          }
//        if(ui->bHelperClearAll->isChecked())
            ui->newRotationText->setPlainText(Cleared);
//            else
//            ui->newRotationText->setPlainText(Str);
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(Str);
    }
}

void MainWindow::on_stayInTop_clicked()
{
    Qt::WindowFlags flags = windowFlags();
    if (ui->stayInTop->isChecked()) {
         flags |= Qt:: WindowStaysOnTopHint;
    }else{
        flags  &= ~Qt:: WindowStaysOnTopHint;
    }
    setWindowFlags(flags);
    show();
}


void MainWindow::on_UScriptSource_textChanged()
{
    QString Source(ui->UScriptSource->toPlainText());
    QList<QRegExp> Regs;
    QStringList To;

    Regs << QRegExp("(\\s*)var\\((\\w+)\\) ([^<;]+)<([^>]+)>;");
    To << "\\1UPROPERTY(Category = \"\\2\", Meta = (\\4))\\1\\3;";
    Regs << QRegExp("(\\s*)var\\([^)]*\\) ([^<;]+)<([^>]+)>;");
    To << "\\1UPROPERTY(Meta = (\\3))\\1\\2;";
    Regs << QRegExp("(\\s*)var\((\\w+)\\) ([^<;]+)<(([^=;>]+=[^=;>]+)+)>;");
    To << "\\1UPROPERTY(Category = \"\\2\", Meta = (\\4))\\1\\3;";
    Regs << QRegExp("(\\s*)var\\((\\w+)\\) ([^;]+);");
    To << "\\1UPROPERTY(Category = \"\\2\")\\1\\3;";
    Regs << QRegExp("(\\s*)var\\(\\) ([^;]+);");
    To << "\\1UPROPERTY()\\1\\2;";
    Regs << QRegExp("(\\s*)var transient ([^;]+);");
    To << "\\1\\2; // #OldNotice: Transient";
    Regs << QRegExp("(\\s*)var ([^;]+);");
    To << "\\1\\2;";
    Regs << QRegExp("(\\s*)UPROPERTY\\(([^\r\n]*)\\)(\\s*)transient ([^;]+);");
    To << "\\1UPROPERTY(Transient, \\2)\\3\\4;";
    Regs << QRegExp("UPROPERTY\\(Transient, \\)");
    To << "UPROPERTY(Transient)";

    ui->UCppSource->setPlainText(QReplace(Source, Regs, To));
}

void MainWindow::on_pasteText_textChanged()
{
    if (pParams.size()<1) {
        QMessageBox msgBox;
         msgBox.setText("Add params before convertation.");
         msgBox.exec();
    }else{
        QString pTxt = ui->pasteText->toPlainText();
        QList<QRegExp> Regs;
        QStringList To;

        Regs << QRegExp("\\s*Begin Map.+Begin Level")
             << QRegExp("End Level.+Begin Surface.+End Surface.+End Map")
             << QRegExp("\\s*End Actor");
        To << "" << "" << "";

        for (std::vector<Params>::iterator i = pParams.begin(); i<pParams.end(); i++) {
            pTxt = pTxt.replace(i->oldAddr, i->newAddr+"\r\n            "+i->key);
        }

        Regs << QRegExp("Location")
             << QRegExp("Rotation")
             << QRegExp("Tag")
             << QRegExp("Layer")
             << QRegExp("\\s*ObjectArchetype[^\r\n]+[\r\n]*")
             << QRegExp("\\s*CollisionComponent[^\r\n]+[\r\n]*")
             << QRegExp("\\s*bNoEncroachCheck[^\r\n]+[\r\n]*")
             << QRegExp("\\s*CollisionType[^\r\n]+[\r\n]*")
             << QRegExp("(\\s{2,})Name\\s*=\\s*([^\r\n]+)")
             << QRegExp("\r\n") << QRegExp("\r") << QRegExp("\n\n") << QRegExp("\n");
        To << "RelativeLocation"
           << "RelativeRotation"
           << "Tags(0)"
           << "Tags(1)"
           << "\r\n" << "\r\n" << "\r\n" << "\r\n"
           << "\\1ActorLabel=\\2"
           << "\n" << "\n" << "\n" << "\r\n";

        QReplace(pTxt, Regs, To);
        pTxt = pTxt.trimmed();

        QString resultStr = "Begin Map\r\n   Begin Level";
        QStringList actors = pTxt.split("Begin Actor");
        pTxt.resize(0);
        int uNumb = 1;
        for (QString str : actors) {
            str = str.trimmed();
            QString type = str.mid(6, str.indexOf(" ")-6);
            if (type == "StaticMeshActor") {
                Regs << QRegExp("Name=StaticMeshComponent0")
                     << QRegExp("Name=[^\"\\s]+(\\s)")
                     << QRegExp("Engine.Default__StaticMeshActor:StaticMeshComponent0")
                     << QRegExp("\\sObjName=\\S+")
                     << QRegExp("(\\s*\\w*DerivedDataKey[^\r\n]+).+([\r\n]+[\\t ]*End Object)(.+)[\r\n]+(\\s+RelativeLocation.+DrawScale[^\r\n]+[\r\n]*)")
                     << QRegExp("(Begin Object.+ Archetype=\\S+\\s+)")
                     << QRegExp("\\s*StaticMeshComponent=StaticMeshComponent[^\r\n]+[\r\n]*")
                     << QRegExp("\\s*Components[^\r\n]+[\r\n]*");
                To << "Name=\"StaticMeshComponent0\""
                   << QString("Name=%1_SMActor_%2\\1").arg(ui->namePrefixText->toPlainText()).arg(uNumb)
                   << "/Script/Engine.Default__StaticMeshActor:StaticMeshComponent0"
                   << ""
                   << "\\1\r\n\\4EndObject\\3\r\n"
                   << "\\1         End Object\r\n         Begin Object Name=\"StaticMeshComponent0\"\r\n"
                   << "\r\n         StaticMeshComponent=StaticMeshComponent0"
                   << "\r\n         RootComponent=StaticMeshComponent0\r\n";

                QReplace(str, Regs, To);

                // Переводимо поворот в градуси
                if  (str.contains("RelativeRotation=")) {
                     QRegExp matcher("RelativeRotation=\\(Pitch=([+-\\d.]+),\\s*Yaw=([+-\\d.]+),\\s*Roll=([+-\\d.]+)\\)");
                     int i = matcher.indexIn(str);
                     float Rotation[3];
                     if (i!=-1) {
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
                str = str.replace("EndObject", "\r\n         End Object");
                // o_O
                str = str.replace("\r\n\r\n", "\r\n").replace("\n\n", "\n").replace("\r\r", "\r").replace("\r\n\r\n", "\r\n");
                resultStr += "\r\n      Begin Actor "+str+"\r\n      End Actor";
            }
        }
        actors.clear();
        ui->newText->setPlainText(resultStr+"\r\n   End Level\r\nBegin Surface\r\nEnd Surface\r\nEnd Map");
        resultStr.resize(0);
        QApplication::clipboard()->setText(ui->newText->toPlainText());
    }
}
