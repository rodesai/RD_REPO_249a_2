#include "gtest/gtest.h"
#include <iostream>
#include "engine/Engine.h"

using namespace Shipping;

void connectLocations(LocationPtr l1,LocationPtr l2,ShippingNetworkPtr nwk,Mile length,Difficulty d,bool expediteSupport){
    SegmentPtr segment,segmentR;
    segment = nwk->SegmentNew(l1->name() + "-" + l2->name(),TransportMode::truck(),PathMode::unexpedited());
    segment->lengthIs(length);
    segment->difficultyIs(d);
    if(expediteSupport) segment->modeIs(PathMode::expedited());
    segmentR = nwk->SegmentNew(l2->name() + "-" + l1->name(),TransportMode::truck(),PathMode::unexpedited());
    segmentR->lengthIs(length);
    segmentR->difficultyIs(d);
    if(expediteSupport) segmentR->modeIs(PathMode::expedited());
    segment->sourceIs(l1->name());
    segmentR->sourceIs(l2->name());
    segment->returnSegmentIs(segmentR->name());
}

void connectLocations(LocationPtr l1,LocationPtr l2,ShippingNetworkPtr nwk,Mile length,Difficulty d){
    connectLocations(l1,l2,nwk,length,d,false);
}

void connectLocations(LocationPtr l1,LocationPtr l2,ShippingNetworkPtr nwk, Mile length){
    connectLocations(l1,l2,nwk,length,1.0);
}

void connectLocations(LocationPtr l1,LocationPtr l2,ShippingNetworkPtr nwk){
    connectLocations(l1,l2,nwk,100,1.0);
}

TEST(Engine, conn_invalid_start){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");

    LocationPtr l1 = nwk->LocationNew("l1",Location::EntityType::port());
    LocationPtr l2 = nwk->LocationNew("l2",Location::EntityType::port());

    connectLocations(l1,l2,nwk);

    ConnPtr conn = nwk->ConnNew("conn");

    Conn::PathSelector selector(NULL,NULL);
    selector.modeIs(PathMode::expedited());
    Conn::PathList paths = conn->paths(selector);

    ASSERT_TRUE(paths.size()==0);

    ShippingNetworkPtr nwk2 = ShippingNetwork::ShippingNetworkIs("network2");
    nwk2->LocationNew("l1",Location::EntityType::port());
    nwk2->LocationNew("l2",Location::EntityType::port());
    connectLocations(nwk2->location("l1"),nwk2->location("l2"),nwk2);

    selector=Conn::PathSelector(NULL,nwk2->location("l1"));
    selector.modeIs(PathMode::expedited());
    paths = conn->paths(selector);

    ASSERT_TRUE(paths.size()==0);
}

TEST(Engine, conn_invalid_end){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");

    LocationPtr l1 = nwk->LocationNew("l1",Location::EntityType::port());
    LocationPtr l2 = nwk->LocationNew("l2",Location::EntityType::port());
    
    connectLocations(l1,l2,nwk);
    
    ConnPtr conn = nwk->ConnNew("conn");
 
    Conn::PathSelector selector(NULL,l1);
    selector.modeIs(PathMode::expedited()); selector.modeIs(PathMode::unexpedited());
    Conn::PathList paths = conn->paths(selector);

    ASSERT_TRUE(paths.size()==1);
    ASSERT_TRUE(paths[0]->pathElementCount() == 1);
    ASSERT_TRUE(paths[0]->pathElement(0)->segment()->name() == "l1-l2");

    ShippingNetworkPtr nwk2 = ShippingNetwork::ShippingNetworkIs("network2");
    nwk2->LocationNew("l2",Location::EntityType::port());

    selector=Conn::PathSelector(NULL,l1,nwk2->location("l2"));
    selector.modeIs(PathMode::expedited());
    paths = conn->paths(selector);

    ASSERT_TRUE(paths.size()==0);
}

TEST(Engine, conn_line){
    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");

    LocationPtr l1 = nwk->LocationNew("l1",Location::EntityType::port());
    LocationPtr l2 = nwk->LocationNew("l2",Location::EntityType::port());
    LocationPtr l3 = nwk->LocationNew("l3",Location::EntityType::port());

    connectLocations(l1,l2,nwk,100,1.0,true);
    connectLocations(l2,l3,nwk,100,1.0,true);

    ConnPtr conn = nwk->ConnNew("conn");
    FleetPtr fleet = nwk->FleetNew("fleet");
    fleet->speedIs(TransportMode::truck(),100);
    fleet->capacityIs(TransportMode::truck(),100);
    fleet->costIs(TransportMode::truck(),100);
 
    Conn::PathSelector selector(NULL,l1,l2);
    selector.modeIs(PathMode::expedited()); selector.modeIs(PathMode::unexpedited());
    Conn::PathList paths = conn->paths(selector);

    ASSERT_TRUE(paths.size()==2);
    ASSERT_TRUE(paths[0]->pathElementCount() == 1);
    ASSERT_TRUE(paths[0]->pathElement(0)->segment()->name() == "l1-l2");
    ASSERT_TRUE(paths[1]->pathElementCount() == 1);
    ASSERT_TRUE(paths[1]->pathElement(0)->segment()->name() == "l1-l2");

    selector = Conn::PathSelector(NULL,l1,l3);
    selector.modeIs(PathMode::expedited()); selector.modeIs(PathMode::unexpedited());
    paths = conn->paths(selector);
  
    ASSERT_TRUE(paths.size()==4);
    ASSERT_TRUE(paths[0]->pathElementCount() == 2);
    ASSERT_TRUE(paths[0]->pathElement(0)->segment()->name() == "l1-l2");
    ASSERT_TRUE(paths[0]->pathElement(1)->segment()->name() == "l2-l3");
    ASSERT_TRUE(paths[1]->pathElementCount() == 2);
    ASSERT_TRUE(paths[1]->pathElement(0)->segment()->name() == "l1-l2");
    ASSERT_TRUE(paths[1]->pathElement(1)->segment()->name() == "l2-l3");
    ASSERT_TRUE(paths[2]->pathElementCount() == 2);
    ASSERT_TRUE(paths[2]->pathElement(0)->segment()->name() == "l1-l2");
    ASSERT_TRUE(paths[2]->pathElement(1)->segment()->name() == "l2-l3");
    ASSERT_TRUE(paths[3]->pathElementCount() == 2);
    ASSERT_TRUE(paths[3]->pathElement(0)->segment()->name() == "l1-l2");
    ASSERT_TRUE(paths[3]->pathElement(1)->segment()->name() == "l2-l3");
    ASSERT_TRUE(paths[1]->pathElementCount() == 2);
    ASSERT_TRUE(paths[1]->pathElement(0)->segment()->name() == "l1-l2");
    ASSERT_TRUE(paths[1]->pathElement(1)->segment()->name() == "l2-l3");
}

TEST(Engine, conn_endpoint_basic){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");

    LocationPtr l1 = nwk->LocationNew("l1",Location::EntityType::port());
    LocationPtr l2 = nwk->LocationNew("l2",Location::EntityType::port());
    LocationPtr l3 = nwk->LocationNew("l3",Location::EntityType::port());
    LocationPtr l4 = nwk->LocationNew("l4",Location::EntityType::port());

    connectLocations(l1,l2,nwk,10.0,1.0,true);
    connectLocations(l1,l3,nwk,10.0,1.0,true);
    connectLocations(l2,l4,nwk,10.0,1.0,true);
    connectLocations(l3,l4,nwk,10.0,1.0,true);

    ConnPtr conn = nwk->ConnNew("conn");
    FleetPtr fleet = nwk->FleetNew("fleet");
    fleet->speedIs(TransportMode::truck(),100);
    fleet->capacityIs(TransportMode::truck(),100);
    fleet->costIs(TransportMode::truck(),100);

    Conn::PathSelector selector(NULL,l1,l4);
    selector.modeIs(PathMode::expedited()); selector.modeIs(PathMode::unexpedited());
    Conn::PathList paths = conn->paths(selector);

    ASSERT_TRUE(paths.size()==8);

    selector = Conn::PathSelector(NULL,l1,l4);
    selector.modeIs(PathMode::expedited());
    paths = conn->paths(selector);

    ASSERT_TRUE(paths.size()==2);

    selector = Conn::PathSelector(NULL,l1,l4);
    selector.modeIs(PathMode::expedited());
    paths = conn->paths(selector);

    ASSERT_TRUE(paths.size()==2);
}

TEST(Engine, conn_endpoint_no_loop_pre_endpoint){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");

    LocationPtr l1 = nwk->LocationNew("l1",Location::EntityType::port());
    LocationPtr l2 = nwk->LocationNew("l2",Location::EntityType::port());
    LocationPtr l3 = nwk->LocationNew("l3",Location::EntityType::port());
    LocationPtr l4 = nwk->LocationNew("l4",Location::EntityType::port());
    LocationPtr l2_1 = nwk->LocationNew("l2_1",Location::EntityType::port());
    LocationPtr l2_2 = nwk->LocationNew("l2_2",Location::EntityType::port());

    connectLocations(l1,l2,nwk);
    connectLocations(l1,l3,nwk);
    connectLocations(l2,l4,nwk);
    connectLocations(l3,l4,nwk);
    connectLocations(l2,l2_1,nwk);
    connectLocations(l2,l2_2,nwk);
    connectLocations(l2_2,l2_1,nwk);

    ConnPtr conn = nwk->ConnNew("conn");
    FleetPtr fleet = nwk->FleetNew("fleet");
    fleet->speedIs(TransportMode::truck(),100);
    fleet->capacityIs(TransportMode::truck(),100);
    fleet->costIs(TransportMode::truck(),100);

    Conn::PathSelector selector = Conn::PathSelector(NULL,l1,l4);
    selector.modeIs(PathMode::expedited()); selector.modeIs(PathMode::unexpedited());
    Conn::PathList paths = conn->paths(selector);               

    ASSERT_TRUE(paths.size()==2);

    PathPtr path = paths[0];
    ASSERT_TRUE(path->pathElementCount() == 2);
    ASSERT_TRUE(path->pathElement(0)->segment()->name() == "l1-l3");
    ASSERT_TRUE(path->pathElement(0)->mode() == PathMode::unexpedited());
    ASSERT_TRUE(path->pathElement(1)->segment()->name() == "l3-l4");
    ASSERT_TRUE(path->pathElement(1)->mode() == PathMode::unexpedited());
    path = paths[1];
    ASSERT_TRUE(path->pathElementCount() == 2);
    ASSERT_TRUE(path->pathElement(0)->segment()->name() == "l1-l2");
    ASSERT_TRUE(path->pathElement(0)->mode() == PathMode::unexpedited());
    ASSERT_TRUE(path->pathElement(1)->segment()->name() == "l2-l4");
    ASSERT_TRUE(path->pathElement(1)->mode() == PathMode::unexpedited());
}

TEST(Engine, conn_endpoint_no_loop_endpoint){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");

    LocationPtr l1 = nwk->LocationNew("l1",Location::EntityType::port());
    LocationPtr l2 = nwk->LocationNew("l2",Location::EntityType::port());
    LocationPtr l3 = nwk->LocationNew("l3",Location::EntityType::port());
    LocationPtr l4 = nwk->LocationNew("l4",Location::EntityType::port());
    LocationPtr l4_1 = nwk->LocationNew("l4_1",Location::EntityType::port());
    LocationPtr l4_2 = nwk->LocationNew("l4_2",Location::EntityType::port());

    connectLocations(l1,l2,nwk);
    connectLocations(l1,l3,nwk);
    connectLocations(l2,l4,nwk);
    connectLocations(l3,l4,nwk);
    connectLocations(l4,l4_1,nwk);
    connectLocations(l4,l4_2,nwk);
    connectLocations(l4_2,l4_1,nwk);

    ConnPtr conn = nwk->ConnNew("conn");
    FleetPtr fleet = nwk->FleetNew("fleet");
    fleet->speedIs(TransportMode::truck(),100);
    fleet->capacityIs(TransportMode::truck(),100);
    fleet->costIs(TransportMode::truck(),100);

    Conn::PathSelector selector(NULL,l1,l4);
    selector.modeIs(PathMode::expedited()); selector.modeIs(PathMode::unexpedited());
    Conn::PathList paths = conn->paths(selector);

    ASSERT_TRUE(paths.size()==2);

    PathPtr path = paths[0];
    ASSERT_TRUE(path->pathElementCount() == 2);
    ASSERT_TRUE(path->pathElement(0)->segment()->name() == "l1-l3");
    ASSERT_TRUE(path->pathElement(0)->mode() == PathMode::unexpedited());
    ASSERT_TRUE(path->pathElement(1)->segment()->name() == "l3-l4");
    ASSERT_TRUE(path->pathElement(1)->mode() == PathMode::unexpedited());
    path = paths[1];
    ASSERT_TRUE(path->pathElementCount() == 2);
    ASSERT_TRUE(path->pathElement(0)->segment()->name() == "l1-l2");
    ASSERT_TRUE(path->pathElement(0)->mode() == PathMode::unexpedited());
    ASSERT_TRUE(path->pathElement(1)->segment()->name() == "l2-l4");
    ASSERT_TRUE(path->pathElement(1)->mode() == PathMode::unexpedited());
}

TEST(Engine, conn_no_endpoint_distance_constraint){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");

    LocationPtr l1 = nwk->LocationNew("l1",Location::EntityType::port());
    LocationPtr l2 = nwk->LocationNew("l2",Location::EntityType::port());
    LocationPtr l3 = nwk->LocationNew("l3",Location::EntityType::port());
    LocationPtr l4 = nwk->LocationNew("l4",Location::EntityType::port());
    LocationPtr l5 = nwk->LocationNew("l5",Location::EntityType::port());
    LocationPtr l6 = nwk->LocationNew("l6",Location::EntityType::port());
    LocationPtr l7 = nwk->LocationNew("l7",Location::EntityType::port());
    LocationPtr l8 = nwk->LocationNew("l8",Location::EntityType::port());

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
    fleet->speedIs(TransportMode::truck(),100);
    fleet->capacityIs(TransportMode::truck(),100);
    fleet->costIs(TransportMode::truck(),100);

    Conn::ConstraintPtr constraint = Conn::DistanceConstraint::DistanceConstraintIs(20);
    Conn::PathSelector selector(constraint,l1);
    selector.modeIs(PathMode::expedited()); selector.modeIs(PathMode::unexpedited());
    Conn::PathList paths = conn->paths(selector);

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

    LocationPtr l1 = nwk->LocationNew("l1",Location::EntityType::port());
    LocationPtr l2 = nwk->LocationNew("l2",Location::EntityType::port());
    LocationPtr l3 = nwk->LocationNew("l3",Location::EntityType::port());
    LocationPtr l4 = nwk->LocationNew("l4",Location::EntityType::port());
    LocationPtr l5 = nwk->LocationNew("l5",Location::EntityType::port());
    LocationPtr l6 = nwk->LocationNew("l6",Location::EntityType::port());
    LocationPtr l7 = nwk->LocationNew("l7",Location::EntityType::port());
    LocationPtr l8 = nwk->LocationNew("l8",Location::EntityType::port());

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
    fleet->speedIs(TransportMode::truck(),100);
    fleet->capacityIs(TransportMode::truck(),100);
    fleet->costIs(TransportMode::truck(),0.25);

    Conn::ConstraintPtr constraint = Conn::CostConstraint::CostConstraintIs(40);
    Conn::PathSelector selector(constraint,l1);
    selector.modeIs(PathMode::expedited()); selector.modeIs(PathMode::unexpedited());
    Conn::PathList paths = conn->paths(selector);

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

    LocationPtr l1 = nwk->LocationNew("l1",Location::EntityType::port());
    LocationPtr l2 = nwk->LocationNew("l2",Location::EntityType::port());
    LocationPtr l3 = nwk->LocationNew("l3",Location::EntityType::port());
    LocationPtr l4 = nwk->LocationNew("l4",Location::EntityType::port());
    LocationPtr l5 = nwk->LocationNew("l5",Location::EntityType::port());
    LocationPtr l6 = nwk->LocationNew("l6",Location::EntityType::port());
    LocationPtr l7 = nwk->LocationNew("l7",Location::EntityType::port());
    LocationPtr l8 = nwk->LocationNew("l8",Location::EntityType::port());

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
    fleet->speedIs(TransportMode::truck(),0.5);
    fleet->capacityIs(TransportMode::truck(),100);
    fleet->costIs(TransportMode::truck(),100);

    Conn::ConstraintPtr constraint = Conn::TimeConstraint::TimeConstraintIs(20.0);
    Conn::PathSelector selector(constraint,l1);
    selector.modeIs(PathMode::expedited()); selector.modeIs(PathMode::unexpedited());
    Conn::PathList paths = conn->paths(selector);

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

    LocationPtr l1 = nwk->LocationNew("l1",Location::EntityType::port());
    LocationPtr l2 = nwk->LocationNew("l2",Location::EntityType::port());
    LocationPtr l3 = nwk->LocationNew("l3",Location::EntityType::port());
    LocationPtr l4 = nwk->LocationNew("l4",Location::EntityType::port());
    LocationPtr l5 = nwk->LocationNew("l5",Location::EntityType::port());
    LocationPtr l6 = nwk->LocationNew("l6",Location::EntityType::port());
    LocationPtr l7 = nwk->LocationNew("l7",Location::EntityType::port());
    LocationPtr l8 = nwk->LocationNew("l8",Location::EntityType::port());

    connectLocations(l1,l2,nwk,10,1.0,true);
    connectLocations(l1,l3,nwk,10,1.0,true);
    connectLocations(l3,l4,nwk,10,1.0,true);
    connectLocations(l2,l5,nwk,11,1.0,false);
    connectLocations(l2,l6,nwk,9,1.0,true);
    connectLocations(l3,l6,nwk,2,1.0,true);
    connectLocations(l4,l7,nwk,1,1.0,false);
    connectLocations(l5,l7,nwk,1,1.0,false);
    connectLocations(l6,l8,nwk,3,1.0,true);

    ConnPtr conn = nwk->ConnNew("conn");
    FleetPtr fleet = nwk->FleetNew("fleet");
    fleet->speedIs(TransportMode::truck(),100);
    fleet->capacityIs(TransportMode::truck(),100);
    fleet->costIs(TransportMode::truck(),100);

    Conn::PathSelector selector(NULL,l1);
    selector.modeIs(PathMode::expedited());
    Conn::PathList paths = conn->paths(selector);

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

TEST(Engine, conn_no_endpoint_distance_time_constraint){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");

    LocationPtr l1 = nwk->LocationNew("l1",Location::EntityType::port());
    LocationPtr l2 = nwk->LocationNew("l2",Location::EntityType::port());
    LocationPtr l3 = nwk->LocationNew("l3",Location::EntityType::port());
    LocationPtr l4 = nwk->LocationNew("l4",Location::EntityType::port());
    LocationPtr l5 = nwk->LocationNew("l5",Location::EntityType::port());
    LocationPtr l6 = nwk->LocationNew("l6",Location::EntityType::port());
    LocationPtr l7 = nwk->LocationNew("l7",Location::EntityType::port());
    LocationPtr l8 = nwk->LocationNew("l8",Location::EntityType::port());

    connectLocations(l1,l2,nwk,10.0);
    connectLocations(l1,l3,nwk,10.0);
    connectLocations(l3,l4,nwk,10.0);
    connectLocations(l2,l5,nwk,11.0);
    connectLocations(l2,l6,nwk,9.0);
    connectLocations(l3,l6,nwk,2.0);
    connectLocations(l4,l7,nwk,1.0);
    connectLocations(l5,l7,nwk,1.0);
    connectLocations(l6,l8,nwk,3.0);

    ConnPtr conn = nwk->ConnNew("conn");
    FleetPtr fleet = nwk->FleetNew("fleet");
    fleet->speedIs(TransportMode::truck(),0.5);
    fleet->capacityIs(TransportMode::truck(),100);
    fleet->costIs(TransportMode::truck(),100);

    Conn::ConstraintPtr constraint;
    constraint = Conn::DistanceConstraint::DistanceConstraintIs(20.0);
    constraint->nextIs(Conn::TimeConstraint::TimeConstraintIs(30.0));
    Conn::PathSelector selector(constraint,l1);
    selector.modeIs(PathMode::expedited()); selector.modeIs(PathMode::unexpedited());
    Conn::PathList paths = conn->paths(selector);

    ASSERT_TRUE(paths.size()==4);

    ASSERT_TRUE(paths[0]->pathElementCount()==1);
    ASSERT_TRUE(paths[0]->pathElement(0)->segment()->name() == "l1-l3");

    ASSERT_TRUE(paths[1]->pathElementCount()==2);
    ASSERT_TRUE(paths[1]->pathElement(0)->segment()->name() == "l1-l3");
    ASSERT_TRUE(paths[1]->pathElement(1)->segment()->name() == "l3-l6");

    ASSERT_TRUE(paths[2]->pathElementCount()==3);
    ASSERT_TRUE(paths[2]->pathElement(0)->segment()->name() == "l1-l3");
    ASSERT_TRUE(paths[2]->pathElement(1)->segment()->name() == "l3-l6");
    ASSERT_TRUE(paths[2]->pathElement(2)->segment()->name() == "l6-l8");

    ASSERT_TRUE(paths[3]->pathElementCount()==1);
    ASSERT_TRUE(paths[3]->pathElement(0)->segment()->name() == "l1-l2");
}


TEST(Engine, SegmentNew){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");
    SegmentPtr segment = nwk->SegmentNew("segment1",TransportMode::truck(),PathMode::unexpedited());

    ASSERT_TRUE(segment);
    ASSERT_TRUE(nwk->segment("segment1"));
}

TEST(Engine, LocationNew){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");
    LocationPtr location = nwk->LocationNew("location1",Location::EntityType::port());

    ASSERT_TRUE(location);
    ASSERT_TRUE(nwk->location("location1"));
}

TEST(Engine, SegmentSource){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");

    SegmentPtr segment1 = nwk->SegmentNew("segment1",TransportMode::truck(),PathMode::unexpedited());
    SegmentPtr segment2 = nwk->SegmentNew("segment2",TransportMode::truck(),PathMode::unexpedited());
    SegmentPtr segment3 = nwk->SegmentNew("segment3",TransportMode::truck(),PathMode::unexpedited());

    LocationPtr location = nwk->LocationNew("location1",Location::EntityType::port());

    segment1->sourceIs(location->name());

    ASSERT_TRUE(segment1->source() == location);
    ASSERT_TRUE(location->segment(1)->name() == "segment1");

    segment2->sourceIs(location->name());

    ASSERT_TRUE(segment2->source() == location);
    ASSERT_TRUE(location->segment(2)->name() == "segment2");

    segment3->sourceIs(location->name());

    ASSERT_TRUE(segment3->source() == location);
    ASSERT_TRUE(location->segment(3)->name() == "segment3");

    segment2->sourceIs("");

    ASSERT_TRUE(location->segmentCount() == 2);
    ASSERT_TRUE(location->segment(1)->name() == "segment1");
    ASSERT_TRUE(location->segment(2)->name() == "segment3");
    ASSERT_FALSE(segment2->source());
}

TEST(Engine, SegmentReturnSegment){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");

    SegmentPtr segment1 = nwk->SegmentNew("segment1",TransportMode::truck(),PathMode::unexpedited());
    SegmentPtr segment2 = nwk->SegmentNew("segment2",TransportMode::truck(),PathMode::unexpedited());

    segment1->returnSegmentIs(segment2->name());

    ASSERT_TRUE(segment1->returnSegment()==segment2);
    ASSERT_TRUE(segment2->returnSegment()==segment1);

    segment1->returnSegmentIs("");

    ASSERT_FALSE(segment1->returnSegment());
    ASSERT_FALSE(segment2->returnSegment());

    SegmentPtr segment3 = nwk->SegmentNew("segment3",TransportMode::truck(),PathMode::unexpedited());

    segment1->returnSegmentIs(segment2->name());

    ASSERT_TRUE(segment1->returnSegment()==segment2);
    ASSERT_TRUE(segment2->returnSegment()==segment1);

    segment2->returnSegmentIs(segment3->name());

    ASSERT_FALSE(segment1->returnSegment());
    ASSERT_TRUE(segment2->returnSegment()==segment3);
    ASSERT_TRUE(segment3->returnSegment()==segment2);
}

TEST(Engine, Location_segmentCount){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");

    SegmentPtr segment1 = nwk->SegmentNew("segment1",TransportMode::truck(),PathMode::unexpedited());
    SegmentPtr segment2 = nwk->SegmentNew("segment2",TransportMode::truck(),PathMode::unexpedited());
    SegmentPtr segment3 = nwk->SegmentNew("segment3",TransportMode::truck(),PathMode::unexpedited());

    LocationPtr location = nwk->LocationNew("location1",Location::EntityType::port());

    ASSERT_TRUE(location->segmentCount()==0);

    segment1->sourceIs("location1");
    ASSERT_TRUE(location->segmentCount()==1);

    segment2->sourceIs("location1");
    ASSERT_TRUE(location->segmentCount()==2);

    segment3->sourceIs("location1");
    ASSERT_TRUE(location->segmentCount()==3);

    segment2->sourceIs("nil");
    ASSERT_TRUE(location->segmentCount()==2);
}

TEST(Engine, Location_segment){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");

    SegmentPtr segment1 = nwk->SegmentNew("segment1",TransportMode::truck(),PathMode::unexpedited());
    SegmentPtr segment2 = nwk->SegmentNew("segment2",TransportMode::truck(),PathMode::unexpedited());
    SegmentPtr segment3 = nwk->SegmentNew("segment3",TransportMode::truck(),PathMode::unexpedited());

    LocationPtr location = nwk->LocationNew("location1",Location::EntityType::port());

    ASSERT_TRUE(location->segmentCount()==0);

    segment1->sourceIs("location1");
    ASSERT_TRUE(location->segment(1) == segment1);

    segment2->sourceIs("location1");
    ASSERT_TRUE(location->segment(1) == segment1);
    ASSERT_TRUE(location->segment(2) == segment2);

    segment3->sourceIs("location1");
    ASSERT_TRUE(location->segment(1) == segment1);
    ASSERT_TRUE(location->segment(2) == segment2);
    ASSERT_TRUE(location->segment(3) == segment3);

    segment2->sourceIs("nil");
    ASSERT_TRUE(location->segment(1) == segment1);
    ASSERT_TRUE(location->segment(2) == segment3);
}

TEST(Engine, Location_entityType){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");

    LocationPtr location = nwk->LocationNew("location1",Location::EntityType::port());
    ASSERT_TRUE(location->entityType() == Location::EntityType::port());

    location = nwk->LocationNew("location2",Location::EntityType::customer());
    ASSERT_TRUE(location->entityType() == Location::EntityType::customer());

    location = nwk->LocationNew("location3",Location::EntityType::truckTerminal());
    ASSERT_TRUE(location->entityType() == Location::EntityType::truckTerminal());

    location = nwk->LocationNew("location4",Location::EntityType::boatTerminal());
    ASSERT_TRUE(location->entityType() == Location::EntityType::boatTerminal());

    location = nwk->LocationNew("location5",Location::EntityType::planeTerminal());
    ASSERT_TRUE(location->entityType() == Location::EntityType::planeTerminal());
}

TEST(Engine, Segment_length){
    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");
    SegmentPtr segment = nwk->SegmentNew("segment",TransportMode::boat(),PathMode::unexpedited());
    ASSERT_TRUE(segment->length() == 1.0);
    segment->lengthIs(2.0);
    ASSERT_TRUE(segment->length() == 2.0);
}

TEST(Engine, Segment_difficulty){
    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");
    SegmentPtr segment = nwk->SegmentNew("segment",TransportMode::boat(),PathMode::unexpedited());
    ASSERT_TRUE(segment->difficulty() == 1.0);
    segment->difficultyIs(2.0);
    ASSERT_TRUE(segment->difficulty() == 2.0);
}

TEST(Engine, Segment_mode){
    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");
    SegmentPtr segment = nwk->SegmentNew("segment",TransportMode::boat(),PathMode::unexpedited());
    ASSERT_TRUE(segment->modeCount() == 1);
    ASSERT_TRUE(segment->mode(PathMode::unexpedited()) == PathMode::unexpedited());
    ASSERT_TRUE(segment->transportMode() == TransportMode::boat());
    StatsPtr stats = nwk->StatsNew("stats");
    ASSERT_TRUE(stats->segmentCount(TransportMode::boat()) == 1);
    segment->modeIs(PathMode::expedited());
    ASSERT_TRUE(segment->modeCount() == 2);
    ASSERT_TRUE(segment->mode(PathMode::unexpedited()) == PathMode::unexpedited());
    ASSERT_TRUE(segment->mode(PathMode::expedited()) == PathMode::expedited());
    ASSERT_TRUE(stats->segmentCount(PathMode::expedited()) == 1);
    ASSERT_TRUE(stats->segmentCount(PathMode::unexpedited()) == 1);
    segment->modeDel(PathMode::expedited());
    ASSERT_TRUE(segment->modeCount() == 1);
    ASSERT_TRUE(segment->mode(PathMode::unexpedited()) == PathMode::unexpedited());
    ASSERT_TRUE(segment->mode(PathMode::expedited()) == PathMode::undef());
    ASSERT_TRUE(stats->segmentCount(PathMode::unexpedited()) == 1);
    ASSERT_TRUE(stats->segmentCount(PathMode::expedited()) == 0);
}

TEST(Engine, Fleet){
    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");
    FleetPtr fleet = nwk->FleetNew("fleet");
    ASSERT_TRUE(fleet->cost(TransportMode::truck()) == DollarPerMile::defaultValue());
    ASSERT_TRUE(fleet->speed(TransportMode::truck()) == MilePerHour::defaultValue());
    ASSERT_TRUE(fleet->capacity(TransportMode::truck()) == PackageNum::defaultValue());
    fleet->costIs(TransportMode::truck(),2.0);
    fleet->speedIs(TransportMode::truck(),3.0);
    fleet->capacityIs(TransportMode::truck(),4.0);
    ASSERT_TRUE(fleet->cost(TransportMode::truck()) == 2.0);
    ASSERT_TRUE(fleet->speed(TransportMode::truck()) == 3.0);
    ASSERT_TRUE(fleet->capacity(TransportMode::truck()) == 4.0);
    fleet->costIs(TransportMode::plane(),2.5);
    fleet->speedIs(TransportMode::plane(),3.5);
    fleet->capacityIs(TransportMode::plane(),4.5);
    ASSERT_TRUE(fleet->cost(TransportMode::plane()) == 2.5);
    ASSERT_TRUE(fleet->speed(TransportMode::plane()) == 3.5);
    ASSERT_TRUE(fleet->capacity(TransportMode::plane()) == 4.5);
    fleet->costIs(TransportMode::boat(),2.5);
    fleet->speedIs(TransportMode::boat(),3.5);
    fleet->capacityIs(TransportMode::boat(),4.5);
    ASSERT_TRUE(fleet->cost(TransportMode::boat()) == 2.5);
    ASSERT_TRUE(fleet->speed(TransportMode::boat()) == 3.5);
    ASSERT_TRUE(fleet->capacity(TransportMode::boat()) == 4.5);
}

TEST(Engine, Path_emptyPath){
    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");
    PathPtr path = Path::PathIs(nwk->LocationNew("l1",Location::EntityType::port()));
    ASSERT_TRUE(path->cost() == 0.0);
    ASSERT_TRUE(path->time() == 0.0);
    ASSERT_TRUE(path->distance() == 0.0);
}

TEST(Engine, Path_pathEnq){
    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");
    PathPtr path = Path::PathIs(nwk->LocationNew("l1",Location::EntityType::port()));
    ASSERT_TRUE(path->cost() == 0.0);
    ASSERT_TRUE(path->time() == 0.0);
    ASSERT_TRUE(path->distance() == 0.0);

    nwk->SegmentNew("s1",TransportMode::truck(),PathMode::unexpedited());
    nwk->SegmentNew("s1r",TransportMode::truck(),PathMode::unexpedited());
    nwk->LocationNew("l1",Location::EntityType::port());
    nwk->LocationNew("l2",Location::EntityType::port());
    nwk->segment("s1")->sourceIs("l1");
    nwk->segment("s1r")->sourceIs("l2");
    nwk->segment("s1")->returnSegmentIs("s1r");
    nwk->segment("s1")->lengthIs(10.1);
    Path::PathElementPtr pathElement = Path::PathElement::PathElementIs(nwk->segment("s1"),PathMode::unexpedited());
    path->pathElementEnq(pathElement,11.1,22.2,10.1);
    ASSERT_TRUE(path->cost() == 11.1);
    ASSERT_TRUE(path->time() == 22.2);
    ASSERT_TRUE(path->distance() == 10.1);

    nwk->SegmentNew("s2",TransportMode::truck(),PathMode::unexpedited());
    nwk->SegmentNew("s2r",TransportMode::truck(),PathMode::unexpedited());
    nwk->LocationNew("l3",Location::EntityType::port());
    nwk->LocationNew("l4",Location::EntityType::port());
    nwk->segment("s2")->sourceIs("l3");
    nwk->segment("s2r")->sourceIs("l4");
    nwk->segment("s2")->returnSegmentIs("s2r");
    nwk->segment("s2")->lengthIs(20.0);
    pathElement = Path::PathElement::PathElementIs(nwk->segment("s2"),PathMode::unexpedited());
    path->pathElementEnq(pathElement,33.3,55.5,20.0);
    ASSERT_TRUE(path->cost() == 44.4);
    ASSERT_TRUE(path->time() == 77.7);
    ASSERT_TRUE(path->distance() == 30.1);
}

TEST(Engine, Stats){

    ShippingNetworkPtr nwk = ShippingNetwork::ShippingNetworkIs("network");

    SegmentPtr segment = nwk->SegmentNew("segment1",TransportMode::truck(),PathMode::unexpedited());
    LocationPtr location = nwk->LocationNew("location1",Location::EntityType::port());
    StatsPtr stat = nwk->StatsNew("stat1");

    ASSERT_TRUE(stat->locationCount(Location::EntityType::boatTerminal()) == 0);
    ASSERT_TRUE(stat->locationCount(Location::EntityType::planeTerminal()) == 0);
    ASSERT_TRUE(stat->locationCount(Location::EntityType::truckTerminal()) == 0); 
    std::cout << "customer count: " << stat->locationCount(Location::EntityType::customer()) << std::endl;
    ASSERT_TRUE(stat->locationCount(Location::EntityType::customer()) == 0);
    ASSERT_TRUE(stat->locationCount(Location::EntityType::port()) == 1);
    ASSERT_TRUE(stat->segmentCount(TransportMode::truck()) == 1);
    ASSERT_TRUE(stat->segmentCount(TransportMode::plane()) == 0);
    ASSERT_TRUE(stat->segmentCount(TransportMode::boat()) == 0);

    segment = nwk->SegmentNew("segment2",TransportMode::plane(),PathMode::unexpedited());

    ASSERT_TRUE(stat->locationCount(Location::EntityType::boatTerminal()) == 0);
    ASSERT_TRUE(stat->locationCount(Location::EntityType::planeTerminal()) == 0);
    ASSERT_TRUE(stat->locationCount(Location::EntityType::truckTerminal()) == 0);
    ASSERT_TRUE(stat->locationCount(Location::EntityType::customer()) == 0);
    ASSERT_TRUE(stat->locationCount(Location::EntityType::port()) == 1);
    ASSERT_TRUE(stat->segmentCount(TransportMode::truck()) == 1);
    ASSERT_TRUE(stat->segmentCount(TransportMode::plane()) == 1);
    ASSERT_TRUE(stat->segmentCount(TransportMode::boat()) == 0);

    segment = nwk->SegmentNew("segment3",TransportMode::boat(),PathMode::unexpedited());

    ASSERT_TRUE(stat->locationCount(Location::EntityType::boatTerminal()) == 0);
    ASSERT_TRUE(stat->locationCount(Location::EntityType::planeTerminal()) == 0);
    ASSERT_TRUE(stat->locationCount(Location::EntityType::truckTerminal()) == 0);
    ASSERT_TRUE(stat->locationCount(Location::EntityType::customer()) == 0);
    ASSERT_TRUE(stat->locationCount(Location::EntityType::port()) == 1);
    ASSERT_TRUE(stat->segmentCount(TransportMode::truck()) == 1);
    ASSERT_TRUE(stat->segmentCount(TransportMode::plane()) == 1);
    ASSERT_TRUE(stat->segmentCount(TransportMode::boat()) == 1);

    segment = nwk->SegmentNew("segment4",TransportMode::plane(),PathMode::unexpedited());

    ASSERT_TRUE(stat->locationCount(Location::EntityType::boatTerminal()) == 0);
    ASSERT_TRUE(stat->locationCount(Location::EntityType::planeTerminal()) == 0);
    ASSERT_TRUE(stat->locationCount(Location::EntityType::truckTerminal()) == 0);
    ASSERT_TRUE(stat->locationCount(Location::EntityType::customer()) == 0);
    ASSERT_TRUE(stat->locationCount(Location::EntityType::port()) == 1);
    ASSERT_TRUE(stat->segmentCount(TransportMode::truck()) == 1);
    ASSERT_TRUE(stat->segmentCount(TransportMode::plane()) == 2);
    ASSERT_TRUE(stat->segmentCount(TransportMode::boat()) == 1);

    nwk->segment("segment3")->modeIs(PathMode::expedited());

    ASSERT_TRUE(stat->segmentCount(PathMode::expedited()) == 1);

    nwk->segment("segment4")->modeIs(PathMode::expedited());

    ASSERT_TRUE(stat->segmentCount(PathMode::expedited()) == 2);

    nwk->segment("segment4")->modeDel(PathMode::expedited());

    ASSERT_TRUE(stat->segmentCount(PathMode::expedited()) == 1);

    nwk->segmentDel("segment3");

    ASSERT_TRUE(stat->locationCount(Location::EntityType::boatTerminal()) == 0);
    ASSERT_TRUE(stat->locationCount(Location::EntityType::planeTerminal()) == 0);
    ASSERT_TRUE(stat->locationCount(Location::EntityType::truckTerminal()) == 0);
    ASSERT_TRUE(stat->locationCount(Location::EntityType::customer()) == 0);
    ASSERT_TRUE(stat->locationCount(Location::EntityType::port()) == 1);
    ASSERT_TRUE(stat->segmentCount(TransportMode::truck()) == 1);
    ASSERT_TRUE(stat->segmentCount(TransportMode::plane()) == 2);
    ASSERT_TRUE(stat->segmentCount(TransportMode::boat()) == 0);
    ASSERT_TRUE(stat->segmentCount(PathMode::expedited()) == 0);
}

