#ifndef WIZANNUALSETTLEMENT_H
#define WIZANNUALSETTLEMENT_H

#include <QDate>
#include <QStringLiteral>
#define qsl(x) QStringLiteral(x)
#include <QWizard>

struct wizAnnualSettlement_IntroPage : public QWizardPage
{
    wizAnnualSettlement_IntroPage(QWidget* p =nullptr);
    void initializePage() override;
    bool validatePage() override;
    Q_OBJECT;
};

struct wizAnnualSettlement : public QWizard
{
    wizAnnualSettlement(QWidget* p =nullptr);
    int year;
    Q_OBJECT;
};

#endif // WIZANNUALSETTLEMENT_H
