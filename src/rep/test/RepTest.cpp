#include "gtest/gtest.h"
#include "rep/Instance.h"
#include <string>
#include <set>
#include <ostream>
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <sys/wait.h>
#include <sys/signal.h>
#include <boost/lexical_cast.hpp>

/* TODO:
    - make mutators idempotent
    - add tests for
        x- input
            x- cannot specify shipment details to Location
            x- cannot ship to a Location
        - shipping to one customer
            - shipping rate is 0
            x- shipment doesn't start until three criteria are fulfilled
            x- cost is as expected
            x- time is as expected
            x- shipments occur at the correct rate
            x- shipment across long segments
        x- shipping via Location
        x- shipment refusals
        x- scheduling fleet change
            x- if you schedule on half-hour increments, does it still work?
            x- what if you specify a time larger than 24?
                x- should we create a class for that?
        x- more than 24 shipments per day
        - shipping to one customer and change rate
            - increase, decrease, zero
        - two simultaneous shipments to two separate customers
*/

using namespace std;

#define STRINGIFY(a) #a
#define STRING(a) STRINGIFY(a)

#define MAYBETHROW(a) {\
    try {\
        a;\
    } catch (...) {\
    }\
}

TEST(StuffWeFailedTheFirstTime, testErrorInvalidValues) {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    Ptr<Instance> fleet = m->instanceNew("fleet", "Fleet");
    if(!fleet) fleet = m->instance("fleet");
    Ptr<Instance> seg = m->instanceNew("seg", "Plane segment");
    seg->attributeIs("length", "1.0");
    seg->attributeIs("difficulty", "1.0");
    seg->attributeIs("expedite support", "yes");
    fleet->attributeIs("Boat, capacity", "10");
    fleet->attributeIs("Boat, cost", "10");
    fleet->attributeIs("Boat, speed", "10");
    
    /* Try some out-of-range/invalid values */
    MAYBETHROW(seg->attributeIs("length", "-10"));
    MAYBETHROW(seg->attributeIs("difficulty", "0.9"));
    MAYBETHROW(seg->attributeIs("difficulty", "6.0"));
    MAYBETHROW(seg->attributeIs("difficulty", "-1"));
    MAYBETHROW(fleet->attributeIs("Boat, capacity", "-1")); 
    MAYBETHROW(fleet->attributeIs("Boat, cost", "-1")); 
    MAYBETHROW(fleet->attributeIs("Boat, speed", "-1")); 
    MAYBETHROW(seg->attributeIs("expedite support", "whatever"));

    EXPECT_EQ(seg->attribute("length"), "1.00");
    EXPECT_EQ(seg->attribute("difficulty"), "1.00");     
    EXPECT_EQ(seg->attribute("expedite support"), "yes");
    //EQUAL(fleet->attribute("Boat, capacity"), "10");
    EXPECT_EQ("10", fleet->attribute("Boat, capacity"));
    EXPECT_EQ(fleet->attribute("Boat, cost"), "10.00");
    EXPECT_EQ(fleet->attribute("Boat, speed"), "10.00");
}

TEST(Activity, ScheduledFleets) {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
    Ptr<Instance> conn = m->instanceNew("conn", "Conn");
    Ptr<Instance> fleet1 = m->instanceNew("fleet1", "Fleet");
    Ptr<Instance> fleet2 = m->instanceNew("fleet2", "Fleet");
    
    // create locations and join them
    Ptr<Instance> loc1 = m->instanceNew("loc1", "Customer");
    Ptr<Instance> loc2 = m->instanceNew("loc2", "Customer");
    Ptr<Instance> seg1 = m->instanceNew("seg1", "Truck segment");
    Ptr<Instance> seg2 = m->instanceNew("seg2", "Truck segment");
    seg1->attributeIs("source", "loc1");
    seg1->attributeIs("length", "2.0");
    seg1->attributeIs("return segment", "seg2");
    seg2->attributeIs("source", "loc2");
    seg2->attributeIs("length", "1.0");

    // assert that the fleets are, indeed, different
    fleet1->attributeIs("Truck, speed", "0.5");
    fleet2->attributeIs("Truck, speed", "2");
    ASSERT_EQ("0.50", fleet1->attribute("Truck, speed"));
    ASSERT_EQ("2.00", fleet2->attribute("Truck, speed"));

    // try invalid time and check that it is unchanged
    fleet1->attributeIs("Start Time", "24");
    ASSERT_EQ("0.00", fleet1->attribute("Start Time"));
    fleet1->attributeIs("Start Time", "25");
    ASSERT_EQ("0.00", fleet1->attribute("Start Time"));
    fleet1->attributeIs("Start Time", "-1");
    ASSERT_EQ("0.00", fleet1->attribute("Start Time"));

    // schedule changes
    fleet1->attributeIs("Start Time", "0");
    fleet2->attributeIs("Start Time", "12");

    // check current travel time (should be 1)
    EXPECT_EQ("2.00 4.00 no; loc1(seg1:2.00:seg2) loc2\n",
        conn->attribute("connect loc1 : loc2"));

    // move to next time; travel time should be 0.5
    m->simulationManager()->timeIs(13);
    EXPECT_EQ("2.00 1.00 no; loc1(seg1:2.00:seg2) loc2\n",
        conn->attribute("connect loc1 : loc2"));

    // check that it moved back
    m->simulationManager()->timeIs(25);
    EXPECT_EQ("2.00 4.00 no; loc1(seg1:2.00:seg2) loc2\n",
        conn->attribute("connect loc1 : loc2"));
    m->simulationManager()->timeIs(37);
    EXPECT_EQ("2.00 1.00 no; loc1(seg1:2.00:seg2) loc2\n",
        conn->attribute("connect loc1 : loc2"));

    // delay the switch back
    fleet1->attributeIs("Start Time", "5");
    m->simulationManager()->timeIs(49);
    EXPECT_EQ("2.00 1.00 no; loc1(seg1:2.00:seg2) loc2\n",
        conn->attribute("connect loc1 : loc2"));
    m->simulationManager()->timeIs(54);
    EXPECT_EQ("2.00 4.00 no; loc1(seg1:2.00:seg2) loc2\n",
        conn->attribute("connect loc1 : loc2"));
}

TEST(Activity, BasicShipments) {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
    Ptr<Instance> conn = m->instanceNew("conn", "Conn");
    ASSERT_TRUE(conn);

    // set truck capacity, cost, and speed
    Ptr<Instance> fleet = m->instanceNew("fleet", "Fleet");
    ASSERT_TRUE(fleet);
    fleet->attributeIs("Truck, speed", "1");
    fleet->attributeIs("Truck, capacity", "10");
    fleet->attributeIs("Truck, cost", "100");

    // create two customers and join them
    Ptr<Instance> loc1 = m->instanceNew("loc1", "Customer");
    Ptr<Instance> loc2 = m->instanceNew("loc2", "Customer");
    Ptr<Instance> seg1 = m->instanceNew("seg1", "Truck segment");
    Ptr<Instance> seg2 = m->instanceNew("seg2", "Truck segment");
    seg1->attributeIs("source", "loc1");
    seg1->attributeIs("length", "1.0");
    seg1->attributeIs("return segment", "seg2");
    seg2->attributeIs("source", "loc2");
    seg2->attributeIs("length", "1.5");
    
    // connectivity
    std::cout << conn->attribute("explore loc1");
    conn->attributeIs("routing", "minHops");

    // check defaults
    EXPECT_EQ("0", loc1->attribute("Transfer Rate"));
    EXPECT_EQ("0", loc1->attribute("Shipment Size"));
    EXPECT_EQ("", loc1->attribute("Destination"));
    EXPECT_EQ("10", seg1->attribute("Capacity"));

    // ensure that no packages have arrived
    EXPECT_EQ("0", loc2->attribute("Shipments Received"));
    EXPECT_EQ("0.00", loc2->attribute("Average Latency"));
    EXPECT_EQ("0.00", loc2->attribute("Total Cost"));

    // specify shipping criteria
    loc1->attributeIs("Transfer Rate", "8");
    loc1->attributeIs("Shipment Size", "10");
    loc1->attributeIs("Destination", "loc2");

    // ensure that no packages have arrived since we haven't set time
    EXPECT_EQ("0", loc2->attribute("Shipments Received"));
    EXPECT_EQ("0.00", loc2->attribute("Average Latency"));
    EXPECT_EQ("0.00", loc2->attribute("Total Cost"));

    // with speed of 1 and length 1, only one shipment should have arrived
    m->simulationManager()->timeIs(4);

    // check that one shipment has arrived
    ASSERT_EQ("1", loc2->attribute("Shipments Received"));
    ASSERT_EQ("1.00", loc2->attribute("Average Latency"));
    ASSERT_EQ("100.00", loc2->attribute("Total Cost"));

    // check segment stats
    EXPECT_EQ("1", seg1->attribute("Shipments Received"));
    EXPECT_EQ("0", seg1->attribute("Shipments Refused"));

    // update "now" and test again
    m->simulationManager()->timeIs(7);
    ASSERT_EQ("2", loc2->attribute("Shipments Received"));
    ASSERT_EQ("1.00", loc2->attribute("Average Latency"));
    ASSERT_EQ("200.00", loc2->attribute("Total Cost"));

    // check segment stats
    EXPECT_EQ("2", seg1->attribute("Shipments Received"));
    EXPECT_EQ("0", seg1->attribute("Shipments Refused"));

    // move forward to next day and stop shipments
    m->simulationManager()->timeIs(25);
    loc1->attributeIs("Transfer Rate", "0");
    m->simulationManager()->timeIs(28);
    EXPECT_EQ("8", seg1->attribute("Shipments Received"));

    // ensure that no more shipments are sent
    m->simulationManager()->timeIs(48);
    EXPECT_EQ("8", seg1->attribute("Shipments Received"));

    // add shipments back
    loc1->attributeIs("Transfer Rate", "8");
    m->simulationManager()->timeIs(73);
    EXPECT_EQ("16", seg1->attribute("Shipments Received"));

}

TEST(Activity, ShipThroughTerminal) {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
    Ptr<Instance> conn = m->instanceNew("conn", "Conn");
    ASSERT_TRUE(conn);

    // set truck capacity, cost, and speed
    Ptr<Instance> fleet = m->instanceNew("fleet", "Fleet");
    ASSERT_TRUE(fleet);
    fleet->attributeIs("Truck, speed", "1");
    fleet->attributeIs("Truck, capacity", "10");
    fleet->attributeIs("Truck, cost", "100");

    // create two customers and join them
    Ptr<Instance> loc1 = m->instanceNew("loc1", "Customer");
    Ptr<Instance> loc2 = m->instanceNew("loc2", "Truck terminal");
    Ptr<Instance> loc3 = m->instanceNew("loc3", "Customer");
    Ptr<Instance> seg1 = m->instanceNew("seg1", "Truck segment");
    Ptr<Instance> seg2 = m->instanceNew("seg2", "Truck segment");
    Ptr<Instance> seg3 = m->instanceNew("seg3", "Truck segment");
    Ptr<Instance> seg4 = m->instanceNew("seg4", "Truck segment");
    seg1->attributeIs("source", "loc1");
    seg1->attributeIs("length", "1.0");
    seg1->attributeIs("return segment", "seg2");
    seg2->attributeIs("source", "loc2");
    seg2->attributeIs("length", "1.5");
    seg3->attributeIs("source", "loc2");
    seg3->attributeIs("length", "1.0");
    seg3->attributeIs("return segment", "seg4");
    seg4->attributeIs("source", "loc3");
    seg4->attributeIs("length", "1.5");
    
    // connectivity
    conn->attributeIs("routing", "minHops");

    // specify shipping criteria
    loc1->attributeIs("Transfer Rate", "8");
    loc1->attributeIs("Shipment Size", "10");

    // this should error
    loc1->attributeIs("Destination", "loc2");
    EXPECT_EQ("", loc1->attribute("Destination"));
    loc1->attributeIs("Destination", "loc3");

    // with speed of 1 and length 1, only one shipment should have arrived
    m->simulationManager()->timeIs(5);

    // check that one shipment has arrived
    ASSERT_EQ("1", loc3->attribute("Shipments Received"));
    ASSERT_EQ("2.00", loc3->attribute("Average Latency"));
    ASSERT_EQ("200.00", loc3->attribute("Total Cost"));

    // check segment stats
    EXPECT_EQ("1", seg1->attribute("Shipments Received"));
    EXPECT_EQ("0", seg1->attribute("Shipments Refused"));

    // update "now" and test again
    m->simulationManager()->timeIs(8);
    ASSERT_EQ("2", loc3->attribute("Shipments Received"));
    ASSERT_EQ("2.00", loc3->attribute("Average Latency"));
    ASSERT_EQ("400.00", loc3->attribute("Total Cost"));

    // check segment stats
    EXPECT_EQ("2", seg1->attribute("Shipments Received"));
    EXPECT_EQ("0", seg1->attribute("Shipments Refused"));
}

TEST(Activity, ShipWithQueue) {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
    Ptr<Instance> conn = m->instanceNew("conn", "Conn");
    ASSERT_TRUE(conn);

    // set truck capacity, cost, and speed
    Ptr<Instance> fleet = m->instanceNew("fleet", "Fleet");
    ASSERT_TRUE(fleet);
    fleet->attributeIs("Truck, speed", "1");
    fleet->attributeIs("Truck, capacity", "10");
    fleet->attributeIs("Truck, cost", "100");

    // create two customers and join them
    Ptr<Instance> loc1 = m->instanceNew("loc1", "Customer");
    Ptr<Instance> loc2 = m->instanceNew("loc2", "Customer");
    Ptr<Instance> seg1 = m->instanceNew("seg1", "Truck segment");
    Ptr<Instance> seg2 = m->instanceNew("seg2", "Truck segment");
    seg1->attributeIs("source", "loc1");
    seg1->attributeIs("length", "1.0");
    seg1->attributeIs("return segment", "seg2");
    seg1->attributeIs("Capacity", "1");
    seg2->attributeIs("source", "loc2");
    seg2->attributeIs("length", "1.5");
    
    // connectivity
    conn->attributeIs("routing", "minHops");

    // specify shipping criteria
    loc1->attributeIs("Transfer Rate", "24");
    loc1->attributeIs("Shipment Size", "100");
    loc1->attributeIs("Destination", "loc2");

    // with speed of 1 and length 1, only one shipment should have arrived
    m->simulationManager()->timeIs(11);

    // check that one shipment has arrived
    EXPECT_EQ("1", loc2->attribute("Shipments Received"));
    EXPECT_EQ("10.00", loc2->attribute("Average Latency"));
    EXPECT_EQ("1000.00", loc2->attribute("Total Cost"));

    /* One shipment should be received and delivered. Another should have JUST
     * arrived, but not be delivered. In the meantime, 10 shipments should be
     * forced to queue up.
     */ 
    EXPECT_EQ("2", seg1->attribute("Shipments Received"));
    EXPECT_EQ("10", seg1->attribute("Shipments Refused"));
}

TEST(Activity, MultipleCarriers) {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
    Ptr<Instance> conn = m->instanceNew("conn", "Conn");
    ASSERT_TRUE(conn);

    // set truck capacity, cost, and speed
    Ptr<Instance> fleet = m->instanceNew("fleet", "Fleet");
    ASSERT_TRUE(fleet);
    fleet->attributeIs("Truck, speed", "1");
    fleet->attributeIs("Truck, capacity", "10");
    fleet->attributeIs("Truck, cost", "100");

    // create two customers and join them
    Ptr<Instance> loc1 = m->instanceNew("loc1", "Customer");
    Ptr<Instance> loc2 = m->instanceNew("loc2", "Customer");
    Ptr<Instance> seg1 = m->instanceNew("seg1", "Truck segment");
    Ptr<Instance> seg2 = m->instanceNew("seg2", "Truck segment");
    seg1->attributeIs("source", "loc1");
    seg1->attributeIs("length", "1.0");
    seg1->attributeIs("return segment", "seg2");
    seg1->attributeIs("Capacity", "10");
    seg2->attributeIs("source", "loc2");
    seg2->attributeIs("length", "1.5");
    
    // connectivity
    std::cout<< conn->attribute("explore loc1");
    conn->attributeIs("routing", "minHops");

    // specify shipping criteria
    loc1->attributeIs("Transfer Rate", "24");
    loc1->attributeIs("Shipment Size", "100");
    loc1->attributeIs("Destination", "loc2");

    // with speed of 1 and length 1, only one shipment should have arrived
    m->simulationManager()->timeIs(2);

    // check that one shipment has arrived
    EXPECT_EQ("1", loc2->attribute("Shipments Received"));
    EXPECT_EQ("1.00", loc2->attribute("Average Latency"));
    EXPECT_EQ("1000.00", loc2->attribute("Total Cost"));

    EXPECT_EQ("2", seg1->attribute("Shipments Received"));
    EXPECT_EQ("0", seg1->attribute("Shipments Refused"));
}

TEST(Activity, HighRate) {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
    Ptr<Instance> conn = m->instanceNew("conn", "Conn");
    ASSERT_TRUE(conn);

    // set truck capacity, cost, and speed
    Ptr<Instance> fleet = m->instanceNew("fleet", "Fleet");
    ASSERT_TRUE(fleet);
    fleet->attributeIs("Truck, speed", "1");
    fleet->attributeIs("Truck, capacity", "10");
    fleet->attributeIs("Truck, cost", "100");

    // create two customers and join them
    Ptr<Instance> loc1 = m->instanceNew("loc1", "Customer");
    Ptr<Instance> loc2 = m->instanceNew("loc2", "Customer");
    Ptr<Instance> seg1 = m->instanceNew("seg1", "Truck segment");
    Ptr<Instance> seg2 = m->instanceNew("seg2", "Truck segment");
    seg1->attributeIs("source", "loc1");
    seg1->attributeIs("length", "1.0");
    seg1->attributeIs("return segment", "seg2");
    seg1->attributeIs("Capacity", "50");
    seg2->attributeIs("source", "loc2");
    seg2->attributeIs("length", "1.5");
    
    // connectivity
    conn->attributeIs("routing", "minHops");

    // specify shipping criteria
    loc1->attributeIs("Transfer Rate", "50");
    loc1->attributeIs("Shipment Size", "10");
    loc1->attributeIs("Destination", "loc2");

    // all the previous day's shipments should have arrived.
    m->simulationManager()->timeIs(25);

    // check that one shipment has arrived
    // this actually depends on a float comparison, so it can vary
    EXPECT_EQ("49", loc2->attribute("Shipments Received"));
    EXPECT_EQ("1.00", loc2->attribute("Average Latency"));
    EXPECT_EQ("4900.00", loc2->attribute("Total Cost"));

    // Not sure how many extra shipments should be received. Probably 2.
    EXPECT_EQ("52", seg1->attribute("Shipments Received"));
    EXPECT_EQ("0", seg1->attribute("Shipments Refused"));
}

TEST(Activity, HighRate2) {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
    Ptr<Instance> conn = m->instanceNew("conn", "Conn");
    ASSERT_TRUE(conn);

    // set truck capacity, cost, and speed
    Ptr<Instance> fleet = m->instanceNew("fleet", "Fleet");
    ASSERT_TRUE(fleet);
    fleet->attributeIs("Truck, speed", "1");
    fleet->attributeIs("Truck, capacity", "1");
    fleet->attributeIs("Truck, cost", "100");

    // create two customers and join them
    Ptr<Instance> loc1 = m->instanceNew("loc1", "Customer");
    Ptr<Instance> loc2 = m->instanceNew("loc2", "Truck terminal");
    Ptr<Instance> loc3 = m->instanceNew("loc3", "Customer");
    Ptr<Instance> seg1 = m->instanceNew("seg1", "Truck segment");
    Ptr<Instance> seg2 = m->instanceNew("seg2", "Truck segment");
    Ptr<Instance> seg3 = m->instanceNew("seg3", "Truck segment");
    Ptr<Instance> seg4 = m->instanceNew("seg4", "Truck segment");
    seg1->attributeIs("source", "loc1");
    seg2->attributeIs("source", "loc2");
    seg3->attributeIs("source", "loc2");
    seg4->attributeIs("source", "loc3");
    seg1->attributeIs("length", "1.0");
    seg2->attributeIs("length", "1.0");
    seg3->attributeIs("length", "1.0");
    seg4->attributeIs("length", "1.0");
    seg1->attributeIs("Capacity", "1");
    seg3->attributeIs("Capacity", "1");
    seg1->attributeIs("return segment", "seg2");
    seg3->attributeIs("return segment", "seg4");
    
    // connectivity
    conn->attributeIs("routing", "minHops");

    // specify shipping criteria
    loc1->attributeIs("Transfer Rate", "24");
    loc1->attributeIs("Shipment Size", "2");
    loc1->attributeIs("Destination", "loc3");

    // start time and test results
    m->simulationManager()->timeIs(24);
    EXPECT_EQ("10", loc3->attribute("Shipments Received"));

    // stop shipments, and wait for the remainder to arrive
    loc1->attributeIs("Transfer Rate", "0");
    m->simulationManager()->timeIs(60);
    EXPECT_EQ("24", loc3->attribute("Shipments Received"));
}

TEST(Activity, LongSegment) {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
    Ptr<Instance> conn = m->instanceNew("conn", "Conn");
    ASSERT_TRUE(conn);

    // set truck capacity, cost, and speed
    Ptr<Instance> fleet = m->instanceNew("fleet", "Fleet");
    ASSERT_TRUE(fleet);
    fleet->attributeIs("Truck, speed", "25");
    fleet->attributeIs("Truck, capacity", "10");
    fleet->attributeIs("Truck, cost", "100");

    // create two customers and join them
    Ptr<Instance> loc1 = m->instanceNew("loc1", "Customer");
    Ptr<Instance> loc2 = m->instanceNew("loc2", "Customer");
    Ptr<Instance> seg1 = m->instanceNew("seg1", "Truck segment");
    Ptr<Instance> seg2 = m->instanceNew("seg2", "Truck segment");
    seg1->attributeIs("source", "loc1");
    seg1->attributeIs("length", "2500.0");
    seg1->attributeIs("return segment", "seg2");
    seg2->attributeIs("source", "loc2");
    seg2->attributeIs("length", "1.5");
    
    // connectivity
    std::cout << conn->attribute("explore loc1");
    conn->attributeIs("routing", "minHops");

    // specify shipping criteria
    loc1->attributeIs("Transfer Rate", "8");
    loc1->attributeIs("Shipment Size", "10");
    loc1->attributeIs("Destination", "loc2");

    // with speed of 1 and length 1, only one shipment should have arrived
    m->simulationManager()->timeIs(104);

    // check that one shipment has arrived
    EXPECT_EQ("1", loc2->attribute("Shipments Received"));
    EXPECT_EQ("100.00", loc2->attribute("Average Latency"));
    EXPECT_EQ("250000.00", loc2->attribute("Total Cost"));
}

Ptr<Instance> addSegment(Ptr<Instance::Manager> m, string name, string mode,
    string source, string returnSegment, string length, string difficulty,
    string expedited) {

    // create segment
    Ptr<Instance> seg = m->instanceNew(name, mode);
    EXPECT_TRUE(seg);

    // set start if value provided
    seg->attributeIs("source", source);
    EXPECT_EQ(seg->attribute("source"), source);

    // set reverse if value provided
    seg->attributeIs("return segment", returnSegment);
    EXPECT_EQ(seg->attribute("return segment"), returnSegment);

    // set length if value provided
    if (length != "") {
        seg->attributeIs("length", length);
        EXPECT_EQ(seg->attribute("length"), length);
    } else {
        // check default
        EXPECT_EQ(seg->attribute("length"), "1.00");
    }

    // set distance if value provided
    if (difficulty != "") {
        seg->attributeIs("difficulty", difficulty);
        EXPECT_EQ(seg->attribute("difficulty"), difficulty);
    } else {
        // check default
        EXPECT_EQ(seg->attribute("difficulty"), "1.00");
    }

    // set distance if value provided
    if (expedited != "") {
        seg->attributeIs("expedite support", expedited);
        EXPECT_EQ(seg->attribute("expedite support"), expedited);
    } else {
        // check default
        EXPECT_EQ(seg->attribute("expedite support"), "no");
    }

    return seg;
}

TEST(Instance, CreateInstanceManager) {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
}

TEST(Instance, InvalidUseOfCustomerAttributes) {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
    Ptr<Instance> loc1 = m->instanceNew("loc1", "Boat terminal");
    ASSERT_TRUE(loc1);
    loc1->attributeIs("Transfer Rate", "10");
    EXPECT_EQ("", loc1->attribute("Transfer Rate"));

    // Create real customer, and try to have it ship to loc1
    Ptr<Instance> loc2 = m->instanceNew("loc2", "Customer");
    loc2->attributeIs("Destination", "loc1");
    EXPECT_EQ("", loc2->attribute("Destination"));

    // Now have the customer ship to another customer
    Ptr<Instance> loc3 = m->instanceNew("loc3", "Customer");
    loc2->attributeIs("Destination", "loc3");
    EXPECT_EQ("loc3", loc2->attribute("Destination"));
}

TEST(Instance, DuplicateObjectsTest) {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
    Ptr<Instance> conn = m->instanceNew("conn", "Conn");
    ASSERT_TRUE(conn);
    Ptr<Instance> stats = m->instanceNew("stats", "Stats");
    ASSERT_TRUE(stats);

    // create dups
    Ptr<Instance> conn1 = m->instanceNew("conn1", "Conn");
    ASSERT_TRUE(conn1);
    Ptr<Instance> stats1 = m->instanceNew("stats1", "Stats");
    ASSERT_TRUE(stats1);

    // assert that they are the same rep objects
    EXPECT_EQ(conn->name(), conn1->name());
    EXPECT_EQ(stats->name(), stats1->name());
}


TEST(Instance, InstanceMapping) {
    // create instances needed for test
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
    Ptr<Instance> seg1 = m->instanceNew("seg1", "Boat segment");
    ASSERT_TRUE(seg1);
    Ptr<Instance> loc1 = m->instanceNew("loc1", "Customer");
    ASSERT_TRUE(loc1);
    Ptr<Instance> fleet = m->instanceNew("fleet", "Fleet");
    ASSERT_TRUE(fleet);
    Ptr<Instance> conn = m->instanceNew("conn", "Conn");
    ASSERT_TRUE(conn);
    Ptr<Instance> stats = m->instanceNew("stats", "Stats");
    ASSERT_TRUE(stats);

    // verify that the instance is mapped
    EXPECT_EQ(m->instance("seg1")->name(), "seg1");
    EXPECT_EQ(m->instance("loc1")->name(), "loc1");
    EXPECT_EQ(m->instance("conn")->name(), "conn");
    EXPECT_EQ(m->instance("stats")->name(), "stats");
    EXPECT_EQ(m->instance("fleet")->name(), "fleet");

    // verify that other instances are not
    EXPECT_FALSE(m->instance("seg11"));
    EXPECT_FALSE(m->instance("loc11"));
    EXPECT_FALSE(m->instance("conn1"));
    EXPECT_FALSE(m->instance("stats1"));
    EXPECT_FALSE(m->instance("fleet1"));
}


TEST(Instance, TypeOutOfBounds) {
    // create instances needed for test
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
    Ptr<Instance> seg1 = m->instanceNew("seg1", "Boat segment");
    ASSERT_TRUE(seg1);
    Ptr<Instance> loc1 = m->instanceNew("loc1", "Boat terminal");
    ASSERT_TRUE(loc1);
    Ptr<Instance> fleet = m->instanceNew("fleet", "Fleet");
    ASSERT_TRUE(fleet);
    Ptr<Instance> conn = m->instanceNew("conn", "Conn");
    ASSERT_TRUE(conn);
    Ptr<Instance> stats = m->instanceNew("stats", "Stats");
    ASSERT_TRUE(stats);

    // test for null name
    ASSERT_FALSE(m->instanceNew("", "Boat segment"));

    // test segment values
    seg1->attributeIs("length", "-1");
    seg1->attributeIs("difficulty", "-1");

    // test fleet values
    fleet->attributeIs("Boat, speed", "-1");
    // this doesn't matter because int wraps around
    fleet->attributeIs("Boat, capacity", "-1");
    fleet->attributeIs("Boat, cost", "-1");

    // test conn values
    EXPECT_EQ(conn->attribute("explore loc1 : expedited distance -1.3 cost -1.2 time -1.1"), "");
    EXPECT_EQ(conn->attribute("explore loc1 : kdkdkdkdkdkdkdkdkdkdkdk distance -1.3 cost -1.2 time -1.1"), "");

    // test stats object
    EXPECT_EQ(stats->attribute("blah"), "");
    EXPECT_EQ(stats->attribute("Plane segment"), "0");
}


TEST(Instance, InstanceDelete) {
    // create instances needed for test
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
    Ptr<Instance> seg1 = m->instanceNew("seg1", "Boat segment");
    ASSERT_TRUE(seg1);
    Ptr<Instance> seg2 = m->instanceNew("seg2", "Boat segment");
    ASSERT_TRUE(seg2);
    seg1->attributeIs("return segment", "seg2");
    ASSERT_EQ(seg2->attribute("return segment"), "seg1");
    Ptr<Instance> loc1 = m->instanceNew("loc1", "Port");
    seg1->attributeIs("source", "loc1");
    ASSERT_EQ(loc1->attribute("segment1"), "seg1");
    Ptr<Instance> stats = m->instanceNew("stats", "Stats");
    ASSERT_TRUE(m->instance("stats"));

    // delete something that does not yet exist
    m->instanceDel("seg-1");

    // delete segment
    m->instanceDel("seg1");
    // segment no longer exists in engine
    ASSERT_FALSE(m->instance("seg1"));
    // segment no longer pointed to by return segment
    EXPECT_EQ(seg2->attribute("return segment"), "");
    // segment no longer pointed to by source
    EXPECT_EQ(loc1->attribute("segment1"), "");
    // can recreate segment by same name
    seg1 = m->instanceNew("seg1", "Boat segment");
    EXPECT_TRUE(m->instance("seg1"));

    // delete location
    seg1->attributeIs("source", "loc1");
    EXPECT_EQ(loc1->attribute("segment1"), "seg1");
    m->instanceDel("loc1");
    // source no longer exists in the engine
    EXPECT_FALSE(m->instance("loc1"));
    // segement no longer points to source
    EXPECT_EQ(seg1->attribute("source"), "");
    // can recreate instance with the same name
    loc1 = m->instanceNew("loc1", "Port");
    EXPECT_TRUE(m->instance("loc1"));

    // test delete of all location types
    Ptr<Instance> boatTerminal = m->instanceNew("boatTerminal", "Boat terminal");
    ASSERT_TRUE(boatTerminal);
    m->instanceDel("boatTerminal");
    EXPECT_FALSE(m->instance("boatTerminal"));
    Ptr<Instance> planeTerminal = m->instanceNew("planeTerminal", "Plane terminal");
    ASSERT_TRUE(planeTerminal);
    m->instanceDel("planeTerminal");
    EXPECT_FALSE(m->instance("planeTerminal"));
    Ptr<Instance> truckTerminal = m->instanceNew("truckTerminal", "Truck terminal");
    ASSERT_TRUE(truckTerminal);
    m->instanceDel("truckTerminal");
    EXPECT_FALSE(m->instance("truckTerminal"));
    Ptr<Instance> customer = m->instanceNew("customer", "Customer");
    ASSERT_TRUE(customer);
    m->instanceDel("customer");
    EXPECT_FALSE(m->instance("customer"));
    Ptr<Instance> port = m->instanceNew("port", "Port");
    ASSERT_TRUE(port);
    m->instanceDel("port");
    EXPECT_FALSE(m->instance("port"));

    // test delete of all segment types
    Ptr<Instance> boatSegment = m->instanceNew("boatSegment", "Boat segment");
    ASSERT_TRUE(boatSegment);
    m->instanceDel("boatSegment");
    EXPECT_FALSE(m->instance("boatSegment"));
    Ptr<Instance> planeSegment = m->instanceNew("planeSegment", "Plane segment");
    ASSERT_TRUE(planeSegment);
    m->instanceDel("planeSegment");
    EXPECT_FALSE(m->instance("planeSegment"));
    Ptr<Instance> truckSegment = m->instanceNew("truckSegment", "Truck segment");
    ASSERT_TRUE(truckSegment);
    m->instanceDel("truckSegment");
    EXPECT_FALSE(m->instance("truckSegment"));
}

TEST(Instance, CreateSegment) {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
    Ptr<Instance> seg1 = addSegment(m, "seg1", "Boat segment", "", "", "", "",
        "");
    ASSERT_TRUE(seg1);

    // check defaults
    EXPECT_EQ(seg1->attribute("length"), "1.00");
    EXPECT_EQ(seg1->attribute("difficulty"), "1.00");
    EXPECT_EQ(seg1->attribute("expedite support"), "no");
    EXPECT_EQ(seg1->attribute("source"), "");
    EXPECT_EQ(seg1->attribute("return segment"), "");

    // test changing of attribute values
    seg1->attributeIs("length", "400.10");
    EXPECT_EQ(seg1->attribute("length"), "400.10");
    seg1->attributeIs("difficulty", "3.20");
    EXPECT_EQ(seg1->attribute("difficulty"), "3.20");
    seg1->attributeIs("difficulty", "3.20");
    EXPECT_EQ(seg1->attribute("difficulty"), "3.20");
    EXPECT_EQ(seg1->attribute("expedite support"), "no");
    seg1->attributeIs("expedite support", "yes");
    EXPECT_EQ(seg1->attribute("expedite support"), "yes");

    // creating a new segment with the same name should fail
    Ptr<Instance> seg2 = m->instanceNew("seg1", "Boat segment");
    ASSERT_FALSE(seg2);

    // adding a nonexistent return segment shouldn't do anything
    seg1->attributeIs("return segment", "seg2");
    EXPECT_EQ(seg1->attribute("return segment"), "");

    // create new segment and set return segment
    seg2 = m->instanceNew("seg2", "Boat segment");
    ASSERT_TRUE(seg2);
    seg1->attributeIs("return segment", "seg2");
    EXPECT_EQ(seg1->attribute("return segment"), "seg2");
    EXPECT_EQ(seg2->attribute("return segment"), "seg1");

    // add segment of the wrong mode
    Ptr<Instance> seg3 = m->instanceNew("seg3", "Plane segment");
    ASSERT_TRUE(seg3);
    seg3->attributeIs("return segment", "seg2");
    EXPECT_EQ(seg2->attribute("return segment"), "seg1");
    EXPECT_EQ(seg3->attribute("return segment"), "");

    // switch expedite segment off
    seg1->attributeIs("expedite support", "no");
    EXPECT_EQ(seg1->attribute("expedite support"), "no");

    // set return segment to empty
    seg1->attributeIs("return segment", "");
    EXPECT_EQ(seg1->attribute("return segment"), "");
    EXPECT_EQ(seg2->attribute("return segment"), "");
}


TEST(Instance, CreateLocation) {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
    Ptr<Instance> loc1 = m->instanceNew("loc1", "Boat terminal");
    ASSERT_TRUE(loc1);

    // check segment that doesn't exist
    EXPECT_EQ(loc1->attribute("segment1"), "");

    // add a segment
    Ptr<Instance> seg1 = m->instanceNew("seg1", "Boat segment");
    ASSERT_TRUE(seg1);
    seg1->attributeIs("source", "loc1");
    EXPECT_EQ(seg1->attribute("source"), "loc1");
    EXPECT_EQ(loc1->attribute("segment1"), "seg1");

    // add a new segment and detach the first
    Ptr<Instance> seg2 = m->instanceNew("seg2", "Boat segment");
    ASSERT_TRUE(seg2);
    seg2->attributeIs("source", "loc1");
    EXPECT_EQ(seg2->attribute("source"), "loc1");
    EXPECT_EQ(loc1->attribute("segment2"), "seg2");
    Ptr<Instance> loc2 = m->instanceNew("loc2", "Boat terminal");
    ASSERT_TRUE(loc2);
    seg1->attributeIs("source", "loc2");
    EXPECT_EQ(seg1->attribute("source"), "loc2");
    EXPECT_EQ(loc2->attribute("segment1"), "seg1");
    EXPECT_EQ(loc1->attribute("segment1"), "seg2"); // seg2 changes index

    // add segment of the wrong type
    Ptr<Instance> seg3 = m->instanceNew("seg3", "Plane segment");
    seg3->attributeIs("source", "loc1");
    EXPECT_NE(seg3->attribute("source"), "loc1");
    EXPECT_EQ(loc1->attribute("segment2"), "");

    // set source to empty
    seg2->attributeIs("source", "");
    EXPECT_EQ(seg3->attribute("source"), "");
}


TEST(Instance, SegmentAndLocationType) {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);

    // create all location types
    Ptr<Instance> boatTerminal = m->instanceNew("boatTerminal", "Boat terminal");
    ASSERT_TRUE(boatTerminal);
    Ptr<Instance> planeTerminal = m->instanceNew("planeTerminal", "Plane terminal");
    ASSERT_TRUE(planeTerminal);
    Ptr<Instance> truckTerminal = m->instanceNew("truckTerminal", "Truck terminal");
    ASSERT_TRUE(truckTerminal);
    Ptr<Instance> customer = m->instanceNew("customer", "Customer");
    ASSERT_TRUE(customer);
    Ptr<Instance> port = m->instanceNew("port", "Port");
    ASSERT_TRUE(port);

    // create all segment types
    Ptr<Instance> boatSegment = m->instanceNew("boatSegment", "Boat segment");
    ASSERT_TRUE(boatSegment);
    Ptr<Instance> planeSegment = m->instanceNew("planeSegment", "Plane segment");
    ASSERT_TRUE(planeSegment);
    Ptr<Instance> truckSegment = m->instanceNew("truckSegment", "Truck segment");
    ASSERT_TRUE(truckSegment);

    // test adding segment to boat terminal
    boatSegment->attributeIs("source", "boatTerminal");
    EXPECT_EQ(boatSegment->attribute("source"), "boatTerminal");
    EXPECT_EQ(boatTerminal->attribute("segment1"), "boatSegment");
    planeSegment->attributeIs("source", "boatTerminal");
    EXPECT_NE(planeSegment->attribute("source"), "boatTerminal");
    truckSegment->attributeIs("source", "boatTerminal");
    EXPECT_NE(truckSegment->attribute("source"), "boatTerminal");

    // test adding segment to truck terminal
    truckSegment->attributeIs("source", "truckTerminal");
    EXPECT_EQ(truckSegment->attribute("source"), "truckTerminal");
    EXPECT_EQ(truckTerminal->attribute("segment1"), "truckSegment");
    planeSegment->attributeIs("source", "truckTerminal");
    EXPECT_NE(planeSegment->attribute("source"), "truckTerminal");
    boatSegment->attributeIs("source", "truckTerminal");
    EXPECT_NE(boatSegment->attribute("source"), "truckTerminal");

    // test adding segment to plane terminal
    planeSegment->attributeIs("source", "planeTerminal");
    EXPECT_EQ(planeSegment->attribute("source"), "planeTerminal");
    EXPECT_EQ(planeTerminal->attribute("segment1"), "planeSegment");
    truckSegment->attributeIs("source", "planeTerminal");
    EXPECT_NE(truckSegment->attribute("source"), "planeTerminal");
    boatSegment->attributeIs("source", "planeTerminal");
    EXPECT_NE(boatSegment->attribute("source"), "planeTerminal");

    // test adding segment to port -- all should succeed
    planeSegment->attributeIs("source", "port");
    EXPECT_EQ(planeSegment->attribute("source"), "port");
    EXPECT_EQ(port->attribute("segment1"), "planeSegment");
    truckSegment->attributeIs("source", "port");
    EXPECT_EQ(truckSegment->attribute("source"), "port");
    EXPECT_EQ(port->attribute("segment2"), "truckSegment");
    boatSegment->attributeIs("source", "port");
    EXPECT_EQ(boatSegment->attribute("source"), "port");
    EXPECT_EQ(port->attribute("segment3"), "boatSegment");

    // test adding customer to customer -- all should succeed
    planeSegment->attributeIs("source", "customer");
    EXPECT_EQ(planeSegment->attribute("source"), "customer");
    EXPECT_EQ(customer->attribute("segment1"), "planeSegment");
    truckSegment->attributeIs("source", "customer");
    EXPECT_EQ(truckSegment->attribute("source"), "customer");
    EXPECT_EQ(customer->attribute("segment2"), "truckSegment");
    boatSegment->attributeIs("source", "customer");
    EXPECT_EQ(boatSegment->attribute("source"), "customer");
    EXPECT_EQ(customer->attribute("segment3"), "boatSegment");
}

TEST(Instance, CreateFleet) {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
    Ptr<Instance> fleet = m->instanceNew("fleet", "Fleet");
    ASSERT_TRUE(fleet);

    // check defaults
    string speedDefault = "1.00";
    string capacityDefault = "1";
    string costDefault = "1.00";
    EXPECT_EQ(fleet->attribute("Boat, speed"), speedDefault);
    EXPECT_EQ(fleet->attribute("Boat, capacity"), capacityDefault);
    EXPECT_EQ(fleet->attribute("Boat, cost"), costDefault);
    EXPECT_EQ(fleet->attribute("Truck, speed"), speedDefault);
    EXPECT_EQ(fleet->attribute("Truck, capacity"), capacityDefault);
    EXPECT_EQ(fleet->attribute("Truck, cost"), costDefault);
    EXPECT_EQ(fleet->attribute("Plane, speed"), speedDefault);
    EXPECT_EQ(fleet->attribute("Plane, capacity"), capacityDefault);
    EXPECT_EQ(fleet->attribute("Plane, cost"), costDefault);

    // check boat attributes
    fleet->attributeIs("Boat, speed", "10.1");
    EXPECT_EQ(fleet->attribute("Boat, speed"), "10.10");
    fleet->attributeIs("Boat, capacity", "20");
    EXPECT_EQ(fleet->attribute("Boat, capacity"), "20");
    fleet->attributeIs("Boat, cost", "30.1");
    EXPECT_EQ(fleet->attribute("Boat, cost"), "30.10");

    // check truck attributes
    fleet->attributeIs("Truck, speed", "14.1");
    EXPECT_EQ(fleet->attribute("Truck, speed"), "14.10");
    fleet->attributeIs("Truck, capacity", "24");
    EXPECT_EQ(fleet->attribute("Truck, capacity"), "24");
    fleet->attributeIs("Truck, cost", "34.1");
    EXPECT_EQ(fleet->attribute("Truck, cost"), "34.10");

    // check plane attributes
    fleet->attributeIs("Plane, speed", "15.1");
    EXPECT_EQ(fleet->attribute("Plane, speed"), "15.10");
    fleet->attributeIs("Plane, capacity", "25");
    EXPECT_EQ(fleet->attribute("Plane, capacity"), "25");
    fleet->attributeIs("Plane, cost", "35.1");
    EXPECT_EQ(fleet->attribute("Plane, cost"), "35.10");
}

TEST(Instance, StatsTest) {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
    Ptr<Instance> stats = m->instanceNew("stats", "Stats");
    ASSERT_TRUE(stats);

    // check defaults
    EXPECT_EQ(stats->attribute("Truck segment"), "0");
    EXPECT_EQ(stats->attribute("Boat segment"), "0");
    EXPECT_EQ(stats->attribute("Plane segment"), "0");
    EXPECT_EQ(stats->attribute("Truck terminal"), "0");
    EXPECT_EQ(stats->attribute("Boat terminal"), "0");
    EXPECT_EQ(stats->attribute("Plane terminal"), "0");
    EXPECT_EQ(stats->attribute("Customer"), "0");
    EXPECT_EQ(stats->attribute("Port"), "0");
    EXPECT_EQ(stats->attribute("expedite percentage"), "0.00");

    // add a segment
    Ptr<Instance> seg1 = m->instanceNew("seg1", "Boat segment");
    ASSERT_TRUE(seg1);
    EXPECT_EQ(stats->attribute("Boat segment"), "1");
    EXPECT_EQ(stats->attribute("expedite percentage"), "0.00");
    seg1->attributeIs("expedite support", "yes");
    EXPECT_EQ(stats->attribute("expedite percentage"), "100.00");

    // test example provided by teacher
    Ptr<Instance> seg2 = m->instanceNew("seg2", "Boat segment");
    seg2->attributeIs("expedite support", "yes");
    Ptr<Instance> seg3 = m->instanceNew("seg3", "Boat segment");
    seg3->attributeIs("expedite support", "yes");
    Ptr<Instance> seg4 = m->instanceNew("seg4", "Boat segment");
    Ptr<Instance> seg5 = m->instanceNew("seg5", "Boat segment");
    Ptr<Instance> seg6 = m->instanceNew("seg6", "Boat segment");
    Ptr<Instance> seg7 = m->instanceNew("seg7", "Boat segment");
    EXPECT_EQ(stats->attribute("expedite percentage"), "42.86");

    // add a location
    Ptr<Instance> loc1 = m->instanceNew("loc1", "Plane terminal");
    EXPECT_EQ(stats->attribute("Plane terminal"), "1");

    // create duplicate stats that should have same values
    Ptr<Instance> stats2 = m->instanceNew("stats2", "Stats");
    ASSERT_TRUE(stats2);
    EXPECT_EQ(stats2->attribute("expedite percentage"), "42.86");
    EXPECT_EQ(stats2->attribute("Plane terminal"), "1");
}

TEST(Instance, StatsCreatedLate) {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);

    // add a segment
    Ptr<Instance> seg1 = m->instanceNew("seg1", "Boat segment");
    ASSERT_TRUE(seg1);
    seg1->attributeIs("expedite support", "yes");

    // test example provided by teacher
    Ptr<Instance> seg2 = m->instanceNew("seg2", "Boat segment");
    seg2->attributeIs("expedite support", "yes");
    Ptr<Instance> seg3 = m->instanceNew("seg3", "Boat segment");
    seg3->attributeIs("expedite support", "yes");
    Ptr<Instance> seg4 = m->instanceNew("seg4", "Boat segment");
    Ptr<Instance> seg5 = m->instanceNew("seg5", "Boat segment");
    Ptr<Instance> seg6 = m->instanceNew("seg6", "Boat segment");
    Ptr<Instance> seg7 = m->instanceNew("seg7", "Boat segment");

    // add a location
    Ptr<Instance> loc1 = m->instanceNew("loc1", "Plane terminal");

    // create stats object late
    Ptr<Instance> stats = m->instanceNew("stats", "Stats");
    ASSERT_TRUE(stats);
    EXPECT_EQ(stats->attribute("Boat segment"), "7");
    EXPECT_EQ(stats->attribute("expedite percentage"), "42.86");
    EXPECT_EQ(stats->attribute("Plane terminal"), "1");
}

TEST(Instance, ConnTest) {
    // create manager and conn
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
    Ptr<Instance> conn = m->instanceNew("conn", "Conn");
    ASSERT_TRUE(conn);

    // return NULL if trying to create another Conn by the same name
    EXPECT_FALSE(m->instanceNew("conn", "Conn"));
    // return the same conn if trying to create a new one
    EXPECT_TRUE(m->instanceNew("notconn", "Conn"));

    // add three locations
    Ptr<Instance> loc1 = m->instanceNew("loc1", "Customer");
    ASSERT_TRUE(loc1);
    Ptr<Instance> loc2 = m->instanceNew("loc2", "Customer");
    ASSERT_TRUE(loc2);
    Ptr<Instance> loc3 = m->instanceNew("loc3", "Customer");
    ASSERT_TRUE(loc3);

    // add intermediate segments between first two locations
    Ptr<Instance> seg1 = m->instanceNew("seg1", "Truck segment");
    ASSERT_TRUE(seg1);
    Ptr<Instance> seg2 = m->instanceNew("seg2", "Truck segment");
    ASSERT_TRUE(seg2);
    seg1->attributeIs("source", "loc1");
    seg2->attributeIs("source", "loc2");
    seg1->attributeIs("return segment", "seg2");
    EXPECT_EQ(seg2->attribute("return segment"), "seg1");

    // test explore
    EXPECT_EQ(conn->attribute("explore loc1 :"), "loc1(seg1:1.00:seg2) loc2\n");

    // add intermediate segments between second two locations
    Ptr<Instance> seg3 = m->instanceNew("seg3", "Truck segment");
    ASSERT_TRUE(seg3);
    Ptr<Instance> seg4 = m->instanceNew("seg4", "Truck segment");
    ASSERT_TRUE(seg4);
    seg3->attributeIs("source", "loc2");
    seg4->attributeIs("source", "loc3");
    seg3->attributeIs("return segment", "seg4");
    EXPECT_EQ(seg4->attribute("return segment"), "seg3");

    // test explore
    EXPECT_EQ(conn->attribute("explore loc1 :"), "loc1(seg1:1.00:seg2) loc2\nloc1(seg1:1.00:seg2) loc2(seg3:1.00:seg4) loc3\n");

    // test connect
    EXPECT_EQ(conn->attribute("connect loc1 : loc2"), "1.00 1.00 no; loc1(seg1:1.00:seg2) loc2\n");

    // connect a location to itself--shouldn't return a path
    EXPECT_EQ(conn->attribute("connect loc1 : loc1"), "");

    // create duplicate conn that should be the same
    Ptr<Instance> conn2 = m->instanceNew("conn2", "Conn");
    ASSERT_TRUE(conn2);
    EXPECT_EQ(conn2->attribute("connect loc1 : loc2"), "1.00 1.00 no; loc1(seg1:1.00:seg2) loc2\n");
}

TEST(Instance, ConnNotConnected) {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
    Ptr<Instance> conn = m->instanceNew("conn", "Conn");
    ASSERT_TRUE(conn);

    // add two locations
    Ptr<Instance> loc1 = m->instanceNew("loc1", "Customer");
    ASSERT_TRUE(loc1);
    Ptr<Instance> loc2 = m->instanceNew("loc2", "Customer");
    ASSERT_TRUE(loc2);

    // add one segment
    Ptr<Instance> seg1 = m->instanceNew("seg1", "Truck segment");
    ASSERT_TRUE(seg1);
    seg1->attributeIs("source", "loc1");
    EXPECT_EQ(loc1->attribute("segment1"), "seg1");

    // shouldn't find any path
    EXPECT_EQ(conn->attribute("explore loc1 :"), "");

    // shouldn't find any connection
    EXPECT_EQ(conn->attribute("connect loc1 : loc2"), "");
}

TEST(Instance, ConnTwoRoutes) {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
    Ptr<Instance> conn = m->instanceNew("conn", "Conn");
    ASSERT_TRUE(conn);
    Ptr<Instance> fleet = m->instanceNew("fleet", "Fleet");
    ASSERT_TRUE(fleet);

    // setup costs and speeds
    fleet->attributeIs("Truck, speed", "50");
    fleet->attributeIs("Truck, cost", "10");
    fleet->attributeIs("Plane, speed", "250");
    fleet->attributeIs("Plane, cost", "15");

    // add two locations
    Ptr<Instance> loc1 = m->instanceNew("loc1", "Customer");
    ASSERT_TRUE(loc1);
    Ptr<Instance> loc2 = m->instanceNew("loc2", "Customer");
    ASSERT_TRUE(loc2);

    // add two paths between them
    Ptr<Instance> seg1 = addSegment(m, "seg1", "Truck segment", "loc1",
        "", "400.00", "1.00", "yes");
    Ptr<Instance> seg2 = addSegment(m, "seg2", "Truck segment", "loc2",
        "seg1", "500.00", "1.00", "yes");
    Ptr<Instance> seg3 = addSegment(m, "seg3", "Plane segment", "loc1",
        "", "450.00", "1.00", "yes");
    Ptr<Instance> seg4 = addSegment(m, "seg4", "Plane segment", "loc2",
        "seg3", "550.00", "1.00", "yes");

    EXPECT_EQ(conn->attribute("connect loc1 : loc2"), "10125.00 1.38 yes; loc1(seg3:450.00:seg4) loc2\n4000.00 8.00 no; loc1(seg1:400.00:seg2) loc2\n6000.00 6.15 yes; loc1(seg1:400.00:seg2) loc2\n6750.00 1.80 no; loc1(seg3:450.00:seg4) loc2\n");
    EXPECT_EQ(conn->attribute("explore loc1 :"), "loc1(seg1:400.00:seg2) loc2\nloc1(seg3:450.00:seg4) loc2\n");
    EXPECT_EQ(conn->attribute("explore loc1 : expedited"), "loc1(seg1:400.00:seg2) loc2\nloc1(seg3:450.00:seg4) loc2\n");
    EXPECT_EQ(conn->attribute("explore loc1 : expedited distance 400"), "loc1(seg1:400.00:seg2) loc2\n");
    EXPECT_EQ(conn->attribute("explore loc1 : expedited distance 400 cost 6000"), "loc1(seg1:400.00:seg2) loc2\n");
    EXPECT_EQ(conn->attribute("explore loc1 : expedited distance 400 cost 5999"), "");
    EXPECT_EQ(conn->attribute("explore loc1 : expedited distance 400 cost 6000 time 6.14"), "");
    EXPECT_EQ(conn->attribute("explore loc1 : expedited distance 400 cost 6000 time 6.16"), "loc1(seg1:400.00:seg2) loc2\n");
    EXPECT_EQ(conn->attribute("explore loc1 : expedited distance 400 time 6.14"), "");
    EXPECT_EQ(conn->attribute("explore loc1 : time 1.39"), "loc1(seg3:450.00:seg4) loc2\n");
    EXPECT_EQ(conn->attribute("explore loc1 : time 2"), "loc1(seg3:450.00:seg4) loc2\n");
}

TEST(Instance, CostTimeCalculation) {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
    Ptr<Instance> conn = m->instanceNew("conn", "Conn");
    ASSERT_TRUE(conn);
    Ptr<Instance> fleet = m->instanceNew("fleet", "Fleet");
    ASSERT_TRUE(fleet);

    // set fleet attributes
    fleet->attributeIs("Truck, speed", "1");
    fleet->attributeIs("Truck, cost", "1");

    // add three locations
    Ptr<Instance> loc1 = m->instanceNew("loc1", "Customer");
    ASSERT_TRUE(loc1);
    Ptr<Instance> loc2 = m->instanceNew("loc2", "Customer");
    ASSERT_TRUE(loc2);

    // add intermediate segments between two locations
    Ptr<Instance> seg1 = m->instanceNew("seg1", "Truck segment");
    ASSERT_TRUE(seg1);
    Ptr<Instance> seg2 = m->instanceNew("seg2", "Truck segment");
    ASSERT_TRUE(seg2);
    seg1->attributeIs("source", "loc1");
    seg2->attributeIs("source", "loc2");
    seg1->attributeIs("return segment", "seg2");
    EXPECT_EQ(seg2->attribute("return segment"), "seg1");
    EXPECT_EQ(loc1->attribute("segment1"), "seg1");
    EXPECT_EQ(loc2->attribute("segment1"), "seg2");

    // turn on expedite support
    seg1->attributeIs("expedite support", "yes");

    // check cost
    EXPECT_EQ(conn->attribute("connect loc1 : loc2"), "1.00 1.00 no; loc1(seg1:1.00:seg2) loc2\n1.50 0.77 yes; loc1(seg1:1.00:seg2) loc2\n");

    // change fleet speed (time should decrease by factor of speed)
    fleet->attributeIs("Truck, speed", "0.50");
    EXPECT_EQ(conn->attribute("connect loc1 : loc2"), "1.00 2.00 no; loc1(seg1:1.00:seg2) loc2\n1.50 1.54 yes; loc1(seg1:1.00:seg2) loc2\n");
    fleet->attributeIs("Truck, speed", "1.00");

    // change fleet cost
    fleet->attributeIs("Truck, cost", "2.00");
    EXPECT_EQ(conn->attribute("connect loc1 : loc2"), "2.00 1.00 no; loc1(seg1:1.00:seg2) loc2\n3.00 0.77 yes; loc1(seg1:1.00:seg2) loc2\n");
    fleet->attributeIs("Truck, cost", "1.00");

    // change seg difficulty
    seg1->attributeIs("difficulty", "2.00");
    EXPECT_EQ(conn->attribute("connect loc1 : loc2"), "2.00 1.00 no; loc1(seg1:1.00:seg2) loc2\n3.00 0.77 yes; loc1(seg1:1.00:seg2) loc2\n");
    seg1->attributeIs("difficulty", "1.00");

    // change seg length
    seg1->attributeIs("length", "2.00");
    EXPECT_EQ(conn->attribute("connect loc1 : loc2"), "2.00 2.00 no; loc1(seg1:2.00:seg2) loc2\n3.00 1.54 yes; loc1(seg1:2.00:seg2) loc2\n");
    seg1->attributeIs("length", "1.00");
}
