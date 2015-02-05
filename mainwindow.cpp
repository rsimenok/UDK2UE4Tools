#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    pParams()
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_convertBut_clicked()
{
    if (pParams.size()<1) {
        QMessageBox msgBox;
         msgBox.setText("Fill old adress, new adress, actor key text fields in settings before convert.");
         msgBox.exec();
    }else{
        QString pTxt = ui->pasteText->toPlainText();
        pTxt = pTxt.replace(QRegExp("Begin Map.+Begin Level"), "").replace(QRegExp("End Level.+Begin Surface.+End Surface.+End Map"), "").replace(QRegExp("End Actor"), "").trimmed();

        for (std::vector<Params>::iterator i = pParams.begin(); i<pParams.end(); i++) {
            pTxt = pTxt.replace(i->oldAddr, i->newAddr+"\r\n            "+i->key);
        }

        QRegExp rx("Begin Actor");
        QStringList actors = pTxt.split(rx);
        QString resultStr = "Begin Map\r\n   Begin Level";
        int uNumb = 1;
        for (int i = 1; i<actors.count(); i++) {
            QString str = actors[i].trimmed();
            QString type = str.mid(6, str.indexOf(" ")-6);
            qDebug() << type;
            if (type == "StaticMeshActor") {
                str = str.replace(QRegExp("Name=\\S+(\\s)"), QString("Name=%1_SMActor_%2\\1").arg(ui->namePrefixText->toPlainText()).arg(uNumb));
                str = str.replace("Engine.Default__StaticMeshActor:StaticMeshComponent0", "/Script/Engine.Default__StaticMeshActor:StaticMeshComponent0");
                str = str.replace("Engine.Default__StaticMeshActor", "/Script/Engine.Default__StaticMeshActor");
                str = str.replace("Name=StaticMeshComponent0", "Name=\"StaticMeshComponent0\"");
                str = str.replace(QRegExp("\\sObjName=\\S+"), "");
                str = str.replace(QRegExp("(Begin Object.+ Archetype=\\S+\\s)"), "\\1         End Object\r\n         Begin Object Name=\"StaticMeshComponent0\"\r\n");

                resultStr += "\r\n      Begin Actor "+str+"\r\n      End Actor";
            }
        }
        ui->newText->setPlainText(resultStr+"\r\n   End Level\r\nBegin Surface\r\nEnd Surface\r\nEnd Map");
    }
}

void MainWindow::on_addParams_clicked()
{
    pParams.push_back(Params(ui->oldAddrText->toPlainText(), ui->newAddrText->toPlainText(), ui->keyText->toPlainText()));
    ui->oldAddrText->setPlainText("");
    ui->newAddrText->setPlainText("");
    ui->keyText->setPlainText("");
    refreshList();
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

    ui->editOldAddr->setPlainText("");
    ui->editNewAddr->setPlainText("");
    ui->editKey->setPlainText("");

    refreshList();
}

void MainWindow::on_listWidget_itemClicked(QListWidgetItem *item)
{
    int id=ui->listWidget->row(item);
    ui->editOldAddr->setPlainText(pParams[id].oldAddr);
    ui->editNewAddr->setPlainText(pParams[id].newAddr);
    ui->editKey->setPlainText(pParams[id].key);
}
