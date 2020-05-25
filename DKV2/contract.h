#ifndef VERTRAG_H
#define VERTRAG_H

#include <QString>
#include <QDate>

#include "helper.h"
#include "finhelper.h"
#include "tabledatainserter.h"
#include "dkdbhelper.h"
#include "creditor.h"


struct contract
{
    // construction
    contract() :td(getTableDef()) { setId(-1);};

    // getter & setter
    void setId(qlonglong id) { td.setValue("id", id);}
    qlonglong id() const{ return td.getValue("id").toLongLong();}

    void setCreditorId(qlonglong kid) {td.setValue("kreditorId", kid);}
    qlonglong creditorId() const{ return td.getValue("KreditorId").toLongLong();}

    void setLabel(QString l) { return td.setValue("Kennung", l);}
    QString label() const { return td.getValue("Kennung").toString();};

    void setInterestRate( int percentpercent) {td.setValue("ZSatz", percentpercent);}
    void setInterestRate( double percent) {td.setValue("ZSatz", int(percent*100));}

    double interestRate() const { return double(td.getValue("ZSatz").toInt())/100.;}
    int interestRateInt() const { return td.getValue("ZSatz").toInt()*100;}

    void setPlannedInvest(int i) { td.setValue("Betrag", i);}
    int plannedInvest() const { return td.getValue("Betrag").toInt();}

    void setReinvesting( bool b) { td.setValue("thesaurierend", b);}
    bool reinvesting() const { return (td.getValue("thesaurierend").toInt() != 0);}

    void setNoticePeriod(int m) { td.setValue("Kfrist", m); if( -1 != m) setPlannedEndDate( EndOfTheFuckingWorld);}
    int noticePeriod() const { return td.getValue("Kfrist").toInt();}

    void setPlannedEndDate( QDate d) { td.setValue("LaufzeitEnde", d); if( d != EndOfTheFuckingWorld) setNoticePeriod(-1);}
    QDate plannedEndDate() const { return td.getValue("LaufzeitEnde").toDate();}

    void setConclusionDate(QDate d) { td.setValue("Vertragsdatum", d);}
    QDate conclusionDate() const { return td.getValue("Vertragsdatum").toDate();}

    // interface
    static const dbtable& getTableDef();
    bool validateAndSaveNewContract(QString& meldung);
    int saveNewContract();

//    bool loadContractFromDb(qlonglong id);
//    bool activateContract(const QDate& aDate);
//    bool bookAnnualInterest(const QDate& YearEnd);
//    bool cancelActiveContract(const QDate& kTermin);
//    bool terminateActiveContract(const QDate& termin);
//    bool deleteInactiveContract();

private:
    // data
    TableDataInserter td;
    // helper
};

// for testing
contract saveRandomContract(qlonglong creditorId);
void saveRandomContracts(int count);

#endif // VERTRAG_H
