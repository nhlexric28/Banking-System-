#ifndef BANKSYSTEM_H
#define BANKSYSTEM_H

#include "Models.h"
#include <vector>
#include <string>

class BankSystem {
private:
    /* ---------------- data stores ---------------- */
    vector<Branch>      branches;
    vector<Teller>      tellers;
    vector<Customer*>   customers;
    vector<Transaction> transactions;

    /* ---------------- configuration ---------------- */
    string bankName;
    int    maxLoginAttempts;
    double minDepositSavings;
    double minDepositCheque;
    double minDepositFixed;
    double minDepositStudent;
    string currency;

    /* ---------------- counters ---------------- */
    int accountCounter;
    int customerCounter;
    int transactionCounter;

    /* ---------------- session ---------------- */
    Teller*  currentTeller;
    Customer* currentCustomer;
    string   currentAccountNumber;

    /* ---------------- file names ---------------- */
    string fileCustomers;
    string fileTransactions;
    string fileBranches;
    string fileTellers;
    string fileConfig;

    /* ---------------- private helpers ---------------- */
    void seedDefaultData();

    void loadConfiguration();
    void saveConfiguration();
    void loadBranches();
    void saveBranches();
    void loadTellers();
    void saveTellers();
    void loadCustomers();
    void saveCustomers();
    void loadTransactions();
    void saveTransactions();
    void rebuildCounters();

    string generateAccountNumber(const string& branchCode);
    string generateCustomerId();
    string generateTransactionId();
    string generateUniquePIN();
    bool   pinExists(const string& pin) const;

    Customer* findCustomerByAccount(const string& accNo, Account** outAccount = 0);
    Customer* findCustomerByIdNumber(const string& idNo);
    Branch*   findBranch(const string& code);
    Account*  findAccountAnywhere(const string& accNo);

    void recordTransaction(const string& accNo, const string& type, double amount,
                           double balanceAfter, const string& performedBy,
                           const string& branchCode, const string& desc);

    void doDeposit(Account* acc, double amount, const string& performedBy,
                   const string& branchCode);
    void doWithdrawal(Account* acc, double amount, const string& performedBy,
                      const string& branchCode);
    void doTransfer(Account* from, Account* to, double amount,
                    const string& performedBy, const string& branchCode);

    void printStatement(const string& accNo);
    void printHeader(const string& title) const;
    bool loginTeller(const string& id, const string& pwd, Teller** outTeller);
    bool verifyCustomerPin(Customer* cust);

public:
    BankSystem();
    ~BankSystem();

    void initialise();
    void run();
    void saveAll();

    /* menus */
    void tellerLogin();
    void tellerMenu();
    void customerLogin();
    void customerMenu();

    /* teller functions */
    void registerCustomer();
    void viewCustomerDetails();
    void processTellerTransaction();
    void tellerReports();
    void searchCustomersMenu();
    void applyInterest();
    void branchMenu();
    void systemMaintenance();

    /* customer self-service */
    void viewBalance();
    void selfDeposit();
    void selfWithdraw();
    void selfTransfer();
    void viewStatement();
    void changePIN();

    /* reports (1.6) */
    void dailyTransactionReport();
    void customerAccountSummary();
    void branchPerformanceReport();

    /* file handling (1.7) */
    void backupData();
    void exportCustomersCSV();
    void exportTransactionsCSV();
    void exportSummaryTXT();
};

#endif
