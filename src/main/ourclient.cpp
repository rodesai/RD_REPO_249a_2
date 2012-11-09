#include <string>
#include <ostream>
#include <iostream>
#include <string>
#include "rep/Instance.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;

int main(int argc, char *argv[]) {
    // create manager
    Ptr<Instance::Manager> manager = shippingInstanceManager();
    if (manager == NULL) {
        cerr << "Unexpected NULL manager." << endl;
        return 1;
    }

    // create stats
    Ptr<Instance> stats = manager->instanceNew("myStats", "Stats");
    if (stats == NULL) {
        cerr << "Unexpected NULL stats." << endl;
        return 1;
    }

    // create fleet and fleet attributes
    Ptr<Instance> fleet = manager->instanceNew("myFleet", "Fleet");
    if (fleet == NULL) {
        cerr << "Unexpected NULL fleet." << endl;
        return 1;
    }
    fleet->attributeIs("Boat, speed", "10.1");
    fleet->attributeIs("Boat, capacity", "20");
    fleet->attributeIs("Boat, cost", "30.1");
    fleet->attributeIs("Truck, speed", "14.1");
    fleet->attributeIs("Truck, capacity", "24");
    fleet->attributeIs("Truck, cost", "34.1");
    fleet->attributeIs("Plane, speed", "15.1");
    fleet->attributeIs("Plane, capacity", "25");
    fleet->attributeIs("Plane, cost", "35.1");

    // create conn
    Ptr<Instance> conn = manager->instanceNew("myConn", "Conn");
    if (conn == NULL) {
        cerr << "Unexpected NULL conn." << endl;
        return 1;
    }

    // set up locations
    Ptr<Instance> loc1 = manager->instanceNew("loc1", "Plane terminal");
    Ptr<Instance> loc2 = manager->instanceNew("loc2", "Customer");
    Ptr<Instance> loc3 = manager->instanceNew("loc3", "Customer");
    Ptr<Instance> loc4 = manager->instanceNew("loc4", "Truck terminal");

    // set up segments
    Ptr<Instance> sega1 = manager->instanceNew("sega1", "Plane segment");
    Ptr<Instance> sega2 = manager->instanceNew("sega2", "Plane segment");
    Ptr<Instance> segb1 = manager->instanceNew("segb1", "Plane segment");
    Ptr<Instance> segb2 = manager->instanceNew("segb2", "Plane segment");
    Ptr<Instance> segc1 = manager->instanceNew("segc1", "Truck segment");
    Ptr<Instance> segc2 = manager->instanceNew("segc2", "Truck segment");
    Ptr<Instance> segd1 = manager->instanceNew("segd1", "Truck segment");
    Ptr<Instance> segd2 = manager->instanceNew("segd2", "Truck segment");
    Ptr<Instance> sege1 = manager->instanceNew("sege1", "Truck segment");
    Ptr<Instance> sege2 = manager->instanceNew("sege2", "Truck segment");

    // add return segments
    sega1->attributeIs("return segment", "sega2");
    segb1->attributeIs("return segment", "segb2");
    segc1->attributeIs("return segment", "segc2");
    segd1->attributeIs("return segment", "segd2");
    sege1->attributeIs("return segment", "sege2");

    // set up lengths
    sega1->attributeIs("length", "350");
    sega2->attributeIs("length", "350");
    segb1->attributeIs("length", "350");
    segb2->attributeIs("length", "350");
    segc1->attributeIs("length", "100");
    segc2->attributeIs("length", "120");
    segd1->attributeIs("length", "180");
    segd2->attributeIs("length", "180");
    sege1->attributeIs("length", "200");
    sege2->attributeIs("length", "195");

    // set up difficulties
    sega1->attributeIs("difficulty", "1.15");
    sega2->attributeIs("difficulty", "2.15");
    segb1->attributeIs("difficulty", "3.15");
    segb2->attributeIs("difficulty", "2.15");
    segc1->attributeIs("difficulty", "4.15");
    segc2->attributeIs("difficulty", "2.15");
    segd1->attributeIs("difficulty", "4.15");
    segd2->attributeIs("difficulty", "2.15");
    sege1->attributeIs("difficulty", "1.15");
    sege2->attributeIs("difficulty", "1.15");

    // set up expedite support
    sega1->attributeIs("expedite support", "yes");
    sega2->attributeIs("expedite support", "no");
    segb1->attributeIs("expedite support", "no");
    segb2->attributeIs("expedite support", "yes");
    segc1->attributeIs("expedite support", "no");
    segc2->attributeIs("expedite support", "no");
    segd1->attributeIs("expedite support", "no");
    segd2->attributeIs("expedite support", "yes");
    sege1->attributeIs("expedite support", "yes");
    sege2->attributeIs("expedite support", "no");

    // set sources
    sega1->attributeIs("source", "loc1");
    sega2->attributeIs("source", "loc2");
    segb1->attributeIs("source", "loc1");
    segb2->attributeIs("source", "loc3");
    segc1->attributeIs("source", "loc2");
    segc2->attributeIs("source", "loc3");
    segd1->attributeIs("source", "loc3");
    segd2->attributeIs("source", "loc4");
    sege1->attributeIs("source", "loc2");
    sege2->attributeIs("source", "loc4");

    // print stats
    cout << "\n*** Stats\n";
    cout << "Truck segments:      " << stats->attribute("Truck segment")<< "\n";
    cout << "Plane segments:      " << stats->attribute("Plane segment")<< "\n";
    cout << "Boat segments:       " << stats->attribute("Boat segment")<< "\n";

    cout << "Expedite percentage: " << stats->attribute("expedite percentage") << "\n";

    cout << "Truck terminals:     " << stats->attribute("Truck terminal")<< "\n";
    cout << "Boat terminals:      " << stats->attribute("Boat terminal")<< "\n";
    cout << "Plane terminals:     " << stats->attribute("Plane terminal")<< "\n";
    cout << "Customers:           " << stats->attribute("Customer")<< "\n";
    cout << "Ports:               " << stats->attribute("Port")<< "\n";


    // try conn
    cout << "\n*** Explore loc1\n";
    cout << conn->attribute("explore loc1: ");

    cout << "\n*** Explore loc1: expedited\n";
    cout << conn->attribute("explore loc1: expedited");

    cout << "\n*** Conn loc1 loc2\n";
    cout << conn->attribute("connect loc1: loc2");

    cout << "\n*** Conn loc1 loc3\n";
    cout << conn->attribute("connect loc1: loc3");

    cout << "\n*** Conn loc1 loc4\n";
    cout << conn->attribute("connect loc1: loc4");

    return 0;
}
