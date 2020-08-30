#include <QDate>
#include <QLocale>

#include "appconfig.h"
#include "helper.h"
#include "dkdbhelper.h"
#include "reporthtml.h"

// statistics pages - Helper Fu
QString tag(QString string, QString tag, QStringList attribs =QStringList())
{
    QString ret(qsl("<")+tag);
    for(auto& s : qAsConst(attribs)) {
        ret += qsl(" ") +s;
    }
    return ret + qsl(">") + string + qsl("</") + tag + qsl(">");
}

QString tableRow4( QString left, QString center, QString center2, QString right)
{
    left   =tag(left,    qsl("td"), {qsl("style='text-align: right;'")});
    center =tag(center,  qsl("td"), {qsl("style='text-align: center;'")});
    center2=tag(center2, qsl("td"), {qsl("style='text-align: center;'")});
    right  =tag(right,   qsl("td"), {qsl("style='text-align: left;'")});
    return tag( left + center + center2 + right, "tr");
}
QString tableRow3( QString left, QString center, QString right)
{
    left   = qsl("<td style='text-align: right;' >") + left   + qsl("</td>");
    center = qsl("<td style='text-align: center;'>") + center + qsl("</td>");
    right  = qsl("<td style='text-align: left;'  >") + right  + qsl("</td>");
    return "<tr>" + left + center + right  + "</tr>";
}
QString tableRow2(QString left, QString right)
{
    left = qsl("<td style='text-align: right;'>") + left  + qsl("</td>");
    right= qsl("<td style='text-align: left;' >") + right + qsl("</td>");
    return qsl("<tr>") + left + right  + qsl("</tr>");
}
QString emptyRow( )
{
    return qsl("<tr><td style='padding: 1px; font-size: small;'></td><td style='padding: 1px; font-size: small';></td></tr>");
}
QString b(QString b) {
    return tag(b, qsl("b"));
}
QString h2(QString v) {
    return tag(v, qsl("h2"));
}
QString h1(QString v) {
    return tag(v, qsl("h1"));
}
QString td( QString v)
{
    return qsl("<td>") + v + qsl("</td>");
}
QString startTable()
{
    return qsl("<table cellpadding='8' bgcolor=#DDD>");
}
QString endTable()
{
    return qsl("</table>");
}
QString row( QString cont)
{
    return qsl("<tr>") + cont + qsl("</tr>");
}
QString startRow()
{
    return qsl("<tr>");
}
QString endRow()
{
    return qsl("</t>");
}
QString newLine(QString line)
{
    return qsl("<br>") + line;
}

QString htmlOverviewTable()
{
    QLocale locale;
    QString ret;
    DbSummary dbs =calculateSummary();
    ret += h1(qsl("Übersicht DKs und DK Geber"));
    ret += appConfig::getRuntimeData(GMBH_ADDRESS1);
    ret += qsl(" - Stand: ") + QDate::currentDate().toString(qsl("dd.MM.yyyy<br>"));
    ret += startTable();
    ret += tableRow2(qsl("Anzahl DK Geber*innen von aktiven Verträgen:"), QString::number(dbs.creditorsWithActiveContracts));
    ret += tableRow2(qsl("Anzahl aktiver Direktkredite:") , b(QString::number(dbs.activeContracts)));
    ret += tableRow2(qsl("Wert der aktiven Direktkredite:")  , b(locale.toCurrencyString(dbs.valueActiveContracts)) + qsl("<br><small>(Ø ") + locale.toCurrencyString(dbs.valueActiveContracts/dbs.activeContracts) + qsl(")</small>"));
    ret += tableRow2(qsl("Durchschnittlicher Zinssatz:<br><small>(Gewichtet mit Vertragswert)</small>"), QString::number(dbs.weightedAverageInterest, 'f', 4) + qsl("%"));
    ret += tableRow2(qsl("Jährliche Zinskosten:"), locale.toCurrencyString(dbs.valueActiveContracts * dbs.weightedAverageInterest/100.));
    ret += tableRow2(qsl("Mittlerer Zinssatz:"), QString::number(dbs.MittlererZins, 'f', 4) + "%");
    ret += emptyRow();
    ret += tableRow2(qsl("Anzahl mit jährl. Zinsauszahlung:"), QString::number(dbs.AnzahlAuszahlende));
    ret += tableRow2(qsl("Summe:"), locale.toCurrencyString(dbs.BetragAuszahlende));
    ret += emptyRow();
    ret += tableRow2(qsl("Anzahl ohne jährl. Zinsauszahlung:"), QString::number(dbs.AnzahlThesaurierende));
    ret += tableRow2(qsl("Wert inkl. Zinsen:"), locale.toCurrencyString(dbs.WertThesaurierende));
    ret += emptyRow();
    ret += tableRow2(qsl("Anzahl ausstehender (inaktiven) DK"), b(QString::number(dbs.inactiveContracts)));
    ret += tableRow2(qsl("Summe ausstehender (inaktiven) DK"), b(locale.toCurrencyString(dbs.valueInactiveContracts)));
    ret += endTable();
    return ret;
}
QString htmlContractsByContractEndTable()
{
    QLocale locale;
    QString ret;
    ret += h1(qsl("Auslaufende Verträge")) + newLine( qsl("Stand: ")  + QDate::currentDate().toString(qsl("dd.MM.yyyy<br>")));
     QVector<ContractEnd> ce;
     calc_contractEnd(ce);
     if( !ce.isEmpty()) {
         ret += startTable();
         ret += tableRow3(h2(qsl("Jahr")), h2(qsl("Anzahl")),  h2(qsl("Summe")));
         for( auto& x: qAsConst(ce))
             ret += tableRow3( QString::number(x.year), QString::number(x.count), locale.toCurrencyString(x.value));
         ret += endTable();
     }
     else
         ret += qsl("<br><br><i>keine Verträge mit vorgemerktem Vertragsende</i>");
     return ret;
}
QString htmlContractsByYearByInterestTable()
{
    QLocale locale;
    QString ret;
    QVector<YZV> yzv;
    calc_anualInterestDistribution( yzv);
    if( !yzv.isEmpty()) {
        ret += h1(qsl("Verteilung der Zinssätze pro Jahr")) + qsl("<br> Stand:")  + QDate::currentDate().toString(qsl("dd.MM.yyyy<br>"));
        ret += startTable() +  startRow();
        ret += td(h2(qsl("Jahr"))) + td( h2(qsl("Zinssatz"))) +td(h2(qsl("Anzahl"))) + td( h2( qsl("Summe")));
        ret += endRow();
        for( auto& x: qAsConst(yzv)) {
            ret += tableRow4( QString::number(x.year), QString(qsl("%1%")).arg(x.intrest, 2, 'g'), QString::number(x.count), locale.toCurrencyString(x.sum));
        }
        ret += endTable();
    }
    return ret;
}
QString htmlContractsByRuntimeTable()
{
    QString ret;
    ret += h1(qsl("Vertragslaufzeiten")) + qsl("<br> Stand:") + QDate::currentDate().toString(qsl("dd.MM.yyyy<br>"));
    ret += startTable();
    QVector<rowData> rows = contractRuntimeDistribution();
    ret += tableRow3( h2(rows[0].text), h2(rows[0].value), h2(rows[0].number));
    for( int i = 1; i < rows.count(); i++)
        ret += tableRow3
                (rows[i].text, rows[i].value, rows[i].number);
    ret += endTable();
    return ret;
}
QString htmlAllContractInfo()
{
    dbtable t(qsl("ContractDataActiveContracts"));
    t.append(dbfield(qsl("Id"), QVariant::Type::Int));
    t.append(dbfield(qsl("KreditorId"), QVariant::Type::Int));
    t.append(dbfield(qsl("Vorname")));
    t.append(dbfield(qsl("Nachname")));
    t.append(dbfield(qsl("Strasse")));
    t.append(dbfield(qsl("Plz")));
    t.append(dbfield(qsl("Stadt")));
    t.append(dbfield(qsl("Email")));
    t.append(dbfield(qsl("Iban")));
    t.append(dbfield(qsl("Bic")));
    t.append(dbfield(qsl("Strasse")));
    t.append(dbfield(qsl("Zinssatz"), QVariant::Type::Double));
    t.append(dbfield(qsl("Wert"), QVariant::Type::Double));
    t.append(dbfield(qsl("Aktivierungsdatum"), QVariant::Type::Date));
    t.append(dbfield(qsl("Kuendigungsfrist"), QVariant::Type::Int));
    t.append(dbfield(qsl("Vertragsende"), QVariant::Type::Date));
    t.append(dbfield(qsl("thesa"), QVariant::Type::Bool));

    QString ret =startTable();
    ret += startRow();
    for(auto& field : t.Fields()) {
        ret +=tag(field.name(), qsl("th"));
    }
    ret += endRow();
    QVector<QSqlRecord> data = executeSql(t.Fields());
    for(auto& rec : qAsConst(data)) {
        ret += startRow();
        for( int i =0; i<rec.count(); i++) {
            ret += td(rec.field(i).value().toString());
        }
        ret += endRow();
    }
    ret += endTable();
    return ret;
}
QString reportHtml(Uebersichten u)
{
    QString html =qsl("<html><body>"
                    "<style>"
                      "table { border-width: 0px; font-family: Verdana; font-size: large; }"
                      "td { }"
                    "</style>"
                    "%1"
                  "</body></html>");
    switch( u ) {
    case OVERVIEW: {
        html =html.arg(htmlOverviewTable());
        break;
    }
    case BY_CONTRACT_END: {
        html =html.arg(htmlContractsByContractEndTable());
        break;
    }
    case INTEREST_DISTRIBUTION: {
        html =html.arg(htmlContractsByYearByInterestTable());
        break;
    }
    case CONTRACT_TERMS: {
        html =html.arg(htmlContractsByRuntimeTable());
        break;
    }
    case ALL_CONTRACT_INFO: {
        html =html.arg(htmlAllContractInfo());
        break;
    }
    default:
    {Q_ASSERT(false);}
    }
    return html;
}

