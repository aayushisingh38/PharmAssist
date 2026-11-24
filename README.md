# PharmAssist: Pharmaceutical Decision Support System

A web-based pharmaceutical decision-support tool built **entirely in C++** using the [Crow](https://crowcpp.org) web framework, with **MySQL** for data and **HTML/CSS** for the interface.

It combines pharmacy knowledge (BPharm) with software development: the C++ program is both the application logic and the web server.

> **Disclaimer:** PharmAssist is an educational project. Its dataset is a small demonstration set and has not been clinically validated. Do not use it for real clinical decisions.

---

## Features

| Module | What it does |
|---|---|
| **Dose Calculation** | Calculates single and daily dose from patient age, weight, route and frequency. Handles weight-based (mg/kg) and fixed dosing, and raises a warning when the dose exceeds the maximum safe limit. |
| **Drug Information** | Shows category, mechanism of action, uses, dosage form, route and side effects for a drug. |
| **Drug Interaction Checker** | Checks two drugs for a known interaction (either order) and shows severity and details. |
| **Contraindication Checker** | Checks a drug against a patient condition using partial matching (typing "ulcer" finds "Peptic ulcer disease"). |
| **Search History** | Logs every dose calculation and shows the 20 most recent, with warnings highlighted. |

---

## Tech Stack

- **C++**: application logic, calculations, decision rules
- **Crow**: header-only C++ web framework (routing, HTTP requests and responses)
- **MySQL** with the **X DevAPI** (`mysqlx/xdevapi.h`): database access from C++
- **HTML and CSS**: page structure and styling
- **vcpkg**: installs Crow and MySQL Connector/C++ for Visual Studio

---

## How It Works

Every module follows the same journey:

```
Browser  →  Crow route (C++)  →  MySQL query  →  decision / calculation  →  HTML result page  →  Browser
```

The 13 routes in `main.cpp` follow just three patterns:

- **Pattern A: file-serving routes.** Read an HTML/CSS file from disk and return it.
- **Pattern B: form-handling routes.** Read form input, query MySQL, decide, build an HTML result and return it.
- **Pattern C: the history route.** Like Pattern B, but loops over many rows to build a table.

Core logic of the dose calculator:

```cpp
if (isWeightBased) {
    singleDose = dosePerKg * weight;      // e.g. paediatric paracetamol, 15 mg/kg
} else {
    singleDose = fixedDose;               // e.g. adult paracetamol, 1000 mg
}
bool flagged = (singleDose > maxSingle) || (totalDailyDose > maxDaily);
```

The medical knowledge lives in the database as data, not hard-coded in C++.

---

## Project Structure

```
PharmAssist/                 # repository root
├── PharmAssist/
│   └── main.cpp             # Crow server, all routes and logic
├── Public/                  # HTML and CSS served by Crow
│   ├── home.html / home.css
│   ├── dose-calculator.html / dose-calculator.css
│   ├── drug-info.html / drug-info.css
│   ├── drug-interaction.html
│   └── contraindications.html
├── PharmAssist.slnx         # Visual Studio solution
└── screenshots/             # screenshots here
```

The SQL scripts (`schema.sql`, `drug-info.sql`, `interactions.sql`, `contraindications.sql`) create the database tables and sample data.

---

## Setup (Windows, Visual Studio)

1. **Install** Visual Studio (Desktop development with C++), MySQL Server and MySQL Workbench.
2. **Install libraries with vcpkg:**
   ```
   vcpkg integrate install
   vcpkg install crow:x64-windows
   vcpkg install mysql-connector-cpp:x64-windows
   ```
3. **Create the database:** in MySQL Workbench, run `schema.sql`, then `drug-info.sql`, `interactions.sql` and `contraindications.sql`. Run each only once.
4. **Configure** the MySQL password in `main.cpp` (`Session session("localhost", 33060, "<your password>")`) and make sure the file paths in `readFile(...)` match where you saved the `Public/` folder.
5. **Build and run** in Visual Studio (`Ctrl+Shift+B`, then `Ctrl+F5`) with the build set to **x64**.
6. Open **http://localhost:18080** in your browser.

---

## Future Enhancements

- Normalise the database with `drug_id` foreign keys across all tables
- Expand the drug dataset using verified sources (DrugBank, FDA labels, BNF)
- Case-insensitive and typo-tolerant search
- More administration routes (IV, topical) and finer age bands (neonate, infant, child, adolescent, adult, geriatric)
- Log all five modules to search history, then add history analytics
- User login with personal history
- Object-oriented refactor (`DatabaseManager`, `DoseCalculator` classes)

---

## Author

**Aayushi Singh**: MSc Bioinformatics. Interested in computer-aided drug design and Bioinformatics.
