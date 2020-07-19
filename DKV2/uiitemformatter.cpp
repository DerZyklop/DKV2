//#include "finhelper.h"
#include <QDate>

#include "booking.h"
#include "helperfin.h"
#include "uiitemformatter.h"

QString DateItemFormatter::displayText(const QVariant& value, const QLocale& )const
{
    QDate date= value.toDate();
    if( date == QDate() || date > QDate(9990, 12, 31))
        return "";
    else
        return date.toString(qsl("dd.MM.yyyy"));
};
void DateItemFormatter::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem alignedOption(option);
    alignedOption.displayAlignment = Qt::AlignCenter;
    QStyledItemDelegate::paint(painter, alignedOption, index);
}

QString PercentItemFormatter::displayText(const QVariant& value, const QLocale& )const
{
    double percent = value.toDouble();
    // data is stored as 100th percent but the query compensatates that
    return qsl("%1%").arg(percent, 2, 'g');
};
void PercentItemFormatter::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem alignedOption(option);
    alignedOption.displayAlignment = Qt::AlignCenter;
    QStyledItemDelegate::paint(painter, alignedOption, index);
}

QString ContractValueItemFormatter::displayText(const QVariant& value, const QLocale& )const
{
    double w = round2(value.toDouble()/100);
    QLocale l;
    if( w < 0)
        return qsl("[") + l.toCurrencyString(-1 *w) + qsl(" ")  + qsl("] offen");
    else
        return l.toCurrencyString(w)+ qsl(" ");
};
void ContractValueItemFormatter::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem alignedOption(option);
    alignedOption.displayAlignment = Qt::AlignRight|Qt::AlignVCenter;
    QStyledItemDelegate::paint(painter, alignedOption, index);
}

void BookingAmountItemFormatter::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem alignedOption(option);
    alignedOption.displayAlignment = Qt::AlignRight|Qt::AlignVCenter;
    QStyledItemDelegate::paint(painter, alignedOption, index);
}

QString BookingAmountItemFormatter::displayText(const QVariant& value, const QLocale& )const
{
    double w = round2(value.toDouble()/100);
    QLocale l;
    if( w <= 0)
        return "[" + l.toCurrencyString(-1 *w) + " "  + "]";
    else
        return l.toCurrencyString(w)+ " ";
};

QString KFristItemFormatter::displayText(const QVariant &value, const QLocale &) const
{
    int v = value.toInt();
    if( v == -1)
        return qsl("(festes Vertragsende)");
    else
        return qsl("%L1 Monate").arg(v);
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
        return qsl("thesaur.");
    else
        return qsl("auszahlend");
}
void thesaItemFormatter::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem alignedOption(option);
    alignedOption.displayAlignment = Qt::AlignCenter;
    QStyledItemDelegate::paint(painter, alignedOption, index);
}

QString bookingTypeFormatter::displayText(const QVariant &value, const QLocale &) const
{
    switch(value.toInt())
    {
    case booking::Type::deposit :
        return qsl("Einzahlung");
    case booking::Type::payout :
        return qsl("Auszahlung");
    case booking::Type::interestPayout:
        return qsl("Zinsauszahlung");
    case booking::Type::interestDeposit:
        return qsl("Zinsanrechnung");
    default:
        return qsl("FEHLER");
    }
}
void bookingTypeFormatter::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem alignedOption(option);
    alignedOption.displayAlignment = Qt::AlignCenter;
    QStyledItemDelegate::paint(painter, alignedOption, index);
}
