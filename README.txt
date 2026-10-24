================================================================
PROGRAMMING 621 — MULTI-BRANCH BANKING MANAGEMENT SYSTEM
README / User Guide
================================================================

FILES IN THIS SUBMISSION
------------------------
- main.cpp ............ Console prototype (single-file build)
- customers.dat ....... Binary customer accounts (encrypted PINs)
- transactions.dat .... Binary transaction history
- branches.dat ........ Binary branch information
- tellers.dat ......... Binary teller credentials (encrypted passwords)
- system_config.txt ... Plain-text system settings and counters
- customers_export.csv, transactions_export.csv, bank_summary_report.txt
                       Example export files produced by the system
- README.txt .......... This file

On first run the program creates the five .dat / .txt data files listed
above in the SAME FOLDER as the executable. When run from Code::Blocks
this is the project folder. Backup copies and CSV/TXT exports are
written to the same folder.

HOW TO COMPILE AND RUN
----------------------
Using Code::Blocks (Windows or Linux):
  1. File > New > Project > Console application > C++.
  2. Name it Programming621BankingSystem.
  3. Choose a folder you can easily find. Select GNU GCC Compiler.
  4. Open main.cpp, select all, replace with the supplied main.cpp.
  5. Build > Build (Ctrl+F9), then Run (Ctrl+F10).

Using the command line:
  g++ main.cpp -o BankingSystem
  ./BankingSystem           (Linux / macOS)
  BankingSystem.exe         (Windows)

DEFAULT TELLER LOGIN CREDENTIALS (for testing)
----------------------------------------------
  Teller ID : T001   Password : teller123   Branch : BR001 (Johannesburg)
  Teller ID : T002   Password : teller123   Branch : BR002 (Durban)
  Teller ID : T003   Password : teller123   Branch : BR003 (Cape Town)

Passwords are stored encrypted in tellers.dat — never as plain text.

HOW TO CREATE A CUSTOMER ACCOUNT AND VIEW THE GENERATED PIN
-----------------------------------------------------------
  1. Start the program and choose "1. Bank Teller Login".
  2. Log in with T001 / teller123.
  3. Choose "1. Register New Customer Account".
  4. Capture: First Name, Surname, SA ID (13 digits, Luhn-valid),
     contact number (10 digits), email (with @ and a domain),
     address, date of birth (DD/MM/YYYY), account type (1–4),
     and initial deposit (must meet the type minimum).
     Example valid SA ID for testing: 8001015009087
  5. The system displays the generated ACCOUNT NUMBER
     (format ACC-BRANCH-XXXXX) and the 5-DIGIT PIN once.
     The PIN is stored encrypted and cannot be shown again.

HOW CUSTOMERS LOG IN AND PERFORM TRANSACTIONS
---------------------------------------------
  1. From the main menu choose "2. Customer Login".
  2. Enter the Account Number (e.g. ACC-BR001-00001) and the 5-digit PIN.
  3. The customer menu allows:
        1. View Account Balance
        2. Deposit Funds
        3. Withdraw Funds
        4. Transfer Funds
        5. View Account Statement
        6. Change PIN
        0. Logout
  Three incorrect PIN attempts will temporarily LOCK the account.
  A teller can unlock it via:
      Teller Menu > 8. System Maintenance > 5. Unlock a Customer Account.

MENU / ASSIGNMENT MAPPING
-------------------------
1.1 Teller and customer PIN authentication; 3 failed attempts; lock flag.
1.2 Teller registration with full field validation; auto-generated
    ACC-BRANCH-XXXXX and 5-digit PIN; Savings / Cheque / Fixed Deposit /
    Student derived account classes (inheritance + polymorphism).
1.3 PIN-authenticated deposits, withdrawals, transfers and statements.
1.4 Three branches with details, inter-branch comparison and reporting.
1.5 Annual interest application to eligible accounts; linear search
    and multi-criterion customer filtering.
1.6 Daily transaction report, customer account summary, branch
    performance report.
1.7 Binary persistence, system_config.txt, timestamped backups,
    CSV exports and a TXT summary report.

TESTING RECOMMENDED STEPS
-------------------------
  1. Run the program and log in as T001 / teller123.
  2. Register a test customer using a valid SA ID (e.g. 8001015009087),
     a 10-digit phone, an email containing @ and a domain, a DD/MM/YYYY
     date of birth, and a deposit meeting the account type minimum.
  3. Record the generated account number and PIN.
  4. Log out, then use "2. Customer Login" to test balance, deposit,
     withdrawal, transfer, statement and PIN change.
  5. Close and reopen the program to confirm the data persists.
  6. Log in as a teller again and run "8. System Maintenance" to test
     backup and CSV/TXT export functions.

SYSTEM SCOPE
------------
- This is a console prototype produced for an academic brief; it is not
  production banking software.
- The encryption routine is a simple reversible transformation used for
  demonstration of encrypted storage. A production system would use a
  vetted salted password-hashing algorithm and secure key management.
- Binary .dat files are compiler- and platform-dependent. Use the same
  program build for testing and marking.
- Interest rates are illustrative; the monthly application uses the
  annual rate divided by 12.

================================================================




