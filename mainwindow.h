/** UDK2UE4Tools: Help transfer actors, code and matinee from UDK to UE4
    Copyright (C) 2015  Roman Simenok, OlexandrI

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. **/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <QListWidgetItem>

namespace Ui {
class MainWindow;
}

struct Params {
    QString oldAddr;
    QString newAddr;

    // Пустий конструктор
    Params():
        oldAddr(""),
        newAddr("")
    {

    }

    // Конструктор із значень
    Params(QString a, QString b):
        oldAddr(a),
        newAddr(b)
    {
        oldAddr = oldAddr.trimmed();
        newAddr = newAddr.trimmed();
    }

    // Конструктор із одного рядка типу як при виведенні через ToString
    Params(QString& FromSave):
        oldAddr(""),
        newAddr("")
    {
        QStringList list = FromSave.split("\t");
        if(list.count() > 1){
            oldAddr = list[0];
            newAddr = list[1];
            oldAddr = oldAddr.trimmed();
            newAddr = newAddr.trimmed();
        }
    }

    bool valid(){
        return oldAddr!="" && newAddr!="";
    }

    QString ToString(){
        return QString("%1\t%2\t%3\n").arg(oldAddr).arg(newAddr);
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    static void ConvertAllRotators(QString& Str, bool clearout = false);
    static void CorrectAllLocation(QString& Str, float CorrectScale[3], float CorrectLoc[3], QString BeforeStr = QString("Location\\s*=\\s*"), bool clearout = false);
    static bool setValWithRelative(QLineEdit* From, unsigned int Id, float* Array, bool lock = true);

private slots:
    void on_addParams_clicked();
    void on_editBut_clicked();
    void on_listWidget_itemClicked(QListWidgetItem *item);
    void on_delBut_clicked();

    void on_stayInTop_clicked();

    void on_UScriptSource_textChanged();

    void on_pasteText_textChanged();

    void on_toolButton_clicked();

    void on_toolButton_2_clicked();

    void on_toolButton_3_toggled(bool checked);

    void syncUSourceScroll1(int nPos);
    void syncUSourceScroll2(int nPos);

    void on_toolButton_4_toggled(bool checked);

    void on_oldRotationText_textChanged();

    void on_HelperAutoClear_toggled(bool checked);

    void on_tabWidget_currentChanged(int index);

    void on_ConvSettScaleX_editingFinished();

    void on_ConvSettScaleY_editingFinished();

    void on_ConvSettScaleZ_editingFinished();

    void on_ConvSettLocatX_editingFinished();

    void on_ConvSettLocatY_editingFinished();

    void on_ConvSettLocatZ_editingFinished();

    void on_toolButton_5_clicked();

private:
    Ui::MainWindow *ui;
    std::vector<Params> pParams;
    void refreshList();
    void clearTextFields(bool isEditable) ;
    QString getDataBetween(QString begin,QString end, QString &source);

    float ConvSettCurrScale[3];
    float ConvSettCurrLocation[3];
    void refreshConvScaleEditLine();
    void refreshConvLocEditLine();
    bool TimeLockChangeEvent;
};

#endif // MAINWINDOW_H
