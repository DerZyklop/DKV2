#ifndef VERTRAG_H
#define VERTRAG_H

#include <QString>
#include <QVector>
#include <QDate>

#include "helper.h"
#include "helperfin.h"
#include "tabledatainserter.h"
#include "dkdbhelper.h"
#include "booking.h"
#include "creditor.h"

struct contract
{
    static const dbtable& getTableDef();
    static const dbtable& getTableDef_deletedContracts();
    // construction
    contract() :td(getTableDef()) { setId(-1);};
    contract(qlonglong id) : td(getTableDef()) { fromDb(id); };
    // getter & setter
    void setId(qlonglong id) { td.setValue("id", id);}
    qlonglong id() const { return td.getValue("id").toLongLong();}
    QString id_aS()   const { return QString::number(id());}
    void setCreditorId(qlonglong kid) {td.setValue("kreditorId", kid);}
    qlonglong creditorId() const{ return td.getValue("KreditorId").toLongLong();}
    void setLabel(QString l) { td.setValue("Kennung", l);}
    QString label() const { return td.getValue("Kennung").toString();};
    void setInterest100th( int percentpercent) {td.setValue("ZSatz", percentpercent);}
    void setInterestRate( double percent) {td.setValue("ZSatz", int(percent*100));}
    double interestRate() const { return double(td.getValue("ZSatz").toInt())/100.;}
    //int interestRate100th() const { return td.getValue("ZSatz").toInt()*100;}
    void setPlannedInvest(double d) { td.setValue("Betrag", ctFromEuro(d));}
    double plannedInvest() const { return euroFromCt( td.getValue("Betrag").toInt());}
    void setReinvesting( bool b =true) { td.setValue("thesaurierend", b);}
    bool reinvesting() const { return (td.getValue("thesaurierend").toInt() != 0);}
    void setNoticePeriod(int m) { td.setValue("Kfrist", m); if( -1 != m) setPlannedEndDate( EndOfTheFuckingWorld);}
    int noticePeriod() const { return td.getValue("Kfrist").toInt();}
    void setPlannedEndDate( QDate d) { td.setValue("LaufzeitEnde", d); if( d != EndOfTheFuckingWorld) setNoticePeriod(-1);}
    QDate plannedEndDate() const { return td.getValue("LaufzeitEnde").toDate();}
    void setConclusionDate(QDate d) { td.setValue("Vertragsdatum", d);}
    QDate conclusionDate() const { return td.getValue("Vertragsdatum").toDate();}

    // interface
    bool validateAndSaveNewContract(QString& meldung);
    int saveNewContract();
    double Value() const;
    double getValue(QDate d) const;
    QDate latestBooking() const;

    bool activate(int amount_ct, const QDate& aDate) const;
    bool activate(double amount, const QDate& aDate) const;
    static bool isActive( qlonglong id);
    bool isActive() const;
    QDate activationDate() const;
//    QDate latestInterestPaymentDate() const;

    int annualSettlement() const;
    bool bookInterest(QDate d) const;
    bool deposit(double amount, QDate d) const;
    bool payout(double amount, QDate d) const;

    bool remove() const;
    static bool remove(qlonglong id);

//    bool bookAnnualInterest(const QDate& YearEnd);
//    bool cancelActiveContract(const QDate& kTermin);

    inline friend bool operator==(const contract& lhs, const contract& rhs)
    {
        return lhs.td == rhs.td;
    }
private:
    // data
    TableDataInserter td;
    // helper
    bool fromDb(qlonglong i);
};

// for testing
contract saveRandomContract(qlonglong creditorId);
void saveRandomContracts(int count);
void activateRandomContracts(int percent);

#endif // VERTRAG_H
