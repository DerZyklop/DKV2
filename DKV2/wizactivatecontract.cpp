#include <iso646.h>

#include <QLabel>
#include <QDateEdit>
#include <QLineEdit>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QDebug>

#include "appconfig.h"
#include "helper.h"
#include "helperfin.h"
#include "wizactivatecontract.h"

wpActivateContract_IntroPage::wpActivateContract_IntroPage(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Aktivierung eines Vertrags"));
}

void wpActivateContract_IntroPage::initializePage()
{
    wpActivateContract* wiz = qobject_cast<wpActivateContract*>(wizard());
    QString subtitle = qsl("Mit dieser Dialogfolge kannst Du den Vertrag <p><b>%1</b> von <b>%2</b> <p>aktivieren, "
                       "so dass die Zinsberechnung beginnt.<br>"
                       "Die Aktivierung muss nach dem Geldeingang durchgeführt werden.<br>");
    setSubTitle(subtitle.arg(wiz->label, wiz->creditorName));
}

wpActiateContract_DatePage::wpActiateContract_DatePage(QWidget* p) : QWizardPage(p)
{
    QDateEdit* de = new QDateEdit;
    de->setDisplayFormat(qsl("dd.MM.yyyy"));
    registerField(qsl("date"), de);
    QVBoxLayout*  layout = new QVBoxLayout;
    layout->addWidget(de);
    setLayout(layout);
}

void wpActiateContract_DatePage::initializePage()
{
    setTitle(qsl("Aktivierungsdatum"));
    setSubTitle(qsl("Das Aktivierungsdatum entspricht dem Datum, zu dem das Geld auf unserem Konto eingegangen ist"));
}

wpActiateContract_AmountPage::wpActiateContract_AmountPage(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Eingegangener Kreditbetrag"));
    setSubTitle(qsl("Gib die Summe ein, die von dem/der DK-Geber*in überwiesen wurde.<br> "
                "Diese entspricht normalerweise dem im Vertrag vereinbarten Kreditbetrag."));
    QVBoxLayout*  layout = new QVBoxLayout;
    QLabel* l = new QLabel(qsl("Betrag in Euro"));
    QLineEdit* le = new QLineEdit;
    registerField(qsl("amount"), le);
    le->setValidator(new QIntValidator(this));
    l->setBuddy(le);
    layout->addWidget(l);
    layout->addWidget(le);
    setLayout(layout);
}

bool wpActiateContract_AmountPage::validatePage()
{
    wpActivateContract* wiz = qobject_cast<wpActivateContract*>(wizard());
    double amount = field(qsl("amount")).toDouble();
    if( amount < dbConfig::readValue(MIN_AMOUNT).toDouble())
        return false;
    setField(qsl("amount"), r2(amount));
    if( wiz->expectedAmount not_eq amount) {
        qInfo() << "activation with different amount";
    }
    return true;
}

wpActivateContract_SummaryPage::wpActivateContract_SummaryPage( QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Zusammenfassung"));
    QCheckBox* cb = new QCheckBox(qsl("Die Eingaben sind korrekt!"));
    registerField(qsl("confirmed"), cb);
    QVBoxLayout* layout = new QVBoxLayout;
    layout-> addWidget(cb);
    setLayout(layout);
    connect(cb, &QCheckBox::stateChanged, this, &wpActivateContract_SummaryPage::onConfirmData_toggled);
}

void wpActivateContract_SummaryPage::initializePage()
{
    QString subt =qsl("Der Vertrag <p><b>%1</b> von <b>%2</b><p> soll mit einem Betrag von <p>"
                  "<b>%3 </b><p> zum %4 aktiviert werden. <br>");
    wpActivateContract* wiz = qobject_cast<wpActivateContract*>(wizard());
    double amount = field(qsl("amount")).toDouble();
    QLocale locale;
    subt = subt.arg(wiz->label, wiz->creditorName, locale.toCurrencyString(amount), field(qsl("date")).toDate().toString(qsl("dd.MM.yyyy")));
    if( amount not_eq wiz->expectedAmount)
        subt += qsl(" <b><small>Der Überweisungsbetrag stimmt nicht mit dem Kreditbetrag überein.</small></b>");
    setSubTitle(subt);
}

bool wpActivateContract_SummaryPage::validatePage()
{
    if( field(qsl("confirmed")).toBool())
        return true;
    return false;
}
void wpActivateContract_SummaryPage::onConfirmData_toggled(int )
{
    emit completeChanged();
}
bool wpActivateContract_SummaryPage::isComplete() const
{
    return field(qsl("confirmed")).toBool();
}

wpActivateContract::wpActivateContract(QWidget* p) : QWizard (p)
{
    QFont f = font(); f.setPointSize(10); setFont(f);
    addPage(new wpActivateContract_IntroPage);
    addPage(new wpActiateContract_DatePage);
    addPage(new wpActiateContract_AmountPage);
    addPage(new wpActivateContract_SummaryPage);
}
