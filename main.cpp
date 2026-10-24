// PROGRAMMING 621 - Multi-Branch Banking Management System
// Educational console prototype. Compile with C++11 or newer.
// NOTE: XOR hashing is a simple demonstration, NOT production-grade encryption.

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <limits>
#include <cstdio>
#include <cstdlib>

using namespace std;

struct Branch {
    string code, name, city;
};

struct Customer {
    string accountNo, fullName, idNumber, phone, email, address, dob;
    string accountType, branchCode;
    string pinHash;
    double balance;
    bool locked;
    int failedAttempts;
};

struct Teller {
    string id, fullName, passwordHash, branchCode;
};

struct Transaction {
    string timestamp, transactionType, fromAccount, toAccount, branchCode;
    double amount;
    string description;
};

// Simple reversible XOR demonstration. Replace with a vetted password-hashing
// algorithm in any real system. Do not use this for real banking credentials.
string hashText(const string& input) {
    string out = input;
    const unsigned char key = 0x5A;
    for (size_t i = 0; i < out.size(); ++i)
        out[i] = static_cast<char>(static_cast<unsigned char>(out[i]) ^ key);
    return out;
}

string nowText() {
    time_t t = time(NULL);
    tm *lt = localtime(&t);
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", lt);
    return buffer;
}

string readLine(const string& prompt) {
    cout << prompt;
    string value;
    getline(cin, value);
    return value;
}

int readInt(const string& prompt, int low, int high) {
    while (true) {
        string s = readLine(prompt);
        stringstream ss(s);
        int n; char extra;
        if (ss >> n && !(ss >> extra) && n >= low && n <= high) return n;
        cout << "Invalid choice. Enter " << low << " to " << high << ".\n";
    }
}

double readAmount(const string& prompt) {
    while (true) {
        string s = readLine(prompt);
        stringstream ss(s);
        double n; char extra;
        if (ss >> n && !(ss >> extra) && n > 0) return n;
        cout << "Enter a valid amount greater than zero.\n";
    }
}

bool allDigits(const string& s) {
    if (s.empty()) return false;
    for (size_t i=0;i<s.size();++i) if (!isdigit(static_cast<unsigned char>(s[i]))) return false;
    return true;
}

bool validSAID(const string& id) {
    if (id.size()!=13 || !allDigits(id)) return false;
    // Luhn checksum used for South African ID validation.
    int sum=0;
    for (int i=12;i>=0;--i) {
        int digit=id[i]-'0';
        if ((12-i)%2==1) {
            digit*=2;
            if (digit>9) digit-=9;
        }
        sum+=digit;
    }
    return sum%10==0;
}

bool validDate(const string& d) {
    if (d.size()!=10 || d[2]!='/' || d[5]!='/') return false;
    string dd=d.substr(0,2), mm=d.substr(3,2), yy=d.substr(6,4);
    if (!allDigits(dd)||!allDigits(mm)||!allDigits(yy)) return false;
    int day=atoi(dd.c_str()), month=atoi(mm.c_str()), year=atoi(yy.c_str());
    if (year<1900 || month<1 || month>12 || day<1) return false;
    int days[]={31, (year%4==0 && (year%100!=0||year%400==0))?29:28,31,30,31,30,31,31,30,31,30,31};
    return day<=days[month-1];
}

bool validEmail(const string& e) {
    size_t at=e.find('@'), dot=e.find('.', at==string::npos?0:at);
    return at!=string::npos && at>0 && dot!=string::npos && dot>at+1 && dot+1<e.size();
}

bool validPhone(const string& p) { return p.size()==10 && allDigits(p); }

double minimumDeposit(const string& type) {
    if(type=="Savings") return 100.0;
    if(type=="Cheque") return 500.0;
    if(type=="Fixed Deposit") return 1000.0;
    return 0.0; // Student
}

class Account {
protected:
    Customer data;
public:
    Account(const Customer& c):data(c){}
    virtual ~Account(){}
    virtual double annualRate() const=0;
    virtual string typeName() const=0;
    double interest() const { return data.balance*annualRate(); }
};
class SavingsAccount: public Account {
public: SavingsAccount(const Customer& c):Account(c){}
    double annualRate() const { return 0.04; }
    string typeName() const { return "Savings"; }
};
class ChequeAccount: public Account {
public: ChequeAccount(const Customer& c):Account(c){}
    double annualRate() const { return 0.01; }
    string typeName() const { return "Cheque"; }
};
class FixedDepositAccount: public Account {
public: FixedDepositAccount(const Customer& c):Account(c){}
    double annualRate() const { return 0.07; }
    string typeName() const { return "Fixed Deposit"; }
};
class StudentAccount: public Account {
public: StudentAccount(const Customer& c):Account(c){}
    double annualRate() const { return 0.03; }
    string typeName() const { return "Student"; }
};

class BankingSystem {
    vector<Branch> branches;
    vector<Customer> customers;
    vector<Teller> tellers;
    vector<Transaction> transactions;
    string currentTellerBranch;

    void loadBranches() {
        branches.clear();
        ifstream in("branches.dat", ios::binary);
        if(in) {
            size_t n=0; in.read(reinterpret_cast<char*>(&n),sizeof(n));
            for(size_t i=0;i<n && in;++i) {
                Branch b; readString(in,b.code); readString(in,b.name); readString(in,b.city); branches.push_back(b);
            }
        }
        if(branches.empty()) {
            branches.push_back({"JHB","Johannesburg Branch","Johannesburg"});
            branches.push_back({"CPT","Cape Town Branch","Cape Town"});
            branches.push_back({"DBN","Durban Branch","Durban"});
            saveBranches();
        }
    }
    void saveBranches() {
        ofstream out("branches.dat",ios::binary|ios::trunc);
        size_t n=branches.size(); out.write(reinterpret_cast<const char*>(&n),sizeof(n));
        for(size_t i=0;i<n;++i){writeString(out,branches[i].code);writeString(out,branches[i].name);writeString(out,branches[i].city);}
    }
    void loadTellers() {
        tellers.clear();
        ifstream in("tellers.dat",ios::binary);
        if(in) {
            size_t n=0; in.read(reinterpret_cast<char*>(&n),sizeof(n));
            for(size_t i=0;i<n && in;++i) {
                Teller t; readString(in,t.id);readString(in,t.fullName);readString(in,t.passwordHash);readString(in,t.branchCode);tellers.push_back(t);
            }
        }
        if(tellers.empty()) {
            tellers.push_back({"T001","Demo Teller",hashText("Bank123"),"JHB"});
            tellers.push_back({"T002","Cape Teller",hashText("Bank123"),"CPT"});
            saveTellers();
        }
    }
    void saveTellers() {
        ofstream out("tellers.dat",ios::binary|ios::trunc);
        size_t n=tellers.size();out.write(reinterpret_cast<const char*>(&n),sizeof(n));
        for(size_t i=0;i<n;++i){writeString(out,tellers[i].id);writeString(out,tellers[i].fullName);writeString(out,tellers[i].passwordHash);writeString(out,tellers[i].branchCode);}
    }
    void loadCustomers() {
        customers.clear(); ifstream in("customers.dat",ios::binary); if(!in)return;
        size_t n=0;in.read(reinterpret_cast<char*>(&n),sizeof(n));
        for(size_t i=0;i<n && in;++i) {
            Customer c;
            readString(in,c.accountNo);readString(in,c.fullName);readString(in,c.idNumber);readString(in,c.phone);
            readString(in,c.email);readString(in,c.address);readString(in,c.dob);readString(in,c.accountType);
            readString(in,c.branchCode);readString(in,c.pinHash);
            in.read(reinterpret_cast<char*>(&c.balance),sizeof(c.balance));
            in.read(reinterpret_cast<char*>(&c.locked),sizeof(c.locked));
            in.read(reinterpret_cast<char*>(&c.failedAttempts),sizeof(c.failedAttempts));
            if(in)customers.push_back(c);
        }
    }
    void saveCustomers() {
        ofstream out("customers.dat",ios::binary|ios::trunc);
        size_t n=customers.size();out.write(reinterpret_cast<const char*>(&n),sizeof(n));
        for(size_t i=0;i<n;++i) {
            const Customer& c=customers[i];
            writeString(out,c.accountNo);writeString(out,c.fullName);writeString(out,c.idNumber);writeString(out,c.phone);
            writeString(out,c.email);writeString(out,c.address);writeString(out,c.dob);writeString(out,c.accountType);
            writeString(out,c.branchCode);writeString(out,c.pinHash);
            out.write(reinterpret_cast<const char*>(&c.balance),sizeof(c.balance));
            out.write(reinterpret_cast<const char*>(&c.locked),sizeof(c.locked));
            out.write(reinterpret_cast<const char*>(&c.failedAttempts),sizeof(c.failedAttempts));
        }
    }
    void loadTransactions() {
        transactions.clear();ifstream in("transactions.dat",ios::binary);if(!in)return;
        size_t n=0;in.read(reinterpret_cast<char*>(&n),sizeof(n));
        for(size_t i=0;i<n && in;++i) {
            Transaction t;readString(in,t.timestamp);readString(in,t.transactionType);readString(in,t.fromAccount);
            readString(in,t.toAccount);readString(in,t.branchCode);in.read(reinterpret_cast<char*>(&t.amount),sizeof(t.amount));readString(in,t.description);
            if(in)transactions.push_back(t);
        }
    }
    void saveTransactions() {
        ofstream out("transactions.dat",ios::binary|ios::trunc);
        size_t n=transactions.size();out.write(reinterpret_cast<const char*>(&n),sizeof(n));
        for(size_t i=0;i<n;++i){const Transaction&t=transactions[i];writeString(out,t.timestamp);writeString(out,t.transactionType);
            writeString(out,t.fromAccount);writeString(out,t.toAccount);writeString(out,t.branchCode);
            out.write(reinterpret_cast<const char*>(&t.amount),sizeof(t.amount));writeString(out,t.description);}
    }
    static void writeString(ofstream& out,const string& s) {
        size_t n=s.size();out.write(reinterpret_cast<const char*>(&n),sizeof(n));out.write(s.data(),n);
    }
    static void readString(ifstream& in,string& s) {
        size_t n=0;in.read(reinterpret_cast<char*>(&n),sizeof(n));
        if(!in || n>100000){s="";in.setstate(ios::failbit);return;}
        s.resize(n);if(n)in.read(&s[0],n);
    }
    int findCustomer(const string& acc) const {
        for(size_t i=0;i<customers.size();++i)if(customers[i].accountNo==acc)return (int)i;
        return -1;
    }
    string makeAccountNo(const string& branch) const {
        int maxNum=0;
        for(size_t i=0;i<customers.size();++i) if(customers[i].branchCode==branch) {
            string a=customers[i].accountNo;
            size_t p=a.rfind('-');
            if(p!=string::npos) maxNum=max(maxNum,atoi(a.substr(p+1).c_str()));
        }
        stringstream ss;ss<<"ACC-"<<branch<<"-"<<setw(5)<<setfill('0')<<(maxNum+1);return ss.str();
    }
    string makePIN() const {
        srand((unsigned int)(time(NULL)+customers.size()*31));
        int p=10000+rand()%90000;stringstream ss;ss<<p;return ss.str();
    }
    void record(const string& type,const string& from,const string& to,const string& branch,double amount,const string& desc) {
        Transaction t={nowText(),type,from,to,branch,amount,desc};transactions.push_back(t);saveTransactions();
    }
    bool verifyPIN(Customer& c) {
        for(int attempt=1;attempt<=3;++attempt) {
            string pin=readLine("Enter 5-digit PIN: ");
            if(hashText(pin)==c.pinHash && pin.size()==5 && allDigits(pin)){c.failedAttempts=0;saveCustomers();return true;}
            cout<<"Incorrect PIN ("<<attempt<<"/3).\n";
        }
        c.locked=true;saveCustomers();cout<<"Account temporarily locked for this program session. Contact teller to reset.\n";return false;
    }
    void showCustomer(const Customer& c) const {
        cout<<"\nAccount: "<<c.accountNo<<"\nName: "<<c.fullName<<"\nID: "<<c.idNumber<<"\nPhone: "<<c.phone
            <<"\nEmail: "<<c.email<<"\nAddress: "<<c.address<<"\nDOB: "<<c.dob<<"\nType: "<<c.accountType
            <<"\nBranch: "<<c.branchCode<<"\nBalance: R"<<fixed<<setprecision(2)<<c.balance<<"\n";
    }
    void createCustomer() {
        Customer c; c.branchCode=currentTellerBranch;
        c.fullName=readLine("Full name (first and surname): ");
        if(c.fullName.find(' ')==string::npos){cout<<"Enter first name and surname.\n";return;}
        c.idNumber=readLine("SA ID (13 digits/checksum): ");
        if(!validSAID(c.idNumber)){cout<<"Invalid SA ID.\n";return;}
        for(size_t i=0;i<customers.size();++i)if(customers[i].idNumber==c.idNumber){cout<<"ID already registered.\n";return;}
        c.phone=readLine("Contact number (10 digits): ");if(!validPhone(c.phone)){cout<<"Invalid phone.\n";return;}
        c.email=readLine("Email: ");if(!validEmail(c.email)){cout<<"Invalid email.\n";return;}
        c.address=readLine("Physical address: ");
        c.dob=readLine("Date of birth (DD/MM/YYYY): ");if(!validDate(c.dob)){cout<<"Invalid date.\n";return;}
        cout<<"Account type: 1 Savings, 2 Cheque, 3 Fixed Deposit, 4 Student\n";
        int t=readInt("Choose: ",1,4);
        const string types[]={"Savings","Cheque","Fixed Deposit","Student"};c.accountType=types[t-1];
        double minimum=minimumDeposit(c.accountType);
        c.balance=readAmount("Initial deposit (minimum R"+to_string((int)minimum)+"): ");
        if(c.balance<minimum){cout<<"Below minimum deposit.\n";return;}
        c.accountNo=makeAccountNo(c.branchCode);string pin=makePIN();c.pinHash=hashText(pin);c.locked=false;c.failedAttempts=0;
        customers.push_back(c);saveCustomers();record("OPENING",c.accountNo,"",c.branchCode,c.balance,"New account");
        cout<<"\nCUSTOMER REGISTERED\nAccount number: "<<c.accountNo<<"\nGenerated PIN (show once): "<<pin<<"\n";
    }
    void tellerMenu() {
        while(true) {
            cout<<"\n--- TELLER MENU ("<<currentTellerBranch<<") ---\n1 Register customer\n2 Find/view customer\n3 Teller-assisted transaction\n4 Branch report\n5 Back up data\n6 Export CSV/text\n0 Logout\n";
            int ch=readInt("Choice: ",0,6);if(ch==0)return;
            if(ch==1)createCustomer();
            else if(ch==2){string a=readLine("Account number: ");int i=findCustomer(a);if(i<0)cout<<"Not found.\n";else if(customers[i].branchCode!=currentTellerBranch)cout<<"Access denied: other branch.\n";else showCustomer(customers[i]);}
            else if(ch==3){string a=readLine("Customer account: ");int i=findCustomer(a);if(i<0){cout<<"Not found.\n";continue;}if(customers[i].branchCode!=currentTellerBranch){cout<<"Access denied.\n";continue;}if(!verifyPIN(customers[i]))continue;customerTransactions(i);}
            else if(ch==4)branchReport(currentTellerBranch);
            else if(ch==5)backup();
            else if(ch==6)exportFiles();
        }
    }
    void customerTransactions(int idx) {
        while(true) {
            cout<<"\n--- CUSTOMER SERVICES ---\n1 Balance/details\n2 Deposit\n3 Withdraw\n4 Transfer\n5 Statement\n6 Change PIN\n0 Back\n";
            int ch=readInt("Choice: ",0,6);if(ch==0)return;
            Customer& c=customers[idx];
            if(ch==1)showCustomer(c);
            else if(ch==2){double a=readAmount("Deposit amount: R");c.balance+=a;saveCustomers();record("DEPOSIT",c.accountNo,"",c.branchCode,a,"Cash deposit");cout<<"Deposit successful.\n";}
            else if(ch==3){double a=readAmount("Withdrawal amount: R");if(a>c.balance)cout<<"Insufficient funds.\n";else{c.balance-=a;saveCustomers();record("WITHDRAWAL",c.accountNo,"",c.branchCode,a,"Cash withdrawal");cout<<"Withdrawal successful.\n";}}
            else if(ch==4){string to=readLine("Recipient account number: ");int j=findCustomer(to);if(j<0){cout<<"Recipient not found.\n";continue;}if(j==idx){cout<<"Cannot transfer to same account.\n";continue;}double a=readAmount("Transfer amount: R");if(a>c.balance){cout<<"Insufficient funds.\n";continue;}c.balance-=a;customers[j].balance+=a;saveCustomers();record("TRANSFER",c.accountNo,to,c.branchCode,a,"Fund transfer");cout<<"Transfer successful.\n";}
            else if(ch==5)statement(c.accountNo);
            else if(ch==6){string old=readLine("Current PIN: ");if(hashText(old)!=c.pinHash){cout<<"Incorrect PIN.\n";continue;}string p=readLine("New 5-digit PIN: ");string p2=readLine("Confirm new PIN: ");if(p.size()!=5||!allDigits(p)||p!=p2||p==old){cout<<"PIN must be five digits, match confirmation, and differ from old PIN.\n";}else{c.pinHash=hashText(p);saveCustomers();cout<<"PIN changed.\n";}}
        }
    }
    void customerLogin() {
        string a=readLine("Account number: ");int i=findCustomer(a);if(i<0){cout<<"Account not found.\n";return;}
        if(customers[i].locked){cout<<"Account is locked. Ask a teller to reset it.\n";return;}
        if(verifyPIN(customers[i]))customerTransactions(i);
    }
    void statement(const string& account) const {
        cout<<"\n--- STATEMENT: "<<account<<" ---\n";
        cout<<left<<setw(20)<<"Date"<<setw(16)<<"Type"<<setw(18)<<"From"<<setw(18)<<"To"<<right<<setw(12)<<"Amount\n";
        for(size_t i=0;i<transactions.size();++i)if(transactions[i].fromAccount==account||transactions[i].toAccount==account) {
            const Transaction&t=transactions[i];cout<<left<<setw(20)<<t.timestamp<<setw(16)<<t.transactionType<<setw(18)<<t.fromAccount<<setw(18)<<t.toAccount<<right<<setw(12)<<fixed<<setprecision(2)<<t.amount<<"\n";
        }
    }
    void viewBranches() const {
        cout<<"\n--- BRANCHES ---\n";
        for(size_t i=0;i<branches.size();++i){int count=0;double total=0;for(size_t j=0;j<customers.size();++j)if(customers[j].branchCode==branches[i].code){++count;total+=customers[j].balance;}
            cout<<branches[i].code<<" | "<<branches[i].name<<" | "<<branches[i].city<<" | Customers: "<<count<<" | Deposits: R"<<fixed<<setprecision(2)<<total<<"\n";}
    }
    void branchReport(const string& branch) const {
        int count=0;double total=0;
        cout<<"\n--- BRANCH PERFORMANCE: "<<branch<<" ---\n";
        for(size_t i=0;i<customers.size();++i)if(customers[i].branchCode==branch){++count;total+=customers[i].balance;}
        cout<<"Customer accounts: "<<count<<"\nTotal balances: R"<<fixed<<setprecision(2)<<total<<"\n";
        int tx=0;double volume=0;for(size_t i=0;i<transactions.size();++i)if(transactions[i].branchCode==branch){++tx;volume+=transactions[i].amount;}
        cout<<"Recorded transactions: "<<tx<<"\nTransaction value: R"<<volume<<"\n";
    }
    void searchCustomers() const {
        string term=readLine("Search by name/account/ID/branch: ");bool found=false;
        for(size_t i=0;i<customers.size();++i) {
            const Customer& c=customers[i];
            if(c.fullName.find(term)!=string::npos||c.accountNo.find(term)!=string::npos||c.idNumber.find(term)!=string::npos||c.branchCode.find(term)!=string::npos){showCustomer(c);found=true;}
        }
        if(!found)cout<<"No matching customers.\n";
    }
    void applyInterest() {
        int changed=0;
        for(size_t i=0;i<customers.size();++i) {
            Customer& c=customers[i];double rate=0;
            if(c.accountType=="Savings")rate=.04;else if(c.accountType=="Cheque")rate=.01;else if(c.accountType=="Fixed Deposit")rate=.07;else if(c.accountType=="Student")rate=.03;
            double amount=c.balance*rate/12.0;
            if(amount>0){c.balance+=amount;record("INTEREST",c.accountNo,"",c.branchCode,amount,"Monthly illustrative interest");++changed;}
        }
        saveCustomers();cout<<"Monthly illustrative interest applied to "<<changed<<" accounts.\n";
    }
    void dailyReport() const {
        string today=nowText().substr(0,10);int count=0;double total=0;
        cout<<"\n--- DAILY TRANSACTION REPORT ("<<today<<") ---\n";
        for(size_t i=0;i<transactions.size();++i)if(transactions[i].timestamp.substr(0,10)==today){const Transaction&t=transactions[i];cout<<t.timestamp<<" | "<<t.transactionType<<" | "<<t.fromAccount<<" -> "<<t.toAccount<<" | R"<<fixed<<setprecision(2)<<t.amount<<"\n";++count;total+=t.amount;}
        cout<<"Count: "<<count<<" | Total value: R"<<total<<"\n";
    }
    void accountSummary() const {
        cout<<"\n--- CUSTOMER ACCOUNT SUMMARY ---\n";
        for(size_t i=0;i<customers.size();++i)cout<<customers[i].accountNo<<" | "<<customers[i].fullName<<" | "<<customers[i].accountType<<" | "<<customers[i].branchCode<<" | R"<<fixed<<setprecision(2)<<customers[i].balance<<"\n";
    }
    void backup() const {
        const char* files[]={"customers.dat","transactions.dat","branches.dat","tellers.dat","system_config.txt"};
        for(int i=0;i<5;++i){ifstream src(files[i],ios::binary);if(!src)continue;string dest=string("backup_")+files[i];ofstream dst(dest.c_str(),ios::binary|ios::trunc);dst<<src.rdbuf();}
        cout<<"Backup copies created in the program folder (backup_*).\n";
    }
    void recover() {
        const char* files[]={"customers.dat","transactions.dat","branches.dat","tellers.dat","system_config.txt"};
        for(int i=0;i<5;++i){string src=string("backup_")+files[i];ifstream in(src.c_str(),ios::binary);if(!in)continue;ofstream out(files[i],ios::binary|ios::trunc);out<<in.rdbuf();}
        loadBranches();loadTellers();loadCustomers();loadTransactions();cout<<"Recovery attempted from available backup files.\n";
    }
    void exportFiles() const {
        ofstream c("customers_export.csv");c<<"Account,Name,ID,Phone,Email,Type,Branch,Balance\n";
        for(size_t i=0;i<customers.size();++i){const Customer&x=customers[i];c<<x.accountNo<<",\""<<x.fullName<<"\","<<x.idNumber<<","<<x.phone<<","<<x.email<<","<<x.accountType<<","<<x.branchCode<<","<<fixed<<setprecision(2)<<x.balance<<"\n";}
        ofstream t("transactions_export.txt");
        for(size_t i=0;i<transactions.size();++i){const Transaction&x=transactions[i];t<<x.timestamp<<" | "<<x.transactionType<<" | "<<x.fromAccount<<" -> "<<x.toAccount<<" | R"<<fixed<<setprecision(2)<<x.amount<<" | "<<x.description<<"\n";}
        cout<<"Exported customers_export.csv and transactions_export.txt.\n";
    }
    void resetCustomerLock() {
        string a=readLine("Account number to unlock: ");int i=findCustomer(a);
        if(i<0){cout<<"Not found.\n";return;}
        customers[i].locked=false;customers[i].failedAttempts=0;saveCustomers();cout<<"Account unlocked.\n";
    }
    void tellerLogin() {
        string id=readLine("Teller ID: "),pw=readLine("Password: ");
        for(size_t i=0;i<tellers.size();++i)if(tellers[i].id==id&&tellers[i].passwordHash==hashText(pw)){
            currentTellerBranch=tellers[i].branchCode;cout<<"Welcome "<<tellers[i].fullName<<". Branch access: "<<currentTellerBranch<<"\n";tellerMenu();return;}
        cout<<"Invalid teller credentials.\n";
    }
public:
    BankingSystem(){loadBranches();loadTellers();loadCustomers();loadTransactions();ofstream cfg("system_config.txt",ios::app);if(cfg.tellp()==0)cfg<<"System=Programming621BankingPrototype\nCurrency=ZAR\n";}
    void run() {
        while(true) {
            cout<<"\n====================================\n MULTI-BRANCH BANKING SYSTEM\n====================================\n";
            cout<<"1 Teller login\n2 Customer login\n3 View branches\n4 Search customers (demo/admin)\n5 Apply monthly interest (demo/admin)\n6 Daily transaction report\n7 Customer account summary\n8 Backup data\n9 Recover backup\n10 Reset customer lock (demo/admin)\n0 Exit\n";
            int ch=readInt("Select: ",0,10);if(ch==0){cout<<"Goodbye.\n";return;}
            try {
                if(ch==1)tellerLogin();else if(ch==2)customerLogin();else if(ch==3)viewBranches();else if(ch==4)searchCustomers();
                else if(ch==5)applyInterest();else if(ch==6)dailyReport();else if(ch==7)accountSummary();else if(ch==8)backup();else if(ch==9)recover();else if(ch==10)resetCustomerLock();
            } catch(const exception& e){cerr<<"Operation error: "<<e.what()<<"\n";}
        }
    }
};

int main() {
    try { BankingSystem system; system.run(); }
    catch(const exception& e){cerr<<"Fatal error: "<<e.what()<<"\n";return 1;}
    return 0;
}
