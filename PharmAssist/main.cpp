#include "crow.h"
#include <mysqlx/xdevapi.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace mysqlx;

// ---------- Helper: read a file's full contents as a string ----------
std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main() {
    crow::SimpleApp app;

    // ---------- Route 1: Serve the HTML form ----------
    CROW_ROUTE(app, "/")([]() {
        std::string html = readFile("C:/Projects/PharmAssist/public/index.html");
        crow::response res(html);
        res.set_header("Content-Type", "text/html");
        return res;
        });

    // ---------- Route 2: Serve the CSS file ----------
    CROW_ROUTE(app, "/style.css")([]() {
        std::string css = readFile("C:/Projects/PharmAssist/public/style.css");
        crow::response res(css);
        res.set_header("Content-Type", "text/css");
        return res;
        });

    // ---------- Route 3: Handle form submission and calculate dose ----------
    CROW_ROUTE(app, "/calculate").methods("POST"_method)
        ([](const crow::request& req) {

        // Parse form data sent from the HTML form
        auto params = crow::query_string("?" + req.body);

        double weight = std::stod(params.get("weight"));
        int age = std::stoi(params.get("age"));
        int dosesPerDay = std::stoi(params.get("frequency"));

        std::string ageGroup = (age < 12) ? "pediatric" : "adult";

        double singleDose = 0, maxSingle = 0, maxDaily = 0;
        int intervalHours = 0;
        bool ruleFound = false;

        try {
            // ---------- Connect to MySQL ----------
            Session session("localhost", 33060, "root", "root");
            Schema db = session.getSchema("pharmassist");
            Table rules = db.getTable("dosing_rules");

            RowResult result = rules.select("dose_per_kg", "fixed_dose",
                "max_single_dose", "max_daily_dose_per_kg",
                "max_daily_dose_fixed", "dose_interval_hours")
                .where("drug_id = 1 AND age_group = :ageGroup AND route = 'oral'")
                .bind("ageGroup", ageGroup)
                .execute();

            Row row = result.fetchOne();
            if (row) {
                ruleFound = true;
                bool isWeightBased = !row[0].isNull();

                if (isWeightBased) {
                    double dosePerKg = row[0].get<double>();
                    double maxDailyPerKg = row[3].get<double>();
                    singleDose = dosePerKg * weight;
                    maxDaily = maxDailyPerKg * weight;
                }
                else {
                    singleDose = row[1].get<double>();
                    maxDaily = row[4].get<double>();
                }
                maxSingle = row[2].get<double>();
                intervalHours = row[5].get<int>();
            }
            session.close();
        }
        catch (const mysqlx::Error& err) {
            crow::response res("<h2>Database error: " + std::string(err.what()) + "</h2>");
            res.set_header("Content-Type", "text/html");
            return res;
        }

        if (!ruleFound) {
            crow::response res("<h2>No dosing rule found for this age group.</h2>");
            res.set_header("Content-Type", "text/html");
            return res;
        }

        // ---------- Calculate total daily dose and check limits ----------
        double totalDailyDose = singleDose * dosesPerDay;
        bool singleExceeds = singleDose > maxSingle;
        bool dailyExceeds = totalDailyDose > maxDaily;
        bool flagged = singleExceeds || dailyExceeds;

        // ---------- Build the result HTML ----------
        std::ostringstream html;
        html << "<!DOCTYPE html><html><head><title>Result</title>"
            << "<link rel='stylesheet' href='/style.css'></head><body>"
            << "<div class='container'>"
            << "<h1>PharmAssist</h1>"
            << "<div class='result-card " << (flagged ? "warning" : "") << "'>"
            << "<h3>Dose Calculation Result</h3>"
            << "<p><strong>Drug:</strong> Paracetamol</p>"
            << "<p><strong>Age group:</strong> " << ageGroup << "</p>"
            << "<p><strong>Calculated single dose:</strong> " << singleDose << " mg</p>"
            << "<p><strong>Frequency:</strong> every " << intervalHours << " hrs</p>"
            << "<p><strong>Calculated daily dose:</strong> " << totalDailyDose << " mg</p>"
            << "<p><strong>Max reference single dose:</strong> " << maxSingle << " mg</p>"
            << "<p><strong>Max reference daily dose:</strong> " << maxDaily << " mg</p>";

        if (flagged) {
            html << "<p class='status'>WARNING: Dose exceeds safe limit. Verify before proceeding.</p>";
        }
        else {
            html << "<p class='status'>Within safe limits.</p>";
        }

        html << "</div><p><a href='/'>Calculate another dose</a></p></div></body></html>";

        crow::response res(html.str());
        res.set_header("Content-Type", "text/html");
        return res;
            });

    app.port(18080).multithreaded().run();
}