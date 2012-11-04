#include "gtest/gtest.h"
#include <iostream>
#include "engine/Engine.h"

using namespace Shipping;

TEST(Engine, Segment){

    ShippingNetwork::Ptr nwk = ShippingNetwork::ShippingNetworkIs("network");
    Segment::Ptr segment = nwk->SegmentNew("segment1",truck_);

    ASSERT_TRUE(segment);
    ASSERT_TRUE(nwk->segment("segment1"));
}

TEST(Engine, Location){

    ShippingNetwork::Ptr nwk = ShippingNetwork::ShippingNetworkIs("network");
    Location::Ptr location = nwk->LocationNew("location1",Location::customer());

    ASSERT_TRUE(location);
    ASSERT_TRUE(nwk->location("location1"));
}

TEST(Engine, SegmentSource){

    ShippingNetwork::Ptr nwk = ShippingNetwork::ShippingNetworkIs("network");

    Segment::Ptr segment1 = nwk->SegmentNew("segment1",truck_);
    Segment::Ptr segment2 = nwk->SegmentNew("segment2",truck_);
    Segment::Ptr segment3 = nwk->SegmentNew("segment3",truck_);

    Location::Ptr location = nwk->LocationNew("location1",Location::customer());

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
    Location::Ptr location = nwk->LocationNew("location1",Location::customer());
    Stats::Ptr stat = nwk->StatsNew("stat1");

    ASSERT_TRUE(stat->locationCount(Location::customer()) == 1);
    ASSERT_TRUE(stat->segmentCount(truck_) == 1);
}

TEST(Engine, Conn){

    ShippingNetwork::Ptr nwk = ShippingNetwork::ShippingNetworkIs("network");

    Location::Ptr location1 = nwk->LocationNew("location1",Location::customer());
    Location::Ptr location2 = nwk->LocationNew("location2",Location::customer());
    Location::Ptr location3 = nwk->LocationNew("location3",Location::customer());
    Location::Ptr location4 = nwk->LocationNew("location4",Location::customer());

    Segment::Ptr segment1 = nwk->SegmentNew("segment1",truck_);
    Segment::Ptr segment2 = nwk->SegmentNew("segment2",truck_);
    Segment::Ptr segment3 = nwk->SegmentNew("segment3",truck_);
    Segment::Ptr segment4 = nwk->SegmentNew("segment4",truck_);
    Segment::Ptr segment1r = nwk->SegmentNew("segment1r",truck_);
    Segment::Ptr segment2r = nwk->SegmentNew("segment2r",truck_);
    Segment::Ptr segment3r = nwk->SegmentNew("segment3r",truck_);
    Segment::Ptr segment4r = nwk->SegmentNew("segment4r",truck_);

    segment1->sourceIs(location1);
    segment2->sourceIs(location1);
    segment3->sourceIs(location2);
    segment4->sourceIs(location3);

    segment1r->sourceIs(location2);
    segment2r->sourceIs(location3);
    segment3r->sourceIs(location4);
    segment4r->sourceIs(location4);

    segment1->returnSegmentIs(segment1r);
    segment2->returnSegmentIs(segment2r);
    segment3->returnSegmentIs(segment3r);
    segment4->returnSegmentIs(segment4r);

    Conn::Ptr conn = nwk->ConnNew("conn");
    Fleet::Ptr fleet = nwk->FleetNew("fleet");
    fleet->speedIs(truck_,100);
    fleet->capacityIs(truck_,100);
    fleet->costIs(truck_,100);

    std::vector<Path::Ptr> paths = conn->connect(location1,location4,nwk.ptr(),fleet.ptr());

    std::cout << "Paths: " << paths.size() << std::endl;
    ASSERT_TRUE(paths.size()==2);
}
