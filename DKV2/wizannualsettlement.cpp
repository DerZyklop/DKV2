
#include <QVariant>
#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>

#include "wizannualsettlement.h"

wizAnnualSettlement_IntroPage::wizAnnualSettlement_IntroPage(QWidget* p)  : QWizardPage(p)
{
    setTitle(qsl("Jahresabrechnung"));
    setSubTitle(qsl("Die Abrechnung für das Folgende Jahr kann gemacht werden:"));
    QLabel* l =new QLabel;
    registerField(qsl("year"), l, "text");
    QCheckBox* csv = new QCheckBox(qsl("Buchungen als csv Datei ausgeben."));
    registerField(qsl("printCsv"), csv);
    QCheckBox* confirm =new QCheckBox(qsl("Jahresabrechnung jetzt durchführen."));
    registerField(qsl("confirm"), confirm);
    QVBoxLayout* bl = new QVBoxLayout;
    bl->addWidget(l);
    bl->addWidget(csv);
    bl->addWidget(confirm);
    setLayout(bl);
}

void wizAnnualSettlement_IntroPage::initializePage()
{
    setField(qsl("printCsv"), true);
    setField(qsl("confirm"), false);
}

bool wizAnnualSettlement_IntroPage::validatePage()
{
    return this->field(qsl("confirm")).toBool();
}

wizAnnualSettlement::wizAnnualSettlement(QWidget* p) : QWizard(p)
{
    addPage(new wizAnnualSettlement_IntroPage);
}
