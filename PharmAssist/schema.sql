-- ============================================
-- PharmAssist - Paracetamol Module
-- Database Schema + Seed Data
-- ============================================

CREATE DATABASE IF NOT EXISTS pharmassist;
USE pharmassist;

-- ---------- Table: drugs ----------
CREATE TABLE IF NOT EXISTS drugs (
    drug_id INT PRIMARY KEY AUTO_INCREMENT,
    drug_name VARCHAR(100) NOT NULL,
    generic_name VARCHAR(100),
    standard_unit VARCHAR(20) DEFAULT 'mg'
);

-- ---------- Table: dosing_rules ----------
CREATE TABLE IF NOT EXISTS dosing_rules (
    rule_id INT PRIMARY KEY AUTO_INCREMENT,
    drug_id INT NOT NULL,
    age_group VARCHAR(20) NOT NULL,      -- 'pediatric' or 'adult'
    route VARCHAR(20) NOT NULL,          -- 'oral', 'IV', etc.
    dose_per_kg DECIMAL(6,2),            -- used for weight-based dosing
    fixed_dose DECIMAL(6,2),             -- used for flat adult dosing
    max_single_dose DECIMAL(6,2) NOT NULL,
    max_daily_dose_per_kg DECIMAL(6,2),  -- pediatric ceiling
    max_daily_dose_fixed DECIMAL(6,2),   -- adult ceiling
    dose_interval_hours INT NOT NULL,
    notes VARCHAR(255)
);

-- ---------- Table: search_history ----------
CREATE TABLE IF NOT EXISTS search_history (
    query_id INT PRIMARY KEY AUTO_INCREMENT,
    drug_id INT NOT NULL,
    weight DECIMAL(5,2),
    age INT,
    route VARCHAR(20),
    calculated_single_dose DECIMAL(6,2),
    calculated_daily_dose DECIMAL(6,2),
    flag_triggered BOOLEAN DEFAULT FALSE,
    query_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- ---------- Seed Data: Paracetamol ----------
INSERT INTO drugs (drug_name, generic_name, standard_unit)
VALUES ('Paracetamol', 'Acetaminophen', 'mg');

-- Assume drug_id = 1 after insert (first row)

-- Pediatric oral rule: 15 mg/kg per dose, every 6 hrs, max daily 60 mg/kg (cap 4000mg)
INSERT INTO dosing_rules
    (drug_id, age_group, route, dose_per_kg, fixed_dose,
     max_single_dose, max_daily_dose_per_kg, max_daily_dose_fixed,
     dose_interval_hours, notes)
VALUES
    (1, 'pediatric', 'oral', 15.00, NULL,
     500.00, 60.00, NULL,
     6, 'Do not exceed 4000mg/day absolute cap even if weight-based calc is higher');

-- Adult oral rule: fixed 500-1000mg per dose, every 6 hrs, max daily 4000mg
INSERT INTO dosing_rules
    (drug_id, age_group, route, dose_per_kg, fixed_dose,
     max_single_dose, max_daily_dose_per_kg, max_daily_dose_fixed,
     dose_interval_hours, notes)
VALUES
    (1, 'adult', 'oral', NULL, 1000.00,
     1000.00, NULL, 4000.00,
     6, 'Standard adult dosing, hepatic impairment requires dose reduction');
