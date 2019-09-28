#include <qdebug.h>
#include "itemformatter.h"

QString DateItemFormatter::displayText(const QVariant& value, const QLocale& )const
{
    QDate date= QDate::fromString(value.toString(), Qt::ISODate);
    if( date == QDate())
        return "";
    else
        return date.toString("dd.MM.yyyy");
};

QString PercentItemFormatter::displayText(const QVariant& value, const QLocale& )const
{
    return value.toString()+"%";
};

QString EuroItemFormatter::displayText(const QVariant& value, const QLocale& )const
{
    return value.toString()+" Euro";
};

QString ActivatedItemFormatter::displayText(const QVariant &value, const QLocale &) const
{
    // the view delivers strings like "true" and "false" for boolean values
    // let the variant resolve this ...
    return value.toBool() ? "Vertrag aktiv" : "Vertrag INAKTIV";
}
