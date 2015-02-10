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
    QString key;

    // Пустий конструктор
    Params():
        oldAddr(""),
        newAddr(""),
        key("")
    {

    }

    // Конструктор із значень
    Params(QString a, QString b, QString c):
        oldAddr(a),
        newAddr(b),
        key(c)
    {
        oldAddr = oldAddr.trimmed();
        newAddr = newAddr.trimmed();
        key     = key.trimmed();
    }

    // Конструктор із одного рядка типу як при виведенні через ToString
    Params(QString& FromSave):
        oldAddr(""),
        newAddr(""),
        key("")
    {
        QStringList list = FromSave.split("\t");
        if(list.count() > 2){
            oldAddr = list[0];
            newAddr = list[1];
            key     = list[2];
            oldAddr = oldAddr.trimmed();
            newAddr = newAddr.trimmed();
            key     = key.trimmed();
        }
    }

    bool valid(){
        return oldAddr!="" && newAddr!="" && key!="";
    }

    QString ToString(){
        return QString("%1\t%2\t%3\n").arg(oldAddr).arg(newAddr).arg(key);
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_convertBut_clicked();
    void on_addParams_clicked();
    void on_editBut_clicked();
    void on_listWidget_itemClicked(QListWidgetItem *item);
    void on_delBut_clicked();

private:
    Ui::MainWindow *ui;
    std::vector<Params> pParams;
    void refreshList();
    void clearTextFields(bool isEditable) ;
    QString getDataBetween(QString begin,QString end, QString &source);
};

#endif // MAINWINDOW_H
