#include "Models.h"
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;

/* ================================================================
   BRANCH
   ================================================================ */
Branch::Branch() : branchCode(""), branchName(""), address(""),
                   contactNumber(""), managerName("") {}

Branch::Branch(const string& code, const string& name, const string& addr,
               const string& contact, const string& mgr)
    : branchCode(code), branchName(name), address(addr),
      contactNumber(contact), managerName(mgr) {}

void Branch::display() const {
    cout << "  Branch Code    : " << branchCode << "\n";
    cout << "  Branch Name    : " << branchName << "\n";
    cout << "  Address        : " << address << "\n";
    cout << "  Contact Number : " << contactNumber << "\n";
    cout << "  Manager        : " << managerName << "\n";
}

void Branch::writeToFile(ofstream& out) const {
    Utils::writeStr(out, branchCode);
    Utils::writeStr(out, branchName);
    Utils::writeStr(out, address);
    Utils::writeStr(out, contactNumber);
    Utils::writeStr(out, managerName);
}

void Branch::readFromFile(ifstream& in) {
    Utils::readStr(in, branchCode);
    Utils::readStr(in, branchName);
    Utils::readStr(in, address);
    Utils::readStr(in, contactNumber);
    Utils::readStr(in, managerName);
}

/* ================================================================
   ACCOUNT (base)
   ================================================================ */
Account::Account()
    : accountNumber(""), accountType(""), balance(0.0), dateOpened(""),
      status("Active"), totalDeposits(0.0), totalWithdrawals(0.0) {}

Account::Account(const string& accNo, const string& type,
                 double initialDeposit, const string& opened)
    : accountNumber(accNo), accountType(type), balance(initialDeposit),
      dateOpened(opened), status("Active"),
      totalDeposits(initialDeposit), totalWithdrawals(0.0) {}

Account::~Account() {}

bool Account::canWithdraw(double amount, string& reason) const {
    if (amount <= 0) { reason = "Amount must be greater than zero."; return false; }

    double lim = withdrawalLimit();
    if (lim > 0 && amount > lim) {
        stringstream ss;
        ss << "Withdrawal exceeds the per-transaction limit of " << Utils::formatMoney(lim) << ".";
        reason = ss.str();
        return false;
    }
    if (amount > balance) { reason = "Insufficient funds available."; return false; }
    return true;
}

double Account::calculateInterest() const {
    /* simple annual interest = balance * rate / 100 */
    return balance * annualInterestRate() / 100.0;
}

void Account::display() const {
    cout << "  Account Number    : " << accountNumber << "\n";
    cout << "  Account Type      : " << accountType << "\n";
    cout << "  Balance           : " << Utils::formatMoney(balance) << "\n";
    cout << "  Date Opened       : " << dateOpened << "\n";
    cout << "  Status            : " << status << "\n";
    cout << "  Interest Rate     : " << fixed << setprecision(2)
         << annualInterestRate() << " % p.a.\n";
    cout << "  Minimum Deposit   : " << Utils::formatMoney(minimumDeposit()) << "\n";
    string extra = extraInfo();
    if (!extra.empty()) cout << extra;
    cout << "  Total Deposits    : " << Utils::formatMoney(totalDeposits) << "\n";
    cout << "  Total Withdrawals : " << Utils::formatMoney(totalWithdrawals) << "\n";
}

void Account::writeToFile(ofstream& out) const {
    int tag = typeTag();
    out.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
    Utils::writeStr(out, accountNumber);
    Utils::writeStr(out, accountType);
    out.write(reinterpret_cast<const char*>(&balance), sizeof(balance));
    Utils::writeStr(out, dateOpened);
    Utils::writeStr(out, status);
    out.write(reinterpret_cast<const char*>(&totalDeposits), sizeof(totalDeposits));
    out.write(reinterpret_cast<const char*>(&totalWithdrawals), sizeof(totalWithdrawals));
}

void Account::readFromFile(ifstream& in) {
    Utils::readStr(in, accountNumber);
    Utils::readStr(in, accountType);
    in.read(reinterpret_cast<char*>(&balance), sizeof(balance));
    Utils::readStr(in, dateOpened);
    Utils::readStr(in, status);
    in.read(reinterpret_cast<char*>(&totalDeposits), sizeof(totalDeposits));
    in.read(reinterpret_cast<char*>(&totalWithdrawals), sizeof(totalWithdrawals));
}

/* ================================================================
   SAVINGS ACCOUNT
   ================================================================ */
SavingsAccount::SavingsAccount()
    : Account(), interestRate(5.50), minimumBalance(100.0) {}

SavingsAccount::SavingsAccount(const string& accNo, double initialDeposit, const string& opened)
    : Account(accNo, "Savings", initialDeposit, opened),
      interestRate(5.50), minimumBalance(100.0) {}

string SavingsAccount::extraInfo() const {
    stringstream ss;
    ss << "  Minimum Balance   : " << Utils::formatMoney(minimumBalance) << "\n";
    ss << "  Daily Withdraw Lim: " << Utils::formatMoney(withdrawalLimit()) << "\n";
    return ss.str();
}

void SavingsAccount::display() const {
    cout << "  >>> SAVINGS ACCOUNT <<<\n";
    Account::display();
}

void SavingsAccount::writeToFile(ofstream& out) const {
    Account::writeToFile(out);
    out.write(reinterpret_cast<const char*>(&interestRate), sizeof(interestRate));
    out.write(reinterpret_cast<const char*>(&minimumBalance), sizeof(minimumBalance));
}

void SavingsAccount::readFromFile(ifstream& in) {
    Account::readFromFile(in);
    in.read(reinterpret_cast<char*>(&interestRate), sizeof(interestRate));
    in.read(reinterpret_cast<char*>(&minimumBalance), sizeof(minimumBalance));
}

/* ================================================================
   CHEQUE ACCOUNT
   ================================================================ */
ChequeAccount::ChequeAccount()
    : Account(), interestRate(1.50), overdraftLimit(2000.0) {}

ChequeAccount::ChequeAccount(const string& accNo, double initialDeposit, const string& opened)
    : Account(accNo, "Cheque", initialDeposit, opened),
      interestRate(1.50), overdraftLimit(2000.0) {}

bool ChequeAccount::canWithdraw(double amount, string& reason) const {
    if (amount <= 0) { reason = "Amount must be greater than zero."; return false; }

    double lim = withdrawalLimit();
    if (lim > 0 && amount > lim) {
        stringstream ss;
        ss << "Withdrawal exceeds the per-transaction limit of " << Utils::formatMoney(lim) << ".";
        reason = ss.str();
        return false;
    }
    if (amount > balance + overdraftLimit) {
        reason = "Exceeds available balance plus the approved overdraft limit.";
        return false;
    }
    return true;
}

string ChequeAccount::extraInfo() const {
    stringstream ss;
    ss << "  Overdraft Limit   : " << Utils::formatMoney(overdraftLimit) << "\n";
    ss << "  Transaction Limit : " << Utils::formatMoney(withdrawalLimit()) << "\n";
    return ss.str();
}

void ChequeAccount::display() const {
    cout << "  >>> CHEQUE ACCOUNT <<<\n";
    Account::display();
}

void ChequeAccount::writeToFile(ofstream& out) const {
    Account::writeToFile(out);
    out.write(reinterpret_cast<const char*>(&interestRate), sizeof(interestRate));
    out.write(reinterpret_cast<const char*>(&overdraftLimit), sizeof(overdraftLimit));
}

void ChequeAccount::readFromFile(ifstream& in) {
    Account::readFromFile(in);
    in.read(reinterpret_cast<char*>(&interestRate), sizeof(interestRate));
    in.read(reinterpret_cast<char*>(&overdraftLimit), sizeof(overdraftLimit));
}

/* ================================================================
   FIXED DEPOSIT ACCOUNT
   ================================================================ */
FixedDepositAccount::FixedDepositAccount()
    : Account(), interestRate(8.50), termMonths(12), maturityDate("") {}

FixedDepositAccount::FixedDepositAccount(const string& accNo, double initialDeposit, const string& opened)
    : Account(accNo, "Fixed Deposit", initialDeposit, opened),
      interestRate(8.50), termMonths(12)
{
    /* maturity = opened date + 12 months (simple string arithmetic for prototype) */
    int d = atoi(opened.substr(0, 2).c_str());
    int m = atoi(opened.substr(3, 2).c_str());
    int y = atoi(opened.substr(6, 4).c_str());
    y += 1;
    stringstream ss;
    ss << setw(2) << setfill('0') << d << "/"
       << setw(2) << setfill('0') << m << "/" << y;
    maturityDate = ss.str();
}

bool FixedDepositAccount::canWithdraw(double amount, string& reason) const {
    stringstream ss;
    ss << "Fixed Deposit funds are locked until maturity on " << maturityDate
       << ". No withdrawals are permitted before this date.";
    reason = ss.str();
    return false;
}

string FixedDepositAccount::extraInfo() const {
    stringstream ss;
    ss << "  Term              : " << termMonths << " months\n";
    ss << "  Maturity Date     : " << maturityDate << "\n";
    return ss.str();
}

void FixedDepositAccount::display() const {
    cout << "  >>> FIXED DEPOSIT ACCOUNT <<<\n";
    Account::display();
}

void FixedDepositAccount::writeToFile(ofstream& out) const {
    Account::writeToFile(out);
    out.write(reinterpret_cast<const char*>(&interestRate), sizeof(interestRate));
    out.write(reinterpret_cast<const char*>(&termMonths), sizeof(termMonths));
    Utils::writeStr(out, maturityDate);
}

void FixedDepositAccount::readFromFile(ifstream& in) {
    Account::readFromFile(in);
    in.read(reinterpret_cast<char*>(&interestRate), sizeof(interestRate));
    in.read(reinterpret_cast<char*>(&termMonths), sizeof(termMonths));
    Utils::readStr(in, maturityDate);
}

/* ================================================================
   STUDENT ACCOUNT
   ================================================================ */
StudentAccount::StudentAccount()
    : Account(), interestRate(3.00), monthlyFee(0.0) {}

StudentAccount::StudentAccount(const string& accNo, double initialDeposit, const string& opened)
    : Account(accNo, "Student", initialDeposit, opened),
      interestRate(3.00), monthlyFee(0.0) {}

string StudentAccount::extraInfo() const {
    stringstream ss;
    ss << "  Monthly Fee       : " << Utils::formatMoney(monthlyFee) << " (free)\n";
    ss << "  Transaction Limit : " << Utils::formatMoney(withdrawalLimit()) << "\n";
    return ss.str();
}

void StudentAccount::display() const {
    cout << "  >>> STUDENT ACCOUNT <<<\n";
    Account::display();
}

void StudentAccount::writeToFile(ofstream& out) const {
    Account::writeToFile(out);
    out.write(reinterpret_cast<const char*>(&interestRate), sizeof(interestRate));
    out.write(reinterpret_cast<const char*>(&monthlyFee), sizeof(monthlyFee));
}

void StudentAccount::readFromFile(ifstream& in) {
    Account::readFromFile(in);
    in.read(reinterpret_cast<char*>(&interestRate), sizeof(interestRate));
    in.read(reinterpret_cast<char*>(&monthlyFee), sizeof(monthlyFee));
}

/* ================================================================
   ACCOUNT FACTORY
   ================================================================ */
Account* createAccountByTag(int tag) {
    switch (tag) {
        case 1: return new SavingsAccount();
        case 2: return new ChequeAccount();
        case 3: return new FixedDepositAccount();
        case 4: return new StudentAccount();
        default: return 0;
    }
}

Account* readAccountFromFile(ifstream& in) {
    int tag = 0;
    in.read(reinterpret_cast<char*>(&tag), sizeof(tag));
    if (!in) return 0;
    Account* acc = createAccountByTag(tag);
    if (!acc) return 0;
    acc->readFromFile(in);
    return acc;
}

/* ================================================================
   CUSTOMER
   ================================================================ */
Customer::Customer()
    : customerId(""), firstName(""), surname(""), idNumber(""),
      contactNumber(""), email(""), address(""), dateOfBirth(""),
      encryptedPin(""), failedAttempts(0), locked(false) {}

Customer::Customer(const string& custId, const string& fname, const string& sname,
                   const string& idNo, const string& contact, const string& mail,
                   const string& addr, const string& dob, const string& pin)
    : customerId(custId), firstName(fname), surname(sname), idNumber(idNo),
      contactNumber(contact), email(mail), address(addr), dateOfBirth(dob),
      encryptedPin(Utils::encryptText(pin)), failedAttempts(0), locked(false) {}

Customer::~Customer() {
    for (size_t i = 0; i < accounts.size(); ++i) delete accounts[i];
    accounts.clear();
}

Account* Customer::findAccount(const string& accNo) const {
    for (size_t i = 0; i < accounts.size(); ++i)
        if (accounts[i]->getAccountNumber() == accNo) return accounts[i];
    return 0;
}

double Customer::totalBalance() const {
    double total = 0.0;
    for (size_t i = 0; i < accounts.size(); ++i) total += accounts[i]->getBalance();
    return total;
}

void Customer::display() const {
    cout << "  Customer ID    : " << customerId << "\n";
    cout << "  Full Name      : " << getFullName() << "\n";
    cout << "  SA ID Number   : " << idNumber << "\n";
    cout << "  Contact Number : " << contactNumber << "\n";
    cout << "  Email Address  : " << email << "\n";
    cout << "  Physical Addr. : " << address << "\n";
    cout << "  Date of Birth  : " << dateOfBirth << "\n";
    cout << "  Account Status : " << (locked ? "LOCKED" : "ACTIVE") << "\n";
    cout << "  Total Balance  : " << Utils::formatMoney(totalBalance()) << "\n";
}

void Customer::displayAccounts() const {
    for (size_t i = 0; i < accounts.size(); ++i) {
        cout << "\n  ------------------------------------------------\n";
        accounts[i]->display();
    }
}

void Customer::writeToFile(ofstream& out) const {
    Utils::writeStr(out, customerId);
    Utils::writeStr(out, firstName);
    Utils::writeStr(out, surname);
    Utils::writeStr(out, idNumber);
    Utils::writeStr(out, contactNumber);
    Utils::writeStr(out, email);
    Utils::writeStr(out, address);
    Utils::writeStr(out, dateOfBirth);
    Utils::writeStr(out, encryptedPin);
    out.write(reinterpret_cast<const char*>(&failedAttempts), sizeof(failedAttempts));
    out.write(reinterpret_cast<const char*>(&locked), sizeof(locked));

    int n = (int)accounts.size();
    out.write(reinterpret_cast<const char*>(&n), sizeof(n));
    for (int i = 0; i < n; ++i) accounts[i]->writeToFile(out);
}

void Customer::readFromFile(ifstream& in) {
    for (size_t i = 0; i < accounts.size(); ++i) delete accounts[i];
    accounts.clear();

    Utils::readStr(in, customerId);
    Utils::readStr(in, firstName);
    Utils::readStr(in, surname);
    Utils::readStr(in, idNumber);
    Utils::readStr(in, contactNumber);
    Utils::readStr(in, email);
    Utils::readStr(in, address);
    Utils::readStr(in, dateOfBirth);
    Utils::readStr(in, encryptedPin);
    in.read(reinterpret_cast<char*>(&failedAttempts), sizeof(failedAttempts));
    in.read(reinterpret_cast<char*>(&locked), sizeof(locked));

    int n = 0;
    in.read(reinterpret_cast<char*>(&n), sizeof(n));
    for (int i = 0; i < n && in; ++i) {
        Account* a = readAccountFromFile(in);
        if (a) accounts.push_back(a);
    }
}

/* ================================================================
   TELLER
   ================================================================ */
Teller::Teller() : tellerId(""), fullName(""), encryptedPassword(""), branchCode("") {}

Teller::Teller(const string& id, const string& name, const string& pwd, const string& branch)
    : tellerId(id), fullName(name),
      encryptedPassword(Utils::encryptText(pwd)), branchCode(branch) {}

void Teller::display() const {
    cout << "  Teller ID   : " << tellerId << "\n";
    cout << "  Full Name   : " << fullName << "\n";
    cout << "  Branch Code : " << branchCode << "\n";
}

void Teller::writeToFile(ofstream& out) const {
    Utils::writeStr(out, tellerId);
    Utils::writeStr(out, fullName);
    Utils::writeStr(out, encryptedPassword);
    Utils::writeStr(out, branchCode);
}

void Teller::readFromFile(ifstream& in) {
    Utils::readStr(in, tellerId);
    Utils::readStr(in, fullName);
    Utils::readStr(in, encryptedPassword);
    Utils::readStr(in, branchCode);
}

/* ================================================================
   TRANSACTION
   ================================================================ */
Transaction::Transaction()
    : transactionId(""), accountNumber(""), type(""), amount(0.0),
      balanceAfter(0.0), dateTime(""), performedBy(""), branchCode(""), description("") {}

Transaction::Transaction(const string& id, const string& acc, const string& t, double amt,
                         double bal, const string& dt, const string& by,
                         const string& br, const string& desc)
    : transactionId(id), accountNumber(acc), type(t), amount(amt),
      balanceAfter(bal), dateTime(dt), performedBy(by),
      branchCode(br), description(desc) {}

void Transaction::writeToFile(ofstream& out) const {
    Utils::writeStr(out, transactionId);
    Utils::writeStr(out, accountNumber);
    Utils::writeStr(out, type);
    out.write(reinterpret_cast<const char*>(&amount), sizeof(amount));
    out.write(reinterpret_cast<const char*>(&balanceAfter), sizeof(balanceAfter));
    Utils::writeStr(out, dateTime);
    Utils::writeStr(out, performedBy);
    Utils::writeStr(out, branchCode);
    Utils::writeStr(out, description);
}

void Transaction::readFromFile(ifstream& in) {
    Utils::readStr(in, transactionId);
    Utils::readStr(in, accountNumber);
    Utils::readStr(in, type);
    in.read(reinterpret_cast<char*>(&amount), sizeof(amount));
    in.read(reinterpret_cast<char*>(&balanceAfter), sizeof(balanceAfter));
    Utils::readStr(in, dateTime);
    Utils::readStr(in, performedBy);
    Utils::readStr(in, branchCode);
    Utils::readStr(in, description);
}
