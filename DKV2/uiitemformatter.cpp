//#include "finhelper.h"
#include <QDate>

#include "helperfin.h"
#include "uiitemformatter.h"

QString DateItemFormatter::displayText(const QVariant& value, const QLocale& )const
{
    QDate date= QDate::fromString(value.toString(), Qt::ISODate);
    if( date == QDate() || date > QDate(9990, 12, 31))
        return "";
    else
        return date.toString("dd.MM.yyyy");
};
void DateItemFormatter::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem alignedOption(option);
    alignedOption.displayAlignment = Qt::AlignCenter;
    QStyledItemDelegate::paint(painter, alignedOption, index);
}

QString PercentItemFormatter::displayText(const QVariant& value, const QLocale& )const
{
    double percent = round2(value.toDouble());
    return QString::number(percent) + "%";
};
void PercentItemFormatter::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem alignedOption(option);
    alignedOption.displayAlignment = Qt::AlignCenter;
    QStyledItemDelegate::paint(painter, alignedOption, index);
}

QString EuroItemFormatter::displayText(const QVariant& value, const QLocale& locale)const
{
    double w = round2(value.toDouble()/100);
    QLocale l(locale);
    l.setNumberOptions(0);
    if( w <= 0)
        return "[" + l.toCurrencyString(-1 *w) + " "  + "] offen";
    else
        return l.toCurrencyString(w)+ " ";
};

void EuroItemFormatter::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem alignedOption(option);
    alignedOption.displayAlignment = Qt::AlignRight|Qt::AlignVCenter;
    QStyledItemDelegate::paint(painter, alignedOption, index);
}

QString KFristItemFormatter::displayText(const QVariant &value, const QLocale &) const
{
    int v = value.toInt();
    if( v == -1)
        return "(festes Vertragsende)";
    else
        return QString("%L1 Monate").arg(v);
}
void KFristItemFormatter::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem alignedOption(option);
    alignedOption.displayAlignment = Qt::AlignCenter;
    QStyledItemDelegate::paint(painter, alignedOption, index);
}

QString thesaItemFormatter::displayText(const QVariant &value, const QLocale &) const
{
    if( value.toBool())
        return "thesaur.";
    else
        return "auszahlend";
}
void thesaItemFormatter::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem alignedOption(option);
    alignedOption.displayAlignment = Qt::AlignCenter;
    QStyledItemDelegate::paint(painter, alignedOption, index);
}
