#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QMap>
#include <QWidget>

struct appConfig
{
    static void setOutDir(const QString& od);
    static void setOutDirInteractive(QWidget* parent =nullptr);
    static QString Outdir();
    static void delOutDir();

    static void setLastDb(const QString&);
    static QString LastDb();
    static void delLastDb();

    static void setCurrentDb(const QString&);
    static QString CurrentDb();
    static void delCurrentDb();

    static void setRuntimeData( const QString& name, const QString& value);
    static QString getRuntimeData( const QString& name, const QString& defaultvalue ="");

    // for testing only
    static void deleteUserData(const QString& name);
    static void deleteRuntimeData(const QString& name);
private:
    static bool testmode;
    static QString keyOutdir;
    static QString keyLastDb;
    static QString keyCurrentDb;
    static void setUserData(const QString& name, const QString& value);
    static QString getUserData( const QString& name, const QString& defaultvalue ="");
    // QString getNumUserData(QString name);

    static QMap<QString, QString> runtimedata;
};

#endif // APPCONFIG_H
