#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include "qencoder.h"
#include <QDateTime>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->comboBoxEncode->addItems(QEncoder::getAllEncodings());

    ui->textEditPreview->setReadOnly(true);
    ui->textEditOutput->setReadOnly(true);

    m_encoderProcess = new QProcess(this);
    //./encoder.exe 启动
    m_encoderProcess->setProgram("./encoder.exe");
    //设置工作目录为当前程序所在目录
    m_encoderProcess->setWorkingDirectory(QCoreApplication::applicationDirPath());
    //设置编码为本地编码,否则中文会乱码
    m_encoderProcess->setProcessChannelMode(QProcess::MergedChannels);
    //启动
    m_encoderProcess->start();

    connect(m_encoderProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        QByteArray output = m_encoderProcess->readAllStandardOutput();
        output = output.trimmed(); // 去掉开头和末尾空白，包括 \n 和 \r
        QStringList outputCmd = QString::fromLocal8Bit(output).split(",", Qt::SkipEmptyParts);
        if(outputCmd.size() < 1){
            return;
        }
        if(outputCmd.at(0) == "change_return"){
            int successCount = outputCmd.at(1).toInt();
            int failCount = outputCmd.at(2).toInt();
            for(int i = 3; i< outputCmd.size(); ++i){
                ui->textEditOutput->append(outputCmd.at(i));
            }
            ui->textEditOutput->append(QString("转换完成, 成功 %1 个, 失败 %2 个").arg(successCount).arg(failCount));
        }
    });

    connect(m_encoderProcess, &QProcess::readyReadStandardError, this, [this]() {
        QByteArray errorOutput = m_encoderProcess->readAllStandardError();
        ui->textEditOutput->append(QString::fromLocal8Bit(errorOutput));
    });

    connect(m_encoderProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        // ui->textEditOutput->append(QString("编码进程已结束，退出代码：%1，退出状态：%2")
        //                           .arg(exitCode)
        //                           .arg(exitStatus == QProcess::NormalExit ? "正常退出" : "异常退出"));
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

QStringList MainWindow::splitLines(const QString &text)
{
    QStringList filters = text.split("\n", Qt::SkipEmptyParts);
    for (int i = 0; i < filters.size(); ++i) {
        filters[i] = filters[i].trimmed(); // 去掉每行前后空格
    }
    return filters;
}

QStringList MainWindow::splitCheckFilters(const QString &text)
{
    QStringList filters = text.split("\n", Qt::SkipEmptyParts);
    for (int i = 0; i < filters.size(); ++i) {
        filters[i] = filters[i].trimmed(); // 去掉每行前后空格
        if (filters[i].startsWith("*.") && filters[i].length() > 2) {

        } else {
            filters.removeAt(i);
        }
    }
    return filters;
}

void MainWindow::on_pushButtonFile_clicked()
{
    //打开文件选择对话框,可多选, 任意类型
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "选择文件", "", "All Files (*.*)");
    if (!fileNames.isEmpty()) {
        for (const auto &fileName : fileNames) {
            ui->textEditSrc->append(fileName);
        }
    }
}


void MainWindow::on_pushButtonFolder_clicked()
{
    //打开文件夹选择对话框
    QString dir = QFileDialog::getExistingDirectory(this, "选择文件夹", "");
    if (!dir.isEmpty()) {
        ui->textEditSrc->append(dir);
    }
}


void MainWindow::on_checkBoxCpp_stateChanged(int arg1)
{
    QStringList filters = splitCheckFilters(ui->textEditFilter->toPlainText());

    if (arg1 == Qt::Checked) {
        if (!filters.contains("*.cpp")) {
            filters.append("*.cpp");
        }
        if (!filters.contains("*.cxx")) {
            filters.append("*.cxx");
        }
        if (!filters.contains("*.cc")) {
            filters.append("*.cc");
        }
    } else {
        filters.removeAll("*.cpp");
        filters.removeAll("*.cxx");
        filters.removeAll("*.cc");
    }
    ui->textEditFilter->setPlainText(filters.join("\n"));
}


void MainWindow::on_checkBoxHpp_stateChanged(int arg1)
{
    QStringList filters = splitCheckFilters(ui->textEditFilter->toPlainText());

    if (arg1 == Qt::Checked) {
        if (!filters.contains("*.hpp")) {
            filters.append("*.hpp");
        }
        if (!filters.contains("*.hxx")) {
            filters.append("*.hxx");
        }
        if (!filters.contains("*.hh")) {
            filters.append("*.hh");
        }
    } else {
        filters.removeAll("*.hpp");
        filters.removeAll("*.hxx");
        filters.removeAll("*.hh");
    }
    ui->textEditFilter->setPlainText(filters.join("\n"));
}


void MainWindow::on_checkBoxC_stateChanged(int arg1)
{
    QStringList filters = splitCheckFilters(ui->textEditFilter->toPlainText());

    for (const QString &line : filters) {
        QString trimmedLine = line.trimmed();
        if (trimmedLine.startsWith("*.") && trimmedLine.length() > 2) {
            qDebug() << trimmedLine << "格式正确";
        } else {
            qDebug() << trimmedLine << "格式错误";
        }
    }


    if (arg1 == Qt::Checked) {
        if (!filters.contains("*.c")) {
            filters.append("*.c");
        }
    } else {
        filters.removeAll("*.c");
    }
    ui->textEditFilter->setPlainText(filters.join("\n"));
}


void MainWindow::on_checkBoxH_stateChanged(int arg1)
{
    QStringList filters = splitCheckFilters(ui->textEditFilter->toPlainText());

    if (arg1 == Qt::Checked) {
        if (!filters.contains("*.h")) {
            filters.append("*.h");
        }
    } else {
        filters.removeAll("*.h");
    }
    ui->textEditFilter->setPlainText(filters.join("\n"));
}


void MainWindow::on_comboBoxEncode_activated(int index)
{

}


void MainWindow::on_pushButtonOk_clicked()
{
    //输出时间
    ui->textEditOutput->append(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));

    QStringList paths = splitLines(ui->textEditSrc->toPlainText());
    QStringList filters = splitCheckFilters(ui->textEditFilter->toPlainText());
    QEncoder::PathCheckResult checkResult = QEncoder::checkPathsExist(paths);
    if (!checkResult.allExists) {
        for (const auto &path : checkResult.notExists) {
            ui->textEditOutput->append(path + ": 路径不存在");
        }
        return;
    }
    QStringList files = QEncoder::collectFilesByFilters(paths, filters);
    ui->textEditPreview->setPlainText(files.join("\n"));

    QString encodeName = ui->comboBoxEncode->currentText();

    QString cmd = QString("change,") + encodeName + "," + files.join(",");

    m_encoderProcess->write(cmd.toUtf8() + "\n");

    ui->textEditOutput->append(QString("共找到 %1 个符合条件的文件，正在转换编码...").arg(files.size()));
}


void MainWindow::on_textEditSrc_textChanged()
{

}


void MainWindow::on_textEditFilter_textChanged()
{

}


void MainWindow::on_textEditPreview_textChanged()
{

}


void MainWindow::on_textEditOutput_textChanged()
{

}







void MainWindow::on_pushButtonPreview_clicked()
{
    //输出时间
    ui->textEditOutput->append(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));

    QStringList paths = splitLines(ui->textEditSrc->toPlainText());
    QStringList filters = splitCheckFilters(ui->textEditFilter->toPlainText());
    QEncoder::PathCheckResult checkResult = QEncoder::checkPathsExist(paths);
    if (!checkResult.allExists) {
        for (const auto &path : checkResult.notExists) {
            ui->textEditOutput->append(path + ": 路径不存在");
        }
        return;
    }
    QStringList files = QEncoder::collectFilesByFilters(paths, filters);
    ui->textEditPreview->setPlainText(files.join("\n"));
    ui->textEditOutput->append(QString("共找到 %1 个符合条件的文件").arg(files.size()));
}


void MainWindow::on_pushButtonClear_clicked()
{
    ui->textEditOutput->clear();
}




