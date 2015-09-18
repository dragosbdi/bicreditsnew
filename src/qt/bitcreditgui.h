// Copyright (c) 2014-2015 The Bitcredit Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCREDIT_QT_BITCREDITGUI_H
#define BITCREDIT_QT_BITCREDITGUI_H

#if defined(HAVE_CONFIG_H)
#include "config/bitcredit-config.h"
#endif

#include "amount.h"

#include <string>
#include <QLabel>
#include <QMainWindow>
#include <QMap>
#include <QMenu>
#include <QPoint>
#include <QSystemTrayIcon>
#include <QModelIndex>

class ClientModel;
class NetworkStyle;
class VoteCoinsDialog;
class MessagePage;
class InvoicePage;
class ReceiptPage;
class MessageModel;
//class BanknodeManager;
class SendMessagesDialog;
class Notificator;
class OptionsModel;
class RPCConsole;
class ExchangeBrowser;
class ChatWindow;
class SendCoinsRecipient;
class UnitDisplayStatusBarControl;
class WalletFrame;
class WalletModel;
class TestPage;

class CWallet;

QT_BEGIN_NAMESPACE
class QAction;
class QProgressBar;
class QProgressDialog;
QT_END_NAMESPACE

/**
  Bitcredit GUI main class. This class represents the main window of the Bitcredit UI. It communicates with both the client and
  wallet models to give the user an up-to-date view of the current core state.
*/
class BitcreditGUI : public QMainWindow
{
    Q_OBJECT

public:
    static const QString DEFAULT_WALLET;

    explicit BitcreditGUI(const NetworkStyle *networkStyle, QWidget *parent = 0);
    ~BitcreditGUI();

    /** Set the client model.
        The client model represents the part of the core that communicates with the P2P network, and is wallet-agnostic.
    */
    void setClientModel(ClientModel *clientModel);
    //std::string theme;
    //QString themestr;
    

    
#ifdef ENABLE_WALLET
    /** Set the wallet model.
        The wallet model represents a bitcredit wallet, and offers access to the list of transactions, address book and sending
        functionality.
    */
    bool addWallet(const QString& name, WalletModel *walletModel);
    bool setCurrentWallet(const QString& name);
    void removeAllWallets();
#endif // ENABLE_WALLET
    bool enableWallet;

protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    bool eventFilter(QObject *object, QEvent *event);
	void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    int m_nMouseClick_X_Coordinate;
    int m_nMouseClick_Y_Coordinate;
private:
    ClientModel *clientModel;
    WalletFrame *walletFrame;
    MessageModel *messageModel;

    UnitDisplayStatusBarControl *unitDisplayControl;
    QLabel *labelEncryptionIcon;
    QLabel *labelConnectionsIcon;
    QLabel *labelBlocksIcon;
    QLabel *progressBarLabel;
    QProgressBar *progressBar;
    QProgressDialog *progressDialog;
    QWidget *wId2;
    QWidget *wId;
    QWidget *wId3;
    QWidget *wId4; 
    QMenuBar *appMenuBar;
    QAction *chatAction;
    QAction *exchangeAction;
    QAction *overviewAction;
    QAction *historyAction;
    QAction *quitAction;
    QAction *sendCoinsAction;
    QAction *usedSendingAddressesAction;
    QAction *usedReceivingAddressesAction;
    QAction *signMessageAction;
    QAction *verifyMessageAction;
    QAction *aboutAction;
    QAction *receiveCoinsAction;
    QAction *optionsAction;
    QAction *toggleHideAction;
    QAction *encryptWalletAction;
    QAction *backupWalletAction;
    QAction *changePassphraseAction;
    QAction *banknodeManagerAction;
    QAction *aboutQtAction;
    QAction *openRPCConsoleAction;
    QAction *openAction;
    QAction *showHelpMessageAction;
    QAction *blockAction;
    QAction *bankstatsAction;
    QAction *paperWalletAction;
	QAction * voteCoinsAction;
    QAction *sendMessagesAction;
    QAction *sendMessagesAnonAction;
    QAction *messageAction;
    QAction *invoiceAction;
    QAction *receiptAction;
    QAction *actionSendReceive;
    QAction *actionSendReceiveMess;
    QAction *actionSendReceiveinv;
    QAction *actionSendReceivestats;
    QAction *testAction;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    Notificator *notificator;
    RPCConsole *rpcConsole;
    //std::string theme;
    //QString str;

    /** Keep track of previous number of blocks, to detect progress */
    int prevBlocks;
    int spinnerFrame;

    /** Create the main UI actions. */
    void createActions(const NetworkStyle *networkStyle);
    /** Create the menu bar and sub-menus. */
    void createMenuBar();
    /** Create the toolbars */
    void createToolBars();
    /** Create system tray icon and notification */
    void createTrayIcon(const NetworkStyle *networkStyle);
    /** Create system tray menu (or setup the dock menu) */
    void createTrayIconMenu();

    /** Enable or disable all wallet-related actions */
    void setWalletActionsEnabled(bool enabled);

    /** Connect core signals to GUI client */
    void subscribeToCoreSignals();
    /** Disconnect core signals from GUI client */
    void unsubscribeFromCoreSignals();
    
    void tb2hover();

signals:
    /** Signal raised when a URI was entered or dragged to the GUI */
    void receivedURI(const QString &uri);

public:
    std::string theme;
    QString themestr;

public slots:
    /** Set number of connections shown in the UI */
    void setNumConnections(int count);
    /** Set number of blocks shown in the UI */
    void setNumBlocks(int count);

    /** Notify the user of an event from the core network or transaction handling code.
       @param[in] title     the message box / notification title
       @param[in] message   the displayed text
       @param[in] style     modality and style definitions (icon and used buttons - buttons only for message boxes)
                            @see CClientUIInterface::MessageBoxFlags
       @param[in] ret       pointer to a bool that will be modified to whether Ok was clicked (modal only)
    */
    void message(const QString &title, const QString &message, unsigned int style, bool *ret = NULL);

#ifdef ENABLE_WALLET
    /** Set the encryption status as shown in the UI.
       @param[in] status            current encryption status
       @see WalletModel::EncryptionStatus
    */
    void setEncryptionStatus(int status);

    bool handlePaymentRequest(const SendCoinsRecipient& recipient);
	void incomingMessage(const QString& sent_datetime, QString from_address, QString to_address, QString message, int type);
    /** Show incoming transaction notification for new transactions. */
    void incomingTransaction(const QString& date, int unit, const CAmount& amount, const QString& type, const QString& address);
#endif // ENABLE_WALLET

private slots:
#ifdef ENABLE_WALLET
    /** Switch to overview (home) page */
    void gotoOverviewPage();
    /** Switch to history (transactions) page */
    void gotoHistoryPage();
	/** Switch to chat page */
    void gotoChatPage();
	/** Switch to exchange browser page */
    void gotoExchangeBrowserPage();
    /** Switch to receive coins page */
    void gotoReceiveCoinsPage();
    /** Switch to send coins page */
    void gotoSendCoinsPage(QString addr = "");
	/** Switch to blockexplorer */
    void gotoBlockBrowser();
    void gotoBankStatisticsPage();
    void gotoSendMessagesPage();
    /** Switch to send anonymous messages page */
    /** Switch to view messages page */
    void gotoMessagesPage();
    /** Switch to invoices page */
    void gotoInvoicesPage();
    /** Switch to receipt page */
    void gotoReceiptPage();
    void gotoBanknodeManagerPage();
    void gotoTestPage();
    /** Switch to vote page */
    /** Switch to vote coins page */
    void gotoVoteCoinsPage(QString addr = "");
 
    /** Show Sign/Verify Message dialog and switch to sign message tab */
    void gotoSignMessageTab(QString addr = "");
    /** Show Sign/Verify Message dialog and switch to verify message tab */
    void gotoVerifyMessageTab(QString addr = "");

    /** Show open dialog */
    void openClicked();
#endif // ENABLE_WALLET
    /** Show configuration dialog */
    void optionsClicked();
    /** Show about dialog */
    void aboutClicked();
    /** Show help message dialog */
    void showHelpMessageClicked();
#ifndef Q_OS_MAC
    /** Handle tray icon clicked */
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
#endif
   

    /** Show window if hidden, unminimize when minimized, rise when obscured or show if hidden and fToggleHidden is true */
    void showNormalIfMinimized(bool fToggleHidden = false);
    /** Simply calls showNormalIfMinimized(true) for use in SLOT() macro */
    void toggleHidden();

    /** called by a timer to check if fRequestShutdown has been set **/
    void detectShutdown();

    /** Show progress dialog e.g. for verifychain */
    void showProgress(const QString &title, int nProgress);
};

class UnitDisplayStatusBarControl : public QLabel
{
    Q_OBJECT

public:
    explicit UnitDisplayStatusBarControl();
    /** Lets the control know about the Options Model (and its signals) */
    void setOptionsModel(OptionsModel *optionsModel);

protected:
    /** So that it responds to left-button clicks */
    void mousePressEvent(QMouseEvent *event);

private:
    OptionsModel *optionsModel;
    QMenu* menu;
    /** Shows context menu with Display Unit options by the mouse coordinates */
    void onDisplayUnitsClicked(const QPoint& point);
    /** Creates context menu, its actions, and wires up all the relevant signals for mouse events. */
    void createContextMenu();

private slots:
    /** When Display Units are changed on OptionsModel it will refresh the display text of the control on the status bar */
    void updateDisplayUnit(int newUnits);
    /** Tells underlying optionsModel to update its current display unit. */
    void onMenuSelection(QAction* action);
};

#endif // BITCREDIT_QT_BITCREDITGUI_H
