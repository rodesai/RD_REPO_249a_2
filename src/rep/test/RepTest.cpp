#include "gtest/gtest.h"
#include "rep/Instance.h"

TEST(Instance, CreateInstanceManager) {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
}

/* TODO:
    - verify output--should there always be decimals? e.g. "1.00"
    - do we have the right float/decimal types?
    - is "0" the right percentage if there are no segments?
    - add tests for
        - stats / conn / fleet
        - check defaults (i.e. what if i ask for the difficulty without defining it?)
        - invalid input
*/

TEST(Instance, CreateSegment) {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
    Ptr<Instance> seg1 = m->instanceNew("seg1", "Boat segment");
    ASSERT_TRUE(seg1);

    // check defaults
    EXPECT_EQ(seg1->attribute("length"), "0.00");
    EXPECT_EQ(seg1->attribute("difficulty"), "1.00");
    EXPECT_EQ(seg1->attribute("expedite support"), "no");
    EXPECT_EQ(seg1->attribute("source"), "");
    EXPECT_EQ(seg1->attribute("return segment"), "");

    // test changing of attribute values
    seg1->attributeIs("length", "400");
    EXPECT_EQ(seg1->attribute("length"), "400.00");
    seg1->attributeIs("difficulty", "3.20");
    EXPECT_EQ(seg1->attribute("difficulty"), "3.20");
    seg1->attributeIs("difficulty", "3.20");
    EXPECT_EQ(seg1->attribute("difficulty"), "3.20");
    EXPECT_EQ(seg1->attribute("expedite support"), "no");
    seg1->attributeIs("expedite support", "yes");
    EXPECT_EQ(seg1->attribute("expedite support"), "yes");

    // creating a new segment with the same name should fail
    Ptr<Instance> seg2 = m->instanceNew("seg1", "Plane segment");
    ASSERT_FALSE(seg2);

    // create new segment and set return segment
    seg2 = m->instanceNew("seg2", "Plane segment");
    ASSERT_TRUE(seg2);
    seg1->attributeIs("return segment", "seg2");
    EXPECT_EQ(seg1->attribute("return segment"), "seg2");
    EXPECT_EQ(seg2->attribute("return segment"), "seg1");
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
    fleet->attributeIs("Boat, speed", "10");
    EXPECT_EQ(fleet->attribute("Boat, speed"), "10.00");
    fleet->attributeIs("Boat, capacity", "20");
    EXPECT_EQ(fleet->attribute("Boat, capacity"), "20");
    fleet->attributeIs("Boat, cost", "30");
    EXPECT_EQ(fleet->attribute("Boat, cost"), "30.00");

    // check truck attributes
    fleet->attributeIs("Truck, speed", "14");
    EXPECT_EQ(fleet->attribute("Truck, speed"), "14.00");
    fleet->attributeIs("Truck, capacity", "24");
    EXPECT_EQ(fleet->attribute("Truck, capacity"), "24");
    fleet->attributeIs("Truck, cost", "34");
    EXPECT_EQ(fleet->attribute("Truck, cost"), "34.00");

    // check plane attributes
    fleet->attributeIs("Plane, speed", "15");
    EXPECT_EQ(fleet->attribute("Plane, speed"), "15.00");
    fleet->attributeIs("Plane, capacity", "25");
    EXPECT_EQ(fleet->attribute("Plane, capacity"), "25");
    fleet->attributeIs("Plane, cost", "35");
    EXPECT_EQ(fleet->attribute("Plane, cost"), "35.00");
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
}

TEST(Instance, ConnTest) {
    // create manager and conn
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
    Ptr<Instance> conn = m->instanceNew("conn", "Conn");
    ASSERT_TRUE(conn);

    // add two segments and connect them
    Ptr<Instance> loc1 = m->instanceNew("loc1", "Customer");
    ASSERT_TRUE(loc1);
    Ptr<Instance> loc2 = m->instanceNew("loc2", "Customer");
    ASSERT_TRUE(loc2);
    Ptr<Instance> seg1 = m->instanceNew("seg1", "Truck segment");
    ASSERT_TRUE(seg1);
    Ptr<Instance> seg2 = m->instanceNew("seg2", "Truck segment");
    ASSERT_TRUE(seg2);
    seg1->attributeIs("source", "loc1");
    seg2->attributeIs("source", "loc2");
    seg1->attributeIs("return segment", "seg2");

    std::cout << "Made it this far.\n";
    std::cout << conn->attribute("explore loc1 :");
}
