#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QStringList splitLines(const QString &text);
    QStringList splitCheckFilters(const QString &text);

private slots:
    void on_pushButtonFile_clicked();

    void on_pushButtonFolder_clicked();

    void on_checkBoxCpp_stateChanged(int arg1);

    void on_checkBoxHpp_stateChanged(int arg1);

    void on_checkBoxC_stateChanged(int arg1);

    void on_checkBoxH_stateChanged(int arg1);

    void on_comboBoxEncode_activated(int index);

    void on_pushButtonOk_clicked();

    void on_textEditSrc_textChanged();

    void on_textEditFilter_textChanged();

    void on_textEditPreview_textChanged();

    void on_textEditOutput_textChanged();




    void on_pushButtonPreview_clicked();

    void on_pushButtonClear_clicked();


private:
    Ui::MainWindow *ui;
    QProcess *m_encoderProcess = nullptr;
};
#endif // MAINWINDOW_H
