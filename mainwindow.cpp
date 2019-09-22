#include <QtCore>

#include "dkdbhelper.h"
#include "filehelper.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "windows.h"
#include <qpair.h>
#include <qfiledialog.h>
#include <QRandomGenerator>
#include <QMessageBox>
#include <qsqlquery.h>
#include <qsqlerror.h>
#include <qsqltablemodel.h>
#include <qsqlquerymodel.h>
#include <qsqlrecord.h>
#include <qmap.h>

void MainWindow::preparePersonTableView()
{   QSqlTableModel* model = new QSqlTableModel(ui->PersonsTable);
    //model->setQuery("SELECT Vorname, Name, Strasse, PLZ, Stadt FROM DkGeber");
    model->setTable("DKGeber");
    model->select();
 
    ui->PersonsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->PersonsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->PersonsTable->setModel(model);
    ui->PersonsTable->hideColumn(0);
}

void MainWindow::setCurrentDbInStatusBar()
{
    QSettings config;
    ui->statusLabel->setText(config.value("db/last").toString());
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
#ifdef QT_DEBUG
    ui->menuDebug->setTitle("Debug");
#endif

    ui->statusBar->addPermanentWidget(ui->statusLabel);
    setCurrentDbInStatusBar();

    setCentralWidget(ui->stackedWidget);
    openAppDefaultDb();


    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_Neue_DB_anlegen_triggered()
{
    QString dbfile = QFileDialog::getSaveFileName(this, "Neue DkVerarbeitungs Datenbank", "*.s3db", "dk-DB Dateien (*.s3db)", nullptr);
    if( dbfile == "")
        return;

    backupFile(dbfile);
    createDKDB(dbfile);
    QSettings config;
    config.setValue("db/last", dbfile);
    openAppDefaultDb();
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
    setCurrentDbInStatusBar();
}

void MainWindow::on_actionProgramm_beenden_triggered()
{
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
    this->close();
}

void MainWindow::on_actionDBoeffnen_triggered()
{
    QString dbfile = QFileDialog::getOpenFileName(this, "DkVerarbeitungs Datenbank", "*.s3db", "dk-DB Dateien (*.s3db)", nullptr);
    if( dbfile == "")
    {
        qDebug() << "keine Datei wurde ausgewählt";
        return;
    }
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
    openAppDefaultDb(dbfile);
}

void MainWindow::on_action_Liste_triggered()
{
    preparePersonTableView();
    ui->stackedWidget->setCurrentIndex(PersonListIndex);
    if( !ui->PersonsTable->currentIndex().isValid())
        ui->PersonsTable->selectRow(0);
}

void MainWindow::on_actioncreateSampleData_triggered()
{
    QList<QString> Vornamen {"Holger", "Volker", "Peter", "Hans", "Susi", "Roland", "Claudia", "Emil", "Evelyn", "Ötzgür"};
    QList<QString> Nachnamen {"Maier", "Müller", "Schmit", "Kramp", "Adams", "Häcker", "Maresch", "Beutl", "Chauchev", "Chen"};
    QList<QString> Strassen {"Hauptstrasse", "Nebenstrasse", "Bahnhofstrasse", "Kirchstraße", "Dorfstrasse"};
    QList <QPair<QString, QString>> Cities {{"68305", "Mannheim"}, {"69123", "Heidelberg"}, {"69123", "Karlsruhe"}, {"90345", "Hamburg"}};
    QRandomGenerator rand(::GetTickCount());
    for( int i = 0; i<30; i++)
    {
        PersonData p;
        p.Vorname  =  Vornamen [rand.bounded(Vornamen.count ())];
        p.Nachname = Nachnamen[rand.bounded(Nachnamen.count())];
        p.Strasse =  Strassen[rand.bounded(Strassen.count())];
        p.Plz = Cities[rand.bounded(Cities.count())].first;
        p.Stadt = Cities[rand.bounded(Cities.count())].second;
        p.Iban = "iban xxxxxxxxxxxxxxxxx";
        p.Bic = "BICxxxxxxxx";
        savePersonDataToDatabase(p);
    }
    static_cast<QSqlTableModel*>(ui->PersonsTable->model())->select();
}

bool MainWindow::savePerson()
{
    PersonData p{ -1/*unused*/, ui->leVorname->text(),
                ui->leNachname->text(),
                ui->leStrasse->text(),
                ui->lePlz->text(),
                ui->leStadt->text(),
                ui->leIban->text(),
                ui->leBic->text()};
    if( p.Vorname == "" || p.Nachname == "" || p.Strasse =="" || p.Plz == "" || p.Stadt == "")
    {
        QMessageBox(QMessageBox::Warning, "Daten nicht gespeichert", "Namens - und Adressfelder dürfen nicht leer sein");
        return false;
    }
    savePersonDataToDatabase(p);
    return true;
}

void MainWindow::on_actionNeuer_DK_Geber_triggered()
{
    ui->stackedWidget->setCurrentIndex(newPersonIndex);
}

void MainWindow::on_saveExit_clicked()
{
    if( savePerson())
    {
        emptyEditPersonFields();
    }
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}

void MainWindow::emptyEditPersonFields()
{
    ui->leVorname->setText("");
    ui->leNachname->setText("");
    ui->leStrasse->setText("");
    ui->lePlz->setText("");
    ui->leStadt->setText("");
    ui->leIban->setText("");
    ui->leBic->setText("");
}

void MainWindow::on_saveNew_clicked()
{
    if( savePerson())
    {
        emptyEditPersonFields();
    }
}

void MainWindow::on_saveList_clicked()
{
    if( savePerson())
    {
        emptyEditPersonFields();
        on_action_Liste_triggered();
    }
}

void MainWindow::on_cancel_clicked()
{
    emptyEditPersonFields();
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}


void MainWindow::on_stackedWidget_currentChanged(int arg1)
{
    if( arg1 < 0)
    {
        qWarning() << "stackedWidget changed to non existing page";
        return;
    }
    switch(arg1)
    {
    case emptyPageIndex:
        break;
    case PersonListIndex:
        break;
    case newPersonIndex:
        break;
    case newContractIndex:
        break;
    default:
    {
        qWarning() << "stackedWidget current change not implemented for this index";
        return;
    }
    }// e.o. switch
}

void MainWindow::on_actionVertrag_anlegen_triggered()
{
    // fill combo box with person data
    ui->cbDKGeber->clear();
    QList<PersonDispStringWithId>Entries; AllPersonsForSelection(Entries);
    for(PersonDispStringWithId Entry :Entries)
    {
        ui->cbDKGeber->addItem( Entry.second, QVariant((Entry.first)));
    }

    // What is the persId of the currently selected person in the person?
    int CurrentlySelectedPersonId (-1);
    QModelIndex mi(ui->PersonsTable->currentIndex().siblingAtColumn(0));
    if( mi.isValid())
    {
        QVariant data(ui->PersonsTable->model()->data(mi));
        bool canConvert(false); data.toInt(&canConvert);
        if( !canConvert)
        {
            qCritical() << "Inded der Personenliste konnte nicht bestimmt werden";
            return;
        }
        CurrentlySelectedPersonId =data.toInt();
        // select the correct person
        for( int i = 0; i < ui->cbDKGeber->count(); i++)
        {
            if( CurrentlySelectedPersonId == ui->cbDKGeber->itemData(i))
            {
                ui->cbDKGeber->setCurrentIndex(i);
                break;
            }
        }
    }
    ui->stackedWidget->setCurrentIndex(newContractIndex);
}
