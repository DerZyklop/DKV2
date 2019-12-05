#include <QSqlQuery>
#include <QVector>

#include "helper.h"
#include "sqlhelper.h"
#include "vertrag.h"
#include "jahresabschluss.h"

jahresabschluss::jahresabschluss()
{
    Jahr = JahreszahlFuerAbschluss();
}

int jahresabschluss::JahreszahlFuerAbschluss()
{LOG_ENTRY_and_EXIT;
    QDate aeltesteZinszahlung = ExecuteSingleValueSql("SELECT min(LetzteZinsberechnung) FROM Vertraege WHERE aktiv != 0").toDate();
    if( aeltesteZinszahlung.month()==12 && aeltesteZinszahlung.day() == 31)
        return aeltesteZinszahlung.year() +1;
    return aeltesteZinszahlung.year();
}

bool jahresabschluss::execute()
{LOG_ENTRY_and_EXIT;
    QSqlQuery sql; sql.prepare( "SELECT Vertraege.id "
                "FROM Vertraege WHERE aktiv != 0");
    if( !(sql.exec() && sql.first()))
    {
        qCritical() << "faild to select contracts: " << sql.lastError() << endl << "in " << sql.lastQuery();
        return false;
    }
    const QDate YearEnd= QDate(Jahr, 12, 31);

    do
    {
        Vertrag v; v.ausDb(sqlVal<int>(sql, "id"), true);
        if( v.verbucheJahreszins(YearEnd))
        {
            if( v.Tesaurierend())
                tesaV.push_back(v);
            else
                n_tesaV.append(v);
        }

    } while(sql.next());

    return true;
}
