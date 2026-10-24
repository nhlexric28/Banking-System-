#include "Utils.h"
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cstdlib>
#include <cctype>
#include <algorithm>

using namespace std;

/* ===================== internal constants ===================== */
namespace {
    const unsigned char XOR_KEY = 0x5A;
    const unsigned char SHIFT   = 7;

    int hexToInt(char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        return 0;
    }

    bool isLeapYear(int y) {
        return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
    }
}

namespace Utils {

/*
   1.1.1 / 1.1.2  ENCRYPTION
   Simple reversible algorithm: XOR with key, add shift, hex encode.
    */
string encryptText(const string& plain) {
    static const char* HEX = "0123456789ABCDEF";
    string out;
    out.reserve(plain.size() * 2);
    for (size_t i = 0; i < plain.size(); ++i) {
        unsigned char c = (unsigned char)plain[i];
        c = (unsigned char)(c ^ XOR_KEY);
        c = (unsigned char)((c + SHIFT) & 0xFF);
        out += HEX[(c >> 4) & 0x0F];
        out += HEX[c & 0x0F];
    }
    return out;
}

string decryptText(const string& cipher) {
    string out;
    if (cipher.size() % 2 != 0) return out;
    for (size_t i = 0; i + 1 < cipher.size(); i += 2) {
        unsigned char c = (unsigned char)((hexToInt(cipher[i]) << 4) | hexToInt(cipher[i + 1]));
        c = (unsigned char)((c - SHIFT) & 0xFF);
        c = (unsigned char)(c ^ XOR_KEY);
        out += (char)c;
    }
    return out;
}

bool verifyText(const string& plain, const string& cipher) {
    return encryptText(plain) == cipher;
}

/* ============================================================
   VALIDATION
   ============================================================ */
bool isAllDigits(const string& s) {
    if (s.empty()) return false;
    for (size_t i = 0; i < s.size(); ++i)
        if (!isdigit((unsigned char)s[i])) return false;
    return true;
}

/* South African ID: 13 digits and must pass the Luhn check digit */
bool validateSAID(const string& id) {
    if (id.size() != 13) return false;
    if (!isAllDigits(id)) return false;

    int sum = 0;
    bool alternate = false;
    for (int i = 12; i >= 0; --i) {
        int d = id[i] - '0';
        if (alternate) {
            d *= 2;
            if (d > 9) d -= 9;
        }
        sum += d;
        alternate = !alternate;
    }
    return (sum % 10 == 0);
}

bool validatePhone(const string& phone) {
    return phone.size() == 10 && isAllDigits(phone);
}

bool validateEmail(const string& email) {
    size_t at = email.find('@');
    if (at == string::npos || at == 0) return false;
    size_t dot = email.find('.', at);
    if (dot == string::npos || dot == at + 1 || dot == email.size() - 1) return false;
    if (email.find(' ') != string::npos) return false;
    return true;
}

bool validateDate(const string& date) {
    if (date.size() != 10) return false;
    if (date[2] != '/' || date[5] != '/') return false;

    string dd = date.substr(0, 2);
    string mm = date.substr(3, 2);
    string yyyy = date.substr(6, 4);

    if (!isAllDigits(dd) || !isAllDigits(mm) || !isAllDigits(yyyy)) return false;

    int d = atoi(dd.c_str());
    int m = atoi(mm.c_str());
    int y = atoi(yyyy.c_str());

    if (y < 1900 || y > 2100) return false;
    if (m < 1 || m > 12) return false;

    int daysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (m == 2 && isLeapYear(y)) daysInMonth[1] = 29;

    if (d < 1 || d > daysInMonth[m - 1]) return false;
    return true;
}

/* ============================================================
   CONSOLE INPUT
   ============================================================ */
string readLine(const string& prompt) {
    cout << prompt;
    string s;
    if (!getline(cin, s)) { cin.clear(); return ""; }
    return trim(s);
}

int readInt(const string& prompt) {
    while (true) {
        string s = readLine(prompt);
        stringstream ss(s);
        int v;
        char extra;
        if ((ss >> v) && !(ss >> extra)) return v;
        cout << "  [!] Invalid input. Please enter a whole number.\n";
    }
}

double readDouble(const string& prompt) {
    while (true) {
        string s = readLine(prompt);
        stringstream ss(s);
        double v;
        char extra;
        if ((ss >> v) && !(ss >> extra)) return v;
        cout << "  [!] Invalid input. Please enter a numeric amount.\n";
    }
}

void pause() {
    cout << "\n  Press Enter to continue...";
    string s;
    getline(cin, s);
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/* ============================================================
   STRING HELPERS
   ============================================================ */
string toUpper(const string& s) {
    string r = s;
    for (size_t i = 0; i < r.size(); ++i) r[i] = (char)toupper((unsigned char)r[i]);
    return r;
}

string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

string formatMoney(double amount) {
    stringstream ss;
    ss << "R " << fixed << setprecision(2) << amount;
    return ss.str();
}

string currentDate() {
    time_t now = time(0);
    tm* lt = localtime(&now);
    char buf[16];
    strftime(buf, sizeof(buf), "%d/%m/%Y", lt);
    return string(buf);
}

string currentDateTime() {
    time_t now = time(0);
    tm* lt = localtime(&now);
    char buf[24];
    strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", lt);
    return string(buf);
}

string generatePIN(int digits) {
    string s;
    for (int i = 0; i < digits; ++i) {
        int d = rand() % 10;
        if (i == 0 && d == 0) d = 1 + rand() % 9;   // no leading zero
        s += (char)('0' + d);
    }
    return s;
}

int randomInt(int min, int max) {
    if (max <= min) return min;
    return min + rand() % (max - min + 1);
}

/* ============================================================
   BINARY STRING I/O  (length-prefixed strings for .dat files)
   ============================================================ */
void writeStr(ostream& out, const string& s) {
    size_t len = s.size();
    out.write(reinterpret_cast<const char*>(&len), sizeof(len));
    if (len > 0) out.write(s.data(), len);
}

void readStr(istream& in, string& s) {
    size_t len = 0;
    in.read(reinterpret_cast<char*>(&len), sizeof(len));
    if (!in) { s.clear(); return; }
    if (len > 100000) { s.clear(); in.setstate(ios::failbit); return; }
    s.resize(len);
    if (len > 0) in.read(&s[0], len);
}

} // namespace Utils
