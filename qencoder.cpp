#include "qencoder.h"
#include <QFile>
#include <QDebug>
#include <QDirIterator>
#include <QRegularExpression>

QEncoder::QEncoder(QObject *parent)
    : QObject(parent)
{

}

QEncoder::~QEncoder() {}



QEncoder::PathCheckResult QEncoder::checkPathsExist(const QStringList &paths) {
    PathCheckResult result;
    for (const auto &path : paths) {
        QFileInfo fileInfo(path);
        if (fileInfo.exists()) {
            result.exists.append(path);
        } else {
            result.notExists.append(path);
        }
    }
    result.allExists = result.notExists.isEmpty();
    return result;
}

QStringList QEncoder::collectFilesByFilters(const QStringList &pathsOrFolders, const QStringList &filters) {
    QStringList result;
    QSet<QString> collected;

    for (const auto &path : pathsOrFolders) {
        QFileInfo fileInfo(path);
        if (fileInfo.isFile()) {
            collected.insert(path);
        } else if (fileInfo.isDir()) {
            QDirIterator it(path, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                it.next();
                if (it.fileInfo().isFile()) {
                    collected.insert(it.filePath());
                }
            }
        }
    }

    QStringList patterns;
    for (const QString &filter : filters) {
        QString trimmed = filter.trimmed();
        QString ext = QRegularExpression::escape(trimmed.mid(2)); // 转义特殊字符
        patterns << ext;
    }
    // 构造正则，匹配字符串结尾
    QString pattern = QString(".*\\.(" + patterns.join("|") + ")$");
    QRegularExpression regex(pattern, QRegularExpression::CaseInsensitiveOption);

    for (const auto &file : collected) {
        QString fileName = QFileInfo(file).fileName(); // 获取文件后缀
        if(regex.match(fileName).hasMatch()){
            result.append(file);
        }
    }
    return result;
}

QStringList QEncoder::getAllEncodings()
{
    return {
        // UTF 系列
        "utf-8",
        "utf-16",
        "utf-16-le",
        "utf-16-be",
        "utf-32",
        "utf-32-le",
        "utf-32-be",

        // ASCII
        "ascii",

        // ISO 系列
        "iso-8859-1",
        "iso-8859-2",
        "iso-8859-3",
        "iso-8859-4",
        "iso-8859-5",
        "iso-8859-6",
        "iso-8859-7",
        "iso-8859-8",
        "iso-8859-9",

        // Windows 编码
        "cp1250",
        "cp1251",
        "cp1252",
        "cp1253",
        "cp1254",
        "cp1255",
        "cp1256",
        "cp1257",
        "cp1258",

        // 中文编码
        "gbk",
        "gb2312",
        "gb18030",
        "big5",

        // 其他
        "latin1",
        "shift_jis",
        "euc-jp",
        "euc-kr",
        "koi8-r"
    };
}
