#include "BankSystem.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <cstdlib>

using namespace std;

/* ================================================================
   CONSTRUCTOR / DESTRUCTOR / INITIALISE
   ================================================================ */
BankSystem::BankSystem() {
    fileCustomers    = "customers.dat";
    fileTransactions = "transactions.dat";
    fileBranches     = "branches.dat";
    fileTellers      = "tellers.dat";
    fileConfig       = "system_config.txt";

    bankName          = "Standard Bank";
    maxLoginAttempts  = 3;
    minDepositSavings = 50.0;
    minDepositCheque  = 100.0;
    minDepositFixed   = 1000.0;
    minDepositStudent = 20.0;
    currency          = "R";

    accountCounter     = 0;
    customerCounter    = 0;
    transactionCounter = 0;

    currentTeller        = 0;
    currentCustomer      = 0;
    currentAccountNumber = "";
}

BankSystem::~BankSystem() {
    for (size_t i = 0; i < customers.size(); ++i) delete customers[i];
    customers.clear();
}

void BankSystem::initialise() {
    loadConfiguration();
    loadBranches();
    loadTellers();
    loadCustomers();
    loadTransactions();
    seedDefaultData();     // only seeds if the branch / teller lists are empty
    rebuildCounters();
    saveAll();
}

void BankSystem::saveAll() {
    saveConfiguration();
    saveBranches();
    saveTellers();
    saveCustomers();
    saveTransactions();
}

/* ================================================================
   1.7  FILE HANDLING - CONFIGURATION  (system_config.txt)
   ================================================================ */
static string cfgValue(const string& line) {
    size_t p = line.find('=');
    if (p == string::npos) return "";
    return Utils::trim(line.substr(p + 1));
}

void BankSystem::loadConfiguration() {
    ifstream in(fileConfig.c_str());
    if (!in) return;

    string line;
    while (getline(in, line)) {
        line = Utils::trim(line);
        if (line.empty() || line[0] == '#') continue;

        if (line.find("BANK_NAME=") == 0)          bankName = cfgValue(line);
        else if (line.find("MAX_LOGIN_ATTEMPTS=") == 0) maxLoginAttempts = atoi(cfgValue(line).c_str());
        else if (line.find("MIN_DEPOSIT_SAVINGS=") == 0) minDepositSavings = atof(cfgValue(line).c_str());
        else if (line.find("MIN_DEPOSIT_CHEQUE=") == 0)  minDepositCheque  = atof(cfgValue(line).c_str());
        else if (line.find("MIN_DEPOSIT_FIXED=") == 0)   minDepositFixed   = atof(cfgValue(line).c_str());
        else if (line.find("MIN_DEPOSIT_STUDENT=") == 0) minDepositStudent = atof(cfgValue(line).c_str());
        else if (line.find("CURRENCY=") == 0)            currency = cfgValue(line);
        else if (line.find("ACCOUNT_COUNTER=") == 0)     accountCounter = atoi(cfgValue(line).c_str());
        else if (line.find("CUSTOMER_COUNTER=") == 0)    customerCounter = atoi(cfgValue(line).c_str());
        else if (line.find("TRANSACTION_COUNTER=") == 0) transactionCounter = atoi(cfgValue(line).c_str());
    }
    in.close();
}

void BankSystem::saveConfiguration() {
    ofstream out(fileConfig.c_str());
    if (!out) return;
    out << "# ============================================\n";
    out << "# Multi-Branch Banking System Configuration\n";
    out << "# ============================================\n";
    out << "BANK_NAME=" << bankName << "\n";
    out << "MAX_LOGIN_ATTEMPTS=" << maxLoginAttempts << "\n";
    out << "MIN_DEPOSIT_SAVINGS=" << minDepositSavings << "\n";
    out << "MIN_DEPOSIT_CHEQUE=" << minDepositCheque << "\n";
    out << "MIN_DEPOSIT_FIXED=" << minDepositFixed << "\n";
    out << "MIN_DEPOSIT_STUDENT=" << minDepositStudent << "\n";
    out << "CURRENCY=" << currency << "\n";
    out << "ACCOUNT_COUNTER=" << accountCounter << "\n";
    out << "CUSTOMER_COUNTER=" << customerCounter << "\n";
    out << "TRANSACTION_COUNTER=" << transactionCounter << "\n";
    out.close();
}

/* ================================================================
   1.7  FILE HANDLING - BINARY .dat FILES
   ================================================================ */
void BankSystem::loadBranches() {
    branches.clear();
    ifstream in(fileBranches.c_str(), ios::binary);
    if (!in) return;

    int count = 0;
    in.read(reinterpret_cast<char*>(&count), sizeof(count));
    if (!in) { in.close(); return; }

    for (int i = 0; i < count; ++i) {
        Branch b;
        b.readFromFile(in);
        if (!in) break;
        branches.push_back(b);
    }
    in.close();
}

void BankSystem::saveBranches() {
    ofstream out(fileBranches.c_str(), ios::binary | ios::trunc);
    if (!out) return;
    int count = (int)branches.size();
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    for (int i = 0; i < count; ++i) branches[i].writeToFile(out);
    out.close();
}

void BankSystem::loadTellers() {
    tellers.clear();
    ifstream in(fileTellers.c_str(), ios::binary);
    if (!in) return;

    int count = 0;
    in.read(reinterpret_cast<char*>(&count), sizeof(count));
    if (!in) { in.close(); return; }

    for (int i = 0; i < count; ++i) {
        Teller t;
        t.readFromFile(in);
        if (!in) break;
        tellers.push_back(t);
    }
    in.close();
}

void BankSystem::saveTellers() {
    ofstream out(fileTellers.c_str(), ios::binary | ios::trunc);
    if (!out) return;
    int count = (int)tellers.size();
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    for (int i = 0; i < count; ++i) tellers[i].writeToFile(out);
    out.close();
}

void BankSystem::loadCustomers() {
    for (size_t i = 0; i < customers.size(); ++i) delete customers[i];
    customers.clear();

    ifstream in(fileCustomers.c_str(), ios::binary);
    if (!in) return;

    int count = 0;
    in.read(reinterpret_cast<char*>(&count), sizeof(count));
    if (!in) { in.close(); return; }

    for (int i = 0; i < count; ++i) {
        Customer* c = new Customer();
        c->readFromFile(in);
        if (!in) { delete c; break; }
        customers.push_back(c);
    }
    in.close();
}

void BankSystem::saveCustomers() {
    ofstream out(fileCustomers.c_str(), ios::binary | ios::trunc);
    if (!out) return;
    int count = (int)customers.size();
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    for (int i = 0; i < count; ++i) customers[i]->writeToFile(out);
    out.close();
}

void BankSystem::loadTransactions() {
    transactions.clear();
    ifstream in(fileTransactions.c_str(), ios::binary);
    if (!in) return;

    int count = 0;
    in.read(reinterpret_cast<char*>(&count), sizeof(count));
    if (!in) { in.close(); return; }

    for (int i = 0; i < count; ++i) {
        Transaction t;
        t.readFromFile(in);
        if (!in) break;
        transactions.push_back(t);
    }
    in.close();
}

void BankSystem::saveTransactions() {
    ofstream out(fileTransactions.c_str(), ios::binary | ios::trunc);
    if (!out) return;
    int count = (int)transactions.size();
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    for (int i = 0; i < count; ++i) transactions[i].writeToFile(out);
    out.close();
}

/* ================================================================
   SEED DEFAULT DATA (1.4  - at least 3 branches)
   ================================================================ */
void BankSystem::seedDefaultData() {
    if (branches.empty()) {
        branches.push_back(Branch("BR001", "Standard Bank Johannesburg CBD",
            "5 Simmonds Street, Johannesburg, 2001", "0116369111", "Thabo Nkosi"));
        branches.push_back(Branch("BR002", "Standard Bank Durban Central",
            "1 Kingsmead Way, Durban, 4001", "0313605000", "Priya Naidoo"));
        branches.push_back(Branch("BR003", "Standard Bank Cape Town Central",
            "1 Adderley Street, Cape Town, 8001", "0214804000", "Johan van Wyk"));
        saveBranches();
    }

    if (tellers.empty()) {
        tellers.push_back(Teller("T001", "Thabo Nkosi",   "teller123", "BR001"));
        tellers.push_back(Teller("T002", "Priya Naidoo",  "teller123", "BR002"));
        tellers.push_back(Teller("T003", "Johan van Wyk", "teller123", "BR003"));
        saveTellers();
    }
}

void BankSystem::rebuildCounters() {
    /* account counter */
    for (size_t i = 0; i < customers.size(); ++i) {
        vector<Account*>& accs = customers[i]->getAccounts();
        for (size_t j = 0; j < accs.size(); ++j) {
            string n = accs[j]->getAccountNumber();
            size_t p = n.find_last_of('-');
            if (p != string::npos) {
                int v = atoi(n.substr(p + 1).c_str());
                if (v > accountCounter) accountCounter = v;
            }
        }
        string cid = customers[i]->getCustomerId();
        size_t q = cid.find_last_of('-');
        if (q != string::npos) {
            int v = atoi(cid.substr(q + 1).c_str());
            if (v > customerCounter) customerCounter = v;
        }
    }

    for (size_t i = 0; i < transactions.size(); ++i) {
        string tid = transactions[i].transactionId;
        size_t p = tid.find_last_of('-');
        if (p != string::npos) {
            int v = atoi(tid.substr(p + 1).c_str());
            if (v > transactionCounter) transactionCounter = v;
        }
    }
}

/* ================================================================
   GENERATORS
   ================================================================ */
string BankSystem::generateAccountNumber(const string& branchCode) {
    accountCounter++;
    stringstream ss;
    ss << "ACC-" << branchCode << "-" << setw(5) << setfill('0') << accountCounter;
    return ss.str();
}

string BankSystem::generateCustomerId() {
    customerCounter++;
    stringstream ss;
    ss << "CUST-" << setw(4) << setfill('0') << customerCounter;
    return ss.str();
}

string BankSystem::generateTransactionId() {
    transactionCounter++;
    stringstream ss;
    ss << "TXN-" << setw(6) << setfill('0') << transactionCounter;
    return ss.str();
}

string BankSystem::generateUniquePIN() {
    string pin;
    int guard = 0;
    do {
        pin = Utils::generatePIN(5);
        guard++;
    } while (pinExists(pin) && guard < 5000);
    return pin;
}

bool BankSystem::pinExists(const string& pin) const {
    string enc = Utils::encryptText(pin);
    for (size_t i = 0; i < customers.size(); ++i)
        if (customers[i]->getEncryptedPin() == enc) return true;
    return false;
}

/* ================================================================
   LOOKUP HELPERS
   ================================================================ */
Customer* BankSystem::findCustomerByAccount(const string& accNo, Account** outAccount) {
    for (size_t i = 0; i < customers.size(); ++i) {
        Account* a = customers[i]->findAccount(accNo);
        if (a) {
            if (outAccount) *outAccount = a;
            return customers[i];
        }
    }
    if (outAccount) *outAccount = 0;
    return 0;
}

Customer* BankSystem::findCustomerByIdNumber(const string& idNo) {
    for (size_t i = 0; i < customers.size(); ++i)
        if (customers[i]->getIdNumber() == idNo) return customers[i];
    return 0;
}

Branch* BankSystem::findBranch(const string& code) {
    for (size_t i = 0; i < branches.size(); ++i)
        if (branches[i].branchCode == code) return &branches[i];
    return 0;
}

Account* BankSystem::findAccountAnywhere(const string& accNo) {
    Account* a = 0;
    findCustomerByAccount(accNo, &a);
    return a;
}

/* ================================================================
   TRANSACTION RECORDING (1.3)
   ================================================================ */
void BankSystem::recordTransaction(const string& accNo, const string& type, double amount,
                                   double balanceAfter, const string& performedBy,
                                   const string& branchCode, const string& desc) {
    Transaction t(generateTransactionId(), accNo, type, amount, balanceAfter,
                  Utils::currentDateTime(), performedBy, branchCode, desc);
    transactions.push_back(t);
    saveTransactions();
    saveConfiguration();     // persist the transaction counter
}

void BankSystem::doDeposit(Account* acc, double amount, const string& performedBy,
                           const string& branchCode) {
    if (amount <= 0) throw BankingException("Deposit amount must be greater than zero.");
    if (amount > 1000000.0) throw BankingException("Deposit exceeds the single-transaction maximum of R 1,000,000.00.");

    acc->addDeposit(amount);
    recordTransaction(acc->getAccountNumber(), "Deposit", amount, acc->getBalance(),
                      performedBy, branchCode, "Cash deposit");
    saveCustomers();

    cout << "\n  [+] Deposit successful.\n";
    cout << "  New balance: " << Utils::formatMoney(acc->getBalance()) << "\n";
}

void BankSystem::doWithdrawal(Account* acc, double amount, const string& performedBy,
                              const string& branchCode) {
    if (amount <= 0) throw BankingException("Withdrawal amount must be greater than zero.");

    string reason;
    if (!acc->canWithdraw(amount, reason)) throw BankingException(reason);

    acc->addWithdrawal(amount);
    recordTransaction(acc->getAccountNumber(), "Withdrawal", amount, acc->getBalance(),
                      performedBy, branchCode, "Cash withdrawal");
    saveCustomers();

    cout << "\n  [+] Withdrawal successful.\n";
    cout << "  New balance: " << Utils::formatMoney(acc->getBalance()) << "\n";
}

void BankSystem::doTransfer(Account* from, Account* to, double amount,
                            const string& performedBy, const string& branchCode) {
    if (amount <= 0) throw BankingException("Transfer amount must be greater than zero.");

    string reason;
    if (!from->canWithdraw(amount, reason)) throw BankingException(reason);

    from->addWithdrawal(amount);
    to->addDeposit(amount);

    recordTransaction(from->getAccountNumber(), "Transfer-Out", amount, from->getBalance(),
                      performedBy, branchCode, "Transfer to " + to->getAccountNumber());
    recordTransaction(to->getAccountNumber(), "Transfer-In", amount, to->getBalance(),
                      performedBy, branchCode, "Transfer from " + from->getAccountNumber());
    saveCustomers();

    cout << "\n  [+] Transfer successful.\n";
    cout << "  Source balance : " << Utils::formatMoney(from->getBalance()) << "\n";
    cout << "  Target balance : " << Utils::formatMoney(to->getBalance()) << "\n";
}

/* ================================================================
   UI HEADER
   ================================================================ */
void BankSystem::printHeader(const string& title) const {
    cout << "\n============================================================\n";
    cout << "  " << bankName << " - Multi-Branch Banking Management System\n";
    cout << "  " << title << "\n";
    cout << "============================================================\n";
}

/* ================================================================
   MAIN LOOP
   ================================================================ */
void BankSystem::run() {
    while (true) {
        Utils::clearScreen();
        printHeader("MAIN MENU");
        cout << "  1. Bank Teller Login\n";
        cout << "  2. Customer Login (Self-Service)\n";
        cout << "  3. View All Branches\n";
        cout << "  4. System Information\n";
        cout << "  0. Exit System\n";

        int choice = Utils::readInt("\n  Enter your choice: ");

        switch (choice) {
            case 1: tellerLogin(); break;
            case 2: customerLogin(); break;
            case 3: branchMenu(); break;
            case 4:
                Utils::clearScreen();
                printHeader("SYSTEM INFORMATION");
                cout << "  Bank Name        : " << bankName << "\n";
                cout << "  Branches         : " << branches.size() << "\n";
                cout << "  Registered Cust. : " << customers.size() << "\n";
                cout << "  Teller Accounts  : " << tellers.size() << "\n";
                cout << "  Transactions     : " << transactions.size() << "\n";
                cout << "  Max Login Tries  : " << maxLoginAttempts << "\n";
                cout << "  Today's Date     : " << Utils::currentDate() << "\n";
                Utils::pause();
                break;
            case 0:
                saveAll();
                cout << "\n  All data saved. Thank you for using "
                     << bankName << " Banking System. Goodbye!\n\n";
                return;
            default:
                cout << "  [!] Invalid option. Please try again.\n";
                Utils::pause();
        }
    }
}

/* ================================================================
   1.1.1  TELLER AUTHENTICATION
   ================================================================ */
bool BankSystem::loginTeller(const string& id, const string& pwd, Teller** outTeller) {
    for (size_t i = 0; i < tellers.size(); ++i) {
        if (Utils::toUpper(tellers[i].getTellerId()) == Utils::toUpper(id)) {
            if (tellers[i].checkPassword(pwd)) {
                *outTeller = &tellers[i];
                return true;
            }
            return false;
        }
    }
    return false;
}

void BankSystem::tellerLogin() {
    Utils::clearScreen();
    printHeader("BANK TELLER LOGIN");

    int attempts = 0;
    while (attempts < maxLoginAttempts) {
        string id  = Utils::readLine("  Teller ID  : ");
        string pwd = Utils::readLine("  Password   : ");

        Teller* t = 0;
        if (loginTeller(id, pwd, &t)) {
            currentTeller = t;
            cout << "\n  [+] Login successful. Welcome, " << t->getFullName()
                 << " (" << t->getBranchCode() << ")!\n";
            Utils::pause();
            tellerMenu();
            return;
        }

        attempts++;
        cout << "  [!] Invalid Teller ID or Password. Attempts remaining: "
             << (maxLoginAttempts - attempts) << "\n\n";
    }

    cout << "\n  [!] Too many failed login attempts. Returning to main menu.\n";
    Utils::pause();
}

/* ================================================================
   TELLER MENU
   ================================================================ */
void BankSystem::tellerMenu() {
    while (true) {
        Utils::clearScreen();
        printHeader("TELLER MENU - " + currentTeller->getBranchCode()
                    + " (" + currentTeller->getFullName() + ")");
        cout << "   1. Register New Customer Account\n";
        cout << "   2. View Customer Details\n";
        cout << "   3. Process Customer Transaction\n";
        cout << "   4. Generate Reports\n";
        cout << "   5. Search & Filter Customers\n";
        cout << "   6. Apply Interest to Eligible Accounts\n";
        cout << "   7. Branch Management\n";
        cout << "   8. System Maintenance (Backup / Export / Unlock)\n";
        cout << "   0. Logout\n";

        int choice = Utils::readInt("\n  Enter your choice: ");

        switch (choice) {
            case 1: registerCustomer();        break;
            case 2: viewCustomerDetails();     break;
            case 3: processTellerTransaction();break;
            case 4: tellerReports();           break;
            case 5: searchCustomersMenu();     break;
            case 6: applyInterest();           break;
            case 7: branchMenu();              break;
            case 8: systemMaintenance();       break;
            case 0:
                currentTeller = 0;
                cout << "\n  [+] Logged out successfully.\n";
                Utils::pause();
                return;
            default:
                cout << "  [!] Invalid option.\n";
                Utils::pause();
        }
    }
}

/* ================================================================
   1.2.1  ACCOUNT REGISTRATION (TELLER FUNCTION)
   ================================================================ */
void BankSystem::registerCustomer() {
    Utils::clearScreen();
    printHeader("REGISTER NEW CUSTOMER ACCOUNT");
    cout << "  Branch: " << currentTeller->getBranchCode() << "\n\n";

    try {
        /* ---- personal details ---- */
        string fname = Utils::readLine("  First Name                    : ");
        if (fname.empty()) throw BankingException("First name is required.");

        string sname = Utils::readLine("  Surname                       : ");
        if (sname.empty()) throw BankingException("Surname is required.");

        /* ---- SA ID number ---- */
        string idNo;
        while (true) {
            idNo = Utils::readLine("  SA ID Number (13 digits)      : ");
            if (!Utils::validateSAID(idNo)) {
                cout << "  [!] Invalid SA ID number (must be 13 digits and pass the Luhn check).\n";
                continue;
            }
            if (findCustomerByIdNumber(idNo) != 0) {
                cout << "  [!] A customer with this SA ID already exists.\n";
                continue;
            }
            break;
        }

        /* ---- contact number ---- */
        string contact;
        while (true) {
            contact = Utils::readLine("  Contact Number (10 digits)    : ");
            if (!Utils::validatePhone(contact)) {
                cout << "  [!] Contact number must be exactly 10 digits.\n";
                continue;
            }
            break;
        }

        /* ---- email ---- */
        string mail;
        while (true) {
            mail = Utils::readLine("  Email Address                 : ");
            if (!Utils::validateEmail(mail)) {
                cout << "  [!] Invalid email address (must contain '@' and a domain).\n";
                continue;
            }
            break;
        }

        /* ---- address ---- */
        string addr = Utils::readLine("  Physical Address              : ");
        if (addr.empty()) throw BankingException("Physical address is required.");

        /* ---- date of birth ---- */
        string dob;
        while (true) {
            dob = Utils::readLine("  Date of Birth (DD/MM/YYYY)    : ");
            if (!Utils::validateDate(dob)) {
                cout << "  [!] Invalid date. Use DD/MM/YYYY format.\n";
                continue;
            }
            break;
        }

        /* ---- account type ---- */
        cout << "\n  Account Types:\n";
        cout << "    1. Savings Account      (min deposit " << Utils::formatMoney(minDepositSavings) << ", 5.50% p.a.)\n";
        cout << "    2. Cheque Account       (min deposit " << Utils::formatMoney(minDepositCheque)  << ", 1.50% p.a., overdraft)\n";
        cout << "    3. Fixed Deposit        (min deposit " << Utils::formatMoney(minDepositFixed)   << ", 8.50% p.a., 12 months)\n";
        cout << "    4. Student Account      (min deposit " << Utils::formatMoney(minDepositStudent) << ", 3.00% p.a.)\n";

        int accType = Utils::readInt("\n  Select Account Type (1-4): ");
        if (accType < 1 || accType > 4)
            throw BankingException("Invalid account type selected.");

        double minDep = minDepositSavings;
        string typeName = "Savings";
        if (accType == 2) { minDep = minDepositCheque;  typeName = "Cheque"; }
        if (accType == 3) { minDep = minDepositFixed;   typeName = "Fixed Deposit"; }
        if (accType == 4) { minDep = minDepositStudent; typeName = "Student"; }

        /* ---- initial deposit ---- */
        double initial = 0.0;
        while (true) {
            initial = Utils::readDouble("  Initial Deposit Amount        : ");
            if (initial < minDep) {
                cout << "  [!] Minimum opening deposit for a " << typeName
                     << " account is " << Utils::formatMoney(minDep) << ".\n";
                continue;
            }
            break;
        }

        /* ---- create the account object (polymorphism) ---- */
        string accNo  = generateAccountNumber(currentTeller->getBranchCode());
        string openDt = Utils::currentDate();

        Account* acc = 0;
        switch (accType) {
            case 1: acc = new SavingsAccount(accNo, initial, openDt);       break;
            case 2: acc = new ChequeAccount(accNo, initial, openDt);        break;
            case 3: acc = new FixedDepositAccount(accNo, initial, openDt);  break;
            case 4: acc = new StudentAccount(accNo, initial, openDt);       break;
        }

        /* ---- generate the unique 5-digit PIN ---- */
        string pin = generateUniquePIN();

        /* ---- create customer ---- */
        Customer* cust = new Customer(generateCustomerId(), fname, sname, idNo,
                                      contact, mail, addr, dob, pin);
        cust->addAccount(acc);
        customers.push_back(cust);

        /* ---- record the opening deposit ---- */
        recordTransaction(accNo, "Deposit", initial, initial,
                          currentTeller->getTellerId(),
                          currentTeller->getBranchCode(),
                          "Account opening deposit");

        saveCustomers();
        saveConfiguration();

        /* ---- show result once to the teller ---- */
        cout << "\n  ============================================================\n";
        cout << "   ACCOUNT CREATED SUCCESSFULLY\n";
        cout << "  ============================================================\n";
        cout << "   Customer ID     : " << cust->getCustomerId() << "\n";
        cout << "   Customer Name   : " << cust->getFullName() << "\n";
        cout << "   Account Number  : " << accNo << "\n";
        cout << "   Account Type    : " << typeName << "\n";
        cout << "   Initial Deposit : " << Utils::formatMoney(initial) << "\n";
        cout << "   Branch Code     : " << currentTeller->getBranchCode() << "\n";
        cout << "  ------------------------------------------------------------\n";
        cout << "   >> GENERATED PIN : " << pin << " <<\n";
        cout << "   (Give this PIN to the customer - it is shown ONLY once)\n";
        cout << "  ============================================================\n";
    }
    catch (const BankingException& ex) {
        cout << "\n  [!] REGISTRATION FAILED: " << ex.what() << "\n";
    }
    catch (const exception& ex) {
        cout << "\n  [!] Unexpected error: " << ex.what() << "\n";
    }

    Utils::pause();
}

/* ================================================================
   VIEW CUSTOMER DETAILS
   ================================================================ */
void BankSystem::viewCustomerDetails() {
    Utils::clearScreen();
    printHeader("VIEW CUSTOMER DETAILS");

    string accNo = Utils::toUpper(Utils::readLine("  Enter Account Number (or 0 to cancel): "));
    if (accNo == "0") return;

    Account* acc = 0;
    Customer* cust = findCustomerByAccount(accNo, &acc);
    if (!cust || !acc) {
        cout << "\n  [!] Account not found.\n";
        Utils::pause();
        return;
    }

    /* branch restriction */
    string branchOfAccount = acc->getAccountNumber().substr(4, 5); // ACC-BRXXX-XXXXX
    if (branchOfAccount != currentTeller->getBranchCode()) {
        cout << "\n  [!] Access denied. This account belongs to branch "
             << branchOfAccount << ", not your branch ("
             << currentTeller->getBranchCode() << ").\n";
        Utils::pause();
        return;
    }

    cout << "\n  ------------------------------------------------\n";
    cust->display();
    cust->displayAccounts();
    cout << "  ------------------------------------------------\n";
    cout << "  Note: PIN is stored encrypted and cannot be displayed.\n";
    Utils::pause();
}

/* ================================================================
   1.3  TRANSACTION PROCESSING (TELLER-ASSISTED)
   ================================================================ */
bool BankSystem::verifyCustomerPin(Customer* cust) {
    if (cust->isLocked()) {
        cout << "\n  [!] This account is currently LOCKED. Ask a supervisor to unlock it.\n";
        return false;
    }

    int attempts = 0;
    while (attempts < maxLoginAttempts) {
        string pin = Utils::readLine("  Verify customer 5-digit PIN: ");
        if (Utils::verifyText(pin, cust->getEncryptedPin())) {
            cust->setFailedAttempts(0);
            saveCustomers();
            return true;
        }
        attempts++;
        cout << "  [!] Incorrect PIN. Attempts remaining: "
             << (maxLoginAttempts - attempts) << "\n";
    }

    cust->setLocked(true);
    saveCustomers();
    cout << "\n  [!] Maximum attempts exceeded. Account has been LOCKED.\n";
    return false;
}

void BankSystem::processTellerTransaction() {
    Utils::clearScreen();
    printHeader("PROCESS CUSTOMER TRANSACTION");

    string accNo = Utils::toUpper(Utils::readLine("  Customer Account Number: "));
    Account* acc = 0;
    Customer* cust = findCustomerByAccount(accNo, &acc);

    if (!cust || !acc) {
        cout << "\n  [!] Account not found.\n";
        Utils::pause();
        return;
    }

    cout << "\n  Customer : " << cust->getFullName() << "\n";
    cout << "  Account  : " << acc->getAccountNumber()
         << "  (" << acc->getAccountType() << ")\n";
    cout << "  Balance  : " << Utils::formatMoney(acc->getBalance()) << "\n\n";

    if (!verifyCustomerPin(cust)) { Utils::pause(); return; }

    cout << "\n  Transaction Types:\n";
    cout << "    1. Cash Deposit\n";
    cout << "    2. Cash Withdrawal\n";
    cout << "    3. Fund Transfer\n";
    cout << "    4. Balance Enquiry\n";
    cout << "    5. Account Statement\n";
    cout << "    0. Cancel\n";

    int t = Utils::readInt("\n  Select transaction type: ");

    try {
        switch (t) {
            case 1: {
                double amt = Utils::readDouble("  Deposit Amount: ");
                doDeposit(acc, amt, currentTeller->getTellerId(),
                          currentTeller->getBranchCode());
                break;
            }
            case 2: {
                double amt = Utils::readDouble("  Withdrawal Amount: ");
                doWithdrawal(acc, amt, currentTeller->getTellerId(),
                             currentTeller->getBranchCode());
                break;
            }
            case 3: {
                string target = Utils::toUpper(Utils::readLine("  Target Account Number: "));
                Account* to = findAccountAnywhere(target);
                if (!to) throw BankingException("Target account not found.");
                if (to == acc) throw BankingException("Cannot transfer to the same account.");
                double amt = Utils::readDouble("  Transfer Amount: ");
                doTransfer(acc, to, amt, currentTeller->getTellerId(),
                           currentTeller->getBranchCode());
                break;
            }
            case 4:
                cout << "\n  Current Balance: " << Utils::formatMoney(acc->getBalance()) << "\n";
                break;
            case 5:
                printStatement(acc->getAccountNumber());
                break;
            case 0:
                cout << "\n  Transaction cancelled.\n";
                break;
            default:
                throw BankingException("Invalid transaction type selected.");
        }
    }
    catch (const BankingException& ex) {
        cout << "\n  [!] TRANSACTION FAILED: " << ex.what() << "\n";
    }
    catch (const exception& ex) {
        cout << "\n  [!] Unexpected error: " << ex.what() << "\n";
    }

    Utils::pause();
}

/* ================================================================
   1.1.2  CUSTOMER LOGIN (SELF-SERVICE)
   ================================================================ */
void BankSystem::customerLogin() {
    Utils::clearScreen();
    printHeader("CUSTOMER SELF-SERVICE LOGIN");

    string accNo = Utils::toUpper(Utils::readLine("  Account Number (ACC-BRXXX-XXXXX): "));

    Account* acc = 0;
    Customer* cust = findCustomerByAccount(accNo, &acc);

    if (!cust || !acc) {
        cout << "\n  [!] Account not found. Please check the account number.\n";
        Utils::pause();
        return;
    }

    if (cust->isLocked()) {
        cout << "\n  [!] This account is temporarily LOCKED due to too many failed login attempts.\n";
        cout << "      Please contact your branch to have it unlocked.\n";
        Utils::pause();
        return;
    }

    int attempts = 0;
    while (attempts < maxLoginAttempts) {
        string pin = Utils::readLine("  Enter your 5-digit PIN: ");

        if (Utils::verifyText(pin, cust->getEncryptedPin())) {
            cust->setFailedAttempts(0);
            saveCustomers();
            currentCustomer      = cust;
            currentAccountNumber = accNo;
            cout << "\n  [+] Login successful. Welcome, " << cust->getFullName() << "!\n";
            Utils::pause();
            customerMenu();
            return;
        }

        attempts++;
        cust->setFailedAttempts(attempts);
        saveCustomers();
        cout << "  [!] Incorrect PIN. Attempts remaining: "
             << (maxLoginAttempts - attempts) << "\n";
    }

    cust->setLocked(true);
    saveCustomers();
    cout << "\n  [!] Maximum login attempts exceeded. Your account has been temporarily LOCKED.\n";
    Utils::pause();
}

/* ================================================================
   CUSTOMER SELF-SERVICE MENU
   ================================================================ */
void BankSystem::customerMenu() {
    while (true) {
        Utils::clearScreen();
        printHeader("CUSTOMER MENU - " + currentAccountNumber);

        Account* acc = findAccountAnywhere(currentAccountNumber);
        if (!acc) { currentCustomer = 0; return; }

        cout << "  Welcome, " << currentCustomer->getFullName() << "\n";
        cout << "  Account Type : " << acc->getAccountType() << "\n\n";

        cout << "   1. View Account Balance\n";
        cout << "   2. Deposit Funds\n";
        cout << "   3. Withdraw Funds\n";
        cout << "   4. Transfer Funds\n";
        cout << "   5. View Account Statement\n";
        cout << "   6. Change PIN\n";
        cout << "   0. Logout\n";

        int choice = Utils::readInt("\n  Enter your choice: ");

        switch (choice) {
            case 1: viewBalance();    break;
            case 2: selfDeposit();    break;
            case 3: selfWithdraw();   break;
            case 4: selfTransfer();   break;
            case 5: viewStatement();  break;
            case 6: changePIN();      break;
            case 0:
                currentCustomer = 0;
                currentAccountNumber = "";
                cout << "\n  [+] Logged out successfully.\n";
                Utils::pause();
                return;
            default:
                cout << "  [!] Invalid option.\n";
                Utils::pause();
        }
    }
}

void BankSystem::viewBalance() {
    Account* acc = findAccountAnywhere(currentAccountNumber);
    if (!acc) return;

    cout << "\n  ------------------------------------------------\n";
    cout << "  Account Number : " << acc->getAccountNumber() << "\n";
    cout << "  Account Type   : " << acc->getAccountType() << "\n";
    cout << "  Available Bal. : " << Utils::formatMoney(acc->getBalance()) << "\n";
    cout << "  Status         : " << acc->getStatus() << "\n";
    cout << "  ------------------------------------------------\n";
    Utils::pause();
}

void BankSystem::selfDeposit() {
    Account* acc = findAccountAnywhere(currentAccountNumber);
    if (!acc) return;

    try {
        double amt = Utils::readDouble("  Deposit Amount: ");
        doDeposit(acc, amt, "SELF", currentAccountNumber.substr(4, 5));
    }
    catch (const BankingException& ex) {
        cout << "\n  [!] DEPOSIT FAILED: " << ex.what() << "\n";
    }
    Utils::pause();
}

void BankSystem::selfWithdraw() {
    Account* acc = findAccountAnywhere(currentAccountNumber);
    if (!acc) return;

    try {
        double amt = Utils::readDouble("  Withdrawal Amount: ");
        doWithdrawal(acc, amt, "SELF", currentAccountNumber.substr(4, 5));
    }
    catch (const BankingException& ex) {
        cout << "\n  [!] WITHDRAWAL FAILED: " << ex.what() << "\n";
    }
    Utils::pause();
}

void BankSystem::selfTransfer() {
    Account* acc = findAccountAnywhere(currentAccountNumber);
    if (!acc) return;

    try {
        string target = Utils::toUpper(Utils::readLine("  Target Account Number: "));
        Account* to = findAccountAnywhere(target);
        if (!to) throw BankingException("Target account not found.");
        if (to == acc) throw BankingException("You cannot transfer to the same account.");

        double amt = Utils::readDouble("  Transfer Amount: ");
        doTransfer(acc, to, amt, "SELF", currentAccountNumber.substr(4, 5));
    }
    catch (const BankingException& ex) {
        cout << "\n  [!] TRANSFER FAILED: " << ex.what() << "\n";
    }
    Utils::pause();
}

void BankSystem::viewStatement() {
    Account* acc = findAccountAnywhere(currentAccountNumber);
    if (!acc) return;

    cout << "\n  ============================================================\n";
    cout << "  ACCOUNT STATEMENT - " << acc->getAccountNumber() << "\n";
    cout << "  Account Holder : " << currentCustomer->getFullName() << "\n";
    cout << "  Current Balance: " << Utils::formatMoney(acc->getBalance()) << "\n";
    cout << "  ============================================================\n";
    printStatement(acc->getAccountNumber());
    Utils::pause();
}

void BankSystem::changePIN() {
    Utils::clearScreen();
    printHeader("CHANGE PIN");

    try {
        string oldPin = Utils::readLine("  Enter current PIN      : ");
        if (!Utils::verifyText(oldPin, currentCustomer->getEncryptedPin()))
            throw BankingException("Current PIN is incorrect.");

        string newPin = Utils::readLine("  Enter new 5-digit PIN  : ");

        /* validation */
        if (newPin.size() != 5 || !Utils::isAllDigits(newPin))
            throw BankingException("PIN must be exactly 5 digits.");

        bool allSame = true;
        for (size_t i = 1; i < newPin.size(); ++i)
            if (newPin[i] != newPin[0]) { allSame = false; break; }
        if (allSame)
            throw BankingException("PIN cannot consist of the same digit repeated.");

        /* sequential check e.g. 12345 or 54321 */
        bool seqUp = true, seqDown = true;
        for (size_t i = 1; i < newPin.size(); ++i) {
            if (newPin[i] != newPin[i-1] + 1) seqUp = false;
            if (newPin[i] != newPin[i-1] - 1) seqDown = false;
        }
        if (seqUp || seqDown)
            throw BankingException("PIN cannot be a simple sequence (e.g. 12345).");

        if (pinExists(newPin))
            throw BankingException("That PIN is already in use. Please choose another.");

        string confirm = Utils::readLine("  Confirm new PIN        : ");
        if (confirm != newPin)
            throw BankingException("PIN confirmation does not match.");

        currentCustomer->setEncryptedPin(Utils::encryptText(newPin));
        currentCustomer->setFailedAttempts(0);
        saveCustomers();

        cout << "\n  [+] PIN changed successfully.\n";
    }
    catch (const BankingException& ex) {
        cout << "\n  [!] PIN CHANGE FAILED: " << ex.what() << "\n";
    }

    Utils::pause();
}

/* ================================================================
   STATEMENT PRINTER
   ================================================================ */
void BankSystem::printStatement(const string& accNo) {
    cout << "\n  --------------------------------------------------------------------------\n";
    cout << "  " << left << setw(21) << "Date/Time"
         << setw(15) << "Type"
         << right << setw(14) << "Amount"
         << setw(16) << "Balance" << "\n";
    cout << "  --------------------------------------------------------------------------\n";

    bool found = false;
    for (size_t i = 0; i < transactions.size(); ++i) {
        if (transactions[i].accountNumber == accNo) {
            found = true;
            cout << "  " << left << setw(21) << transactions[i].dateTime
                 << setw(15) << transactions[i].type
                 << right << setw(14) << fixed << setprecision(2) << transactions[i].amount
                 << setw(16) << transactions[i].balanceAfter << "\n";
        }
    }

    if (!found) cout << "  No transactions found for this account.\n";
    cout << "  --------------------------------------------------------------------------\n";
}

/* ================================================================
   1.5.2  SEARCH AND FILTERING
   ================================================================ */
void BankSystem::searchCustomersMenu() {
    while (true) {
        Utils::clearScreen();
        printHeader("CUSTOMER SEARCH AND FILTERING");
        cout << "   1. Search by Account Number\n";
        cout << "   2. Search by SA ID Number\n";
        cout << "   3. Search by Surname / Name (partial match)\n";
        cout << "   4. Filter by Account Type\n";
        cout << "   5. Filter by Minimum Balance\n";
        cout << "   0. Back\n";

        int choice = Utils::readInt("\n  Enter your choice: ");
        if (choice == 0) return;

        int results = 0;

        switch (choice) {
            case 1: {
                string q = Utils::toUpper(Utils::readLine("  Account Number: "));
                Account* a = 0;
                Customer* c = findCustomerByAccount(q, &a);
                if (c && a) {
                    cout << "\n  Match found:\n";
                    c->display();
                    cout << "\n  Account details:\n";
                    a->display();
                    results = 1;
                }
                break;
            }
            case 2: {
                string q = Utils::readLine("  SA ID Number: ");
                Customer* c = findCustomerByIdNumber(q);
                if (c) {
                    cout << "\n  Match found:\n";
                    c->display();
                    results = 1;
                }
                break;
            }
            case 3: {
                string q = Utils::toUpper(Utils::readLine("  Enter name or surname: "));
                cout << "\n  Results:\n";
                for (size_t i = 0; i < customers.size(); ++i) {
                    string full = Utils::toUpper(customers[i]->getFullName());
                    if (full.find(q) != string::npos) {
                        cout << "  ------------------------------------------\n";
                        customers[i]->display();
                        results++;
                    }
                }
                break;
            }
            case 4: {
                cout << "\n  Account Types: 1=Savings  2=Cheque  3=Fixed Deposit  4=Student\n";
                string t = Utils::readLine("  Enter type name or number: ");
                string typeName;
                if (t == "1") typeName = "Savings";
                else if (t == "2") typeName = "Cheque";
                else if (t == "3") typeName = "Fixed Deposit";
                else if (t == "4") typeName = "Student";
                else typeName = t;

                cout << "\n  Results:\n";
                for (size_t i = 0; i < customers.size(); ++i) {
                    vector<Account*>& as = customers[i]->getAccounts();
                    for (size_t j = 0; j < as.size(); ++j) {
                        if (as[j]->getAccountType() == typeName) {
                            cout << "  ------------------------------------------\n";
                            cout << "  " << customers[i]->getFullName()
                                 << " | " << as[j]->getAccountNumber()
                                 << " | " << Utils::formatMoney(as[j]->getBalance()) << "\n";
                            results++;
                        }
                    }
                }
                break;
            }
            case 5: {
                double minBal = Utils::readDouble("  Minimum Balance (R): ");
                cout << "\n  Results:\n";
                for (size_t i = 0; i < customers.size(); ++i) {
                    vector<Account*>& as = customers[i]->getAccounts();
                    for (size_t j = 0; j < as.size(); ++j) {
                        if (as[j]->getBalance() >= minBal) {
                            cout << "  ------------------------------------------\n";
                            cout << "  " << customers[i]->getFullName()
                                 << " | " << as[j]->getAccountNumber()
                                 << " | " << as[j]->getAccountType()
                                 << " | " << Utils::formatMoney(as[j]->getBalance()) << "\n";
                            results++;
                        }
                    }
                }
                break;
            }
            default:
                cout << "  [!] Invalid option.\n";
        }

        cout << "\n  Total matches found: " << results << "\n";
        Utils::pause();
    }
}

/* ================================================================
   1.5.1  INTEREST CALCULATION AND APPLICATION
   ================================================================ */
void BankSystem::applyInterest() {
    Utils::clearScreen();
    printHeader("APPLY INTEREST TO ELIGIBLE ACCOUNTS");

    cout << "  This will apply the annual interest rate to every active account.\n";
    string confirm = Utils::readLine("  Type YES to continue: ");
    if (Utils::toUpper(confirm) != "YES") {
        cout << "\n  Operation cancelled.\n";
        Utils::pause();
        return;
    }

    int applied = 0;
    double totalInterest = 0.0;

    cout << "\n  ------------------------------------------------------------------\n";
    cout << "  " << left << setw(22) << "Account"
         << setw(18) << "Type"
         << right << setw(14) << "Balance"
         << setw(14) << "Interest" << "\n";
    cout << "  ------------------------------------------------------------------\n";

    for (size_t i = 0; i < customers.size(); ++i) {
        vector<Account*>& as = customers[i]->getAccounts();
        for (size_t j = 0; j < as.size(); ++j) {
            Account* a = as[j];
            if (a->getStatus() != "Active") continue;

            double interest = a->calculateInterest();
            if (interest <= 0.0) continue;

            a->addDeposit(interest);
            recordTransaction(a->getAccountNumber(), "Interest", interest,
                              a->getBalance(), currentTeller->getTellerId(),
                              currentTeller->getBranchCode(),
                              "Annual interest applied at "
                              + to_string(a->annualInterestRate()) + "% p.a.");

            cout << "  " << left << setw(22) << a->getAccountNumber()
                 << setw(18) << a->getAccountType()
                 << right << setw(14) << fixed << setprecision(2) << (a->getBalance() - interest)
                 << setw(14) << interest << "\n";

            applied++;
            totalInterest += interest;
        }
    }

    saveCustomers();
    cout << "  ------------------------------------------------------------------\n";
    cout << "  Accounts credited : " << applied << "\n";
    cout << "  Total interest    : " << Utils::formatMoney(totalInterest) << "\n";
    Utils::pause();
}

/* ================================================================
   1.4  BRANCH MANAGEMENT
   ================================================================ */
void BankSystem::branchMenu() {
    while (true) {
        Utils::clearScreen();
        printHeader("BRANCH MANAGEMENT");
        cout << "   1. View All Branches\n";
        cout << "   2. View Branch Details\n";
        cout << "   3. Inter-Branch Comparison\n";
        cout << "   0. Back\n";

        int choice = Utils::readInt("\n  Enter your choice: ");

        if (choice == 1) {
            cout << "\n  ------------------------------------------------\n";
            for (size_t i = 0; i < branches.size(); ++i) {
                cout << "  " << branches[i].branchCode << "  |  "
                     << branches[i].branchName << "  |  "
                     << branches[i].contactNumber << "\n";
            }
            cout << "  ------------------------------------------------\n";
            Utils::pause();
        }
        else if (choice == 2) {
            string code = Utils::toUpper(Utils::readLine("  Enter Branch Code (e.g. BR001): "));
            Branch* b = findBranch(code);
            if (!b) {
                cout << "\n  [!] Branch not found.\n";
            } else {
                cout << "\n  ------------------------------------------------\n";
                b->display();

                /* statistics */
                int accCount = 0; double totalBal = 0.0;
                for (size_t i = 0; i < customers.size(); ++i) {
                    vector<Account*>& as = customers[i]->getAccounts();
                    for (size_t j = 0; j < as.size(); ++j) {
                        if (as[j]->getAccountNumber().substr(4, 5) == code) {
                            accCount++;
                            totalBal += as[j]->getBalance();
                        }
                    }
                }
                cout << "  Accounts Held  : " << accCount << "\n";
                cout << "  Total Deposits : " << Utils::formatMoney(totalBal) << "\n";
                cout << "  ------------------------------------------------\n";
            }
            Utils::pause();
        }
        else if (choice == 3) {
            cout << "\n  ------------------------------------------------------------------\n";
            cout << "  " << left << setw(10) << "Code"
                 << setw(34) << "Branch Name"
                 << right << setw(12) << "Accounts"
                 << setw(18) << "Total Balance" << "\n";
            cout << "  ------------------------------------------------------------------\n";

            for (size_t i = 0; i < branches.size(); ++i) {
                string code = branches[i].branchCode;
                int accCount = 0; double totalBal = 0.0;

                for (size_t k = 0; k < customers.size(); ++k) {
                    vector<Account*>& as = customers[k]->getAccounts();
                    for (size_t j = 0; j < as.size(); ++j) {
                        if (as[j]->getAccountNumber().substr(4, 5) == code) {
                            accCount++;
                            totalBal += as[j]->getBalance();
                        }
                    }
                }

                cout << "  " << left << setw(10) << code
                     << setw(34) << branches[i].branchName
                     << right << setw(12) << accCount
                     << setw(18) << fixed << setprecision(2) << totalBal << "\n";
            }
            cout << "  ------------------------------------------------------------------\n";
            Utils::pause();
        }
        else if (choice == 0) {
            return;
        }
        else {
            cout << "  [!] Invalid option.\n";
            Utils::pause();
        }
    }
}

/* ================================================================
   1.6  REPORTING AND ANALYTICS
   ================================================================ */
void BankSystem::tellerReports() {
    while (true) {
        Utils::clearScreen();
        printHeader("REPORTS AND ANALYTICS - " + currentTeller->getBranchCode());
        cout << "   1. Daily Transaction Report\n";
        cout << "   2. Customer Account Summary\n";
        cout << "   3. Branch Performance Report\n";
        cout << "   0. Back\n";

        int choice = Utils::readInt("\n  Enter your choice: ");

        switch (choice) {
            case 1: dailyTransactionReport();    break;
            case 2: customerAccountSummary();    break;
            case 3: branchPerformanceReport();   break;
            case 0: return;
            default:
                cout << "  [!] Invalid option.\n";
                Utils::pause();
        }
    }
}

void BankSystem::dailyTransactionReport() {
    Utils::clearScreen();
    printHeader("DAILY TRANSACTION REPORT");

    string today = Utils::currentDate();
    cout << "  Date: " << today << "\n\n";

    int count = 0;
    double totalDeposits = 0.0, totalWithdrawals = 0.0, totalTransfers = 0.0;

    cout << "  --------------------------------------------------------------------------\n";
    cout << "  " << left << setw(14) << "Txn ID"
         << setw(22) << "Account"
         << setw(15) << "Type"
         << right << setw(14) << "Amount"
         << "  " << left << setw(8) << "By" << "\n";
    cout << "  --------------------------------------------------------------------------\n";

    for (size_t i = 0; i < transactions.size(); ++i) {
        if (transactions[i].dateTime.substr(0, 10) != today) continue;
        if (transactions[i].branchCode != currentTeller->getBranchCode()) continue;

        const Transaction& t = transactions[i];
        cout << "  " << left << setw(14) << t.transactionId
             << setw(22) << t.accountNumber
             << setw(15) << t.type
             << right << setw(14) << fixed << setprecision(2) << t.amount
             << "  " << left << setw(8) << t.performedBy << "\n";

        if (t.type == "Deposit")       totalDeposits    += t.amount;
        if (t.type == "Withdrawal")    totalWithdrawals += t.amount;
        if (t.type == "Transfer-Out")  totalTransfers   += t.amount;

        count++;
    }

    cout << "  --------------------------------------------------------------------------\n";
    cout << "  Transactions today : " << count << "\n";
    cout << "  Total Deposits     : " << Utils::formatMoney(totalDeposits) << "\n";
    cout << "  Total Withdrawals  : " << Utils::formatMoney(totalWithdrawals) << "\n";
    cout << "  Total Transfers Out: " << Utils::formatMoney(totalTransfers) << "\n";
    cout << "  Net Cash Flow      : "
         << Utils::formatMoney(totalDeposits - totalWithdrawals) << "\n";
    Utils::pause();
}

void BankSystem::customerAccountSummary() {
    Utils::clearScreen();
    printHeader("CUSTOMER ACCOUNT SUMMARY - " + currentTeller->getBranchCode());

    int custCount = 0, accCount = 0;
    double grandTotal = 0.0;

    cout << "  --------------------------------------------------------------------------\n";
    cout << "  " << left << setw(24) << "Customer"
         << setw(22) << "Account Number"
         << setw(15) << "Type"
         << right << setw(16) << "Balance" << "\n";
    cout << "  --------------------------------------------------------------------------\n";

    for (size_t i = 0; i < customers.size(); ++i) {
        vector<Account*>& as = customers[i]->getAccounts();
        for (size_t j = 0; j < as.size(); ++j) {
            if (as[j]->getAccountNumber().substr(4, 5) != currentTeller->getBranchCode())
                continue;

            cout << "  " << left << setw(24) << customers[i]->getFullName()
                 << setw(22) << as[j]->getAccountNumber()
                 << setw(15) << as[j]->getAccountType()
                 << right << setw(16) << fixed << setprecision(2) << as[j]->getBalance()
                 << "\n";

            grandTotal += as[j]->getBalance();
            accCount++;
        }
        custCount++;
    }

    cout << "  --------------------------------------------------------------------------\n";
    cout << "  Customers : " << custCount << "   Accounts : " << accCount << "\n";
    cout << "  Total Holdings: " << Utils::formatMoney(grandTotal) << "\n";
    Utils::pause();
}

void BankSystem::branchPerformanceReport() {
    Utils::clearScreen();
    printHeader("BRANCH PERFORMANCE REPORT");

    string today = Utils::currentDate();

    cout << "  --------------------------------------------------------------------------\n";
    cout << "  " << left << setw(10) << "Branch"
         << setw(30) << "Name"
         << right << setw(10) << "Accts"
         << setw(16) << "Holdings"
         << setw(10) << "Txns Today" << "\n";
    cout << "  --------------------------------------------------------------------------\n";

    for (size_t b = 0; b < branches.size(); ++b) {
        string code = branches[b].branchCode;

        int accCount = 0, txnCount = 0;
        double totalBal = 0.0;

        for (size_t i = 0; i < customers.size(); ++i) {
            vector<Account*>& as = customers[i]->getAccounts();
            for (size_t j = 0; j < as.size(); ++j) {
                if (as[j]->getAccountNumber().substr(4, 5) == code) {
                    accCount++;
                    totalBal += as[j]->getBalance();
                }
            }
        }

        for (size_t i = 0; i < transactions.size(); ++i) {
            if (transactions[i].branchCode == code &&
                transactions[i].dateTime.substr(0, 10) == today)
                txnCount++;
        }

        cout << "  " << left << setw(10) << code
             << setw(30) << branches[b].branchName
             << right << setw(10) << accCount
             << setw(16) << fixed << setprecision(2) << totalBal
             << setw(10) << txnCount << "\n";
    }

    cout << "  --------------------------------------------------------------------------\n";
    Utils::pause();
}

/* ================================================================
   1.7  SYSTEM MAINTENANCE - BACKUP / EXPORT / UNLOCK
   ================================================================ */
static bool copyFileBinary(const string& src, const string& dst) {
    ifstream in(src.c_str(), ios::binary);
    if (!in) return false;
    ofstream out(dst.c_str(), ios::binary | ios::trunc);
    if (!out) { in.close(); return false; }
    out << in.rdbuf();
    in.close();
    out.close();
    return true;
}

void BankSystem::backupData() {
    Utils::clearScreen();
    printHeader("BACKUP AND RECOVERY");

    saveAll();

    string stamp = Utils::currentDate();
    /* replace slashes so the name is filesystem friendly */
    for (size_t i = 0; i < stamp.size(); ++i) if (stamp[i] == '/') stamp[i] = '-';

    int ok = 0;
    if (copyFileBinary(fileCustomers,    "backup_customers_"    + stamp + ".dat")) ok++;
    if (copyFileBinary(fileTransactions, "backup_transactions_" + stamp + ".dat")) ok++;
    if (copyFileBinary(fileBranches,     "backup_branches_"     + stamp + ".dat")) ok++;
    if (copyFileBinary(fileTellers,      "backup_tellers_"      + stamp + ".dat")) ok++;
    if (copyFileBinary(fileConfig,       "backup_config_"       + stamp + ".txt")) ok++;

    cout << "\n  [+] Backup complete. " << ok << " file(s) copied with timestamp "
         << stamp << ".\n";
    Utils::pause();
}

void BankSystem::exportCustomersCSV() {
    ofstream out("customers_export.csv");
    if (!out) {
        cout << "\n  [!] Could not create customers_export.csv\n";
        Utils::pause();
        return;
    }

    out << "CustomerID,FullName,SA_ID,Contact,Email,Address,DOB,AccountNumber,"
        << "AccountType,Balance,Status,Branch\n";

    for (size_t i = 0; i < customers.size(); ++i) {
        Customer* c = customers[i];
        vector<Account*>& as = c->getAccounts();
        for (size_t j = 0; j < as.size(); ++j) {
            out << c->getCustomerId() << ","
                << c->getFullName() << ","
                << c->getIdNumber() << ","
                << c->getContactNumber() << ","
                << c->getEmail() << ","
                << c->getAddress() << ","
                << c->getDateOfBirth() << ","
                << as[j]->getAccountNumber() << ","
                << as[j]->getAccountType() << ","
                << fixed << setprecision(2) << as[j]->getBalance() << ","
                << as[j]->getStatus() << ","
                << as[j]->getAccountNumber().substr(4, 5) << "\n";
        }
    }

    out.close();
    cout << "\n  [+] Exported to customers_export.csv\n";
}

void BankSystem::exportTransactionsCSV() {
    ofstream out("transactions_export.csv");
    if (!out) {
        cout << "\n  [!] Could not create transactions_export.csv\n";
        Utils::pause();
        return;
    }

    out << "TransactionID,AccountNumber,Type,Amount,BalanceAfter,DateTime,"
        << "PerformedBy,Branch,Description\n";

    for (size_t i = 0; i < transactions.size(); ++i) {
        const Transaction& t = transactions[i];
        out << t.transactionId << ","
            << t.accountNumber << ","
            << t.type << ","
            << fixed << setprecision(2) << t.amount << ","
            << t.balanceAfter << ","
            << t.dateTime << ","
            << t.performedBy << ","
            << t.branchCode << ","
            << t.description << "\n";
    }

    out.close();
    cout << "\n  [+] Exported to transactions_export.csv\n";
}

void BankSystem::exportSummaryTXT() {
    ofstream out("bank_summary_report.txt");
    if (!out) {
        cout << "\n  [!] Could not create bank_summary_report.txt\n";
        Utils::pause();
        return;
    }

    out << "===============================================================\n";
    out << " " << bankName << " - MULTI-BRANCH BANKING MANAGEMENT SYSTEM\n";
    out << " SUMMARY REPORT\n";
    out << " Generated: " << Utils::currentDateTime() << "\n";
    out << "===============================================================\n\n";

    out << "BRANCHES\n";
    out << "---------------------------------------------------------------\n";
    for (size_t i = 0; i < branches.size(); ++i) {
        out << "  " << branches[i].branchCode << " - " << branches[i].branchName
            << " (" << branches[i].contactNumber << ")\n";
    }

    out << "\nCUSTOMERS AND ACCOUNTS\n";
    out << "---------------------------------------------------------------\n";
    double grandTotal = 0.0;
    int totalAccounts = 0;
    for (size_t i = 0; i < customers.size(); ++i) {
        out << "  " << customers[i]->getCustomerId() << "  "
            << customers[i]->getFullName() << "\n";
        vector<Account*>& as = customers[i]->getAccounts();
        for (size_t j = 0; j < as.size(); ++j) {
            out << "      " << as[j]->getAccountNumber()
                << "  " << as[j]->getAccountType()
                << "  " << Utils::formatMoney(as[j]->getBalance()) << "\n";
            grandTotal += as[j]->getBalance();
            totalAccounts++;
        }
    }

    out << "\n---------------------------------------------------------------\n";
    out << "  Total Customers : " << customers.size() << "\n";
    out << "  Total Accounts  : " << totalAccounts << "\n";
    out << "  Total Holdings  : " << Utils::formatMoney(grandTotal) << "\n";
    out << "  Total Transact. : " << transactions.size() << "\n";
    out << "===============================================================\n";

    out.close();
    cout << "\n  [+] Exported to bank_summary_report.txt\n";
}

void BankSystem::systemMaintenance() {
    while (true) {
        Utils::clearScreen();
        printHeader("SYSTEM MAINTENANCE");
        cout << "   1. Backup All Data Files\n";
        cout << "   2. Export Customers to CSV\n";
        cout << "   3. Export Transactions to CSV\n";
        cout << "   4. Export Full Summary to TXT\n";
        cout << "   5. Unlock a Customer Account\n";
        cout << "   0. Back\n";

        int choice = Utils::readInt("\n  Enter your choice: ");

        switch (choice) {
            case 1:
                backupData();
                break;
            case 2:
                exportCustomersCSV();
                Utils::pause();
                break;
            case 3:
                exportTransactionsCSV();
                Utils::pause();
                break;
            case 4:
                exportSummaryTXT();
                Utils::pause();
                break;
            case 5: {
                string accNo = Utils::toUpper(Utils::readLine("  Customer Account Number: "));
                Account* a = 0;
                Customer* c = findCustomerByAccount(accNo, &a);
                if (!c) {
                    cout << "\n  [!] Account not found.\n";
                } else {
                    c->setLocked(false);
                    c->setFailedAttempts(0);
                    saveCustomers();
                    cout << "\n  [+] Account " << accNo << " (" << c->getFullName()
                         << ") has been UNLOCKED.\n";
                }
                Utils::pause();
                break;
            }
            case 0:
                return;
            default:
                cout << "  [!] Invalid option.\n";
                Utils::pause();
        }
    }
}
