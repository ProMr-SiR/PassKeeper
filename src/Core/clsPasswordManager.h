#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <vector>
#include <string>
#include "../Lib/clsString.h"

using namespace std;

/**
 * @class clsPasswordManager
 * @brief Core class responsible for managing password entries securely.
 * It handles loading, saving, modifying, and deleting credentials from
 * the ESP32 NVS (Non-Volatile Storage) using the Preferences library.
 */
class clsPasswordManager
{
private:
    // Entry Operational Mode
    enum enMode { EmptyMode = 0, UpdateMode = 1, AddNewMode = 2 };
    enMode _Mode;

    // Account Data Fields
    int _ID;            // Unique identifier for the account
    string _Name;       // Service name (e.g. GitHub)
    string _Username;   // Username or email
    string _Password;   // Plain text password

    /**
     * @brief Parses a delimited string into a clsPasswordManager object.
     * @param Line The delimited string (e.g. "1#//#GitHub#//#test@g.com#//#123").
     * @param Seperator The string separator used between data fields.
     * @return clsPasswordManager object instantiated with the parsed data.
     */
    static clsPasswordManager _ConvertLineToObject(string Line, string Seperator = "#//#")
    {
        vector<string> vData = clsString::Split(Line, Seperator);
        
        // Ensure string is fully formed
        if (vData.size() < 4)
            return _GetEmptyObject();
        
        // Construct the object in UpdateMode representing a loaded entry
        clsPasswordManager obj(enMode::UpdateMode, stoi(vData[0]), vData[1], vData[2], vData[3]);
        return obj;
    }

    /**
     * @brief Serializes a clsPasswordManager object into a single string line.
     * @param Entry The object to serialize.
     * @param Seperator The string separator.
     * @return string The serialized string format.
     */
    static string _ConvertObjectToLine(clsPasswordManager Entry, string Seperator = "#//#")
    {
        string Line = "";
        Line += to_string(Entry.ID()) + Seperator;
        Line += Entry.Name() + Seperator;
        Line += Entry.Username() + Seperator;
        Line += Entry.Password();
        return Line;
    }

    /**
     * @brief Loads all saved credentials from the NVS Preferences.
     * @return vector<clsPasswordManager> A vector of all stored entries.
     */
    static vector<clsPasswordManager> _LoadAllFromPreferences()
    {
        vector<clsPasswordManager> vEntries;
        Preferences prefs;
        
        // Open the 'passkaper' namespace in read-only mode
        prefs.begin("passkaper", true);

        // Retrieve total number of entries

        int count = prefs.getInt("count", 0);

        for (int i = 0; i < count; i++)
        {
            string key = "entry_" + to_string(i);
            string line = prefs.getString(key.c_str(), "").c_str();

            if (!line.empty())
            {
                clsPasswordManager entry = _ConvertLineToObject(line);
                // Exclude empty objects
                if (!entry.IsEmpty()) 
                    vEntries.push_back(entry);
            }
        }
        prefs.end();
        return vEntries;
    }

    /**
     * @brief Overwrites all preferences with the provided vector of entries.
     * @param vEntries The current vector of valid entries to be saved.
     */
    static void _SaveAllToPreferences(const vector<clsPasswordManager>& vEntries)
    {
        Preferences prefs;
        // Open namespace in read/write mode
        prefs.begin("passkaper", false);

        int oldCount = prefs.getInt("count", 0);
        int newCount = vEntries.size();

        // Update the new entry count
        prefs.putInt("count", newCount);

        // Overwrite existing keys with new data
        for (int i = 0; i < newCount; i++) {
            string key = "entry_" + to_string(i);
            string line = _ConvertObjectToLine(vEntries[i]);
            prefs.putString(key.c_str(), line.c_str());
        }

        // Clean up remaining dangling keys from old larger list

        for (int i = newCount; i < oldCount; i++) {
            string key = "entry_" + to_string(i);
            prefs.remove(key.c_str());
        }

        prefs.end();
    }

    /**
     * @brief Helper function to commit a new entry into the preferences.
     */
    void _AddNew()
    {
        vector<clsPasswordManager> vEntries = _LoadAllFromPreferences();
        vEntries.push_back(*this); // Add current object to vector
        _SaveAllToPreferences(vEntries);
    }

    /**
     * @brief Generates an empty dummy object to indicate failed lookups.
     * @return clsPasswordManager Default initialized empty object.
     */
    static clsPasswordManager _GetEmptyObject()
    {
        return clsPasswordManager(enMode::EmptyMode, -1, "", "", "");
    }

public:
    // -------------------------------------------------------------
    // Constructor
    // -------------------------------------------------------------
    clsPasswordManager(enMode Mode, int ID, string Name, string Username, string Password)
        : _Mode(Mode), _ID(ID), _Name(Name), _Username(Username), _Password(Password)
    {
    }

    // -------------------------------------------------------------
    // Getters
    // -------------------------------------------------------------
    int    ID()                const      { return _ID; }
    string Name()              const      { return _Name; }
    string Username()          const      { return _Username; }
    string Password()          const      { return _Password; }

    // -------------------------------------------------------------
    // Setters
    // -------------------------------------------------------------
    void SetName(string Name)         { _Name = Name; }
    void SetUsername(string Username)  { _Username = Username; }
    void SetPassword(string Password) { _Password = Password; }

    /**
     * @brief Checks if the current instance is empty.
     * @return bool True if empty.
     */
    bool IsEmpty() const { return (_Mode == enMode::EmptyMode); }

    // -------------------------------------------------------------
    // Public Operations
    // -------------------------------------------------------------
    enum enSaveResults { svFaildEmptyObject = 0, svSucceeded = 1, svFaildIDExists = 2 };
    
    /**
     * @brief Commits the object state to NVS Storage.
     * Validates mode to determine whether to AddNew or Update existing.
     * @return enSaveResults Status of the save operation.
     */
    enSaveResults Save()
    {
        switch (_Mode)
        {
        case enMode::EmptyMode:
            return svFaildEmptyObject;

        case enMode::UpdateMode:
        {
            // Load existing, find match by ID, update, and save
            vector<clsPasswordManager> vEntries = _LoadAllFromPreferences();
            for (auto& entry : vEntries) {
                if (entry._ID == _ID) {
                    // Update entry properties
                    entry = *this;
                    break;
                }
            }
            _SaveAllToPreferences(vEntries);
            return svSucceeded;
        }

        case enMode::AddNewMode:
        {
            // Verify ID uniqueness before adding
            if (clsPasswordManager::IsEntryExistByID(_ID))
            {
                return svFaildIDExists;
            }
            else
            {
                _AddNew();
                _Mode = enMode::UpdateMode;
                return svSucceeded;
            }
        }
        }
        return svFaildEmptyObject;
    }

    /**
     * @brief Finds a password entry by its Service Name.
     * @param Name The name of the service to look up.
     * @return clsPasswordManager the found object, or empty object if not found.
     */
    static clsPasswordManager FindByName(string Name)
    {
        Name = clsString::UpperAllString(Name);
        vector<clsPasswordManager> vEntries = _LoadAllFromPreferences();
        for (const auto& entry : vEntries) {
            if (clsString::UpperAllString(entry.Name()) == Name) {
                return entry;
            }
        }
        return _GetEmptyObject();
    }

    /**
     * @brief Finds a password entry by its unique ID.
     * @param ID The ID to look up.
     * @return clsPasswordManager the found object, or empty object if not found.
     */
    static clsPasswordManager FindByID(int ID)
    {
        vector<clsPasswordManager> vEntries = _LoadAllFromPreferences();
        for (const auto& entry : vEntries) {
            if (entry.ID() == ID) {
                return entry;
            }
        }
        return _GetEmptyObject();
    }

    static bool IsEntryExistByID(int ID)
    {
        return !FindByID(ID).IsEmpty();
    }

    /**
     * @brief Deletes a password entry permanently from storage.
     * @param ID The ID of the entry to delete.
     */
    static void DeleteEntry(int ID)
    {
        vector<clsPasswordManager> vEntries = _LoadAllFromPreferences();
        for (auto it = vEntries.begin(); it != vEntries.end(); ++it) {
            if (it->ID() == ID) {
                vEntries.erase(it);
                break;
            }
        }
        // Save back the updated list without the deleted entry
        _SaveAllToPreferences(vEntries);
    }

    static vector<clsPasswordManager> GetEntriesList()
    {
        return _LoadAllFromPreferences();
    }

    /**
     * @brief Creates and saves a new password entry to storage.
     * Automatically assigns the next available unique ID.
     */
    static void AddNewEntry(string Name, string Username, string Password)
    {
        // Find the maximum ID to auto-increment
        vector<clsPasswordManager> vEntries = _LoadAllFromPreferences();
        int nextID = 1;
        for (const auto& e : vEntries) {
            if (e.ID() >= nextID) nextID = e.ID() + 1;
        }

        clsPasswordManager newEntry(enMode::AddNewMode, nextID, Name, Username, Password);
        newEntry.Save();
    }

    /**
     * @brief Retrieves a list of unique service names.
     * Useful for building the main menu hierarchy (e.g. grouping accounts by service).
     * @return vector<string> List of unique service names.
     */
    static vector<string> GetUniqueServiceNames()
    {
        vector<clsPasswordManager> vEntries = _LoadAllFromPreferences();
        vector<string> vNames;
        
        for (const auto& entry : vEntries) {
            bool found = false;
            for (const auto& existing : vNames) {
                if (existing == entry.Name()) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                vNames.push_back(entry.Name());
            }
        }
        return vNames;
    }

    /**
     * @brief Retrieves all account entries belonging to a specific service.
     * @param ServiceName The exact name of the service (e.g. GitHub).
     * @return vector<clsPasswordManager> A filtered list of accounts.
     */
    static vector<clsPasswordManager> GetEntriesByService(string ServiceName)
    {
        vector<clsPasswordManager> vAll = _LoadAllFromPreferences();
        vector<clsPasswordManager> vFiltered;
        
        for (const auto& entry : vAll) {
            if (entry.Name() == ServiceName) {
                vFiltered.push_back(entry);
            }
        }
        return vFiltered;
    }
};
