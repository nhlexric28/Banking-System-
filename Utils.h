#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <iostream>
#include <stdexcept>

using namespace std;

/* Custom exception used throughout the system so that
   validation errors can be thrown and caught with try / catch. */
class BankingException : public std::runtime_error {
public:
    explicit BankingException(const string& msg) : std::runtime_error(msg) {}
};

namespace Utils {

    /* ---------------- Encryption (1.1.1 / 1.1.2) ---------------- */
    string encryptText(const string& plain);            // simple XOR + shift + hex
    string decryptText(const string& cipher);           // reverse (used for display/testing)
    bool   verifyText(const string& plain, const string& cipher);

    /* ---------------- Validation (1.2.1) ---------------- */
    bool isAllDigits(const string& s);
    bool validateSAID(const string& id);                // 13 digits + Luhn check
    bool validatePhone(const string& phone);            // 10 digits
    bool validateEmail(const string& email);            // must contain @ and domain
    bool validateDate(const string& date);              // DD/MM/YYYY

    /* ---------------- Console input helpers ---------------- */
    string readLine(const string& prompt);
    int    readInt(const string& prompt);
    double readDouble(const string& prompt);
    void   pause();
    void   clearScreen();

    /* ---------------- String / misc helpers ---------------- */
    string toUpper(const string& s);
    string trim(const string& s);
    string formatMoney(double amount);
    string currentDate();                               // DD/MM/YYYY
    string currentDateTime();                           // DD/MM/YYYY HH:MM:SS
    string generatePIN(int digits);
    int    randomInt(int min, int max);

    /* ---------------- Binary string I/O (file handling 1.7) ---------------- */
    void writeStr(ostream& out, const string& s);
    void readStr(istream& in, string& s);
}

#endif
