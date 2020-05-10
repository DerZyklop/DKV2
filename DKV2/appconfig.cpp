
#include <QString>
#include <QDebug>
#include <QSettings>
#include <QStandardPaths>
#include <QFileDialog>

#include "helper.h"
#include "appconfig.h"

/* statics */
#ifndef QT_DEBUG
QString appConfig::keyOutdir = "outdir";
QString appConfig::keyLastDb = "db/last";
QString appConfig::keyCurrentDb = "db/current";
#else
QString appConfig::keyOutdir = "dbg-outdir";
QString appConfig::keyLastDb = "dbg-db/last";
QString appConfig::keyCurrentDb = "dbg-db/current";
#endif
QMap<QString, QString> appConfig::runtimedata;

/* static */
void appConfig::setOutDir(const QString& od)
{   LOG_CALL_W(od);
    setUserData(keyOutdir, od);
}
/* static */
void appConfig::setOutDirInteractive(QWidget* parent)
{   LOG_CALL;
    QString dir(getUserData(keyOutdir, QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)));
    dir = QFileDialog::getExistingDirectory(parent, "Ausgabeverzeichnis", dir,
               QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    appConfig::setOutDir(dir);
}
/* static */
QString appConfig::Outdir()
{
    QString od;

    do {
        od = getUserData(keyOutdir, QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
        if( od.isEmpty())
            setOutDirInteractive();
    } while (od.isEmpty());
    return od;
}

/* static */
void appConfig::setLastDb(const QString& filename)
{   LOG_CALL_W(filename);
    setUserData(keyLastDb, filename);
}
/* static */
QString appConfig::LastDb()
{
    QString ldb = getUserData(keyLastDb);
    qDebug() << "lastDb read as " << ldb;
    return ldb;
}

/* static */
void appConfig::setCurrentDb(const QString& path)
{   LOG_CALL_W(path);
    setRuntimeData( keyCurrentDb, path);
}
/* static */
QString appConfig::CurrentDb()
{
    QString cdb = getRuntimeData(keyCurrentDb);
    return cdb;
}

/* private */
/* static */
void appConfig::setUserData(const QString& name, const QString& value)
{
    QSettings config;
    config.setValue(name, value);
}
/* static */
QString appConfig::getUserData( const QString& name, const QString& defaultvalue)
{
    QSettings config;
    return config.value(name, defaultvalue).toString();
}
/* static */
void appConfig::deleteUserData(const QString& name)
{
    QSettings config;
    config.remove(name);
}
// QString getNumUserData(QString name);

/* static */
void appConfig::setRuntimeData( const QString& name, const QString& value)
{
    runtimedata.insert(name,value);
}
/* static */
QString appConfig::getRuntimeData( const QString& name, const QString& defaultvalue)
{
    return runtimedata.value(name, defaultvalue);
}
/* static */
void appConfig::deleteRuntimeData(const QString& name)
{
    runtimedata.remove(name);
}
