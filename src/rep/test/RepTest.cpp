#include "gtest/gtest.h"
#include "rep/Instance.h"

TEST(Instance, CreateInstanceManager) {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
}

/* TODO:
    x- verify that I do NOT need to delete a ref (e.g. from instanceDel)
    x- need to make value type not negative for unspecified (hour, dollar, mile)
    - do we need to support the removal of a source? (e.g. seg->attributeIs("source", ""))
    x- support deletion of segment and location
    - add tests for
        - use m->instance() to check that conn / stats / fleet have the right name
        - names are empty strings? check piazza
        - check road to the location itself
        - invalid input on attribute, attributeIs (especially for conn)
        x- make sure that calling new on stats / conn / fleet will return the same one
        x- stats / conn / fleet
        x- if the stats object is created late, is it up-to-date?
        x- check defaults (i.e. what if i ask for the difficulty without defining it?)
        x- if the conn already exits, and you try to create new, do you get null?
        x- conn if segment does not have return segment
        x- turning off expedite support for segments
        x- set return segment as segment that doesn't exist (wrong name)
        x- set source to location that doesn't exist
        x- create a case where there are two paths and one is expedited
    x- are we outputting to std::err?
    x- add errors for all incorrect read operations
    x- verify output--should there always be decimals? e.g. "1.00"
    x- do we have the right float/decimal types?
    x- is "0" the right percentage if there are no segments?
*/

/* TODO: design questions
    - the name Path::location() is confusing
    - all of the expedited types are confusing
*/

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

TEST(Instance, TypeOutOfBounds) {
    // create instances needed for test
    Ptr<Instance::Manager> m = shippingInstanceManager();
    ASSERT_TRUE(m);
    Ptr<Instance> seg1 = m->instanceNew("seg1", "Boat segment");
    ASSERT_TRUE(seg1);
    Ptr<Instance> fleet = m->instanceNew("fleet", "Fleet");
    ASSERT_TRUE(fleet);

    // test segment values
    seg1->attributeIs("length", "400");
    seg1->attributeIs("difficulty", "3.20");
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

    // delete something that cannot be deleted
    m->instanceDel("stats");
    EXPECT_TRUE(m->instance("stats"));

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

    // create duplicate fleet that should have the same values
    Ptr<Instance> fleet2 = m->instanceNew("fleet2", "Fleet");
    EXPECT_EQ(fleet->attribute("Plane, speed"), "15.00");
    EXPECT_EQ(fleet->attribute("Plane, capacity"), "25");
    EXPECT_EQ(fleet2->attribute("Plane, cost"), "35.00");
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

// test: too slow when not expedited, too costly when expedited
// go for 
