# Parking Management System (C)

A console-based Parking Management System written in C that supports vehicle entry registration, fee management by vehicle type, and an admin panel to monitor parked vehicles, exit vehicles, and view parking history. Data is persisted locally using a binary file so records remain available after restarting the program.

---

## Features

### User (Customer) Module
- Register a vehicle entry with:
  - Vehicle type (Bike / Car / Bus / Truck)
  - Vehicle number
- Automatically stores:
  - Entry time
  - Entry fee (based on vehicle type)
- Prevents duplicate active entries for the same vehicle number

### Admin Module
- Admin login authentication
- View **currently parked vehicles**, including:
  - Vehicle type
  - Vehicle number
  - Entry time
  - Entry fee
- Process vehicle exit:
  - Stores exit time
  - Calculates total fee
- View complete record history (all entries/exits)
- Search records by vehicle number

---

## Fee Structure

### Entry Fee (by vehicle type)
- Bike: 20
- Car: 50
- Bus: 100
- Truck: 120

### Hourly Rate (by vehicle type)
- Bike: 10/hour
- Car: 25/hour
- Bus: 50/hour
- Truck: 60/hour

### Total Fee Calculation
By default, the system calculates:

**Total Fee = Entry Fee + (Hourly Rate × Hours Parked)**

- Parking hours are **rounded up** to the next full hour.
- Minimum charge is **1 hour**.

> If you want total fee to be **entry fee only**, you can modify `computeTotalFee()` to return only `entryFee`.

---

## Project Structure

This project is implemented as a **single C file**:

- `parking.c` — main source code  
- `parking_records.dat` — auto-generated binary database file (created after first run)

---

## Requirements

- C compiler (GCC recommended)
- Works on:
  - Windows (MinGW)
  - Linux
  - macOS

---

## How to Compile and Run

### Linux / macOS
```bash
gcc parking.c -o parking
./parking
