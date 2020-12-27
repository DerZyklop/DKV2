#ifndef BOOKING_H
#define BOOKING_H

#include <QDate>
#include "helper.h"
#include "dbtable.h"

struct booking
{
enum class Type{
    non, // means all
    deposit = 1, payout  = 2,
    reInvestInterest = 4,
    annualInterestDeposit = 8
    };
static QString displayString(Type t);
inline static int bookingTypeToInt(booking::Type t) {
    return static_cast<int>(t);
}

qlonglong contractId =-1;
Type type =Type::non;
QDate date =EndOfTheFuckingWorld;
double amount =0.;
    // construction
    booking(qlonglong cId, booking::Type t = Type::non, QDate d =EndOfTheFuckingWorld, double a =0.) : contractId(cId), type(t), date(d), amount(a) {};
    // comparison for tests
    inline friend bool operator==(const booking& lhs, const booking& rhs)
    {
        QString error;
        if( (lhs.type != rhs.type)) error =qsl("comparing bookings: different types");
        if( (lhs.date != rhs.date)) error += qsl(", comparing bookings: different dates");
        if( (lhs.amount != rhs.amount)) error += qsl(", comparing bookings: different amounts");
        if( (lhs.contractId!= rhs.contractId)) error += qsl(", comparing bookings: different contractIds");
        if(error.isEmpty())
            return true;
        else {
            qInfo() << error;
            return false;
        }
    }
    // statics
    static const dbtable& getTableDef();
    static const dbtable& getTableDef_deletedBookings();
    static bool bookDeposit(   const qlonglong contractId, const QDate date, const double amount);
    static bool bookPayout(    const qlonglong contractId, const QDate date, const double amount);
    static bool bookReInvestInterest(const qlonglong contractId, const QDate date, const double amount);
    static bool bookAnnualInterestDeposit( const qlonglong contractId, const QDate date, const double amount);
    static QString typeName(Type t);
private:
    static bool doBooking( const booking::Type, const qlonglong contractId, const QDate date, const double amount);
};

struct bookings
{
    static QDate dateOfnextSettlement();
    static QVector<booking> bookingsFromSql(QString where, QString order=QString());
    static QVector<booking> getBookings(qlonglong cid, QDate from =BeginingOfTime, QDate to =EndOfTheFuckingWorld);
    static QVector<booking> getAnnualSettelments(int year);
};

#endif // BOOKING_H

const QVector<booking>& bookingsFromSql(QString& where);
