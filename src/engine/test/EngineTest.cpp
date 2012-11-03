#include "gtest/gtest.h"
#include "engine/Engine.h"

using namespace Shipping;

TEST(Engine, Segment){

    ShippingNetwork::Ptr nwk = ShippingNetwork::ShippingNetworkIs("network");
    Segment::Ptr segment = nwk->SegmentNew("segment1",Segment::truckSegment());

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

    Segment::Ptr segment1 = nwk->SegmentNew("segment1",Segment::truckSegment());
    Segment::Ptr segment2 = nwk->SegmentNew("segment2",Segment::truckSegment());
    Segment::Ptr segment3 = nwk->SegmentNew("segment3",Segment::truckSegment());

    Location::Ptr location = nwk->LocationNew("location1",Location::customer());

    segment1->sourceIs(location);

    ASSERT_TRUE(segment1->source() == location);
    ASSERT_TRUE(location->segmentID(0) == "segment1");

    segment2->sourceIs(location);

    ASSERT_TRUE(segment2->source() == location);
    ASSERT_TRUE(location->segmentID(1) == "segment2");

    segment3->sourceIs(location);

    ASSERT_TRUE(segment3->source() == location);
    ASSERT_TRUE(location->segmentID(2) == "segment3");

    segment2->sourceIs(NULL);

    ASSERT_TRUE(location->segmentCount() == 2);
    ASSERT_TRUE(location->segmentID(0) == "segment1");
    ASSERT_TRUE(location->segmentID(1) == "segment3");
}

TEST(Engine, SegmentReturnSegment){

    ShippingNetwork::Ptr nwk = ShippingNetwork::ShippingNetworkIs("network");

    Segment::Ptr segment1 = nwk->SegmentNew("segment1",Segment::truckSegment());
    Segment::Ptr segment2 = nwk->SegmentNew("segment2",Segment::truckSegment());

    segment1->returnSegmentIs(segment2);

    ASSERT_TRUE(segment1->returnSegment()==segment2);
    ASSERT_TRUE(segment2->returnSegment()==segment1);

    segment1->returnSegmentIs(NULL);
    
    ASSERT_FALSE(segment1->returnSegment());
    ASSERT_FALSE(segment2->returnSegment());

    Segment::Ptr segment3 = nwk->SegmentNew("segment3",Segment::truckSegment());

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
 
   Segment::Ptr segment = nwk->SegmentNew("segment1",Segment::truckSegment());
   Location::Ptr location = nwk->LocationNew("location1",Location::customer());
   Stats::Ptr stat = nwk->StatsNew("stat1");

   ASSERT_TRUE(stat->locationCount(Location::customer()) == 1);
   ASSERT_TRUE(stat->segmentCount(Segment::truckSegment()) == 1);
}
