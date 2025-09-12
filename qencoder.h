#ifndef QENCODER_H
#define QENCODER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>

class QEncoder : public QObject
{
    Q_OBJECT

public:
    explicit QEncoder(QObject *parent = nullptr);
    ~QEncoder();

    struct PathCheckResult {
        QStringList exists;
        QStringList notExists;
        bool allExists;
    };

    static PathCheckResult checkPathsExist(const QStringList &paths);
    static QStringList collectFilesByFilters(const QStringList &pathsOrFolders, const QStringList &filters) ;

    static QStringList getAllEncodings();
};

#endif // QENCODER_H
