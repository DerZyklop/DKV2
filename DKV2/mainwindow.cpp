#include <QtGlobal>
#if defined(Q_OS_WIN)
#include "windows.h"
#else
#include <stdlib.h>
#endif

#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QSqlTableModel>
#include <QSortFilterProxyModel>
#include <QSqlRelationalTableModel>
#include <QPdfWriter>
#include <QPainter>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "appconfig.h"
#include "wiznewdatabase.h"
#include "wizopenornewdatabase.h"
#include "appconfig.h"
#include "csvwriter.h"
#include "uiitemformatter.h"
#include "dkdbhelper.h"
#include "dbstatistics.h"
#include "letters.h"
#include "transaktionen.h"


// construction, destruction
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{   LOG_CALL;
    ui->setupUi(this);
#ifndef QT_DEBUG
    ui->action_menu_debug_create_sample_data->setVisible(false);
#endif

    ui->statusBar->addPermanentWidget(ui->statusLabel);

    setCentralWidget(ui->stackedWidget);
    if( appConfig::CurrentDb().isEmpty()){
        on_action_menu_database_new_triggered();
        if( appConfig::CurrentDb().isEmpty()){
            QMessageBox::critical(this, "Ganz schlecht", "Ohne Datenbank kann Dkv2 nicht laufen");
            close();
        }
    }
    if( !useDb(appConfig::CurrentDb()))
        // there should be a valid DB - checked in main.cpp
        Q_ASSERT(!"useDb failed in construcor of mainwindow");

    ui->CreditorsTableView->setStyleSheet(qsl("QTableView::item { padding-right: 10px; padding-left: 10px; }"));
    ui->contractsTableView->setStyleSheet(qsl("QTableView::item { padding-right: 10px; padding-left: 10px; }"));

    ui->bookingsTableView->setItemDelegateForColumn(2, new bookingTypeFormatter);

    ui->fontComboBox->setEditable(false);
    ui->fontComboBox->setWritingSystem(QFontDatabase::Latin);
    ui->fontComboBox->setFontFilters(QFontComboBox::ScalableFonts | QFontComboBox::ProportionalFonts);
    ui->spinFontSize->setMinimum(8);
    ui->spinFontSize->setMaximum(11);
    connect(ui->wPreview, SIGNAL(paintRequested(QPrinter*)), SLOT(doPaint(QPrinter*)));

    ui->stackedWidget->setCurrentIndex(startPageIndex);
}
MainWindow::~MainWindow()
{   LOG_CALL;
    delete ui;
}

// generell functions
bool MainWindow::useDb(const QString& dbfile)
{   LOG_CALL;
    if( open_databaseForApplication(dbfile)) {
        appConfig::setCurrentDb(dbfile);
        showDbInStatusbar(dbfile);
        return true;
    }
    qCritical() << "the databse could not be used for this application";
    return false;
}
void MainWindow::showDbInStatusbar( QString filename)
{   LOG_CALL_W (filename);
    if( filename.isEmpty()) {
        filename = appConfig::CurrentDb();
    }
    ui->statusLabel->setText( filename);
}

// whenever the stackedWidget changes ...
void MainWindow::on_stackedWidget_currentChanged(int arg1)
{   LOG_CALL;
    if( arg1 < 0) {
        qWarning() << "stackedWidget changed to non existing page";
        return;
    }
    switch(arg1)
    {
    case startPageIndex:
        prepare_startPage();
        ui->action_menu_creditors_delete->setEnabled(false);
        ui->menu_contracts_subm_print_lists->setEnabled(false);
        break;
    case creditorsListPageIndex:
        ui->action_menu_creditors_delete->setEnabled(true);
        ui->menu_contracts_subm_print_lists->setEnabled(false);
        break;
    case contractsListPageIndex:
        ui->action_menu_creditors_delete->setEnabled(false);
        ui->menu_contracts_subm_print_lists->setEnabled(true);
        break;
    case overviewsPageIndex:
        ui->action_menu_creditors_delete->setEnabled(false);
        ui->menu_contracts_subm_print_lists->setEnabled(false);
        break;
    case printPreviewPageIndex:
        ui->action_menu_creditors_delete->setEnabled(false);
        ui->menu_contracts_subm_print_lists->setEnabled(false);
    default:
        qWarning() << "stackedWidget current change not implemented for this index";
    }// e.o. switch
    return;
}

// the empty "welcome" page
void MainWindow::prepare_startPage()
{   LOG_CALL;
    busycursor b;
    QString messageHtml {qsl("<table width='100%'><tr><td><h2>Willkommen zu DKV2- Deiner Verwaltung von Direktrediten</h2></td></tr>")};

    dbStats stats(dbStats::calculate);
    ;
    double allContractsValue = stats.allContracts[dbStats::t_nt].value;
    if( allContractsValue > 0) {
        QLocale l;
        QString valueRow = qsl("<tr><td>Die Summer aller DK beträgt <big><font color=red>") + l.toCurrencyString(allContractsValue) + qsl("</font></big></td></tr>");
        messageHtml += valueRow;
    }
    messageHtml += qsl("<tr><td><img src=\":/res/splash.png\"/></td></tr></table>");
    qDebug() <<"welcome Screen html: " << Qt::endl << messageHtml << Qt::endl;
    ui->teWelcome->setText(messageHtml);
}
void MainWindow::on_action_menu_database_start_triggered()
{   LOG_CALL;
    ui->stackedWidget->setCurrentIndex(startPageIndex);
}

// Database Menu
QString askUserDbFilename(QString title, bool onlyExistingFiles=false)
{   // this function is used with creaetDbCopy, createAnony.DbCopy but NOT newDb
    LOG_CALL;
    wpFileSelectionWiz wiz(getMainWindow());
    wiz.title =title;
    wiz.subtitle =qsl("Mit dieser Dialogfolge wählst Du eine DKV2 Datenbank aus");
    wiz.fileTypeDescription =qsl("dk-DB Dateien (*.dkdb)");
    if( (wiz.existingFile=onlyExistingFiles))
        wiz.bffTitle =qsl("Wähle eine existierende dkdb Datei aus");
    else
        wiz.bffTitle =qsl("Wähle eine dkdb Datei aus oder gib einen neuen Dateinamen ein");

    QFileInfo lastdb (appConfig::CurrentDb());
    if( lastdb.exists())
        wiz.openInFolder=lastdb.path();
    else
        wiz.openInFolder =QStandardPaths::writableLocation((QStandardPaths::DocumentsLocation));

    wiz.exec();
    return wiz.field(qsl("selectedFile")).toString();
}
QPair<QString, createNew> MainWindow::askUserNextDb()
{   LOG_CALL;
    QString currentDb {absoluteCanonicalPath(appConfig::CurrentDb())};
    wizOpenOrNewDb wizOpenOrNew (getMainWindow());
    QFont f = wizOpenOrNew.font(); f.setPointSize(10); wizOpenOrNew.setFont(f);
    if( QDialog::Accepted != wizOpenOrNew.exec()) {
        qInfo() << "wizard OpenOrNew was canceled by the user";
        return {currentDb, false};
    }
    QString dbPath =absoluteCanonicalPath(wizOpenOrNew.selectedFile);
    {
        busycursor b;
        if( ! wizOpenOrNew.field(qsl("createNewDb")).toBool()) {
            // an existing db was selected
            if( currentDb == dbPath) {
                qInfo() << "already open DB was selected";
                return {dbPath, false};
            }
            qInfo() << "existing db " << dbPath << "was selected";
            if( ! useDb( dbPath)) {
                return {qsl(""), false};
            }
            return {dbPath, false};
        }
        // a new db should be created -> ask project details
        closeAllDatabaseConnections();
        bool ret = true;
        ret &= create_DK_databaseFile(dbPath);
        ret &= useDb(dbPath);
        if( ! ret)
            return {qsl(""), true};
        appConfig::setLastDb(dbPath);
    }
    on_actionProjektkonfiguration_ndern_triggered();
    return {dbPath, true};
}
void MainWindow::on_action_menu_database_new_triggered()
{   LOG_CALL;
    QPair<QString, createNew> dbRet = askUserNextDb();
    if( dbRet.first == qsl("")) {
        if( dbRet.second) {
            QMessageBox::critical(this, "Fehler", "Die neue Datenbank konnte nicht angelegt werden. Die Ausführung wird abgebrochen");
            close();
        } else {
            QMessageBox::critical(this, "Fehler", "Die ausgewählte Datenbank konnte nicht angelegt werden. Die Ausführung wird abgebrochen");
            close();
        }
        qInfo() << "user canceled file selection";
        return;
    }
    prepare_startPage();
    ui->stackedWidget->setCurrentIndex(startPageIndex);
}

void MainWindow::on_action_menu_database_copy_triggered()
{   LOG_CALL;
    QString dbfile = askUserDbFilename(qsl("Dateiname der Kopie Datenbank angeben."));
    if( dbfile == qsl(""))
        return;

    busycursor b;
    if( !create_DB_copy(dbfile, false)) {
        QMessageBox::information(this, qsl("Fehler beim Kopieren"), qsl("Die Datenbankkopie konnte nicht angelegt werden. "
                                                               "Weitere Info befindet sich in der LOG Datei"));
        qCritical() << "creating copy failed";
    }
    return;
}
void MainWindow::on_action_menu_database_anonymous_copy_triggered()
{   LOG_CALL;
    QString dbfile = askUserDbFilename(qsl("Dateiname der Anonymisierten Kopie angeben."));
    if( dbfile == qsl(""))
        return;
    busycursor b;
    if( !create_DB_copy(dbfile, true)) {
        QMessageBox::information(this, qsl("Fehler beim Kopieren"),
                                 qsl("Die anonymisierte Datenbankkopie konnte nicht angelegt werden. "
                                     "Weitere Info befindet sich in der LOG Datei"));
        qCritical() << "creating depersonaliced copy failed";
    }
    return;
}
void MainWindow::on_actionProjektkonfiguration_ndern_triggered()
{   LOG_CALL;
    wizConfigureProjectWiz wiz(getMainWindow());
    if(wiz.exec() == QDialog::Accepted)
        wiz.updateDbConfig();
}
void MainWindow::on_action_menu_database_configure_outdir_triggered()
{   LOG_CALL;
    appConfig::setOutDirInteractive(this);
}
void MainWindow::on_action_menu_database_program_exit_triggered()
{   LOG_CALL;
    ui->stackedWidget->setCurrentIndex(startPageIndex);
    this->close();
}

// Creditor list view page
void MainWindow::on_action_menu_creditors_listview_triggered()
{   LOG_CALL;
    busycursor b;
    prepare_CreditorsListPage();
    if( ! ui->CreditorsTableView->currentIndex().isValid())
        ui->CreditorsTableView->selectRow(0);

    ui->stackedWidget->setCurrentIndex(creditorsListPageIndex);
}
void MainWindow::prepare_CreditorsListPage()
{   LOG_CALL;
    busycursor b;
    QSqlTableModel* model = new QSqlTableModel(ui->CreditorsTableView);
    model->setTable(qsl("Kreditoren"));
    model->setFilter(qsl("Vorname LIKE '%") + ui->le_CreditorsFilter->text() + qsl("%' OR Nachname LIKE '%") + ui->le_CreditorsFilter->text() + qsl("%'"));
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

    ui->CreditorsTableView->setEditTriggers(QTableView::NoEditTriggers);
    ui->CreditorsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->CreditorsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->CreditorsTableView->setAlternatingRowColors(true);
    ui->CreditorsTableView->setSortingEnabled(true);
    ui->CreditorsTableView->setModel(model);
    ui->CreditorsTableView->hideColumn(0);
    ui->CreditorsTableView->resizeColumnsToContents();
}
void MainWindow::on_le_CreditorsFilter_editingFinished()
{   LOG_CALL;
    busycursor b;
    prepare_CreditorsListPage();
}
int  MainWindow::id_SelectedCreditor()
{   LOG_CALL;
    // What is the persId of the currently selected person in the person?
    QModelIndex mi(ui->CreditorsTableView->currentIndex().siblingAtColumn(0));
    if( mi.isValid()) {
        QVariant data(ui->CreditorsTableView->model()->data(mi));
        return data.toInt();
    }
    qCritical() << "Index der Personenliste konnte nicht bestimmt werden";
    return -1;
}
void MainWindow::on_btn_reset_filter_creditors_clicked()
{   LOG_CALL;
    busycursor b;
    ui->le_CreditorsFilter->setText(qsl(""));
    prepare_CreditorsListPage();
}
void MainWindow::on_action_menu_creditors_delete_triggered()
{
    on_action_cmenu_delete_creaditor_triggered();
}

// Context Menue in Creditor Table
void MainWindow::on_CreditorsTableView_customContextMenuRequested(const QPoint &pos)
{   LOG_CALL;
    QModelIndex index = ui->CreditorsTableView->indexAt(pos).siblingAtColumn(0);
    if( index.isValid()) {
        QMenu menu( qsl("PersonContextMenu"), this);
        menu.addAction(ui->action_cmenu_edit_creditor);
        menu.addAction(ui->action_cmenu_delete_creaditor);
        menu.addAction(ui->action_cmenu_go_contracts);
        menu.exec(ui->CreditorsTableView->mapToGlobal(pos));
        return;
    }
}
void MainWindow::on_action_cmenu_edit_creditor_triggered()
{   LOG_CALL;
    QModelIndex mi(ui->CreditorsTableView->currentIndex());
    QVariant index = ui->CreditorsTableView->model()->data(mi.siblingAtColumn(0));

    editCreditor(index.toInt());
    updateListViews();
}
void MainWindow::on_action_cmenu_delete_creaditor_triggered()
{   LOG_CALL;
    const QTableView * const tv = ui->CreditorsTableView;
    QModelIndex mi(tv->currentIndex());
    qlonglong index = tv->model()->data(mi.siblingAtColumn(0)).toLongLong();
    creditor c (index);

    QString msg( qsl("Soll der Kreditgeber %1 %2 (id %3) gelöscht werden?"));
    msg =msg.arg(c.getValue(qsl("Vorname")).toString(), c.getValue(qsl("Nachname")).toString(), QString::number(index));

    if( QMessageBox::Yes != QMessageBox::question(this, qsl("Kreditgeber löschen?"), msg))
        return;
    busycursor b;

    if( c.remove())
        prepare_CreditorsListPage();
    else
        QMessageBox::information(this, qsl("Löschen unmöglich"), qsl("Ein Kreditor mit aktiven oder beendeten Verträgen kann nicht gelöscht werden"));
}
void MainWindow::on_action_cmenu_go_contracts_triggered()
{   LOG_CALL;
    busycursor b;
    QModelIndex mi(ui->CreditorsTableView->currentIndex());
    QString index = ui->CreditorsTableView->model()->data(mi.siblingAtColumn(0)).toString();
    ui->le_ContractsFilter->setText(qsl("kreditor:") +index);
    on_action_menu_contracts_listview_triggered();
}

// new creditor and contract Wiz
void MainWindow::updateListViews()
{
    QSqlTableModel* temp;

    if( ui->stackedWidget->currentIndex() == creditorsListPageIndex) {
        if( (temp =qobject_cast<QSqlTableModel*>(ui->CreditorsTableView->model())))
            temp->select();
    }
    if( ui->stackedWidget->currentIndex() == contractsListPageIndex) {
        if( (temp =qobject_cast<QSqlTableModel*>(ui->contractsTableView->model())))
                temp->select();
        if( (temp =qobject_cast<QSqlTableModel*>(ui->bookingsTableView ->model())))
                temp->select();
    }
    if( ui->stackedWidget->currentIndex() == overviewsPageIndex)
        on_action_menu_contracts_statistics_view_triggered();
}
void MainWindow::on_actionNeu_triggered()
{
    newCreditorAndContract();
    updateListViews();
}

// Contract List
void MainWindow::on_action_menu_contracts_listview_triggered()
{   LOG_CALL;
    showDeletedContracts =false;
    prepare_contracts_list_view();
    if( !ui->contractsTableView->currentIndex().isValid())
        ui->contractsTableView->selectRow(0);

    ui->stackedWidget->setCurrentIndex(contractsListPageIndex);
}
QString filterFromFilterphrase(QString fph)
{
    if( fph.startsWith(qsl("kreditor:")))
    {
        bool conversionOK = true;
        qlonglong contractId = fph.rightRef(fph.length()-9).toInt(&conversionOK);
        if( ! conversionOK)
            return "";
        else
            return qsl("KreditorId=") + QString::number(contractId);
    }
    return fph.isEmpty() ? QString() :
           (qsl("Kreditorin LIKE '%") + fph + qsl("%' OR Vertragskennung LIKE '%") + fph + qsl("%'"));
}

void MainWindow::prepare_deleted_contracts_list_view()
{ LOG_CALL;
    enum column_pos_del {
        cp_vid,
        cp_Creditor_id,
        cp_Creditor,
        cp_ContractLabel,
        cp_ContractActivation,
        cp_ContractTermination,
        cp_InitialValue,
        cp_InterestRate,
        cp_InterestMode,
        cp_Interest,
        cp_TotalDeposit,
        cp_FinalPayout
    };

    QSqlTableModel* model = new QSqlTableModel(this);
    model->setTable(qsl("vVertraege_geloescht"));
    model->setFilter( filterFromFilterphrase(ui->le_ContractsFilter->text()));
    qDebug() << "contract list model filter: " << model->filter();

    model->setHeaderData(cp_Creditor, Qt::Horizontal, qsl("Nachname, Vorname der Vertragspartnerin / des Vertragsparnters"), Qt::ToolTipRole);
    model->setHeaderData(cp_ContractLabel, Qt::Horizontal, qsl("Eindeutige Identifizierung des Vertrags"), Qt::ToolTipRole);
    model->setHeaderData(cp_ContractActivation, Qt::Horizontal, qsl("Datum der Vertragsaktivierung / Beginn der Zinsberechnung"), Qt::ToolTipRole);
    model->setHeaderData(cp_ContractTermination, Qt::Horizontal, qsl("Datum des Vertragsende"), Qt::ToolTipRole);
    model->setHeaderData(cp_InitialValue, Qt::Horizontal, qsl("Höhe der Ersteinlage"), Qt::ToolTipRole);
    model->setHeaderData(cp_InterestRate, Qt::Horizontal, qsl("Zinsfuss"), Qt::ToolTipRole);
    model->setHeaderData(cp_InterestMode, Qt::Horizontal, qsl("Verträge können Auszahlend, Thesaurierend oder mit festem Zins vereinbart sein"), Qt::ToolTipRole);
    model->setHeaderData(cp_Interest, Qt::Horizontal, qsl("Angefallene, nicht bereits zur Laufzeit ausgezahlte Zinsen"), Qt::ToolTipRole);
    model->setHeaderData(cp_TotalDeposit, Qt::Horizontal, qsl("Summe aller Einzahlungen"), Qt::ToolTipRole);
    model->setHeaderData(cp_FinalPayout, Qt::Horizontal, qsl("Ausgezahltes finales Guthaben"), Qt::ToolTipRole);

    QTableView*& tv = ui->contractsTableView;
    tv->setModel(model);
    if ( !model->select()) {
        qCritical() << "Model selection failed" << model->lastError();
        return;
    }
    tv->setEditTriggers(QTableView::NoEditTriggers);
    tv->setSelectionMode(QAbstractItemView::SingleSelection);
    tv->setSelectionBehavior(QAbstractItemView::SelectRows);
    tv->setAlternatingRowColors(true);
    tv->setSortingEnabled(true);
    tv->setItemDelegateForColumn(cp_InitialValue, new CurrencyFormatter(tv));
    tv->setItemDelegateForColumn(cp_Interest, new CurrencyFormatter(tv));
    tv->setItemDelegateForColumn(cp_TotalDeposit, new CurrencyFormatter(tv));
    tv->setItemDelegateForColumn(cp_FinalPayout, new CurrencyFormatter(tv));
    tv->hideColumn(cp_vid);
    tv->hideColumn(cp_Creditor_id);

    tv->resizeColumnsToContents();
    auto c = connect(ui->contractsTableView->selectionModel(),
            SIGNAL(currentChanged (const QModelIndex & , const QModelIndex & )),
            SLOT(currentChange_ctv(const QModelIndex & , const QModelIndex & )));

    if( ! model->rowCount()) {
        ui->bookingsTableView->setModel(new QSqlTableModel(this));
    } else
        tv->setCurrentIndex(model->index(0, 1));
}

void MainWindow::prepare_valid_contraccts_list_view()
{ LOG_CALL;
    enum colmn_Pos {
        cp_vid,
        cp_Creditor_id,
        cp_Creditor,
        cp_ContractLabel,
        cp_ContractDate,
        cp_ContractValue,
        cp_InterestRate,
        cp_InterestMode,
        cp_ActivationDate,
        cp_InterestBearing,
        cp_Interest,
        cp_LastBooking,
        cp_ContractEnd
    };

    QSqlTableModel* model = new QSqlTableModel(this);
    model->setTable(qsl("vVertraege_alle_4view"));
    model->setFilter( filterFromFilterphrase(ui->le_ContractsFilter->text()));
    model->setHeaderData(cp_Creditor, Qt::Horizontal, qsl("KreditorIn"));
    model->setHeaderData(cp_Creditor, Qt::Horizontal, qsl("Nachname, Vorname der Vertragspartnerin / des Vertragsparnters"), Qt::ToolTipRole);
    model->setHeaderData(cp_ContractLabel, Qt::Horizontal, qsl("Vertragskennung"));
    model->setHeaderData(cp_ContractLabel, Qt::Horizontal, qsl("Die Vertragskennung identifiziert den Vertrag eindeutig"), Qt::ToolTipRole);
    model->setHeaderData(cp_ContractDate, Qt::Horizontal, qsl(""), Qt::ToolTipRole);
    model->setHeaderData(cp_ContractValue, Qt::Horizontal, qsl("Bei aktiven Verträgen: Höhe der Ersteinlage, sonst der im Vertrag vereinbarte Kreditbetrag"), Qt::ToolTipRole);
    model->setHeaderData(cp_InterestRate, Qt::Horizontal, qsl(""), Qt::ToolTipRole);
    model->setHeaderData(cp_InterestMode, Qt::Horizontal, qsl("Verträge können Auszahlend, Thesaurierend oder mit festem Zins vereinbart sein"), Qt::ToolTipRole);
    model->setHeaderData(cp_ActivationDate, Qt::Horizontal, qsl("Datum des ersten Geldeingangs und Beginn der Zinsberechnung"), Qt::ToolTipRole);
    model->setHeaderData(cp_InterestBearing, Qt::Horizontal, qsl("Verzinsliches\nGuthaben"));
    model->setHeaderData(cp_InterestBearing, Qt::Horizontal, qsl("Bei thesaurierenden Verträgen: Einlage und angesparte Zinsen"), Qt::ToolTipRole);
    model->setHeaderData(cp_Interest, Qt::Horizontal, qsl("Angesparter\nZins"));
    model->setHeaderData(cp_Interest, Qt::Horizontal, qsl("Nicht ausgezahlte Zinsen bei Verträgen mit fester Verzinsung und thesaurierenden Verträgen"), Qt::ToolTipRole);
    model->setHeaderData(cp_LastBooking, Qt::Horizontal, qsl("Letztes\nBuchungsdatum"));
    model->setHeaderData(cp_ContractEnd, Qt::Horizontal, qsl("Kündigungsfrist/ \nVertragsende"));

    qDebug() << "contract list model filter: " << model->filter();

    QTableView*& tv = ui->contractsTableView;
    tv->setModel(model);
    if ( !model->select()) {
        qCritical() << "Model selection failed" << model->lastError();
        return;
    }

    tv->setEditTriggers(QTableView::NoEditTriggers);
    tv->setSelectionMode(QAbstractItemView::SingleSelection);
    tv->setSelectionBehavior(QAbstractItemView::SelectRows);
    tv->setAlternatingRowColors(true);
    tv->setSortingEnabled(true);
    tv->setItemDelegateForColumn(cp_ContractValue, new CurrencyFormatter(tv));
    tv->setItemDelegateForColumn(cp_InterestBearing, new CurrencyFormatter(tv));
    tv->setItemDelegateForColumn(cp_Interest, new CurrencyFormatter(tv));
    tv->hideColumn(cp_vid);
    tv->hideColumn(cp_Creditor_id);


    tv->resizeColumnsToContents();
    auto c = connect(ui->contractsTableView->selectionModel(),
            SIGNAL(currentChanged (const QModelIndex & , const QModelIndex & )),
            SLOT(currentChange_ctv(const QModelIndex & , const QModelIndex & )));

    if( ! model->rowCount()) {
        ui->bookingsTableView->setModel(new QSqlTableModel(this));
    } else
        tv->setCurrentIndex(model->index(0, 1));
}

void MainWindow::prepare_contracts_list_view()
{   LOG_CALL;
    busycursor b;

    if( showDeletedContracts)
        prepare_deleted_contracts_list_view();
    else
        prepare_valid_contraccts_list_view();
}

int  MainWindow::get_current_id_from_contracts_list()
{   LOG_CALL;
    QModelIndex mi(ui->contractsTableView->currentIndex().siblingAtColumn(0));
    if( mi.isValid()) {
        QVariant data(ui->contractsTableView->model()->data(mi));
        return data.toInt();
    }
    return -1;
}
void MainWindow::on_le_ContractsFilter_editingFinished()
{   LOG_CALL;
    prepare_contracts_list_view();
}
void MainWindow::on_reset_contracts_filter_clicked()
{   LOG_CALL;
    ui->le_ContractsFilter->setText(QString());
    prepare_contracts_list_view();
}
void MainWindow::currentChange_ctv(const QModelIndex & newI, const QModelIndex & )
{
    // todo: do all init only once, this function should only do the
    // setFilter and the select()
    QModelIndex indexIndex = newI.siblingAtColumn(0);
    int index =ui->contractsTableView->model()->data(indexIndex).toInt();
    QSqlTableModel* model = new QSqlTableModel(this);
    if( showDeletedContracts) {
        model->setTable(qsl("exBuchungen"));
        model->setFilter(qsl("exBuchungen.VertragsId=") + QString::number(index));
    } else {
        model->setTable(qsl("Buchungen"));
        model->setFilter(qsl("Buchungen.VertragsId=") + QString::number(index));
    }
    model->setSort(0, Qt::SortOrder::DescendingOrder);

    ui->bookingsTableView->setModel(model);
    model->select();
    ui->bookingsTableView->setSortingEnabled(false);
    ui->bookingsTableView->hideColumn(0);
    ui->bookingsTableView->hideColumn(1);
    ui->bookingsTableView->setItemDelegateForColumn(2, new DateItemFormatter);
    ui->bookingsTableView->setItemDelegateForColumn(3, new bookingTypeFormatter);
    ui->bookingsTableView->setItemDelegateForColumn(4, new BookingAmountItemFormatter);
}

// contract list context menu
void MainWindow::on_contractsTableView_customContextMenuRequested(const QPoint &pos)
{   LOG_CALL;
    if( showDeletedContracts)
        return;
    QTableView*& tv = ui->contractsTableView;
    QModelIndex index = tv->indexAt(pos).siblingAtColumn(0);

    contract c(index.data().toInt());
    bool gotTerminationDate = c.noticePeriod() == -1;

    QMenu menu( qsl("ContractContextMenu"), this);
    if(c.isActive())
    {
        if( gotTerminationDate)
            ui->action_cmenu_terminate_contract->setText(qsl("Vertrag beenden"));
        else
            ui->action_cmenu_terminate_contract->setText(qsl("Vertrag kündigen"));
        menu.addAction(ui->action_cmenu_terminate_contract);
        menu.addAction(ui->action_cmenu_change_contract);
    }
    else
    {
        menu.addAction(ui->action_cmenu_activate_contract);
        menu.addAction(ui->action_cmenu_delete_inactive_contract); // passive Verträge können gelöscht werden
    }
    menu.exec(ui->CreditorsTableView->mapToGlobal(pos));
    return;
}
void MainWindow::on_action_cmenu_activate_contract_triggered()
{   LOG_CALL;
    activateContract(get_current_id_from_contracts_list());
    updateListViews();
}
void MainWindow::on_action_cmenu_terminate_contract_triggered()
{   LOG_CALL;
    QModelIndex mi(ui->contractsTableView->currentIndex());
    if( !mi.isValid()) return;
    int index = ui->contractsTableView->model()->data(mi.siblingAtColumn(0)).toInt();
    terminateContract(index);
    updateListViews();
}
void MainWindow::on_action_cmenu_delete_inactive_contract_triggered()
{   LOG_CALL;
    QModelIndex mi(ui->contractsTableView->currentIndex());
    if( !mi.isValid()) return;

    deleteInactiveContract(ui->contractsTableView->model()->data(mi.siblingAtColumn(0)).toLongLong());
    updateListViews();
}
void MainWindow::on_action_cmenu_change_contract_triggered()
{   // deposit or payout...
    QModelIndex mi(ui->contractsTableView->currentIndex());
    if( !mi.isValid()) return;
    qlonglong contractId = ui->contractsTableView->model()->data(mi.siblingAtColumn(0)).toLongLong();
    changeContractValue(contractId);
    updateListViews();
}
// new creditor or contract from contract menu
void MainWindow::on_action_Neu_triggered()
{
    on_actionNeu_triggered();
}
// terminated contracts list
void MainWindow::on_actionBeendete_Vertr_ge_anzeigen_triggered()
{
    showDeletedContracts =true;
    prepare_contracts_list_view();
    if( !ui->contractsTableView->currentIndex().isValid())
        ui->contractsTableView->selectRow(0);
    ui->stackedWidget->setCurrentIndex(contractsListPageIndex);
}
// statistics
void MainWindow::on_action_menu_contracts_statistics_view_triggered()
{   LOG_CALL;
    QComboBox* combo =ui->comboUebersicht;
    if(combo->count() == 0) {
        combo->addItem(qsl("Übersicht aller Kredite"),                QVariant(OVERVIEW));
        combo->addItem(qsl("Ausgezahlte Zinsen pro Jahr"),             QVariant(PAYED_INTEREST_BY_YEAR));
        combo->addItem(qsl("Anzahl auslaufender Verträge nach Jahr"), QVariant(BY_CONTRACT_END));
        combo->addItem(qsl("Anzahl Verträge nach Zinssatz und Jahr"), QVariant(INTEREST_DISTRIBUTION));
        combo->addItem(qsl("Anzahl Verträge nach Laufzeiten"),        QVariant(CONTRACT_TERMS));
        combo->addItem(qsl("Überblick über alle Verträge"),           QVariant(ALL_CONTRACT_INFO));
        combo->setCurrentIndex(0);
    }

    on_comboUebersicht_currentIndexChanged(combo->currentIndex());
    ui->stackedWidget->setCurrentIndex(overviewsPageIndex);
}
void MainWindow::on_comboUebersicht_currentIndexChanged(int i)
{   LOG_CALL;
    ui->txtOverview->setText( reportHtml(static_cast<Uebersichten>( i)));
}
void MainWindow::on_pbPrint_clicked()
{   LOG_CALL;
    QString filename = appConfig::Outdir();
    filename += qsl("\\") + QDate::currentDate().toString("yyyy-MM-dd_");
    filename += Statistics_Filenames[ui->comboUebersicht->currentIndex()];
    filename += qsl(".pdf");
    QPdfWriter write(filename);
    ui->txtOverview->print(&write);
    showFileInFolder(filename);
}
// anual settlement
void MainWindow::on_action_menu_contracts_anual_interest_settlement_triggered()
{   LOG_CALL;
    annualSettlement();
    updateListViews();
}
// list creation csv, printouts
void MainWindow::on_action_menu_contracts_print_lists_triggered()
{   LOG_CALL;
    if( !createCsvActiveContracts())
        QMessageBox::critical(this, qsl("Fehler"), qsl("Die Datei konnte nicht angelegt werden. Ist sie z.B. in Excel geöffnet?"));
}
void MainWindow::on_actionAktuelle_Auswahl_triggered()
{
    csvwriter csv;
    QSqlTableModel* model = qobject_cast<QSqlTableModel*>(ui->contractsTableView->model());
    QSqlRecord rec =model->record();
    for( int i=0; i<rec.count(); i++) {
        csv.addColumn(rec.fieldName(i));
    }
    for( int i=0; i<model->rowCount(); i++) {
        QSqlRecord rec =model->record(i);
        for( int j=0; j<rec.count(); j++) {
            csv.appendToRow(rec.value(j).toString());
        }
    }
    csv.saveAndShowInExplorer(QDate::currentDate().toString("yyyy-MM-dd_Vertragsliste.csv"));
}
// debug funktions
void MainWindow::on_action_menu_debug_create_sample_data_triggered()
{   LOG_CALL;
    busycursor b;
    create_sampleData();
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex());
    on_action_menu_contracts_listview_triggered();
}
void MainWindow::on_action_menu_debug_show_log_triggered()
{   LOG_CALL;
    #if defined(Q_OS_WIN)
    ::ShellExecuteA(nullptr, "open", logFilePath().toUtf8(), "", QDir::currentPath().toUtf8(), 1);
    #else
    QString cmd = QStringLiteral("open ") + logFilePath();
    system(cmd.toUtf8().constData());
    #endif
}
void MainWindow::on_actionDatenbank_Views_schreiben_triggered()
{
    insert_views(QSqlDatabase::database());
}
// about
void MainWindow::on_action_about_DKV2_triggered()
{   LOG_CALL;
    QString msg;
    msg = qsl("Lieber Anwender. \nDKV2 wird von seinen Entwicklern kostenlos zur Verfügung gestellt.\n");
    msg += qsl("Es wurde mit viel Arbeit und Sorgfalt entwickelt. Wenn Du es nützlich findest: Viel Spaß bei der Anwendung!!\n");
    msg += qsl("Allerdings darfst Du es nicht verkaufen oder bezahlte Dienste für Einrichtung oder Unterstützung anbieten.\n");
    msg += qsl("DKV2 könnte Fehler enthalten. Wenn Du sie uns mitteilst werden sie vielleicht ausgebessert.\n");
    msg += qsl("Aber aus der Verwendung kannst Du keine Rechte ableiten. Verwende DKV2 so, wie es ist - ");
    msg += qsl("sollten Fehler auftreten übernehmen wir weder Haftung noch Verantwortung - dafür hast Du sicher Verständnis.\n");
    msg += qsl("Viel Spaß mit DKV2 !");
    QMessageBox::information(this, qsl("I n f o"), msg);
}

void MainWindow::on_actionTEST_triggered()
{
    LOG_CALL;
    // input nec. to display the dialog: a Vector of bookings
    toBePrinted.clear();
    toBePrinted = bookings::getAnnualSettelments(2019);
    if (!toBePrinted.size()) {
        qWarning() << "nothing to be printed";
        return;
    }
    currentBooking = toBePrinted.begin();

    prepare_printPreview();
    ui->stackedWidget->setCurrentIndex(printPreviewPageIndex);
}
/////////////////////////////////////////////////
//              PRINTING wprev.                //
/////////////////////////////////////////////////
QString letterName(booking b)
{
    QString txt = qsl("<table width=100%><tr><td align=center style='padding-top:5px;padding-bottom:5px;'>%1, %2; %3<br><b>%4</b></td></tr></table>");
    contract cont(b.contractId);
    creditor cred(cont.creditorId());
    QString lettertype = booking::displayString(b.type) +qsl(" ");
    lettertype += (b.type == booking::Type::annualInterestDeposit) ? QString::number(b.date.year() - 1) : b.date.toString();

    txt = txt.arg(cred.lastname(), cred.firstname(), cont.label(), lettertype);
    return txt;
}

void MainWindow::prepare_printPreview()
{
    LOG_CALL;
    ui->btnPrevBooking->setEnabled(currentBooking != toBePrinted.cbegin());
    ui->btnNextBooking->setEnabled((currentBooking +1) != toBePrinted.cend());

    ui->lblLetter->setText (letterName(*currentBooking));

    QFont f(qsl("Verdana"));
    ui->fontComboBox->setCurrentFont(f);
    ui->spinFontSize->setValue(10);
}

void MainWindow::on_btnNextBooking_clicked()
{
    LOG_CALL;
    if ((currentBooking+1) == toBePrinted.end())
        return;
    else
        currentBooking = currentBooking +1;
    prepare_printPreview();
}

void MainWindow::on_btnPrevBooking_clicked()
{
    LOG_CALL;
    if (currentBooking == toBePrinted.begin())
        return;
    else
        currentBooking = currentBooking -1;
    prepare_printPreview();
}

void MainWindow::on_btnUpdatePreview_clicked()
{
    ui->wPreview->updatePreview();
}

void MainWindow::doPaint(QPrinter* pri)
{
    QPainter p(dynamic_cast<QPaintDevice*>(pri));
    p.drawText(QPoint(100, 100), "Hallo World");

    // Logo
    // Adresse
    // Datum
    // Anrede
    // Fußzeile
    // text(e)
}
