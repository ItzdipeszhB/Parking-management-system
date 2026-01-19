#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_RECORDS 1000
#define VEH_NO_LEN 20
#define FILE_NAME "parking_records.dat"

// ---- Admin credentials (change these) ----
#define ADMIN_USER "dipesh"
#define ADMIN_PASS "1234"

// ---- Vehicle types ----
typedef enum {
    BIKE = 1,
    CAR = 2,
    BUS = 3,
    TRUCK = 4
} VehicleType;

typedef struct {
    int id;
    VehicleType type;
    char vehicleNo[VEH_NO_LEN];

    time_t entryTime;
    time_t exitTime;       // 0 means still parked

    double entryFee;
    double totalFee;       // computed at exit
} Record;

static Record records[MAX_RECORDS];
static int recordCount = 0;

// ---- Utility: fees by vehicle type ----
double getEntryFee(VehicleType type) {
    switch (type) {
        case BIKE:  return 20.0;
        case CAR:   return 50.0;
        case BUS:   return 100.0;
        case TRUCK: return 120.0;
        default:    return 0.0;
    }
}

// Optional: hourly rate by vehicle type (for total fee)
double getHourlyRate(VehicleType type) {
    switch (type) {
        case BIKE:  return 10.0;
        case CAR:   return 25.0;
        case BUS:   return 50.0;
        case TRUCK: return 60.0;
        default:    return 0.0;
    }
}

const char* typeToString(VehicleType type) {
    switch (type) {
        case BIKE: return "Bike";
        case CAR: return "Car";
        case BUS: return "Bus";
        case TRUCK: return "Truck";
        default: return "Unknown";
    }
}

void formatTime(time_t t, char *buf, size_t n) {
    if (t == 0) {
        snprintf(buf, n, "N/A");
        return;
    }
    struct tm *tm_info = localtime(&t);
    strftime(buf, n, "%Y-%m-%d %H:%M:%S", tm_info);
}

// ---- File I/O ----
void loadRecords() {
    FILE *fp = fopen(FILE_NAME, "rb");
    if (!fp) {
        recordCount = 0;
        return;
    }
    fread(&recordCount, sizeof(int), 1, fp);
    if (recordCount < 0) recordCount = 0;
    if (recordCount > MAX_RECORDS) recordCount = MAX_RECORDS;

    fread(records, sizeof(Record), recordCount, fp);
    fclose(fp);
}

void saveRecords() {
    FILE *fp = fopen(FILE_NAME, "wb");
    if (!fp) {
        printf("Error: could not save data.\n");
        return;
    }
    fwrite(&recordCount, sizeof(int), 1, fp);
    fwrite(records, sizeof(Record), recordCount, fp);
    fclose(fp);
}

// ---- Basic input helpers ----
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

int readIntSafe(const char *prompt) {
    int x;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &x) == 1) {
            clearInputBuffer();
            return x;
        }
        printf("Invalid input. Try again.\n");
        clearInputBuffer();
    }
}

void readStringSafe(const char *prompt, char *out, size_t n) {
    printf("%s", prompt);
    if (fgets(out, (int)n, stdin)) {
        size_t len = strlen(out);
        if (len > 0 && out[len - 1] == '\n') out[len - 1] = '\0';
    }
}

// ---- Search helpers ----
int findActiveByVehicleNo(const char *vehNo) {
    for (int i = 0; i < recordCount; i++) {
        if (records[i].exitTime == 0 && strcmp(records[i].vehicleNo, vehNo) == 0) {
            return i;
        }
    }
    return -1;
}

int findAnyByVehicleNo(const char *vehNo) {
    for (int i = 0; i < recordCount; i++) {
        if (strcmp(records[i].vehicleNo, vehNo) == 0) return i;
    }
    return -1;
}

int nextId() {
    int maxId = 0;
    for (int i = 0; i < recordCount; i++) {
        if (records[i].id > maxId) maxId = records[i].id;
    }
    return maxId + 1;
}

// ---- Core features ----
void registerEntry() {
    if (recordCount >= MAX_RECORDS) {
        printf("Parking system record limit reached.\n");
        return;
    }

    printf("\n--- Vehicle Entry Registration ---\n");
    printf("Vehicle Types:\n");
    printf("1. Bike  (Entry Fee: %.2f)\n", getEntryFee(BIKE));
    printf("2. Car   (Entry Fee: %.2f)\n", getEntryFee(CAR));
    printf("3. Bus   (Entry Fee: %.2f)\n", getEntryFee(BUS));
    printf("4. Truck (Entry Fee: %.2f)\n", getEntryFee(TRUCK));

    int typeInt = readIntSafe("Select vehicle type (1-4): ");
    if (typeInt < 1 || typeInt > 4) {
        printf("Invalid vehicle type.\n");
        return;
    }

    char vehNo[VEH_NO_LEN];
    readStringSafe("Enter vehicle number (e.g., BA-12-PA-1234): ", vehNo, sizeof(vehNo));
    if (strlen(vehNo) == 0) {
        printf("Vehicle number cannot be empty.\n");
        return;
    }

    // Prevent duplicate active parking for same vehicle number
    if (findActiveByVehicleNo(vehNo) != -1) {
        printf("This vehicle is already parked (active entry exists).\n");
        return;
    }

    Record r;
    r.id = nextId();
    r.type = (VehicleType)typeInt;
    strncpy(r.vehicleNo, vehNo, VEH_NO_LEN - 1);
    r.vehicleNo[VEH_NO_LEN - 1] = '\0';

    r.entryTime = time(NULL);
    r.exitTime = 0;

    r.entryFee = getEntryFee(r.type);
    r.totalFee = 0.0;

    records[recordCount++] = r;
    saveRecords();

    char tbuf[64];
    formatTime(r.entryTime, tbuf, sizeof(tbuf));
    printf("\nEntry registered successfully.\n");
    printf("Ticket ID: %d\n", r.id);
    printf("Vehicle: %s (%s)\n", r.vehicleNo, typeToString(r.type));
    printf("Entry Time: %s\n", tbuf);
    printf("Entry Fee: %.2f\n", r.entryFee);
}

double computeTotalFee(Record *r) {
    // Total Fee = entry fee + hourly rate * hours(rounded up)
    // If you want ONLY entry fee, then just return r->entryFee.
    double entryFee = r->entryFee;
    double rate = getHourlyRate(r->type);

    double seconds = difftime(r->exitTime, r->entryTime);
    if (seconds < 0) seconds = 0;

    // hours rounded up to next hour
    int hours = (int)(seconds / 3600.0);
    if (((int)seconds) % 3600 != 0) hours += 1;
    if (hours < 1) hours = 1; // minimum 1 hour charge

    return entryFee + rate * hours;
}

void listParkedVehicles() {
    printf("\n--- Currently Parked Vehicles ---\n");
    int found = 0;

    printf("%-5s %-10s %-18s %-20s %-12s\n", "ID", "Type", "VehicleNo", "EntryTime", "EntryFee");
    printf("--------------------------------------------------------------------------\n");

    for (int i = 0; i < recordCount; i++) {
        if (records[i].exitTime == 0) {
            found = 1;
            char et[64];
            formatTime(records[i].entryTime, et, sizeof(et));
            printf("%-5d %-10s %-18s %-20s %-12.2f\n",
                   records[i].id, typeToString(records[i].type), records[i].vehicleNo, et, records[i].entryFee);
        }
    }

    if (!found) printf("No vehicles are currently parked.\n");
}

void exitVehicle() {
    printf("\n--- Vehicle Exit ---\n");
    char vehNo[VEH_NO_LEN];
    readStringSafe("Enter vehicle number to exit: ", vehNo, sizeof(vehNo));

    int idx = findActiveByVehicleNo(vehNo);
    if (idx == -1) {
        printf("No active parked record found for this vehicle.\n");
        return;
    }

    records[idx].exitTime = time(NULL);
    records[idx].totalFee = computeTotalFee(&records[idx]);

    saveRecords();

    char et[64], xt[64];
    formatTime(records[idx].entryTime, et, sizeof(et));
    formatTime(records[idx].exitTime, xt, sizeof(xt));

    printf("\nExit processed.\n");
    printf("Ticket ID: %d\n", records[idx].id);
    printf("Vehicle: %s (%s)\n", records[idx].vehicleNo, typeToString(records[idx].type));
    printf("Entry Time: %s\n", et);
    printf("Exit Time:  %s\n", xt);
    printf("Entry Fee:  %.2f\n", records[idx].entryFee);
    printf("Total Fee:  %.2f\n", records[idx].totalFee);
}

void listAllHistory() {
    printf("\n--- All Records (History) ---\n");
    if (recordCount == 0) {
        printf("No records found.\n");
        return;
    }

    printf("%-5s %-10s %-18s %-20s %-20s %-10s %-10s\n",
           "ID", "Type", "VehicleNo", "EntryTime", "ExitTime", "EntryFee", "TotalFee");
    printf("------------------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < recordCount; i++) {
        char et[64], xt[64];
        formatTime(records[i].entryTime, et, sizeof(et));
        formatTime(records[i].exitTime, xt, sizeof(xt));

        printf("%-5d %-10s %-18s %-20s %-20s %-10.2f %-10.2f\n",
               records[i].id, typeToString(records[i].type), records[i].vehicleNo, et, xt,
               records[i].entryFee, records[i].totalFee);
    }
}

void searchVehicle() {
    printf("\n--- Search Vehicle ---\n");
    char vehNo[VEH_NO_LEN];
    readStringSafe("Enter vehicle number to search: ", vehNo, sizeof(vehNo));

    int any = 0;
    for (int i = 0; i < recordCount; i++) {
        if (strcmp(records[i].vehicleNo, vehNo) == 0) {
            any = 1;
            char et[64], xt[64];
            formatTime(records[i].entryTime, et, sizeof(et));
            formatTime(records[i].exitTime, xt, sizeof(xt));

            printf("\nRecord Found:\n");
            printf("ID: %d\n", records[i].id);
            printf("Type: %s\n", typeToString(records[i].type));
            printf("Vehicle No: %s\n", records[i].vehicleNo);
            printf("Entry Time: %s\n", et);
            printf("Exit Time:  %s\n", xt);
            printf("Entry Fee:  %.2f\n", records[i].entryFee);
            printf("Total Fee:  %.2f\n", records[i].totalFee);
        }
    }

    if (!any) printf("No record found for this vehicle number.\n");
}

int adminLogin() {
    char user[50], pass[50];
    printf("\n--- Admin Login ---\n");
    readStringSafe("Username: ", user, sizeof(user));
    readStringSafe("Password: ", pass, sizeof(pass));

    if (strcmp(user, ADMIN_USER) == 0 && strcmp(pass, ADMIN_PASS) == 0) {
        printf("Login successful.\n");
        return 1;
    }
    printf("Login failed.\n");
    return 0;
}

void adminMenu() {
    if (!adminLogin()) return;

    while (1) {
        printf("\n=== ADMIN MENU ===\n");
        printf("1. View currently parked vehicles\n");
        printf("2. Exit a vehicle (set exit time + fee)\n");
        printf("3. View all records (history)\n");
        printf("4. Search vehicle by number\n");
        printf("0. Logout\n");

        int choice = readIntSafe("Enter choice: ");
        switch (choice) {
            case 1: listParkedVehicles(); break;
            case 2: exitVehicle(); break;
            case 3: listAllHistory(); break;
            case 4: searchVehicle(); break;
            case 0: return;
            default: printf("Invalid choice.\n");
        }
    }
}

void userMenu() {
    while (1) {
        printf("\n=== USER MENU ===\n");
        printf("1. Register vehicle entry\n");
        printf("0. Back\n");

        int choice = readIntSafe("Enter choice: ");
        switch (choice) {
            case 1: registerEntry(); break;
            case 0: return;
            default: printf("Invalid choice.\n");
        }
    }
}

int main() {
    loadRecords();

    while (1) {
        printf("\n==============================\n");
        printf("   BANIYA PARKING MANAGEMENT SYSTEM\n");
        printf("==============================\n");
        printf("1. User\n");
        printf("2. Admin\n");
        printf("0. Exit\n");

        int choice = readIntSafe("Enter choice: ");
        switch (choice) {
            case 1: userMenu(); break;
            case 2: adminMenu(); break;
            case 0:
                saveRecords();
                printf("Goodbye.\n");
                return 0;
            default: printf("Invalid choice.\n");
        }
    }
}
