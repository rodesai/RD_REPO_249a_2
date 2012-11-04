#include "gtest/gtest.h"
#include <iostream>
#include "engine/Engine.h"

using namespace Shipping;

void connectLocations(Location::Ptr l1,Location::Ptr l2,ShippingNetwork::Ptr nwk){
    Segment::Ptr segment,segmentR;
    segment = nwk->SegmentNew(l1->name() + "-" + l2->name(),truck_);
    segmentR = nwk->SegmentNew(l2->name() + "-" + l1->name(),truck_);
    segment->sourceIs(l1);
    segmentR->sourceIs(l2);
    segment->returnSegmentIs(segmentR);
}

TEST(Engine, Segment){

    ShippingNetwork::Ptr nwk = ShippingNetwork::ShippingNetworkIs("network");
    Segment::Ptr segment = nwk->SegmentNew("segment1",truck_);

    ASSERT_TRUE(segment);
    ASSERT_TRUE(nwk->segment("segment1"));
}

TEST(Engine, Location){

    ShippingNetwork::Ptr nwk = ShippingNetwork::ShippingNetworkIs("network");
    Location::Ptr location = nwk->LocationNew("location1",Location::port());

    ASSERT_TRUE(location);
    ASSERT_TRUE(nwk->location("location1"));
}

TEST(Engine, SegmentSource){

    ShippingNetwork::Ptr nwk = ShippingNetwork::ShippingNetworkIs("network");

    Segment::Ptr segment1 = nwk->SegmentNew("segment1",truck_);
    Segment::Ptr segment2 = nwk->SegmentNew("segment2",truck_);
    Segment::Ptr segment3 = nwk->SegmentNew("segment3",truck_);

    Location::Ptr location = nwk->LocationNew("location1",Location::port());

    segment1->sourceIs(location);

    ASSERT_TRUE(segment1->source() == location);
    ASSERT_TRUE(location->segmentID(1) == "segment1");

    segment2->sourceIs(location);

    ASSERT_TRUE(segment2->source() == location);
    ASSERT_TRUE(location->segmentID(2) == "segment2");

    segment3->sourceIs(location);

    ASSERT_TRUE(segment3->source() == location);
    ASSERT_TRUE(location->segmentID(3) == "segment3");

    segment2->sourceIs(NULL);

    ASSERT_TRUE(location->segmentCount() == 2);
    ASSERT_TRUE(location->segmentID(1) == "segment1");
    ASSERT_TRUE(location->segmentID(2) == "segment3");
}

TEST(Engine, SegmentReturnSegment){

    ShippingNetwork::Ptr nwk = ShippingNetwork::ShippingNetworkIs("network");

    Segment::Ptr segment1 = nwk->SegmentNew("segment1",truck_);
    Segment::Ptr segment2 = nwk->SegmentNew("segment2",truck_);

    segment1->returnSegmentIs(segment2);

    ASSERT_TRUE(segment1->returnSegment()==segment2);
    ASSERT_TRUE(segment2->returnSegment()==segment1);

    segment1->returnSegmentIs(NULL);
    
    ASSERT_FALSE(segment1->returnSegment());
    ASSERT_FALSE(segment2->returnSegment());

    Segment::Ptr segment3 = nwk->SegmentNew("segment3",truck_);

    segment1->returnSegmentIs(segment2);
    
    ASSERT_TRUE(segment1->returnSegment()==segment2);
    ASSERT_TRUE(segment2->returnSegment()==segment1);

    segment2->returnSegmentIs(segment3);

    ASSERT_FALSE(segment1->returnSegment());
    ASSERT_TRUE(segment2->returnSegment()==segment3);
    ASSERT_TRUE(segment3->returnSegment()==segment2);
}

TEST(Engine, Stat){

    ShippingNetwork::Ptr nwk = ShippingNetwork::ShippingNetworkIs("network");
 
    Segment::Ptr segment = nwk->SegmentNew("segment1",truck_);
    Location::Ptr location = nwk->LocationNew("location1",Location::port());
    Stats::Ptr stat = nwk->StatsNew("stat1");

    ASSERT_TRUE(stat->locationCount(Location::port()) == 1);
    ASSERT_TRUE(stat->segmentCount(truck_) == 1);
}

TEST(Engine, conn_endpoint_basic){

    ShippingNetwork::Ptr nwk = ShippingNetwork::ShippingNetworkIs("network");

    Location::Ptr l1 = nwk->LocationNew("l1",Location::port());
    Location::Ptr l2 = nwk->LocationNew("l2",Location::port());
    Location::Ptr l3 = nwk->LocationNew("l3",Location::port());
    Location::Ptr l4 = nwk->LocationNew("l4",Location::port());

    connectLocations(l1,l2,nwk);
    connectLocations(l1,l3,nwk);
    connectLocations(l2,l4,nwk);
    connectLocations(l3,l4,nwk);

    Conn::Ptr conn = nwk->ConnNew("conn");
    Fleet::Ptr fleet = nwk->FleetNew("fleet");
    fleet->speedIs(truck_,100);
    fleet->capacityIs(truck_,100);
    fleet->costIs(truck_,100);

    Conn::PathList paths = conn->paths(nwk.ptr(),fleet.ptr(),NULL,"l1","l4");

    ASSERT_TRUE(paths.size()==2);

    Path::Ptr path = paths[0];
    ASSERT_TRUE(path->pathElementCount() == 2);
}

TEST(Engine, conn_endpoint_no_loop_pre_endpoint){

    ShippingNetwork::Ptr nwk = ShippingNetwork::ShippingNetworkIs("network");

    Location::Ptr l1 = nwk->LocationNew("l1",Location::port());
    Location::Ptr l2 = nwk->LocationNew("l2",Location::port());
    Location::Ptr l3 = nwk->LocationNew("l3",Location::port());
    Location::Ptr l4 = nwk->LocationNew("l4",Location::port());
    Location::Ptr l2_1 = nwk->LocationNew("l2_1",Location::port());
    Location::Ptr l2_2 = nwk->LocationNew("l2_2",Location::port());


    connectLocations(l1,l2,nwk);
    connectLocations(l1,l3,nwk);
    connectLocations(l2,l4,nwk);
    connectLocations(l3,l4,nwk);
    connectLocations(l2,l2_1,nwk);
    connectLocations(l2,l2_2,nwk);
    connectLocations(l2_2,l2_1,nwk);

    Conn::Ptr conn = nwk->ConnNew("conn");
    Fleet::Ptr fleet = nwk->FleetNew("fleet");
    fleet->speedIs(truck_,100);
    fleet->capacityIs(truck_,100);
    fleet->costIs(truck_,100);

    Conn::PathList paths = conn->paths(nwk.ptr(),fleet.ptr(),NULL,"l1","l4");

    ASSERT_TRUE(paths.size()==2);

    Path::Ptr path = paths[0];
    ASSERT_TRUE(path->pathElementCount() == 2);
}

TEST(Engine, conn_endpoint_no_loop_endpoint){

    ShippingNetwork::Ptr nwk = ShippingNetwork::ShippingNetworkIs("network");

    Location::Ptr l1 = nwk->LocationNew("l1",Location::port());
    Location::Ptr l2 = nwk->LocationNew("l2",Location::port());
    Location::Ptr l3 = nwk->LocationNew("l3",Location::port());
    Location::Ptr l4 = nwk->LocationNew("l4",Location::port());
    Location::Ptr l4_1 = nwk->LocationNew("l4_1",Location::port());
    Location::Ptr l4_2 = nwk->LocationNew("l4_2",Location::port());


    connectLocations(l1,l2,nwk);
    connectLocations(l1,l3,nwk);
    connectLocations(l2,l4,nwk);
    connectLocations(l3,l4,nwk);
    connectLocations(l4,l4_1,nwk);
    connectLocations(l4,l4_2,nwk);
    connectLocations(l4_2,l4_1,nwk);

    Conn::Ptr conn = nwk->ConnNew("conn");
    Fleet::Ptr fleet = nwk->FleetNew("fleet");
    fleet->speedIs(truck_,100);
    fleet->capacityIs(truck_,100);
    fleet->costIs(truck_,100);

    Conn::PathList paths = conn->paths(nwk.ptr(),fleet.ptr(),NULL,"l1","l4");

    ASSERT_TRUE(paths.size()==2);

    Path::Ptr path = paths[0];
    ASSERT_TRUE(path->pathElementCount() == 2);
}

TEST(Engine, conn_no_endpoint_constraints){

    ShippingNetwork::Ptr nwk = ShippingNetwork::ShippingNetworkIs("network");

    Location::Ptr l1 = nwk->LocationNew("l1",Location::port());
    Location::Ptr l2 = nwk->LocationNew("l2",Location::port());
    Location::Ptr l3 = nwk->LocationNew("l3",Location::port());
    Location::Ptr l4 = nwk->LocationNew("l4",Location::port());
    Location::Ptr l5 = nwk->LocationNew("l5",Location::port());
    Location::Ptr l6 = nwk->LocationNew("l6",Location::port());
    Location::Ptr l7 = nwk->LocationNew("l7",Location::port());
    Location::Ptr l8 = nwk->LocationNew("l8",Location::port());

    connectLocations(l1,l2,nwk);
    connectLocations(l1,l3,nwk);
    connectLocations(l1,l4,nwk);
    connectLocations(l2,l5,nwk);
    connectLocations(l2,l6,nwk);
    connectLocations(l3,l6,nwk);
    connectLocations(l3,l4,nwk);
    connectLocations(l4,l7,nwk);
    connectLocations(l5,l7,nwk);
    connectLocations(l6,l8,nwk);

    Conn::Ptr conn = nwk->ConnNew("conn");
    Fleet::Ptr fleet = nwk->FleetNew("fleet");
    fleet->speedIs(truck_,100);
    fleet->capacityIs(truck_,100);
    fleet->costIs(truck_,100);

    Conn::PathList paths = conn->paths(nwk.ptr(),fleet.ptr(),NULL,"l1","l4");

    ASSERT_TRUE(paths.size()==2);

    Path::Ptr path = paths[0];
    ASSERT_TRUE(path->pathElementCount() == 2);
}
