
#include <QString>
#include <QMessageBox>
#include <QDate>

#include "askdatedlg.h"
#include "helper.h"
#include "finhelper.h"
#include "letters.h"
#include "transaktionen.h"


void aktiviereVertrag(int contractId)
{   LOG_CALL;
    // todo: wizard UI Geldeingang:
    // - Wann?
    // - Wieviel? -> go
    askDateDlg dlg( nullptr, QDate::currentDate());
    dlg.setMsg("<H3>Mit der Aktivierung des Vertrags beginnt die Zinsberechnung. <br>Bitte geben Sie das Datum des Geldeingangs ein:</H3>");
    dlg.setDateLabel("Die Verzinsung beginnt am");
    if( QDialog::Accepted != dlg.exec())
        return;

    contract v(contractId);
    if( !v.activate(dlg.getDate(), v.plannedInvest()))
        qCritical() << "activation failed";
}

void beendeVertrag(int )
{   LOG_CALL;
Q_ASSERT(!"repair");
    contract v;
}
void VertragsEnde_LaufzeitEnde( contract& )
{   LOG_CALL;
Q_ASSERT(!"repair");
    return;
}
void VertragsEnde_MitKFrist( contract& )
{   LOG_CALL;
Q_ASSERT(!"repair");
}
void VertragsEnde_PassiverV(contract& )
{   LOG_CALL;
Q_ASSERT(!"repair");
}

