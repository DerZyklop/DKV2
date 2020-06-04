#ifndef TABLEDATAINSERTER_H
#define TABLEDATAINSERTER_H

#include <QSqlDatabase>
#include <QSqlRecord>

#include "dbtable.h"

class TableDataInserter
{
public:
    // constr. destrc. & access fu
    //TableDataInserter(){}
    TableDataInserter(const dbtable& t);
    void init(const dbtable& t);
    bool setValue(const QString& field, const QVariant& v);
    bool setValues(const QSqlRecord rec);
    QVariant getValue(const QString& f) const { return record.field(f).value();}
    QSqlRecord getRecord() const {return record;}
    // interface
    int InsertData(QSqlDatabase db = QSqlDatabase::database()) const;
    int InsertOrReplaceData(QSqlDatabase db = QSqlDatabase::database()) const;
    int UpdateData() const;

private:
    // data
    QString tablename;
    QSqlRecord record;
    //int lastRecord;
    // helper
    QString getInsertRecordSQL() const;
    QString getInsertOrReplaceRecordSQL() const;
    QString getUpdateRecordSQL() const;
};
#endif // TABLEDATAINSERTER_H
