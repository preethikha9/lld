/* 
Requirements
1. should be able to park car, bike (abstract class vehicle)
2. multiple floors
3. multiple entry and exit 
4. cash / card while exit 
5. displayboard on each floor 
6. payment class for calculating payment 
7. admin and Attendant


objects associated with this requirements 
1. Vehicle (Abstract)
    Car (inherits Vehicle)
    Bike (inherits Vehicle)
2. Parking Spot  (Abstract)
    Car
    Bike 
3. Floor
4. DisplayBoard
5. Gate  (Abstract)
    Entry 
    Exit
6. Payment
7. User
    Admin
    Attendant
8. Parking Lot (Singleton class)
*/

#include <iostream>
#include <vector>
#include <ctime>
#include <string>
#include <algorithm>
#include <climits>
#include <unistd.h>
using namespace std;

// ------------------ Vehicle ------------------
class Vehicle {
protected:
    string licensePlate;
    time_t entryTime;
    double costPerHour;
public:
    Vehicle(string plate, double cost) : licensePlate(plate), costPerHour(cost) {
        entryTime = time(nullptr);
    }
    virtual string getType() = 0;
    string getLicensePlate() { return licensePlate; }
    time_t getEntryTime() { return entryTime; }
    double getCostPerHour() { return costPerHour; }
    virtual ~Vehicle() {}
};

class Car : public Vehicle {
public:
    Car(string plate) : Vehicle(plate, 50.0) {}
    string getType() override { return "Car"; }
};

class Bike : public Vehicle {
public:
    Bike(string plate) : Vehicle(plate, 20.0) {}
    string getType() override { return "Bike"; }
};

// ------------------ Display Board ------------------
class DisplayBoard {
    int availableCarSpots;
    int availableBikeSpots;
public:
    void update(int car, int bike) {
        availableCarSpots = car;
        availableBikeSpots = bike;
    }
    void show() {
        cout << "Available Car Spots: " << availableCarSpots << endl;
        cout << "Available Bike Spots: " << availableBikeSpots << endl;
    }
};

// ------------------ Parking Spot ------------------
class ParkingSpot {
protected:
    int spotId;
    int distanceFromGate;
    bool isOccupied;
    Vehicle* vehicle;
public:
    ParkingSpot(int id, int dist) : spotId(id), distanceFromGate(dist), isOccupied(false), vehicle(nullptr) {}
    virtual string getSpotType() = 0;
    bool isFree() { return !isOccupied; }
    void assignVehicle(Vehicle* v) { vehicle = v; isOccupied = true; }
    void removeVehicle() { vehicle = nullptr; isOccupied = false; }
    Vehicle* getVehicle() { return vehicle; }
    int getSpotId() { return spotId; }
    int getDistanceFromGate() { return distanceFromGate; }
    virtual ~ParkingSpot() {}
};

class CarSpot : public ParkingSpot {
public:
    CarSpot(int id, int dist) : ParkingSpot(id, dist) {}
    string getSpotType() override { return "Car"; }
};

class BikeSpot : public ParkingSpot {
public:
    BikeSpot(int id, int dist) : ParkingSpot(id, dist) {}
    string getSpotType() override { return "Bike"; }
};

// ------------------ Floor ------------------
class Floor {
    int floorNumber;
    vector<ParkingSpot*> carSpots;
    vector<ParkingSpot*> bikeSpots;
    DisplayBoard* displayBoard;
public:
    Floor(int num, vector<pair<int, int>> carSpotInfo, vector<pair<int, int>> bikeSpotInfo) : floorNumber(num) {
        for (auto& info : carSpotInfo)
            carSpots.push_back(new CarSpot(info.first, info.second));
        for (auto& info : bikeSpotInfo)
            bikeSpots.push_back(new BikeSpot(info.first, info.second));
        displayBoard = new DisplayBoard();
        updateDisplayBoard();
    }

    ParkingSpot* getNearestFreeSpot(string type) {
        vector<ParkingSpot*>* targetSpots = (type == "Car") ? &carSpots : &bikeSpots;
        vector<ParkingSpot*> freeSpots;

        for (auto& s : *targetSpots) {
            if (s->isFree()) freeSpots.push_back(s);
        }

        if (freeSpots.empty()) return nullptr;

        sort(freeSpots.begin(), freeSpots.end(), [](ParkingSpot* a, ParkingSpot* b) {
            return a->getDistanceFromGate() < b->getDistanceFromGate();
        });

        return freeSpots.front();
    }

    void updateDisplayBoard() {
        int freeCars = 0, freeBikes = 0;
        for (auto& s : carSpots) if (s->isFree()) freeCars++;
        for (auto& s : bikeSpots) if (s->isFree()) freeBikes++;
        displayBoard->update(freeCars, freeBikes);
    }

    void showDisplayBoard() {
        cout << "Floor " << floorNumber << ":\n";
        displayBoard->show();
    }

    vector<ParkingSpot*> getAllSpots() {
        vector<ParkingSpot*> all = carSpots;
        all.insert(all.end(), bikeSpots.begin(), bikeSpots.end());
        return all;
    }
};

// ------------------ Gate ------------------
class Gate {
protected:
    int gateId;
public:
    Gate(int id) : gateId(id) {}
    virtual void processVehicle(Vehicle* vehicle) = 0;
};

class EntryGate : public Gate {
public:
    EntryGate(int id) : Gate(id) {}
    void processVehicle(Vehicle* vehicle) override {
        cout << "Vehicle with plate " << vehicle->getLicensePlate() << " entered through Gate " << gateId << endl;
    }
};

class ExitGate : public Gate {
public:
    ExitGate(int id) : Gate(id) {}
    void processVehicle(Vehicle* vehicle) override {
        cout << "Vehicle with plate " << vehicle->getLicensePlate() << " exited through Gate " << gateId << endl;
    }
};

// ------------------ Payment ------------------
class Payment {
public:
    double calculateFee(time_t entry, time_t exit, double ratePerHour) {
        double hours = difftime(exit, entry) / 3600.0;
        return max(1.0, hours) * ratePerHour;
    }

    void processPayment(string mode, double amount) {
        cout << "Payment of Rs. " << amount << " received via " << mode << endl;
    }
};

// ------------------ User ------------------
class User {
protected:
    string name;
    int id;
public:
    virtual string getRole() = 0;
    virtual ~User() {}
};

class Admin : public User {
public:
    string getRole() override { return "Admin"; }
};

class Attendant : public User {
public:
    string getRole() override { return "Attendant"; }
};

// ------------------ ParkingLot ------------------
class ParkingLot {
    static ParkingLot* instance;
    vector<Floor*> floors;
    vector<Gate*> entryGates;
    vector<Gate*> exitGates;

    ParkingLot() {}
public:
    static ParkingLot* getInstance() {
        if (!instance) instance = new ParkingLot();
        return instance;
    }

    void addFloor(Floor* f) { floors.push_back(f); }
    void addGate(Gate* g) {
        if (dynamic_cast<EntryGate*>(g)) entryGates.push_back(g);
        else exitGates.push_back(g);
    }

    ParkingSpot* assignSpot(Vehicle* v) {
        ParkingSpot* nearest = nullptr;
        int minDistance = INT_MAX;

        for (auto& f : floors) {
            ParkingSpot* s = f->getNearestFreeSpot(v->getType());
            if (s && s->getDistanceFromGate() < minDistance) {
                nearest = s;
                minDistance = s->getDistanceFromGate();
            }
        }

        if (nearest) {
            nearest->assignVehicle(v);
            for (auto& f : floors) f->updateDisplayBoard();
        }
        return nearest;
    }

    void freeSpot(ParkingSpot* s) {
        Vehicle* v = s->getVehicle();
        Payment p;
        time_t exitTime = time(nullptr);
        double fee = p.calculateFee(v->getEntryTime(), exitTime, v->getCostPerHour());
        p.processPayment("Cash", fee); // You can change to "Card" if needed
        s->removeVehicle();
        for (auto& f : floors) f->updateDisplayBoard();
    }

    void showStatus() {
        for (auto& f : floors) f->showDisplayBoard();
    }
};

ParkingLot* ParkingLot::instance = nullptr;

// ------------------ Main ------------------
int main() {
    ParkingLot* lot = ParkingLot::getInstance();

    // Each pair: {spotId, distanceFromGate}
    Floor* floor1 = new Floor(1, {{1, 10}, {2, 15}}, {{3, 5}, {4, 12}});
    Floor* floor2 = new Floor(2, {{5, 8}}, {{6, 9}});
    lot->addFloor(floor1);
    lot->addFloor(floor2);

    EntryGate* eg = new EntryGate(1);
    ExitGate* xg = new ExitGate(2);
    lot->addGate(eg);
    lot->addGate(xg);

    Vehicle* v1 = new Car("TN01AB1234");
    Vehicle* v2 = new Bike("TN01XY9876");

    eg->processVehicle(v1);
    ParkingSpot* s1 = lot->assignSpot(v1);
    eg->processVehicle(v2);
    ParkingSpot* s2 = lot->assignSpot(v2);

    lot->showStatus();

    sleep(2); // simulate parking duration

    xg->processVehicle(v1);
    lot->freeSpot(s1);
    xg->processVehicle(v2);
    lot->freeSpot(s2);

    lot->showStatus();
    return 0;
}
