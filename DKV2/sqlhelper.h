#ifndef SQLHELPER_H
#define SQLHELPER_H

#include <QVariant>
#include <QDate>
#include <QSqlQuery>
#include <QSqlRecord>

#include "dbfield.h"

bool tableExists(const QString& tablename, const QString& con="");
bool tableExists(const QString& tablename, QSqlDatabase db);

QVector<QString> getFieldsFromTablename(const QString& tablename, const QString& con = "");

QString SelectQueryFromFields(const QVector<dbfield>& fields, const QString& where);

//QSqlRecord ExecuteSingleRecordSql(const QVector<dbfield>& fields, const QString& where, const QString& con="");
QSqlRecord ExecuteSingleRecordSql(const QVector<dbfield>& fields, const QString& where, QSqlDatabase db= QSqlDatabase::database(QLatin1String(QSqlDatabase::defaultConnection)));

QVariant ExecuteSingleValueSql(const QString& s, QSqlDatabase db);
QVariant   ExecuteSingleValueSql( const QString& s, const QString& connection="");
QVariant   ExecuteSingleValueSql( const QString& field, const QString& table, const QString& where, const QString& con="");

int ExecuteUpdateSql(const QString& table, const QString& field, const QVariant& newValue, const QString& where, const QString& con="");

int getHighestTableId(const QString& tablename, const QString& con="");

QString JsonFromRecord( QSqlRecord r);

template <class T>
T sqlVal(QSqlQuery sql, QString fname);

#endif // SQLHELPER_H
