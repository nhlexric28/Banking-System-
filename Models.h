#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <vector>
#include <fstream>
#include "Utils.h"

using namespace std;

/* ================================================================
   1.4  BRANCH  (struct as required)
   ================================================================ */
struct Branch {
    string branchCode;
    string branchName;
    string address;
    string contactNumber;
    string managerName;

    Branch();
    Branch(const string& code, const string& name, const string& addr,
           const string& contact, const string& mgr);

    void display() const;
    void writeToFile(ofstream& out) const;
    void readFromFile(ifstream& in);
};

/* ================================================================
   1.2.2  ACCOUNT HIERARCHY  (Base + 4 derived classes)
   ================================================================ */
class Account {
protected:
    string accountNumber;
    string accountType;
    double balance;
    string dateOpened;
    string status;                 // Active / Dormant
    double totalDeposits;
    double totalWithdrawals;

public:
    Account();
    Account(const string& accNo, const string& type, double initialDeposit, const string& opened);
    virtual ~Account();

    /* getters */
    string getAccountNumber()    const { return accountNumber; }
    string getAccountType()      const { return accountType; }
    double getBalance()          const { return balance; }
    string getDateOpened()       const { return dateOpened; }
    string getStatus()           const { return status; }
    double getTotalDeposits()    const { return totalDeposits; }
    double getTotalWithdrawals() const { return totalWithdrawals; }

    /* setters / mutators */
    void setBalance(double b)       { balance = b; }
    void setStatus(const string& s) { status = s; }
    void addDeposit(double a)       { balance += a; totalDeposits += a; }
    void addWithdrawal(double a)    { balance -= a; totalWithdrawals += a; }

    /* pure virtual – each account type defines its own rules */
    virtual double minimumDeposit()     const = 0;
    virtual double annualInterestRate() const = 0;
    virtual double withdrawalLimit()    const = 0;   // per transaction (0 = no limit)
    virtual int    typeTag()            const = 0;   // 1..4 used for file storage
    virtual string extraInfo()          const { return ""; }

    /* virtual behaviour */
    virtual bool   canWithdraw(double amount, string& reason) const;
    virtual double calculateInterest() const;
    virtual void   display() const;

    /* binary serialisation */
    virtual void writeToFile(ofstream& out) const;
    virtual void readFromFile(ifstream& in);
};

/* ---------------- Derived account types ---------------- */
class SavingsAccount : public Account {
private:
    double interestRate;      // % per annum
    double minimumBalance;
public:
    SavingsAccount();
    SavingsAccount(const string& accNo, double initialDeposit, const string& opened);

    double minimumDeposit()     const { return 50.0; }
    double annualInterestRate() const { return interestRate; }
    double withdrawalLimit()    const { return 5000.0; }
    int    typeTag()            const { return 1; }

    string extraInfo() const;
    void   display() const;
    void   writeToFile(ofstream& out) const;
    void   readFromFile(ifstream& in);
};

class ChequeAccount : public Account {
private:
    double interestRate;
    double overdraftLimit;
public:
    ChequeAccount();
    ChequeAccount(const string& accNo, double initialDeposit, const string& opened);

    double minimumDeposit()     const { return 100.0; }
    double annualInterestRate() const { return interestRate; }
    double withdrawalLimit()    const { return 10000.0; }
    int    typeTag()            const { return 2; }

    bool   canWithdraw(double amount, string& reason) const;
    string extraInfo() const;
    void   display() const;
    void   writeToFile(ofstream& out) const;
    void   readFromFile(ifstream& in);
};

class FixedDepositAccount : public Account {
private:
    double interestRate;
    int    termMonths;
    string maturityDate;
public:
    FixedDepositAccount();
    FixedDepositAccount(const string& accNo, double initialDeposit, const string& opened);

    double minimumDeposit()     const { return 1000.0; }
    double annualInterestRate() const { return interestRate; }
    double withdrawalLimit()    const { return 0.0; }   // locked until maturity
    int    typeTag()            const { return 3; }

    bool   canWithdraw(double amount, string& reason) const;
    string extraInfo() const;
    void   display() const;
    void   writeToFile(ofstream& out) const;
    void   readFromFile(ifstream& in);
};

class StudentAccount : public Account {
private:
    double interestRate;
    double monthlyFee;
public:
    StudentAccount();
    StudentAccount(const string& accNo, double initialDeposit, const string& opened);

    double minimumDeposit()     const { return 20.0; }
    double annualInterestRate() const { return interestRate; }
    double withdrawalLimit()    const { return 2000.0; }
    int    typeTag()            const { return 4; }

    string extraInfo() const;
    void   display() const;
    void   writeToFile(ofstream& out) const;
    void   readFromFile(ifstream& in);
};

/* factory helpers used when loading accounts back from disk */
Account* createAccountByTag(int tag);
Account* readAccountFromFile(ifstream& in);

/* ================================================================
   CUSTOMER  (1.1.2  +  1.2.1)
   ================================================================ */
class Customer {
private:
    string customerId;         // CUST-0001
    string firstName;
    string surname;
    string idNumber;           // 13 digit SA ID
    string contactNumber;      // 10 digits
    string email;
    string address;
    string dateOfBirth;        // DD/MM/YYYY
    string encryptedPin;       // encrypted 5-digit PIN
    int    failedAttempts;
    bool   locked;
    vector<Account*> accounts;

public:
    Customer();
    Customer(const string& custId, const string& fname, const string& sname,
             const string& idNo, const string& contact, const string& mail,
             const string& addr, const string& dob, const string& pin);
    ~Customer();

    /* getters */
    string getCustomerId()     const { return customerId; }
    string getFirstName()      const { return firstName; }
    string getSurname()        const { return surname; }
    string getFullName()       const { return firstName + " " + surname; }
    string getIdNumber()       const { return idNumber; }
    string getContactNumber()  const { return contactNumber; }
    string getEmail()          const { return email; }
    string getAddress()        const { return address; }
    string getDateOfBirth()    const { return dateOfBirth; }
    string getEncryptedPin()   const { return encryptedPin; }
    int    getFailedAttempts() const { return failedAttempts; }
    bool   isLocked()          const { return locked; }

    vector<Account*>& getAccounts() { return accounts; }

    /* setters */
    void setEncryptedPin(const string& p) { encryptedPin = p; }
    void setFailedAttempts(int n)         { failedAttempts = n; }
    void setLocked(bool l)                { locked = l; }
    void addAccount(Account* a)           { accounts.push_back(a); }

    Account* findAccount(const string& accNo) const;
    double   totalBalance() const;

    void display() const;
    void displayAccounts() const;

    void writeToFile(ofstream& out) const;
    void readFromFile(ifstream& in);
};

/* ================================================================
   TELLER  (1.1.1)
   ================================================================ */
class Teller {
private:
    string tellerId;              // T001
    string fullName;
    string encryptedPassword;
    string branchCode;            // restricts teller to their branch
public:
    Teller();
    Teller(const string& id, const string& name, const string& pwd, const string& branch);

    string getTellerId()          const { return tellerId; }
    string getFullName()          const { return fullName; }
    string getEncryptedPassword() const { return encryptedPassword; }
    string getBranchCode()        const { return branchCode; }

    void setPassword(const string& pwd) { encryptedPassword = Utils::encryptText(pwd); }
    bool checkPassword(const string& pwd) const { return Utils::verifyText(pwd, encryptedPassword); }

    void display() const;
    void writeToFile(ofstream& out) const;
    void readFromFile(ifstream& in);
};

/* ================================================================
   TRANSACTION  (1.3)
   ================================================================ */
class Transaction {
public:
    string transactionId;
    string accountNumber;
    string type;            // Deposit / Withdrawal / Transfer-In / Transfer-Out / Interest
    double amount;
    double balanceAfter;
    string dateTime;
    string performedBy;     // teller ID or "SELF"
    string branchCode;
    string description;

    Transaction();
    Transaction(const string& id, const string& acc, const string& t, double amt,
                double bal, const string& dt, const string& by,
                const string& br, const string& desc);

    void writeToFile(ofstream& out) const;
    void readFromFile(ifstream& in);
};

#endif
