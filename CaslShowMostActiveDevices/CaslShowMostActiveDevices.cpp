#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include "sqlite/sqlite3.h"
#include <filesystem>
#include <Windows.h>

using namespace std;
namespace fs = std::filesystem;

wstring getWideStringPath(const fs::path& path) {
    return path.wstring();
}

string convertWideStringToString(const wstring& wstr) {
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (bufferSize == 0) {
        throw runtime_error("Error converting wide string to UTF-8 string");
    }
    vector<char> buffer(bufferSize);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, buffer.data(), bufferSize, nullptr, nullptr);
    return string(buffer.data());
}

int main()
{
    vector<string> tables = { "event", "event_converted", "event_d128", "event_dozor", "event_sia", "event_vbd4" };

    wstring wideDatabasePath = getWideStringPath(fs::current_path() / "data.db");
    string databasePath = convertWideStringToString(wideDatabasePath);

    if (!fs::exists(wideDatabasePath))
    {
        wcout << L"Database file does not exist at path: " << wideDatabasePath << endl;
        wcin.get();
        return 1;
    }

    sqlite3* db;
    int rc = sqlite3_open(databasePath.c_str(), &db);

    if (rc)
    {
        cout << "Can't open database: " << sqlite3_errmsg(db) << endl;
        return rc;
    }
    else
    {
        cout << "Connected to DB!" << endl;
    }

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    for (const auto& tableName : tables)
    {
        string countQuery = "SELECT COUNT(DISTINCT " + tableName + ".device_id) "
            "FROM " + tableName + " "
            "LEFT JOIN device ON device.device_id = " + tableName + ".device_id;";

        sqlite3_stmt* countStmt;
        rc = sqlite3_prepare_v2(db, countQuery.c_str(), -1, &countStmt, nullptr);

        if (rc != SQLITE_OK)
        {
            cout << "Failed to prepare count statement for table " << tableName << ": " << sqlite3_errmsg(db) << endl;
            continue;
        }

        int rowCount = 0;
        if (sqlite3_step(countStmt) == SQLITE_ROW)
        {
            rowCount = sqlite3_column_int(countStmt, 0);
        }

        sqlite3_finalize(countStmt);

        int limit = (rowCount < 30) ? rowCount : 30;
        if (limit == 0) continue;

        string query = "SELECT device.number as ppk_number, COUNT(" + tableName + ".device_id) as messages "
            "FROM " + tableName + " "
            "LEFT JOIN device ON device.device_id = " + tableName + ".device_id "
            "GROUP BY " + tableName + ".device_id "
            "ORDER BY messages DESC LIMIT " + to_string(limit) + ";";

        sqlite3_stmt* stmt;
        rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);

        if (rc != SQLITE_OK)
        {
            cout << "Failed to prepare statement for table " << tableName << ": " << sqlite3_errmsg(db) << endl;
            continue;
        }

        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE);
        cout << "\n" << string(3, ' ') << limit << " most active devices in table \"" << tableName << "\":\n";
        SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << string(10, ' ') << left << setw(20) << "Device number" << setw(15) << "Messages" << endl;
        cout << setfill('-') << setw(50) << "-" << setfill(' ') << endl;
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            string deviceNumber = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            int messageCount = sqlite3_column_int(stmt, 1);

            SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            cout << string(10, ' ') << left << setw(20) << deviceNumber << setw(15) << messageCount << endl;
        }
        cout << setfill('-') << setw(50) << "-" << setfill(' ') << endl;

        if (rc != SQLITE_DONE)
        {
            cout << "Error selecting from table " << tableName << ": " << sqlite3_errmsg(db) << endl;
        }

        sqlite3_finalize(stmt);
    }

    sqlite3_close(db);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    cout << "\nDisconnected from DB!" << endl;
    
    cout << "\nPress Enter to exit...";
    cin.ignore();
    cin.get();
    return 0;
}

// NO COLORS, SIMPLE STRUCTURE

//#include <iostream>
//#include <vector>
//#include <string>
//#include "sqlite/sqlite3.h"
//#include <filesystem>
//#include <Windows.h>
//
//using namespace std;
//namespace fs = std::filesystem;
//
//wstring getWideStringPath(const fs::path& path) {
//    return path.wstring();
//}
//
//string convertWideStringToString(const wstring& wstr) {
//    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
//    if (bufferSize == 0) {
//        throw runtime_error("Error converting wide string to UTF-8 string");
//    }
//    vector<char> buffer(bufferSize);
//    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, buffer.data(), bufferSize, nullptr, nullptr);
//    return string(buffer.data());
//}
//
//int main()
//{
//    vector<string> tables = { "event", "event_converted", "event_d128", "event_dozor", "event_sia", "event_vbd4" };
//
//    wstring wideDatabasePath = getWideStringPath(fs::current_path() / "data.db");
//    string databasePath = convertWideStringToString(wideDatabasePath);
//
//    if (!fs::exists(wideDatabasePath))
//    {
//        wcout << L"Database file does not exist at path: " << wideDatabasePath << endl;
//        wcin.get();
//        return 1;
//    }
//
//    sqlite3* db;
//    int rc = sqlite3_open(databasePath.c_str(), &db);
//
//    if (rc)
//    {
//        cout << "Can't open database: " << sqlite3_errmsg(db) << endl;
//        return rc;
//    }
//    else
//    {
//        cout << "Connected to DB!" << endl;
//    }
//
//    for (const auto& tableName : tables)
//    {
//        // Count messages in each table
//        string countQuery = "SELECT COUNT(DISTINCT " + tableName + ".device_id) "
//            "FROM " + tableName + " "
//            "LEFT JOIN device ON device.device_id = " + tableName + ".device_id;";
//
//        sqlite3_stmt* countStmt;
//        rc = sqlite3_prepare_v2(db, countQuery.c_str(), -1, &countStmt, nullptr);
//
//        if (rc != SQLITE_OK)
//        {
//            cout << "Failed to prepare count statement for table " << tableName << ": " << sqlite3_errmsg(db) << endl;
//            continue;
//        }
//
//        int rowCount = 0;
//        if (sqlite3_step(countStmt) == SQLITE_ROW)
//        {
//            rowCount = sqlite3_column_int(countStmt, 0);
//        }
//
//        sqlite3_finalize(countStmt);
//
//        int limit = (rowCount < 30) ? rowCount : 30;
//        if (limit == 0) continue;
//
//        string query = "SELECT device.number as ppk_number, COUNT(" + tableName + ".device_id) as messages "
//            "FROM " + tableName + " "
//            "LEFT JOIN device ON device.device_id = " + tableName + ".device_id "
//            "GROUP BY " + tableName + ".device_id "
//            "ORDER BY messages DESC LIMIT " + to_string(limit) + ";";
//
//        sqlite3_stmt* stmt;
//        rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
//
//        if (rc != SQLITE_OK)
//        {
//            cout << "Failed to prepare statement for table " << tableName << ": " << sqlite3_errmsg(db) << endl;
//            continue;
//        }
//
//        cout << "\n" << limit << " most active devices in table " << tableName << ":\n";
//
//        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
//        {
//            string deviceNumber = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
//            int messageCount = sqlite3_column_int(stmt, 1);
//
//            cout << "Device number: " << deviceNumber << ", \tMessages: " << messageCount << endl;
//        }
//
//        if (rc != SQLITE_DONE)
//        {
//            cout << "Error selecting from table " << tableName << ": " << sqlite3_errmsg(db) << endl;
//        }
//
//        sqlite3_finalize(stmt);
//    }
//
//    sqlite3_close(db);
//    cout << "\nDisconnected from DB!" << endl;
//
//    cout << "\nPress Enter to exit...";
//    cin.get();
//    return 0;
//}