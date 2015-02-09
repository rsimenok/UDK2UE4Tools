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

    Params(QString a, QString b, QString c){
        oldAddr = a;
        newAddr = b;
        key = c;
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
