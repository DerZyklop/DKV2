#ifndef TEST_VIEWS_H
#define TEST_VIEWS_H

#include <QSqlDatabase>
#include <QObject>

#include "../DKV2/creditor.h"
#include "../DKV2/contract.h"

#include "testhelper.h"

class test_views : public QObject
{
    Q_OBJECT
public:
    explicit test_views(QObject *parent = nullptr) : QObject(parent){}
    ~test_views(){}

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void test_wertPassiveVertraege_oneContract();
};

#endif // TEST_VIEWS_H
