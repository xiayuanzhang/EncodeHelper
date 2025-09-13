#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include "qencoder.h"
#include <QDateTime>
#include <QCloseEvent>
#include <QDesktopServices>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_settings = new QSettings("config.ini", QSettings::IniFormat, this);
    if(!m_settings->contains("init")){
        m_settings->setValue("init", "0");
        m_settings->setValue("encode", "utf-8");
        m_settings->setValue("filter", "*.cpp\r\n*.h\r\n*.hpp\r\n*.c\r\n*.cc\r\n*.hh\r\n*.cxx\r\n*.hxx");
        m_settings->setValue("paths", "");
        m_settings->setValue("normalEncode","utf-8,utf-16,utf-16-be,utf-32,utf-32-le,"
                                             "utf-32-be,ascii,gbk,gb2312,gb18030");
        m_settings->setValue("backup","true");
        m_settings->sync();
    }

    ui->comboBoxEncode->addItems(QEncoder::getAllEncodings());
    ui->comboBoxEncode->setCurrentText(m_settings->value("encode","utf-8").toString());
    ui->textEditFilter->setPlainText(m_settings->value("filter","*.cpp\r\n*.h\r\n*.hpp\r\n*.c\r\n*.cc\r\n*.hh\r\n*.cxx\r\n*.hxx").toString());
    ui->textEditSrc->setPlainText(m_settings->value("paths","").toString());
    QStringList filters = splitCheckFilters(ui->textEditFilter->toPlainText());
    ui->checkBoxCpp->setChecked(filters.contains("*.cpp") || filters.contains("*.cxx") || filters.contains("*.cc"));
    ui->checkBoxHpp->setChecked(filters.contains("*.hpp") || filters.contains("*.hxx") || filters.contains("*.hh"));
    ui->checkBoxC->setChecked(filters.contains("*.c"));
    ui->checkBoxH->setChecked(filters.contains("*.h"));
    ui->checkBoxBack->setChecked(m_settings->value("backup","true").toString() == "true");


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
                appendLogColor(outputCmd.at(i));
            }
            appendLogColor(QString("转换完成, 成功 %1 个, 失败 %2 个").arg(successCount).arg(failCount));
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

void MainWindow::appendLogColor(const QString &info)
{
    QStringList normalEmcode = m_settings->value("normalEncode","utf-8").toString().split(",", Qt::SkipEmptyParts);
    QStringList unusualncode = QEncoder::getAllEncodings();
    for (const QString &s : normalEmcode) {
        unusualncode.removeAll(s);
    }

    if(info.contains("错误")){//红色显示
        ui->textEditOutput->setTextColor(Qt::red);
    }else if(info.contains("警告")){//橙色显示
        ui->textEditOutput->setTextColor(QColor(255, 140, 0));
    }else{//黑色显示
        //ui->textEditOutput->setTextColor(Qt::black);
        //非常见编码格式, 橙色显示
        bool unusualFound = false;
        for (const QString &k : unusualncode) {
            if (info.contains(k, Qt::CaseInsensitive)) {
                unusualFound = true;
                break;
            }
        }
        if(unusualFound){
            ui->textEditOutput->setTextColor(QColor(255, 140, 0));
        }else{
            ui->textEditOutput->setTextColor(Qt::black);
        }
    }
    ui->textEditOutput->append(info);
}

void MainWindow::appendLog(const QString &info)
{
    ui->textEditOutput->append(info);
}

// 递归复制文件夹
void copyDir(const QString &srcPath, const QString &dstPath)
{
    QDir srcDir(srcPath);
    if (!srcDir.exists())
        return;

    QDir dstDir(dstPath);
    if (!dstDir.exists())
        dstDir.mkpath(".");

    QFileInfoList entries = srcDir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);
    for (const QFileInfo &entry : entries) {
        QString srcFilePath = entry.absoluteFilePath();
        QString dstFilePath = dstPath + "/" + entry.fileName();

        if (entry.isDir()) {
            copyDir(srcFilePath, dstFilePath);
        } else {
            QFile::copy(srcFilePath, dstFilePath);
        }
    }
}

void MainWindow::backupPaths(const QStringList &paths)
{
    QString exeDir = QCoreApplication::applicationDirPath();  // exe 所在目录
    QDir backupRoot(exeDir + "/backup");
    if (!backupRoot.exists()) {
        backupRoot.mkpath(".");
    }

    for (const QString &path : paths) {
        QFileInfo info(path);

        // 把路径中的 ":" 和 "/"、"\" 替换成 "_"
        QString normalizedPath = info.absolutePath();
        normalizedPath.replace(":", "_");
        normalizedPath.replace("\\", "_");
        normalizedPath.replace("/", "_");

        if (info.isFile()) {
            // 文件 -> backup/files/<盘符_路径>/filename
            QDir filesBackupDir(backupRoot.filePath("files/" + normalizedPath));
            if (!filesBackupDir.exists()) {
                filesBackupDir.mkpath(".");
            }

            QString dstFile = filesBackupDir.filePath(info.fileName());
            if (!QFile::copy(info.absoluteFilePath(), dstFile)) {
                appendLog("文件复制失败:" + info.absoluteFilePath());
            } else {
                appendLog("文件备份成功:" + info.absoluteFilePath() + " -> " + dstFile);
            }

        } else if (info.isDir()) {
            // 文件夹 -> backup/<盘符_路径>/<目录名/...>
            QString dstFolder = backupRoot.filePath(normalizedPath);
            QDir().mkpath(dstFolder);

            QString finalDst = dstFolder + "/" + info.fileName();
            copyDir(info.absoluteFilePath(), finalDst);
            appendLog("文件夹备份成功:" + info.absoluteFilePath() + " -> " + finalDst);
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_settings->setValue("encode", ui->comboBoxEncode->currentText());
    m_settings->setValue("filter", ui->textEditFilter->toPlainText());
    m_settings->setValue("paths", ui->textEditSrc->toPlainText());
    m_settings->setValue("backup", ui->checkBoxBack->isChecked() ? "true" : "false");
    m_settings->sync();
    // if (m_encoderProcess) {
    //     m_encoderProcess->write("exit\n");
    //     m_encoderProcess->waitForFinished(3000); // 等待最多3秒
    //     if (m_encoderProcess->state() != QProcess::NotRunning) {
    //         m_encoderProcess->kill(); // 强制终止
    //     }
    // }
    event->accept();
}

void MainWindow::on_pushButtonFile_clicked()
{
    //打开文件选择对话框,可多选, 任意类型
    QString startDir = m_settings->value("lastChoseFile", "").toString();
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "选择文件", startDir, "All Files (*.*)");
    if (!fileNames.isEmpty()) {
        for (const auto &fileName : fileNames) {
            ui->textEditSrc->append(fileName);
        }

        // 更新 lastDir
        QFileInfo fi(fileNames.first());
        m_settings->setValue("lastChoseFile", fi.absolutePath());
    }
}


void MainWindow::on_pushButtonFolder_clicked()
{
    QString startDir = m_settings->value("lastChoseFDir", "").toString();
    //打开文件夹选择对话框
    QString dir = QFileDialog::getExistingDirectory(this, "选择文件夹", startDir);
    if (!dir.isEmpty()) {
        ui->textEditSrc->append(dir);

        // 更新 lastDir
        m_settings->setValue("lastChoseFDir", dir);
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
    if(m_encoderProcess->state() != QProcess::Running){
        ui->textEditOutput->append("编码进程未运行, 停止工作");
        return;
    }
    qDebug() << "编码进程ID:" << m_encoderProcess->processId();


    QStringList paths = splitLines(ui->textEditSrc->toPlainText());
    QStringList filters = splitCheckFilters(ui->textEditFilter->toPlainText());
    QEncoder::PathCheckResult checkResult = QEncoder::checkPathsExist(paths);
    if (!checkResult.allExists) {
        for (const auto &path : checkResult.notExists) {
            ui->textEditOutput->append(path + ": 路径不存在");
        }
        return;
    }
    if(ui->checkBoxBack->isChecked()){
        backupPaths(paths);
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







void MainWindow::on_pushButtonBackFolder_clicked()
{
    //使用资源管理器打开 ./backup 文件夹
    QString backupDir = QCoreApplication::applicationDirPath() + "/backup";
    QDesktopServices::openUrl(QUrl::fromLocalFile(backupDir));

}


void MainWindow::on_pushButtonBackClear_clicked()
{
    //将backup移动到资源管理器回收站,重新创建backup文件夹
    QString backupDir = QCoreApplication::applicationDirPath() + "/backup";
    QDir dir(backupDir);
    if(dir.exists()){
        dir.removeRecursively();
    }
    dir.mkpath(backupDir);
    ui->textEditOutput->append("备份文件夹已清空");
}


void MainWindow::on_checkBoxBack_stateChanged(int arg1)
{

}

