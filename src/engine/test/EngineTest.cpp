#include "gtest/gtest.h"
#include <iostream>
#include "engine/Engine.h"

using namespace Shipping;

void connectLocations(LocationPtr l1,LocationPtr l2,ShippingNetworkPtr nwk,Mile length,Difficulty d,Segment::ExpediteSupport expediteSupport){
    SegmentPtr segment,segmentR;
    segment = nwk->SegmentNew(l1->name() + "-" + l2->name(),truck_);
    segment->lengthIs(length);
    segment->difficultyIs(d);
    segment->expediteSupportIs(expediteSupport);
    segmentR = nwk->SegmentNew(l2->name() + "-" + l1->name(),truck_);
    segmentR->lengthIs(length);
    segmentR->difficultyIs(d);
    segmentR->expediteSupportIs(expediteSupport);
    segment->sourceIs(l1);
    segmentR->sourceIs(l2);
    segment->returnSegmentIs(segmentR);
}

void connectLocations(LocationPtr l1,LocationPtr l2,ShippingNetworkPtr nwk,Mile length,Difficulty d){
    connectLocations(l1,l2,nwk,length,d,Segment::expediteSupported());
}

void connectLocations(LocationPtr l1,LocationPtr l2,ShippingNetworkPtr nwk, Mile length){
    connectLocations(l1,l2,nwk,length,1.0);
}

void connectLocations(LocationPtr l1,LocationPtr l2,ShippingNetworkPtr nwk){
    connectLocations(l1,l2,nwk,100,1.0);
}

TEST(Engine, Segment){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");
    SegmentPtr segment = nwk->SegmentNew("segment1",truck_);

    ASSERT_TRUE(segment);
    ASSERT_TRUE(nwk->segment("segment1"));
}

TEST(Engine, Location){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");
    LocationPtr location = nwk->LocationNew("location1",Location::port());

    ASSERT_TRUE(location);
    ASSERT_TRUE(nwk->location("location1"));
}

TEST(Engine, SegmentSource){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");

    SegmentPtr segment1 = nwk->SegmentNew("segment1",truck_);
    SegmentPtr segment2 = nwk->SegmentNew("segment2",truck_);
    SegmentPtr segment3 = nwk->SegmentNew("segment3",truck_);

    LocationPtr location = nwk->LocationNew("location1",Location::port());

    segment1->sourceIs(location);

    ASSERT_TRUE(segment1->source() == location);
    ASSERT_TRUE(location->segment(1)->name() == "segment1");

    segment2->sourceIs(location);

    ASSERT_TRUE(segment2->source() == location);
    ASSERT_TRUE(location->segment(2)->name() == "segment2");

    segment3->sourceIs(location);

    ASSERT_TRUE(segment3->source() == location);
    ASSERT_TRUE(location->segment(3)->name() == "segment3");

    segment2->sourceIs(NULL);

    ASSERT_TRUE(location->segmentCount() == 2);
    ASSERT_TRUE(location->segment(1)->name() == "segment1");
    ASSERT_TRUE(location->segment(2)->name() == "segment3");
}

TEST(Engine, SegmentReturnSegment){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");

    SegmentPtr segment1 = nwk->SegmentNew("segment1",truck_);
    SegmentPtr segment2 = nwk->SegmentNew("segment2",truck_);

    segment1->returnSegmentIs(segment2);

    ASSERT_TRUE(segment1->returnSegment()==segment2);
    ASSERT_TRUE(segment2->returnSegment()==segment1);

    segment1->returnSegmentIs(NULL);
    
    ASSERT_FALSE(segment1->returnSegment());
    ASSERT_FALSE(segment2->returnSegment());

    SegmentPtr segment3 = nwk->SegmentNew("segment3",truck_);

    segment1->returnSegmentIs(segment2);
    
    ASSERT_TRUE(segment1->returnSegment()==segment2);
    ASSERT_TRUE(segment2->returnSegment()==segment1);

    segment2->returnSegmentIs(segment3);

    ASSERT_FALSE(segment1->returnSegment());
    ASSERT_TRUE(segment2->returnSegment()==segment3);
    ASSERT_TRUE(segment3->returnSegment()==segment2);
}

TEST(Engine, Stat){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");
 
    SegmentPtr segment = nwk->SegmentNew("segment1",truck_);
    LocationPtr location = nwk->LocationNew("location1",Location::port());
    StatsPtr stat = nwk->StatsNew("stat1");

    ASSERT_TRUE(stat->locationCount(Location::port()) == 1);
    ASSERT_TRUE(stat->segmentCount(truck_) == 1);
}

TEST(Engine, conn_endpoint_basic){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");

    LocationPtr l1 = nwk->LocationNew("l1",Location::port());
    LocationPtr l2 = nwk->LocationNew("l2",Location::port());
    LocationPtr l3 = nwk->LocationNew("l3",Location::port());
    LocationPtr l4 = nwk->LocationNew("l4",Location::port());

    connectLocations(l1,l2,nwk);
    connectLocations(l1,l3,nwk);
    connectLocations(l2,l4,nwk);
    connectLocations(l3,l4,nwk);

    ConnPtr conn = nwk->ConnNew("conn");
    FleetPtr fleet = nwk->FleetNew("fleet");
    fleet->speedIs(truck_,100);
    fleet->capacityIs(truck_,100);
    fleet->costIs(truck_,100);

    Conn::PathList paths = conn->paths(NULL,"l1","l4");

    ASSERT_TRUE(paths.size()==2);

    PathPtr path = paths[0];
    ASSERT_TRUE(path->pathElementCount() == 2);
}

TEST(Engine, conn_endpoint_no_loop_pre_endpoint){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");

    LocationPtr l1 = nwk->LocationNew("l1",Location::port());
    LocationPtr l2 = nwk->LocationNew("l2",Location::port());
    LocationPtr l3 = nwk->LocationNew("l3",Location::port());
    LocationPtr l4 = nwk->LocationNew("l4",Location::port());
    LocationPtr l2_1 = nwk->LocationNew("l2_1",Location::port());
    LocationPtr l2_2 = nwk->LocationNew("l2_2",Location::port());

    connectLocations(l1,l2,nwk);
    connectLocations(l1,l3,nwk);
    connectLocations(l2,l4,nwk);
    connectLocations(l3,l4,nwk);
    connectLocations(l2,l2_1,nwk);
    connectLocations(l2,l2_2,nwk);
    connectLocations(l2_2,l2_1,nwk);

    ConnPtr conn = nwk->ConnNew("conn");
    FleetPtr fleet = nwk->FleetNew("fleet");
    fleet->speedIs(truck_,100);
    fleet->capacityIs(truck_,100);
    fleet->costIs(truck_,100);

    Conn::PathList paths = conn->paths(NULL,"l1","l4");

    ASSERT_TRUE(paths.size()==2);

    PathPtr path = paths[0];
    ASSERT_TRUE(path->pathElementCount() == 2);
}

TEST(Engine, conn_endpoint_no_loop_endpoint){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");

    LocationPtr l1 = nwk->LocationNew("l1",Location::port());
    LocationPtr l2 = nwk->LocationNew("l2",Location::port());
    LocationPtr l3 = nwk->LocationNew("l3",Location::port());
    LocationPtr l4 = nwk->LocationNew("l4",Location::port());
    LocationPtr l4_1 = nwk->LocationNew("l4_1",Location::port());
    LocationPtr l4_2 = nwk->LocationNew("l4_2",Location::port());

    connectLocations(l1,l2,nwk);
    connectLocations(l1,l3,nwk);
    connectLocations(l2,l4,nwk);
    connectLocations(l3,l4,nwk);
    connectLocations(l4,l4_1,nwk);
    connectLocations(l4,l4_2,nwk);
    connectLocations(l4_2,l4_1,nwk);

    ConnPtr conn = nwk->ConnNew("conn");
    FleetPtr fleet = nwk->FleetNew("fleet");
    fleet->speedIs(truck_,100);
    fleet->capacityIs(truck_,100);
    fleet->costIs(truck_,100);

    Conn::PathList paths = conn->paths(NULL,"l1","l4");

    ASSERT_TRUE(paths.size()==2);

    PathPtr path = paths[0];
    ASSERT_TRUE(path->pathElementCount() == 2);
}

TEST(Engine, conn_no_endpoint_distance_constraint){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");

    LocationPtr l1 = nwk->LocationNew("l1",Location::port());
    LocationPtr l2 = nwk->LocationNew("l2",Location::port());
    LocationPtr l3 = nwk->LocationNew("l3",Location::port());
    LocationPtr l4 = nwk->LocationNew("l4",Location::port());
    LocationPtr l5 = nwk->LocationNew("l5",Location::port());
    LocationPtr l6 = nwk->LocationNew("l6",Location::port());
    LocationPtr l7 = nwk->LocationNew("l7",Location::port());
    LocationPtr l8 = nwk->LocationNew("l8",Location::port());

    connectLocations(l1,l2,nwk,10);
    connectLocations(l1,l3,nwk,10);
    connectLocations(l3,l4,nwk,10);
    connectLocations(l2,l5,nwk,11);
    connectLocations(l2,l6,nwk,9);
    connectLocations(l3,l6,nwk,2);
    connectLocations(l4,l7,nwk,1);
    connectLocations(l5,l7,nwk,1);
    connectLocations(l6,l8,nwk,3);

    ConnPtr conn = nwk->ConnNew("conn");
    FleetPtr fleet = nwk->FleetNew("fleet");
    fleet->speedIs(truck_,100);
    fleet->capacityIs(truck_,100);
    fleet->costIs(truck_,100);

    Conn::ConstraintPtr constraint = Conn::DistanceConstraint::DistanceConstraintIs(20);
    Conn::PathList paths = conn->paths(constraint,"l1");

    ASSERT_TRUE(paths.size()==6);

    ASSERT_TRUE(paths[0]->pathElementCount()==1);
    ASSERT_TRUE(paths[0]->pathElement(0)->segment()->name() == "l1-l3");

    ASSERT_TRUE(paths[1]->pathElementCount()==2);
    ASSERT_TRUE(paths[1]->pathElement(0)->segment()->name() == "l1-l3");
    ASSERT_TRUE(paths[1]->pathElement(1)->segment()->name() == "l3-l6");

    ASSERT_TRUE(paths[2]->pathElementCount()==3);
    ASSERT_TRUE(paths[2]->pathElement(0)->segment()->name() == "l1-l3");
    ASSERT_TRUE(paths[2]->pathElement(1)->segment()->name() == "l3-l6");
    ASSERT_TRUE(paths[2]->pathElement(2)->segment()->name() == "l6-l8");

    ASSERT_TRUE(paths[3]->pathElementCount()==2);
    ASSERT_TRUE(paths[3]->pathElement(0)->segment()->name() == "l1-l3");
    ASSERT_TRUE(paths[3]->pathElement(1)->segment()->name() == "l3-l4");
    
    ASSERT_TRUE(paths[4]->pathElementCount()==1);
    ASSERT_TRUE(paths[4]->pathElement(0)->segment()->name() == "l1-l2");

    ASSERT_TRUE(paths[5]->pathElementCount()==2);
    ASSERT_TRUE(paths[5]->pathElement(0)->segment()->name() == "l1-l2");
    ASSERT_TRUE(paths[5]->pathElement(1)->segment()->name() == "l2-l6");
}

TEST(Engine, conn_no_endpoint_cost_constraint){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");

    LocationPtr l1 = nwk->LocationNew("l1",Location::port());
    LocationPtr l2 = nwk->LocationNew("l2",Location::port());
    LocationPtr l3 = nwk->LocationNew("l3",Location::port());
    LocationPtr l4 = nwk->LocationNew("l4",Location::port());
    LocationPtr l5 = nwk->LocationNew("l5",Location::port());
    LocationPtr l6 = nwk->LocationNew("l6",Location::port());
    LocationPtr l7 = nwk->LocationNew("l7",Location::port());
    LocationPtr l8 = nwk->LocationNew("l8",Location::port());

    connectLocations(l1,l2,nwk,40.0,2.0);
    connectLocations(l1,l3,nwk,40.0,2.0);
    connectLocations(l3,l4,nwk,40.0,2.0);
    connectLocations(l2,l5,nwk,44.0,2.0);
    connectLocations(l2,l6,nwk,36.0,2.0);
    connectLocations(l3,l6,nwk,8.0,2.0);
    connectLocations(l4,l7,nwk,4.0,2.0);
    connectLocations(l5,l7,nwk,4.0,2.0);
    connectLocations(l6,l8,nwk,12.0,2.0);

    ConnPtr conn = nwk->ConnNew("conn");
    FleetPtr fleet = nwk->FleetNew("fleet");
    fleet->speedIs(truck_,100);
    fleet->capacityIs(truck_,100);
    fleet->costIs(truck_,0.25);

    Conn::ConstraintPtr constraint = Conn::CostConstraint::CostConstraintIs(40);
    Conn::PathList paths = conn->paths(constraint,"l1");

    ASSERT_TRUE(paths.size()==6);

    ASSERT_TRUE(paths[0]->pathElementCount()==1);
    ASSERT_TRUE(paths[0]->pathElement(0)->segment()->name() == "l1-l3");

    ASSERT_TRUE(paths[1]->pathElementCount()==2);
    ASSERT_TRUE(paths[1]->pathElement(0)->segment()->name() == "l1-l3");
    ASSERT_TRUE(paths[1]->pathElement(1)->segment()->name() == "l3-l6");

    ASSERT_TRUE(paths[2]->pathElementCount()==3);
    ASSERT_TRUE(paths[2]->pathElement(0)->segment()->name() == "l1-l3");
    ASSERT_TRUE(paths[2]->pathElement(1)->segment()->name() == "l3-l6");
    ASSERT_TRUE(paths[2]->pathElement(2)->segment()->name() == "l6-l8");

    ASSERT_TRUE(paths[3]->pathElementCount()==2);
    ASSERT_TRUE(paths[3]->pathElement(0)->segment()->name() == "l1-l3");
    ASSERT_TRUE(paths[3]->pathElement(1)->segment()->name() == "l3-l4");

    ASSERT_TRUE(paths[4]->pathElementCount()==1);
    ASSERT_TRUE(paths[4]->pathElement(0)->segment()->name() == "l1-l2");

    ASSERT_TRUE(paths[5]->pathElementCount()==2);
    ASSERT_TRUE(paths[5]->pathElement(0)->segment()->name() == "l1-l2");
    ASSERT_TRUE(paths[5]->pathElement(1)->segment()->name() == "l2-l6");
}

TEST(Engine, conn_no_endpoint_time_constraint){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");

    LocationPtr l1 = nwk->LocationNew("l1",Location::port());
    LocationPtr l2 = nwk->LocationNew("l2",Location::port());
    LocationPtr l3 = nwk->LocationNew("l3",Location::port());
    LocationPtr l4 = nwk->LocationNew("l4",Location::port());
    LocationPtr l5 = nwk->LocationNew("l5",Location::port());
    LocationPtr l6 = nwk->LocationNew("l6",Location::port());
    LocationPtr l7 = nwk->LocationNew("l7",Location::port());
    LocationPtr l8 = nwk->LocationNew("l8",Location::port());

    connectLocations(l1,l2,nwk,5.0);
    connectLocations(l1,l3,nwk,5.0);
    connectLocations(l3,l4,nwk,5.0);
    connectLocations(l2,l5,nwk,5.5);
    connectLocations(l2,l6,nwk,4.5);
    connectLocations(l3,l6,nwk,1.0);
    connectLocations(l4,l7,nwk,0.5);
    connectLocations(l5,l7,nwk,0.5);
    connectLocations(l6,l8,nwk,1.5);

    ConnPtr conn = nwk->ConnNew("conn");
    FleetPtr fleet = nwk->FleetNew("fleet");
    fleet->speedIs(truck_,0.5);
    fleet->capacityIs(truck_,100);
    fleet->costIs(truck_,100);

    Conn::ConstraintPtr constraint = Conn::TimeConstraint::TimeConstraintIs(20.0);
    Conn::PathList paths = conn->paths(constraint,"l1");

    ASSERT_TRUE(paths.size()==6);

    ASSERT_TRUE(paths[0]->pathElementCount()==1);
    ASSERT_TRUE(paths[0]->pathElement(0)->segment()->name() == "l1-l3");

    ASSERT_TRUE(paths[1]->pathElementCount()==2);
    ASSERT_TRUE(paths[1]->pathElement(0)->segment()->name() == "l1-l3");
    ASSERT_TRUE(paths[1]->pathElement(1)->segment()->name() == "l3-l6");

    ASSERT_TRUE(paths[2]->pathElementCount()==3);
    ASSERT_TRUE(paths[2]->pathElement(0)->segment()->name() == "l1-l3");
    ASSERT_TRUE(paths[2]->pathElement(1)->segment()->name() == "l3-l6");
    ASSERT_TRUE(paths[2]->pathElement(2)->segment()->name() == "l6-l8");

    ASSERT_TRUE(paths[3]->pathElementCount()==2);
    ASSERT_TRUE(paths[3]->pathElement(0)->segment()->name() == "l1-l3");
    ASSERT_TRUE(paths[3]->pathElement(1)->segment()->name() == "l3-l4");

    ASSERT_TRUE(paths[4]->pathElementCount()==1);
    ASSERT_TRUE(paths[4]->pathElement(0)->segment()->name() == "l1-l2");

    ASSERT_TRUE(paths[5]->pathElementCount()==2);
    ASSERT_TRUE(paths[5]->pathElement(0)->segment()->name() == "l1-l2");
    ASSERT_TRUE(paths[5]->pathElement(1)->segment()->name() == "l2-l6");
}

TEST(Engine, conn_no_endpoint_expedited_constraint){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");

    LocationPtr l1 = nwk->LocationNew("l1",Location::port());
    LocationPtr l2 = nwk->LocationNew("l2",Location::port());
    LocationPtr l3 = nwk->LocationNew("l3",Location::port());
    LocationPtr l4 = nwk->LocationNew("l4",Location::port());
    LocationPtr l5 = nwk->LocationNew("l5",Location::port());
    LocationPtr l6 = nwk->LocationNew("l6",Location::port());
    LocationPtr l7 = nwk->LocationNew("l7",Location::port());
    LocationPtr l8 = nwk->LocationNew("l8",Location::port());

    connectLocations(l1,l2,nwk,10,1.0,Segment::expediteSupported());
    connectLocations(l1,l3,nwk,10,1.0,Segment::expediteSupported());
    connectLocations(l3,l4,nwk,10,1.0,Segment::expediteSupported());
    connectLocations(l2,l5,nwk,11,1.0,Segment::expediteUnsupported());
    connectLocations(l2,l6,nwk,9,1.0,Segment::expediteSupported());
    connectLocations(l3,l6,nwk,2,1.0,Segment::expediteSupported());
    connectLocations(l4,l7,nwk,1,1.0,Segment::expediteUnsupported());
    connectLocations(l5,l7,nwk,1,1.0,Segment::expediteUnsupported());
    connectLocations(l6,l8,nwk,3,1.0,Segment::expediteSupported());

    ConnPtr conn = nwk->ConnNew("conn");
    FleetPtr fleet = nwk->FleetNew("fleet");
    fleet->speedIs(truck_,100);
    fleet->capacityIs(truck_,100);
    fleet->costIs(truck_,100);

    Conn::ConstraintPtr constraint = Conn::ExpediteConstraint::ExpediteConstraintIs(Segment::expediteSupported());
    Conn::PathList paths = conn->paths(constraint,"l1");

    ASSERT_TRUE(paths.size()==10);

    ASSERT_TRUE(paths[0]->pathElementCount()==1);
    ASSERT_TRUE(paths[0]->pathElement(0)->segment()->name() == "l1-l3");

    ASSERT_TRUE(paths[1]->pathElementCount()==2);
    ASSERT_TRUE(paths[1]->pathElement(0)->segment()->name() == "l1-l3");
    ASSERT_TRUE(paths[1]->pathElement(1)->segment()->name() == "l3-l6");

    ASSERT_TRUE(paths[2]->pathElementCount()==3);
    ASSERT_TRUE(paths[2]->pathElement(0)->segment()->name() == "l1-l3");
    ASSERT_TRUE(paths[2]->pathElement(1)->segment()->name() == "l3-l6");
    ASSERT_TRUE(paths[2]->pathElement(2)->segment()->name() == "l6-l8");

    ASSERT_TRUE(paths[3]->pathElementCount()==3);
    ASSERT_TRUE(paths[3]->pathElement(0)->segment()->name() == "l1-l3");
    ASSERT_TRUE(paths[3]->pathElement(1)->segment()->name() == "l3-l6");
    ASSERT_TRUE(paths[3]->pathElement(2)->segment()->name() == "l6-l2");

    ASSERT_TRUE(paths[4]->pathElementCount()==2);
    ASSERT_TRUE(paths[4]->pathElement(0)->segment()->name() == "l1-l3");
    ASSERT_TRUE(paths[4]->pathElement(1)->segment()->name() == "l3-l4");

    ASSERT_TRUE(paths[5]->pathElementCount()==1);
    ASSERT_TRUE(paths[5]->pathElement(0)->segment()->name() == "l1-l2");

    ASSERT_TRUE(paths[6]->pathElementCount()==2);
    ASSERT_TRUE(paths[6]->pathElement(0)->segment()->name() == "l1-l2");
    ASSERT_TRUE(paths[6]->pathElement(1)->segment()->name() == "l2-l6");

    ASSERT_TRUE(paths[7]->pathElementCount()==3);
    ASSERT_TRUE(paths[7]->pathElement(0)->segment()->name() == "l1-l2");
    ASSERT_TRUE(paths[7]->pathElement(1)->segment()->name() == "l2-l6");
    ASSERT_TRUE(paths[7]->pathElement(2)->segment()->name() == "l6-l8");

    ASSERT_TRUE(paths[8]->pathElementCount()==3);
    ASSERT_TRUE(paths[8]->pathElement(0)->segment()->name() == "l1-l2");
    ASSERT_TRUE(paths[8]->pathElement(1)->segment()->name() == "l2-l6");
    ASSERT_TRUE(paths[8]->pathElement(2)->segment()->name() == "l6-l3");

    ASSERT_TRUE(paths[9]->pathElementCount()==4);
    ASSERT_TRUE(paths[9]->pathElement(0)->segment()->name() == "l1-l2");
    ASSERT_TRUE(paths[9]->pathElement(1)->segment()->name() == "l2-l6");
    ASSERT_TRUE(paths[9]->pathElement(2)->segment()->name() == "l6-l3");
    ASSERT_TRUE(paths[9]->pathElement(3)->segment()->name() == "l3-l4");
}

