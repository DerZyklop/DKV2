#ifndef TEST_CONTRACT_H
#define TEST_CONTRACT_H

#include <QSqlDatabase>
#include <QObject>

#include "testhelper.h"

class test_contract : public QObject
{
    Q_OBJECT
public:
    explicit test_contract(QObject *parent = nullptr) : QObject(parent){}
    ~test_contract(){}
private:
    // helper
signals:
private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    // the actual tests
    void test_createContract();
    void test_set_get_interest();
    void test_activateContract();
    void test_randomContract();
    void test_randomContracts();
    void test_write_read_contract();
    void deposit_inactive_contract_fails();
    void too_high_payout_fails();
    void unsequenced_bookings_fail();

    void test_annualSettlement_inactive_fails();
    void test_annualSettlement_fullYear();
    void test_annualSettlement_twoYear();
    void test_deposit01();
    void test_depositFailsOn_1_1();
    void test_deposit_wSettlement();
    void test_payout();
    void test_payout_wSettlement();

    void test_activationDate();
    //void test_latestSettlementDate();
    void test_getValue_byDate();
    void test_getValue_byDate_wInterestPayout();
    void test_finalize();
};

#endif // TEST_CONTRACT_H
