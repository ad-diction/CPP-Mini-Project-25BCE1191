/*
 * Project Title : Hospital Patient + Visit Tracker – "Clinic Register"
 * Student Name  : [Your Name]
 * Register No   : [Your Reg No]
 * Department    : [Your Department]
 * Course        : Object-Oriented Programming with C++
 * Faculty       : dinakaran.m@vit.ac.in
 *
 * Concepts Used : OOP (classes, constructors, encapsulation),
 *                 STL (vector, string), fstream file handling,
 *                 Input validation, Menu-driven system
 * Compiled with : em++ main.cpp -o index.html  (WebAssembly / Emscripten)
 *                 g++ main.cpp -o clinic        (local GCC)
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
using namespace std;

/* ============================================================
   CONSTANTS
   ============================================================ */
const string PATIENT_FILE = "patients.txt";
const string VISIT_FILE   = "visits.txt";

/* ============================================================
   CLASS: Patient
   Stores basic registration details for one patient.
   All data members are private; accessed via public functions.
   ============================================================ */
class Patient {
private:
    int    patientID;   /* Unique identifier              */
    string name;        /* Full name                      */
    int    age;         /* Age in years                   */
    string phone;       /* 10-digit contact number        */

public:
    /* Constructor – initialises all fields at object creation */
    Patient(int id, const string& n, int a, const string& ph)
        : patientID(id), name(n), age(a), phone(ph) {}

    /* Getters – read-only access to private members */
    int    getID()    const { return patientID; }
    string getName()  const { return name; }
    int    getAge()   const { return age; }
    string getPhone() const { return phone; }

    /* Display patient card to console */
    void display() const {
        cout << "  Patient ID : " << patientID  << "\n"
             << "  Name       : " << name        << "\n"
             << "  Age        : " << age         << "\n"
             << "  Phone      : " << phone       << "\n";
    }

    /* Serialise to one CSV line for file storage */
    string toFileLine() const {
        return to_string(patientID) + "," + name + "," +
               to_string(age) + "," + phone;
    }

    /* Factory method – parse one CSV line back into a Patient */
    static Patient fromFileLine(const string& line) {
        stringstream ss(line);
        string tok;
        int    id, age;
        string name, phone;

        getline(ss, tok, ','); id    = stoi(tok);
        getline(ss, name, ',');
        getline(ss, tok, ','); age   = stoi(tok);
        getline(ss, phone);

        return Patient(id, name, age, phone);
    }
};

/* ============================================================
   CLASS: Visit
   Records one clinic visit for a specific patient.
   ============================================================ */
class Visit {
private:
    int    patientID;    /* Links visit to a Patient        */
    string date;         /* Format: DD-MM-YYYY              */
    string diagnosis;    /* Doctor's diagnosis note         */
    string prescription; /* Prescribed medicines / advice   */

public:
    /* Constructor */
    Visit(int pid, const string& d, const string& diag, const string& pres)
        : patientID(pid), date(d), diagnosis(diag), prescription(pres) {}

    /* Getters */
    int    getPatientID()    const { return patientID; }
    string getDate()         const { return date; }
    string getDiagnosis()    const { return diagnosis; }
    string getPrescription() const { return prescription; }

    /* Display one visit record */
    void display() const {
        cout << "    Date         : " << date         << "\n"
             << "    Diagnosis    : " << diagnosis    << "\n"
             << "    Prescription : " << prescription << "\n";
    }

    /* Serialise – pipe '|' separates fields inside prescription */
    string toFileLine() const {
        /* Replace any '|' inside text fields to avoid parse errors */
        string safeDiag = diagnosis;    replace(safeDiag.begin(), safeDiag.end(), '|', ' ');
        string safePres = prescription; replace(safePres.begin(), safePres.end(), '|', ' ');
        return to_string(patientID) + "|" + date + "|" +
               safeDiag + "|" + safePres;
    }

    /* Factory method */
    static Visit fromFileLine(const string& line) {
        stringstream ss(line);
        string tok, date, diag, pres;
        int pid;

        getline(ss, tok,  '|'); pid  = stoi(tok);
        getline(ss, date, '|');
        getline(ss, diag, '|');
        getline(ss, pres);

        return Visit(pid, date, diag, pres);
    }
};

/* ============================================================
   CLASS: HospitalSystem
   Orchestrates patients and visits; owns all business logic.
   Uses vector<Patient> and vector<Visit> as STL containers.
   ============================================================ */
class HospitalSystem {
private:
    vector<Patient> patients; /* In-memory patient registry  */
    vector<Visit>   visits;   /* In-memory visit log         */
    int             nextID;   /* Auto-increment patient ID   */

    /* ---- private helpers ---------------------------------- */

    /* Find index of patient by ID; returns -1 if not found */
    int findPatientIndex(int id) const {
        for (int i = 0; i < (int)patients.size(); i++)
            if (patients[i].getID() == id) return i;
        return -1;
    }

    /* Check whether an ID is already registered */
    bool idExists(int id) const {
        return findPatientIndex(id) != -1;
    }

    /* Validate phone: exactly 10 numeric digits */
    bool validPhone(const string& ph) const {
        if (ph.size() != 10) return false;
        for (char c : ph)
            if (c < '0' || c > '9') return false;
        return true;
    }

    /* Validate date: basic DD-MM-YYYY format check */
    bool validDate(const string& dt) const {
        if (dt.size() != 10) return false;
        if (dt[2] != '-' || dt[5] != '-') return false;
        for (int i = 0; i < 10; i++) {
            if (i == 2 || i == 5) continue;
            if (dt[i] < '0' || dt[i] > '9') return false;
        }
        int day   = stoi(dt.substr(0, 2));
        int month = stoi(dt.substr(3, 2));
        int year  = stoi(dt.substr(6, 4));
        return day >= 1 && day <= 31 &&
               month >= 1 && month <= 12 &&
               year >= 2000;
    }

    /* Read a non-empty trimmed line from stdin */
    string readLine(const string& prompt) const {
        string line;
        cout << prompt;
        cout.flush();
        getline(cin, line);
        /* trim trailing whitespace */
        while (!line.empty() && (line.back() == '\r' || line.back() == ' '))
            line.pop_back();
        return line;
    }

    /* Read integer; returns true on success */
    bool readInt(const string& prompt, int& out) const {
        string line = readLine(prompt);
        try {
            out = stoi(line);
            return true;
        } catch (...) {
            return false;
        }
    }

public:
    /* Constructor – loads data from files */
    HospitalSystem() : nextID(1001) {
        loadPatients();
        loadVisits();
    }

    /* ---- FILE I/O ----------------------------------------- */

    void loadPatients() {
        ifstream fin(PATIENT_FILE);
        if (!fin.is_open()) return;
        string line;
        while (getline(fin, line)) {
            if (line.empty()) continue;
            try {
                Patient p = Patient::fromFileLine(line);
                patients.push_back(p);
                if (p.getID() >= nextID) nextID = p.getID() + 1;
            } catch (...) { /* skip malformed lines */ }
        }
        fin.close();
        cout << "[Info] Loaded " << patients.size() << " patient(s).\n";
    }

    void savePatients() const {
        ofstream fout(PATIENT_FILE);
        if (!fout.is_open()) {
            cout << "[Error] Cannot save patients file.\n";
            return;
        }
        for (const Patient& p : patients)
            fout << p.toFileLine() << "\n";
        fout.close();
    }

    void loadVisits() {
        ifstream fin(VISIT_FILE);
        if (!fin.is_open()) return;
        string line;
        while (getline(fin, line)) {
            if (line.empty()) continue;
            try {
                visits.push_back(Visit::fromFileLine(line));
            } catch (...) {}
        }
        fin.close();
        cout << "[Info] Loaded " << visits.size() << " visit record(s).\n";
    }

    void saveVisits() const {
        ofstream fout(VISIT_FILE);
        if (!fout.is_open()) {
            cout << "[Error] Cannot save visits file.\n";
            return;
        }
        for (const Visit& v : visits)
            fout << v.toFileLine() << "\n";
        fout.close();
    }

    /* ---- OPERATION 1: Register Patient -------------------- */
    void registerPatient() {
        cout << "\n--- REGISTER NEW PATIENT ---\n";

        int id;
        if (!readInt("Enter Patient ID (number): ", id)) {
            cout << "[Error] Invalid ID. Must be a number.\n";
            return;
        }
        if (id <= 0) {
            cout << "[Error] Patient ID must be positive.\n";
            return;
        }
        if (idExists(id)) {
            cout << "[Error] Patient ID " << id << " is already registered.\n";
            return;
        }

        string name = readLine("Enter Patient Name      : ");
        if (name.empty()) {
            cout << "[Error] Name cannot be empty.\n";
            return;
        }

        int age;
        if (!readInt("Enter Age               : ", age) || age <= 0 || age > 130) {
            cout << "[Error] Invalid age.\n";
            return;
        }

        string phone = readLine("Enter Phone (10 digits) : ");
        if (!validPhone(phone)) {
            cout << "[Error] Phone must be exactly 10 numeric digits.\n";
            return;
        }

        patients.push_back(Patient(id, name, age, phone));
        if (id >= nextID) nextID = id + 1;
        savePatients();
        cout << "[Success] Patient registered with ID " << id << ".\n";
    }

    /* ---- OPERATION 2: Add Visit Record -------------------- */
    void addVisit() {
        cout << "\n--- ADD VISIT RECORD ---\n";

        int id;
        if (!readInt("Enter Patient ID: ", id)) {
            cout << "[Error] Invalid ID.\n";
            return;
        }
        int idx = findPatientIndex(id);
        if (idx == -1) {
            cout << "[Error] No patient found with ID " << id
                 << ". Register patient first.\n";
            return;
        }

        cout << "\n  Patient found: ";
        patients[idx].display();

        string date = readLine("\nEnter Date (DD-MM-YYYY) : ");
        if (!validDate(date)) {
            cout << "[Error] Invalid date format. Use DD-MM-YYYY "
                    "(e.g., 05-04-2026).\n";
            return;
        }

        string diag = readLine("Enter Diagnosis         : ");
        if (diag.empty()) {
            cout << "[Error] Diagnosis cannot be empty.\n";
            return;
        }

        string pres = readLine("Enter Prescription Note : ");
        if (pres.empty()) pres = "None";

        visits.push_back(Visit(id, date, diag, pres));
        saveVisits();
        cout << "[Success] Visit recorded for patient " << id << ".\n";
    }

    /* ---- OPERATION 3: Search Patient + Visit History ------ */
    void searchPatient() const {
        cout << "\n--- SEARCH PATIENT ---\n";

        int id;
        if (!readInt("Enter Patient ID: ", id)) {
            cout << "[Error] Invalid ID.\n";
            return;
        }
        int idx = findPatientIndex(id);
        if (idx == -1) {
            cout << "[Error] Patient ID " << id << " not found.\n";
            return;
        }

        cout << "\n========== PATIENT RECORD ==========\n";
        patients[idx].display();

        /* Collect all visits for this patient */
        int count = 0;
        for (const Visit& v : visits) {
            if (v.getPatientID() == id) {
                if (count == 0) cout << "\n  --- Visit History ---\n";
                cout << "  Visit #" << (++count) << ":\n";
                v.display();
                cout << "\n";
            }
        }
        if (count == 0)
            cout << "  [No visit records found for this patient]\n";
        else
            cout << "  Total visits: " << count << "\n";
        cout << "====================================\n";
    }

    /* ---- REPORT 1: Frequent Visitors (visits > N) --------- */
    void reportFrequentVisitors() const {
        cout << "\n--- FREQUENT VISITOR REPORT ---\n";

        int n;
        if (!readInt("Show patients with more than how many visits? : ", n)
            || n < 0) {
            cout << "[Error] Invalid number.\n";
            return;
        }

        bool found = false;
        for (const Patient& p : patients) {
            int cnt = 0;
            for (const Visit& v : visits)
                if (v.getPatientID() == p.getID()) cnt++;
            if (cnt > n) {
                cout << "\n  Patient ID " << p.getID()
                     << " | " << p.getName()
                     << " | Visits: " << cnt << "\n";
                found = true;
            }
        }
        if (!found)
            cout << "  No patients found with more than " << n << " visit(s).\n";
    }

    /* ---- REPORT 2: Visits in a given month (MM-YYYY) ------ */
    void reportMonthlyVisits() const {
        cout << "\n--- MONTHLY VISIT REPORT ---\n";
        string month = readLine("Enter month to check (MM-YYYY, e.g. 04-2026): ");
        if (month.size() != 7 || month[2] != '-') {
            cout << "[Error] Invalid format. Use MM-YYYY.\n";
            return;
        }

        int count = 0;
        for (const Visit& v : visits) {
            /* Date stored as DD-MM-YYYY; characters 3-9 = MM-YYYY */
            if (v.getDate().size() == 10 &&
                v.getDate().substr(3) == month) {
                count++;
            }
        }
        cout << "  Total visits recorded in " << month << ": " << count << "\n";
    }

    /* ---- OPERATION 4: List All Patients ------------------- */
    void listAllPatients() const {
        cout << "\n--- ALL REGISTERED PATIENTS ---\n";
        if (patients.empty()) {
            cout << "  [No patients registered yet]\n";
            return;
        }
        for (const Patient& p : patients) {
            cout << "  ----------------------------------------\n";
            p.display();
        }
        cout << "  ----------------------------------------\n";
        cout << "  Total: " << patients.size() << " patient(s)\n";
    }
};

/* ============================================================
   MAIN – menu-driven entry point
   ============================================================ */
int main() {
    cout << "\n============================================\n";
    cout << "   CLINIC REGISTER – Patient & Visit Tracker\n";
    cout << "============================================\n";

    HospitalSystem hs;

    int choice;
    do {
        cout << "\n--- MAIN MENU ---\n";
        cout << "1. Register New Patient\n";
        cout << "2. Add Visit Record\n";
        cout << "3. Search Patient & Visit History\n";
        cout << "4. List All Patients\n";
        cout << "5. Report: Frequent Visitors (visits > N)\n";
        cout << "6. Report: Total Visits This Month\n";
        cout << "0. Exit\n";
        cout << "Enter choice: ";
        cout.flush();

        string line;
        getline(cin, line);
        try { choice = stoi(line); }
        catch (...) { choice = -1; }

        switch (choice) {
            case 1: hs.registerPatient();        break;
            case 2: hs.addVisit();               break;
            case 3: hs.searchPatient();          break;
            case 4: hs.listAllPatients();        break;
            case 5: hs.reportFrequentVisitors(); break;
            case 6: hs.reportMonthlyVisits();   break;
            case 0: cout << "\nGoodbye!\n";      break;
            default: cout << "[Error] Invalid choice. Try again.\n";
        }
    } while (choice != 0);

    return 0;
}
