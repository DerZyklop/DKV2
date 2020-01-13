#include <QtCore>

#if defined(Q_OS_WIN)
#include "windows.h"
#else
#include <stdlib.h>
#endif
#include <QPair>
#include <QFileDialog>
#include <QRandomGenerator>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QSqlField>
#include <QMap>

#include <QPdfWriter>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "askdatedlg.h"
#include "helper.h"
#include "filehelper.h"
#include "itemformatter.h"
#include "sqlhelper.h"
#include "finhelper.h"
#include "vertrag.h"
#include "kreditor.h"
#include "dkdbhelper.h"
#include "jahresabschluss.h"
#include "frmjahresabschluss.h"

// construction, destruction
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{LOG_ENTRY_and_EXIT;
    ui->setupUi(this);
//#ifdef QT_DEBUG
    ui->menuDebug->setTitle("Debug");
//#endif

    ui->leBetrag->setValidator(new QIntValidator(0,999999,this));
    ui->statusBar->addPermanentWidget(ui->statusLabel);

    setCentralWidget(ui->stackedWidget);
    DatenbankZurAnwendungOeffnen();
    DbInStatuszeileAnzeigen();
    prepareWelcomeMsg();

    ui->txtAnmerkung->setTabChangesFocus(true);
    ui->PersonsTableView->setStyleSheet("QTableView::item { padding-right: 10px; padding-left: 15px; }");
    ui->contractsTableView->setStyleSheet("QTableView::item { padding-right: 10px; padding-left: 15px; }");

    ui->cbKFrist->addItem("festes Vertragsende", QVariant(-1));
    for (int i=3; i<25; i++)
        ui->cbKFrist->addItem(QString::number(i), QVariant(i));

    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setSplash(QSplashScreen* s)
{
    splash = s;
    startTimer(3333);
}

void MainWindow::DbInStatuszeileAnzeigen()
{
    QSettings config;
    ui->statusLabel->setText(config.value("db/last").toString());
}

void MainWindow::prepareWelcomeMsg()
{
    QString message="<H2>Willkommen zu DKV2- Deiner Verwaltung von Direktrediten</H2>";

    QStringList warnings;
    CheckDbConsistency( warnings);

    foreach(QString warning, warnings)
    {
        message += "<br><font color='red'>" +warning +"</font>";
    }
    message += "<br><img src=':/res/Logo.PNG'>";
    ui->label->setText(message);
}
// whenever the stackedWidget changes ...
void MainWindow::on_stackedWidget_currentChanged(int arg1)
{    LOG_ENTRY_and_EXIT;
    if( arg1 < 0)
    {
        qWarning() << "stackedWidget changed to non existing page";
        return;
    }
    switch(arg1)
    {
    case emptyPageIndex:
        prepareWelcomeMsg();
        ui->actionKreditgeber_loeschen->setEnabled(false);
        ui->actionVertrag_passiv_loeschen->setEnabled(false);
        break;
    case PersonListIndex:
        ui->actionKreditgeber_loeschen->setEnabled(true);
        ui->actionVertrag_passiv_loeschen->setEnabled(false);
        break;
    case newPersonIndex:
        ui->actionKreditgeber_loeschen->setEnabled(false);
        ui->actionVertrag_passiv_loeschen->setEnabled(false);
        break;
    case newContractIndex:
        ui->actionKreditgeber_loeschen->setEnabled(false);
        ui->actionVertrag_passiv_loeschen->setEnabled(false);
        break;
    case ContractsListIndex:
        ui->actionKreditgeber_loeschen->setEnabled(false);
        ui->actionVertrag_passiv_loeschen->setEnabled(true);
        break;
    case bookingsListIndex:
        ui->actionKreditgeber_loeschen->setEnabled(false);
        ui->actionVertrag_passiv_loeschen->setEnabled(false);
        break;
    default:
    {
        qWarning() << "stackedWidget current change not implemented for this index";
        return;
    }
    }// e.o. switch
}

// file menu
void MainWindow::on_actionzur_ck_triggered()
{
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}
void MainWindow::on_action_Neue_DB_anlegen_triggered()
{LOG_ENTRY_and_EXIT;
    QString dbfile = QFileDialog::getSaveFileName(this, "Neue DkVerarbeitungs Datenbank", "*.dkdb", "dk-DB Dateien (*.dkdb)", nullptr);
    if( dbfile == "")
        return;

    DKDatenbankAnlegen(dbfile);
    QSettings config;
    config.setValue("db/last", dbfile);
    DatenbankZurAnwendungOeffnen();
    DbInStatuszeileAnzeigen();

    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}
void MainWindow::on_actionDBoeffnen_triggered()
{LOG_ENTRY_and_EXIT;
    QString dbfile = QFileDialog::getOpenFileName(this, "DkVerarbeitungs Datenbank", "*.dkdb", "dk-DB Dateien (*.dkdb)", nullptr);
    if( dbfile == "")
    {
        qDebug() << "keine Datei wurde vom Anwender ausgewählt";
        return;
    }
    DatenbankZurAnwendungOeffnen(dbfile);

    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}
void MainWindow::on_actionProgramm_beenden_triggered()
{LOG_ENTRY_and_EXIT;
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
    this->close();
}

// person list page
void MainWindow::preparePersonTableView()
{LOG_ENTRY_and_EXIT;
    QSqlTableModel* model = new QSqlTableModel(ui->PersonsTableView);
    model->setTable("Kreditoren");
    model->setFilter("Vorname LIKE '%" + ui->leFilter->text() + "%' OR Nachname LIKE '%" + ui->leFilter->text() + "%'");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

    ui->PersonsTableView->setEditTriggers(QTableView::NoEditTriggers);
    ui->PersonsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->PersonsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->PersonsTableView->setAlternatingRowColors(true);
    ui->PersonsTableView->setSortingEnabled(true);
    ui->PersonsTableView->setModel(model);
    ui->PersonsTableView->hideColumn(0);
    ui->PersonsTableView->resizeColumnsToContents();
}
void MainWindow::on_action_Liste_triggered()
{LOG_ENTRY_and_EXIT;
    preparePersonTableView();
    if( !ui->PersonsTableView->currentIndex().isValid())
        ui->PersonsTableView->selectRow(0);

    ui->stackedWidget->setCurrentIndex(PersonListIndex);
}
void MainWindow::on_actionKreditgeber_loeschen_triggered()
{LOG_ENTRY_and_EXIT;
    QString msg( "Soll der Kreditgeber ");
    QModelIndex mi(ui->PersonsTableView->currentIndex());
    QString Vorname = ui->PersonsTableView->model()->data(mi.siblingAtColumn(1)).toString();
    QString Nachname = ui->PersonsTableView->model()->data(mi.siblingAtColumn(2)).toString();
    QString index = ui->PersonsTableView->model()->data(mi.siblingAtColumn(0)).toString();
    msg += Vorname + " " + Nachname + " (id " + index + ") mit allen Verträgen und Buchungen gelöscht werden?";
    if( QMessageBox::Yes != QMessageBox::question(this, "Kreditgeber löschen?", msg))
        return;

    if( Kreditor::Loeschen(index.toInt()))
        preparePersonTableView();
    else
        Q_ASSERT(!bool("could not remove kreditor and contracts"));
}

void MainWindow::on_actionVertraege_zeigen_triggered()
{
    QModelIndex mi(ui->PersonsTableView->currentIndex());
    QString index = ui->PersonsTableView->model()->data(mi.siblingAtColumn(0)).toString();
    ui->leVertraegeFilter->setText(index);
    on_actionListe_der_Vertraege_anzeigen_triggered();
}
void MainWindow::on_PersonsTableView_customContextMenuRequested(const QPoint &pos)
{LOG_ENTRY_and_EXIT;
    QModelIndex index = ui->PersonsTableView->indexAt(pos).siblingAtColumn(0);
    if( index.isValid())
    {
        QVariant data(ui->PersonsTableView->model()->data(index));
        bool canConvert(false); data.toInt(&canConvert);
        if( canConvert)
        {
            QMenu menu( "PersonContextMenu", this);
            menu.addAction(ui->actionDkGeberBearbeiten);
            menu.addAction(ui->actionVertrag_anlegen);
            menu.addAction( ui->actionKreditgeber_loeschen);
            menu.addAction(ui->actionVertraege_zeigen);
            menu.exec(ui->PersonsTableView->mapToGlobal(pos));
        }
        else
            qCritical() << "Conversion error: model data is not int";
        return;
    }
}

// debug funktions
void MainWindow::on_actioncreateSampleData_triggered()
{LOG_ENTRY_and_EXIT;
    BeispieldatenAnlegen();
    preparePersonTableView();
    prepareContractListView();
    if( ui->stackedWidget->currentIndex() == OverviewIndex)
        on_action_Uebersicht_triggered();
}
void MainWindow::on_actionanzeigenLog_triggered()
{LOG_ENTRY_and_EXIT;
    #if defined(Q_OS_WIN)
    ::ShellExecuteA(nullptr, "open", logFilePath().toUtf8(), "", QDir::currentPath().toUtf8(), 1);
    #else
    QString cmd = "open " + logFilePath().toUtf8();
    system(cmd.toUtf8().constData());
    #endif
}

// new DK Geber
void MainWindow::on_actionNeuer_DK_Geber_triggered()
{LOG_ENTRY_and_EXIT;
    ui->stackedWidget->setCurrentIndex(newPersonIndex);
}
int MainWindow::KreditgeberSpeichern()
{LOG_ENTRY_and_EXIT;

    Kreditor k;
    k.setValue("Vorname", ui->leVorname->text().trimmed());
    k.setValue("Nachname", ui->leNachname->text().trimmed());
    k.setValue("Strasse", ui->leStrasse->text().trimmed());
    k.setValue("Plz", ui->lePlz->text().trimmed());
    k.setValue("Stadt", ui->leStadt->text().trimmed());
    QString email = ui->leEMail->text().trimmed().toLower();
    k.setValue("Email", email);
    k.setValue("Anmerkung", ui->txtAnmerkung->toPlainText());
    k.setValue("IBAN", ui->leIban->text().trimmed());
    k.setValue("BIC", ui->leBic->text().trimmed());

    QString errortext;
    if( !k.isValid(errortext))
    {
        errortext = "Die Daten konnten nicht gespeichert werden: <br>" + errortext;
        QMessageBox::information(this, "Fehler", errortext );
        qDebug() << "prüfung der Kreditor Daten:" << errortext;
        return -1;
    }
    int kid = -1;
    if( ui->lblPersId->text() != "")
    {
        kid = ui->lblPersId->text().toInt();
        k.setValue("Id", kid);     // update not insert
        k.Update();
    }
    else
       kid = k.Speichern();

    if(kid == -1)
    {
        QMessageBox::information( this, "Fehler", "Der Datensatz konnte nicht gespeichert werden. "
                     "Ist die E-Mail Adresse einmalig? Gibt es die Adressdaten in der Datenbank bereits?"
                     "\nBitte überprüfen Sie ihre Eingaben");
        qCritical() << "Kreditgeber konnte nicht gespeichert werden";
        return -1;
    }

    return kid;
}
void MainWindow::KreditorFormulardatenLoeschen()
{LOG_ENTRY_and_EXIT;
    ui->leVorname->setText("");
    ui->leNachname->setText("");
    ui->leStrasse->setText("");
    ui->lePlz->setText("");
    ui->leStadt->setText("");
    ui->leEMail->setText("");
    ui->txtAnmerkung->setPlainText("");
    ui->leIban->setText("");
    ui->leBic->setText("");
    ui->lblPersId->setText("");
}
void MainWindow::KreditorFormulardatenBelegen(int id)
{
    QSqlRecord rec = ExecuteSingleRecordSql(dkdbstructur["Kreditoren"].Fields(), "Id=" +QString::number(id));
    ui->leVorname->setText(rec.field("Vorname").value().toString());
    ui->leNachname->setText(rec.field("Nachname").value().toString());
    ui->leStrasse->setText(rec.field("Strasse").value().toString());
    ui->lePlz->setText(rec.field("Plz").value().toString());
    ui->leStadt->setText(rec.field("Stadt").value().toString());
    ui->leEMail->setText(rec.field("Email").value().toString());
    ui->txtAnmerkung->setPlainText(rec.field("Anmerkung").value().toString());
    ui->leIban  ->setText(rec.field("IBAN").value().toString());
    ui->leBic  ->setText(rec.field("BIC").value().toString());
}
void MainWindow::on_actionDkGeberBearbeiten_triggered()
{LOG_ENTRY_and_EXIT;

    QModelIndex mi(ui->PersonsTableView->currentIndex());
    QVariant index = ui->PersonsTableView->model()->data(mi.siblingAtColumn(0));
    ui->lblPersId->setText(index.toString());

    KreditorFormulardatenBelegen(index.toInt());
    ui->stackedWidget->setCurrentIndex(newPersonIndex);
}

void MainWindow::on_saveNew_clicked()
{LOG_ENTRY_and_EXIT;
    if( KreditgeberSpeichern() != -1)
        KreditorFormulardatenLoeschen();
}
void MainWindow::on_saveList_clicked()
{LOG_ENTRY_and_EXIT;
    if( KreditgeberSpeichern() != -1)
    {
        KreditorFormulardatenLoeschen();
        on_action_Liste_triggered();
    }
}

void MainWindow::on_saveExit_clicked() // speichern und zu "Vertrag anlegen"
{LOG_ENTRY_and_EXIT;
    int kid = KreditgeberSpeichern();
    if(  kid != -1)
    {
        KreditorFormulardatenLoeschen();
        on_actionVertrag_anlegen_triggered(kid);
    }
}
void MainWindow::on_cancel_clicked()
{LOG_ENTRY_and_EXIT;
    KreditorFormulardatenLoeschen();
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}

// new Contract
Vertrag MainWindow::VertragsdatenAusFormular()
{LOG_ENTRY_and_EXIT;
    int KreditorId = ui->comboKreditoren->itemData(ui->comboKreditoren->currentIndex()).toInt();
    QString Kennung = ui->leKennung->text();
    double Betrag = ui->leBetrag->text().remove('.').toDouble();
    bool thesaurierend = ui->chkbThesaurierend->checkState() == Qt::Checked;
    double Wert = thesaurierend ? Betrag : 0.;
    int ZinsId = ui->cbZins->itemData(ui->cbZins->currentIndex()).toInt();
    QDate Vertragsdatum = ui->deVertragsabschluss->date();

    QDate LaufzeitEnde = ui->deLaufzeitEnde->date();
    QDate StartZinsberechnung = LaufzeitEnde;
    int kFrist = ui->cbKFrist->currentData().toInt();

    return Vertrag(KreditorId, Kennung, Betrag, Wert, ZinsId, Vertragsdatum,
                   thesaurierend, false/*aktiv*/,StartZinsberechnung, kFrist, LaufzeitEnde);
}

bool MainWindow::saveNewContract()
{LOG_ENTRY_and_EXIT;
    Vertrag c =VertragsdatenAusFormular();

    QString errortext;
    if( !c.isNewContractValid(errortext))
    {
        QMessageBox::information( this, "Fehler", errortext);
        return false;
    }
    return true;
}
void MainWindow::clearNewContractFields()
{LOG_ENTRY_and_EXIT;
    ui->leKennung->setText("");
    ui->leBetrag->setText("");
    ui->chkbThesaurierend->setChecked(true);
}

// switch to "Vertrag anlegen"
void MainWindow::FillKreditorDropdown()
{LOG_ENTRY_and_EXIT;
    ui->comboKreditoren->clear();
    QList<QPair<int, QString>> Personen;
    Kreditor k; k.KreditorenMitId(Personen);
    for(auto Entry :Personen)
    {
        ui->comboKreditoren->addItem( Entry.second, QVariant((Entry.first)));
    }
}
void MainWindow::FillRatesDropdown()
{LOG_ENTRY_and_EXIT;
    QList<ZinsAnzeigeMitId> InterrestCbEntries; ZinssaetzeFuerAuswahlliste(InterrestCbEntries);
    ui->cbZins->clear();
    for(ZinsAnzeigeMitId Entry : InterrestCbEntries)
    {
        ui->cbZins->addItem(Entry.second, QVariant(Entry.first));
    }
    ui->cbZins->setCurrentIndex(InterrestCbEntries.count()-1);
}
void MainWindow::comboKreditorenAnzeigeNachKreditorenId(int KreditorenId)
{LOG_ENTRY_and_EXIT;
    if( KreditorenId < 0) return;
    // select the correct person
    for( int i = 0; i < ui->comboKreditoren->count(); i++)
    {
        if( KreditorenId == ui->comboKreditoren->itemData(i))
        {
            ui->comboKreditoren->setCurrentIndex(i);
            break;
        }
    }
}
void MainWindow::on_cbKFrist_currentIndexChanged(int index)
{
    if( -1 == ui->cbKFrist->itemData(index).toInt())
    {   // Vertragsende wird fest vorgegeben
        if( EndOfTheFuckingWorld == ui->deLaufzeitEnde->date())
        {
            ui->deLaufzeitEnde->setDate(QDate::currentDate().addYears(5));
        }
    }
    else
    {   // Vertragsende wird durch Kündigung eingeleitet
        ui->deLaufzeitEnde->setDate(EndOfTheFuckingWorld);
    }
}

void MainWindow::on_speichereVertragZurKreditorenListe_clicked()
{LOG_ENTRY_and_EXIT;
    if( saveNewContract())
    {
        clearNewContractFields();
        ui->stackedWidget->setCurrentIndex(PersonListIndex);
    }
}
void MainWindow::on_saveContractGoContracts_clicked()
{LOG_ENTRY_and_EXIT;
    if( saveNewContract())
    {
        clearNewContractFields();
        prepareContractListView();
        ui->stackedWidget->setCurrentIndex(ContractsListIndex);
    }
}
void MainWindow::on_cancelCreateContract_clicked()
{LOG_ENTRY_and_EXIT;
    clearNewContractFields();
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}

int MainWindow::getPersonIdFromKreditorenList()
{LOG_ENTRY_and_EXIT;
    // What is the persId of the currently selected person in the person?
    QModelIndex mi(ui->PersonsTableView->currentIndex().siblingAtColumn(0));
    if( mi.isValid())
    {
        QVariant data(ui->PersonsTableView->model()->data(mi));
        return data.toInt();
    }
    qCritical() << "Index der Personenliste konnte nicht bestimmt werden";
    return -1;
}

void MainWindow::on_actionVertrag_anlegen_triggered(int id)
{LOG_ENTRY_and_EXIT;
    FillKreditorDropdown();
    FillRatesDropdown();
    ui->leKennung->setText( ProposeKennung());
    comboKreditorenAnzeigeNachKreditorenId( id != -1 ? id : getPersonIdFromKreditorenList());
    Vertrag cd; // this is to get the defaults of the class definition
    ui->deLaufzeitEnde->setDate(cd.LaufzeitEnde());
    ui->deVertragsabschluss->setDate(cd.Vertragsabschluss());
    ui->chkbThesaurierend->setChecked(cd.Thesaurierend());

    ui->stackedWidget->setCurrentIndex(newContractIndex);
}


void MainWindow::on_leBetrag_editingFinished()
{
    ui->leBetrag->setText(QString("%L1").arg(ui->leBetrag->text().toDouble()));
}

// List of contracts
void MainWindow::prepareContractListView()
{LOG_ENTRY_and_EXIT;
    QVector<dbfield> fields;
    fields.append(dkdbstructur["Vertraege"]["id"]);
    fields.append(dkdbstructur["Vertraege"]["Kennung"]);
    fields.append(dkdbstructur["Kreditoren"]["Vorname"]);
    fields.append(dkdbstructur["Kreditoren"]["Nachname"]);
    fields.append(dkdbstructur["Vertraege"]["Betrag"]);
    fields.append(dkdbstructur["Vertraege"]["Wert"]);
    fields.append(dkdbstructur["Zinssaetze"]["Zinssatz"]);
    fields.append(dkdbstructur["Vertraege"]["Vertragsdatum"]);
    fields.append(dkdbstructur["Vertraege"]["LetzteZinsberechnung"]);
    fields.append(dkdbstructur["Vertraege"]["aktiv"]);
    fields.append(dkdbstructur["Vertraege"]["LaufzeitEnde"]);
    fields.append(dkdbstructur["Vertraege"]["Kfrist"]);
    QSqlQueryModel* model = new QSqlQueryModel(ui->contractsTableView);
    model->setQuery(ContractList_SQL(fields, ui->leVertraegeFilter->text()));

    colIndexFieldActiveInContractList = fields.indexOf(dkdbstructur["Vertraege"]["aktiv"]);
    ui->contractsTableView->setModel(model);
    ui->contractsTableView->setEditTriggers(QTableView::NoEditTriggers);
    ui->contractsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->contractsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->contractsTableView->setAlternatingRowColors(true);
    ui->contractsTableView->setSortingEnabled(true);
    ui->contractsTableView->setItemDelegateForColumn(fields.indexOf(dkdbstructur["Vertraege"]["Betrag"]), new EuroItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(fields.indexOf(dkdbstructur["Vertraege"]["Wert"]), new WertEuroItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(fields.indexOf(dkdbstructur["Zinssaetze"]["Zinssatz"]), new PercentItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(fields.indexOf(dkdbstructur["Vertraege"]["Vertragsdatum"]), new DateItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(fields.indexOf(dkdbstructur["Vertraege"]["LaufzeitEnde"]), new DateItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(fields.indexOf(dkdbstructur["Vertraege"]["LetzteZinsberechnung"]), new DateItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(fields.indexOf(dkdbstructur["Vertraege"]["aktiv"]), new ActivatedItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(fields.indexOf(dkdbstructur["Vertraege"]["Kfrist"]), new KFristItemFormatter(ui->contractsTableView));
    ui->contractsTableView->resizeColumnsToContents();
    ui->contractsTableView->hideColumn(0);

    QSortFilterProxyModel *m=new QSortFilterProxyModel(this);
    m->setDynamicSortFilter(true);
    m->setSourceModel(model);
    ui->contractsTableView->setModel(m);
    ui->contractsTableView->setSortingEnabled(true);
}
void MainWindow::on_actionListe_der_Vertraege_anzeigen_triggered()
{LOG_ENTRY_and_EXIT;
    prepareContractListView();
    if( !ui->contractsTableView->currentIndex().isValid())
        ui->contractsTableView->selectRow(0);

    ui->stackedWidget->setCurrentIndex(ContractsListIndex);
}
void MainWindow::on_contractsTableView_customContextMenuRequested(const QPoint &pos)
{LOG_ENTRY_and_EXIT;
    QModelIndex indexClickTarget = ui->contractsTableView->indexAt(pos);
    QModelIndex indexActive = indexClickTarget.siblingAtColumn(colIndexFieldActiveInContractList); // contract active

    QMenu menu( "PersonContextMenu", this);
    bool isActive (ui->contractsTableView->model()->data(indexActive).toInt() ? true : false);
    if( isActive)
    {
        menu.addAction(ui->actionVertrag_Beenden);
    }
    else
    {
        menu.addAction(ui->actionactivateContract);
        menu.addAction(ui->actionVertrag_passiv_loeschen); // passive Verträge können gelöscht werden
    }
    menu.exec(ui->PersonsTableView->mapToGlobal(pos));
    return;
}
int MainWindow::getContractIdFromContractsList()
{LOG_ENTRY_and_EXIT;
    QModelIndex mi(ui->contractsTableView->currentIndex().siblingAtColumn(0));
    if( mi.isValid())
    {
        QVariant data(ui->contractsTableView->model()->data(mi));
        return data.toInt();
    }
    return -1;
}

void MainWindow::on_actionactivateContract_triggered()
{LOG_ENTRY_and_EXIT;
    askDateDlg dlg( this, QDate::currentDate());
    if( QDialog::Accepted == dlg.exec())
    {
        Vertrag v;
        v.ausDb(getContractIdFromContractsList(), true);
        if( v.aktiviereVertrag(dlg.getDate()))
            prepareContractListView();
    }
}
void MainWindow::on_actionVertrag_passiv_loeschen_triggered()
{LOG_ENTRY_and_EXIT;
    QModelIndex mi(ui->contractsTableView->currentIndex());
    if( !mi.isValid()) return;
    QString Vorname = ui->contractsTableView->model()->data(mi.siblingAtColumn(1)).toString();
    QString Nachname = ui->contractsTableView->model()->data(mi.siblingAtColumn(2)).toString();
    QString index = ui->contractsTableView->model()->data(mi.siblingAtColumn(0)).toString();

    QString msg( "Soll der Vertrag von ");

    msg += Vorname + " " + Nachname + " (id " + index + ") gelöscht werden?";
    if( QMessageBox::Yes != QMessageBox::question(this, "Kreditvertrag löschen", msg))
        return;

    Vertrag v; v.setVid(index.toInt());
    v.passivenVertragLoeschen();
    prepareContractListView();
}

void MainWindow::on_leFilter_editingFinished()
{LOG_ENTRY_and_EXIT;
    preparePersonTableView();
}

void MainWindow::on_pbPersonFilterZuruecksetzen_clicked()
{LOG_ENTRY_and_EXIT;
    ui->leFilter->setText("");
    preparePersonTableView();
}

void MainWindow::on_leVertraegeFilter_editingFinished()
{LOG_ENTRY_and_EXIT;
    prepareContractListView();
}

void MainWindow::on_FilterVertraegeZuruecksetzen_clicked()
{LOG_ENTRY_and_EXIT;
    ui->leVertraegeFilter->setText("");
    prepareContractListView();
}

void MainWindow::on_actionVertrag_Beenden_triggered()
{LOG_ENTRY_and_EXIT;
    QModelIndex mi(ui->contractsTableView->currentIndex());
    if( !mi.isValid()) return;
    int index = ui->contractsTableView->model()->data(mi.siblingAtColumn(0)).toInt();
    // Vertrag beenden -> Zins berechnen und m Auszahlungsbetrag anzeigen, dann löschen
    Vertrag v;
    v.ausDb(index, true);
    double WertBisHeute = v.Wert() + ZinsesZins(v.Zinsfuss(), v.Wert(), v.StartZinsberechnung(), QDate::currentDate(), v.Thesaurierend());
    QString getDateMsg("<h2>Wenn Sie einen Vertrag beenden wird der Zins abschließend"
                " berechnet und der Auszahlungsbetrag ermittelt.<br></h2>"
                "Um den Vertrag von %1 %2 mit dem aktuellen Wert %3 Euro jetzt zu beenden "
                "wählen Sie das Datum des Vertragendes ein und klicken Sie OK");
    getDateMsg = getDateMsg.arg(v.Vorname(), v.Nachname(), QString::number(WertBisHeute));

    askDateDlg dlg( this, QDate::currentDate());
    dlg.setMsg(getDateMsg);
    dlg.setDateLabel("Ende der Zinsberechnung nach der Kündigungsfrist:");
    if( QDialog::Accepted != dlg.exec())
    {
        qDebug() << "Delete contract was aborted by the user";
        return;
    }

    double davonZins =ZinsesZins(v.Zinsfuss(), v.Wert(), v.StartZinsberechnung(), dlg.getDate(), v.Thesaurierend());
    double neuerWert =v.Wert() +davonZins;

    QString confirmDeleteMsg("<h3>Vertragsabschluß</h3><br>Wert zum Vertragsende: %1 Euro<br>Zins der letzten Zinsphase: %2 Euro<br>"\
                             "Soll der Vertrag gelöscht werden?");
    confirmDeleteMsg = confirmDeleteMsg.arg(QString::number(neuerWert), QString::number(davonZins));
    if( QMessageBox::Yes != QMessageBox::question(this, "Vertrag löschen?", confirmDeleteMsg))
        return;
    if( !v.aktivenVertragLoeschen(dlg.getDate()))
        qCritical() << "Deleting the contract failed";
    prepareContractListView();
}

QString MainWindow::prepareOverviewPage(Uebersichten u)
{LOG_ENTRY_and_EXIT;

    QString lbl ("<html><body><style>h1 { padding: 2em; } "
                 "table, td { padding: 15px; border-width: 1px; border-style: solid; border-color: black; } "
                 "th { padding: 4px; border-width: 3px; border-style: solid; border-color: black; } "
                "</style>");
    QLocale locale;

    switch( u )
    {
    case UEBERSICHT:
    {
        DbSummary dbs;
        berechneZusammenfassung(dbs);
        lbl += QString("<h2>Übersicht DKs und DK Geber</h2><br> Stand: ") + QDate::currentDate().toString("dd.MM.yyyy<br>") +
          "<table>" +

          "<tr><td style='padding: 15px;'>Anzahl DK Geber*innen: </td><td style='text-align: left; padding: 15px;'>" + QString::number(dbs.AnzahlDkGeber) +"</td></tr>" +
          "<tr><td style='padding: 15px;' >Anzahl Direktkredite: </td><td style='padding: 15px; text-align: left;'>" + QString::number(dbs.AnzahlAktive) +"</td></tr>" +
          "<tr><td style='padding: 15px;'>Summe Direktkredite:  </td><td style='text-align: right:'>" + locale.toCurrencyString(dbs.BetragAktive) +"</td></tr>" +
          "<tr><td style='padding: 15px;'>Wert der DK inklusive Zinsen</td><td style='text-align: right'>"+ locale.toCurrencyString(dbs.WertAktive) + "</td></tr>" +

          "<tr><td></td><td></td></tr>" +
          "<tr><td>Anzahl der DK mit jährl. Zinsauszahlung: </td><td align:left>" + QString::number(dbs.AnzahlAuszahlende) +"</td></tr>" +
          "<tr><td>Summe: </td><td align:right>" + locale.toCurrencyString(dbs.BetragAuszahlende) +"</td></tr>" +
          "<tr><td></td><td></td></tr>" +

          "<tr><td>Anzahl der DK ohne jährl. Zinsauszahlung: </td><td align:left>" + QString::number(dbs.AnzahlThesaurierende) +"</td></tr>" +
          "<tr><td>Summe: </td><td align:right>" + locale.toCurrencyString(dbs.BetragThesaurierende) +"</td></tr>" +
          "<tr><td>Wert inkl. Zinsen: </td><td align:right>" + locale.toCurrencyString(dbs.WertThesaurierende) +"</td></tr>" +

          "<tr><td></td><td></td></tr>" +
          "<tr><td>Anzahl noch ausstehender (inaktiven) DK </td><td align:left>" + QString::number(dbs.AnzahlPassive) +"</td></tr>" +
          "<tr><td>Summe noch ausstehender (inaktiven) DK </td><td align:right>" + locale.toCurrencyString(dbs.BetragPassive) +"</td></tr>" +
               "</table>";
        break;
    }
    case VERTRAGSENDE:
    {
        lbl += "<h2>Auslaufende Verträge </h2> Stand:"  + QDate::currentDate().toString("dd.MM.yyyy<br>");
        QVector<ContractEnd> ce;
        berechneVertragsenden(ce);
        if( !ce.isEmpty())
        {
            lbl += "<table><thead><tr><td> Jahr </td><td> Anzahl </td><td> Summe </td></tr></thead>";
            for( auto x: ce)
            {
                lbl += "<tr><td align:left>" + QString::number(x.year) + "</td><td align:center>" +
                       QString::number(x.count) + "</td><td align:right>" + locale.toCurrencyString(x.value) + "</td></tr>";
            }
            lbl += "</table>";
        }
        else
            lbl += "<br><br><i>keine Verträge mit vorgemerktem Vertragsende</i>";
        break;
    }
    case ZINSVERTEILUNG:
    {
        QLocale locale;
        QVector<YZV> yzv;
        berechneJahrZinsVerteilung( yzv);
        if( !yzv.isEmpty())
        {
            lbl += "<h2>Verteilung der Zinssätze pro Jahr </h2> Stand:"  + QDate::currentDate().toString("dd.MM.yyyy<br>") +
             "<table>" +
             "<thead><tr><td style=\"padding:4px;\"> Jahr </td><td style=\"padding:4px;\"> Zinssatz </td><td style=\"padding:4px;\"> Anzahl</td><td style=\"padding:4px;\"> Summe </td></tr></thead>";
            for( auto x: yzv)
            {
                lbl += "<tr><td>" + QString::number(x.year) + "</td><td align:center>" +
                       QString::number(x.intrest) + " %</td><td align:center>" + QString::number(x.count) + "</td><td style='text-align:right'>" + locale.toCurrencyString(x.sum) + "</td></tr>";
            }
            lbl += "</table>";
        }
        break;
    }
    case LAUFZEITEN:
    {
        lbl += "<h2>Vertragslaufzeiten</h2> Stand:" + QDate::currentDate().toString("dd.MM.yyyy<br>");
        lbl += "<br>" + LaufzeitenVerteilungHtml();
    }
//    default:
//    {
//        ;
//    }
    }
    lbl += "</body></html>";
    qDebug() << "\n" << lbl << endl;
    return lbl;
}

void MainWindow::on_action_Uebersicht_triggered()
{LOG_ENTRY_and_EXIT;
    if(ui->comboUebersicht->count() == 0)
    {
        ui->comboUebersicht->clear();
        ui->comboUebersicht->addItem("Übersicht aller Kredite", QVariant(UEBERSICHT));
        ui->comboUebersicht->addItem("Anzahl auslaufender Verträge nach Jahr", QVariant(VERTRAGSENDE));
        ui->comboUebersicht->addItem("Anzahl Verträge nach Zinssatz und Jahr", QVariant(ZINSVERTEILUNG));
        ui->comboUebersicht->addItem("Anzahl Verträge nach Laufzeiten", QVariant(LAUFZEITEN));
        ui->comboUebersicht->setCurrentIndex(0);
    }
    int currentIndex = ui->comboUebersicht->currentIndex();
    Uebersichten u = static_cast<Uebersichten>( ui->comboUebersicht->itemData(currentIndex).toInt());
    ui->txtOverview->setText( prepareOverviewPage(u));
    ui->stackedWidget->setCurrentIndex(OverviewIndex);
}

void MainWindow::on_tblViewBookingsSelectionChanged(const QItemSelection& to, const QItemSelection& )
{
    QString json =ui->tblViewBookings->model()->data(to.indexes().at(0).siblingAtColumn(6)).toString();
    ui->lblYson->setText(json);
}
void MainWindow::on_actionShow_Bookings_triggered()
{LOG_ENTRY_and_EXIT;

    QSqlRelationalTableModel* model = new QSqlRelationalTableModel(ui->tblViewBookings);
    model->setTable("Buchungen");
    model->setSort(0,Qt::DescendingOrder );
    model->setRelation(2, QSqlRelation("Buchungsarten", "id", "Art"));

    model->select();

    ui->tblViewBookings->setModel(model);
    ui->tblViewBookings->hideColumn(6);
    ui->tblViewBookings->resizeColumnsToContents();

    connect(ui->tblViewBookings->selectionModel(),
        SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, SLOT(on_tblViewBookingsSelectionChanged(const QItemSelection&, const QItemSelection&)));

    ui->stackedWidget->setCurrentIndex(bookingsListIndex);
}

void MainWindow::on_actionJahreszinsabrechnung_triggered()
{LOG_ENTRY_and_EXIT;
    jahresabschluss Abschluss;

    QString msg = "Der Jahresabschluss für das Jahr "
                  + QString::number(Abschluss.abzuschliessendesJahr())
                  + " kann gemacht werden\n\n";
    msg += "Dabei werden die Zinsen für alle Verträge berechnet. Der Wert von thesaurierenden Verträgen wird angepasst\n";
    msg += "Dieser Vorgang kann nicht rückgängig gemacht werden. Möchtest Du fortfahren?";

    if( QMessageBox::Yes != QMessageBox::question(this, "Jahresabschluss", msg))
        return;
    Abschluss.execute();
    frmJahresabschluss dlgJA(Abschluss, this);
    dlgJA.exec();
    on_actionListe_der_Vertraege_anzeigen_triggered( );
}

void MainWindow::on_actionAusgabeverzeichnis_festlegen_triggered()
{
    QString dir;
    QSettings config;
    config.value("outdir", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));

    dir = QFileDialog::getExistingDirectory(this, "Ausgabeverzeichnis", dir,
                                        QFileDialog::ShowDirsOnly
                                            | QFileDialog::DontResolveSymlinks);
    config.setValue("outdir", dir);
}

void MainWindow::on_actionAktive_Vertraege_CSV_triggered()
{
    CsvActiveContracts();
}

void MainWindow::on_comboUebersicht_currentIndexChanged(int )
{
    on_action_Uebersicht_triggered();
}

void MainWindow::on_pbPrint_clicked()
{
    QSettings config;
    QString filename = config.value("outdir").toString();

    filename += "\\" + QDate::currentDate().toString("yyyy-MM-dd_");
    filename += Uebersichten_kurz[ui->comboUebersicht->currentIndex()];
    filename += ".pdf";
    QPdfWriter write(filename);
    ui->txtOverview->print(&write);
    showFileInFolder(filename);
}

