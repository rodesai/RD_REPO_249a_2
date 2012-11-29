#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <ostream>
#include <iostream>
#include <string>
#include "Instance.h"

using namespace std;

void buildnwaytree(Ptr<Instance::Manager> manager, uint32_t fanout_start,uint32_t fanout_end, string leaftype, string leafbasename, string roottype, string rootname, string segmenttype){
    Ptr<Instance> root = manager->instanceNew(rootname,roottype);
    for(uint32_t i = fanout_start ; i <= fanout_end ; i++){
        ostringstream tmp;
        tmp.str("");
        tmp << leafbasename << "-" << i;
        string leafname = tmp.str();
        //std::cout << "looking for leaf with name: " << leafname << std::endl;
        Ptr<Instance> leaf = manager->instance(leafname);
        if(!leaf){
            //std::cout << "couldnt find leaf, creating new" << std::endl;
            leaf = manager->instanceNew(leafname,leaftype);
        }
        string segname = leafname + string("->") + rootname;
        string segrname = rootname + string("->") + leafname;
        //std::cout << "Creating segments with name: " << segname << ", " << segrname << std::endl;
        Ptr<Instance> seg = manager->instanceNew(segname,segmenttype);
        Ptr<Instance> segr = manager->instanceNew(segrname,segmenttype);
        seg->attributeIs("source",leafname);
        segr->attributeIs("source",rootname);
        seg->attributeIs("return segment",segrname);
        seg->attributeIs("Capacity","1");
        seg->attributeIs("length","10.0");
        segr->attributeIs("Capacity","1");
        segr->attributeIs("length","10.0");
    }
}

void assigninjectionparams(Ptr<Instance::Manager> manager, uint32_t start,uint32_t end, string basename, string destination, uint32_t rate, uint32_t size, uint32_t size_max=0){
    for(uint32_t i = start ; i <= end ; i++){
        ostringstream tmp;
        tmp.str(""); tmp << basename << "-" << i;
        string name = tmp.str();
        tmp.str(""); tmp << rate;
        string transferrate = tmp.str();
        int32_t thesize = size;
        if(size_max && size_max > size){
            thesize += rand() % (size_max-size);
        }
        tmp.str(""); tmp << thesize;
        string shipmentsize = tmp.str();
        //std::cout << "shipment size: " << shipmentsize << std::endl;
        Ptr<Instance> cust = manager->instance(name);
        cust->attributeIs("Transfer Rate",transferrate);
        cust->attributeIs("Shipment Size",shipmentsize);
        cust->attributeIs("Destination",destination);
    }
}

int main(int argc, char *argv[]) {

    if(argc != 2){
        std::cout << "adaptive <minHops|minDistance|minTime>\n";
        return 1;
    }

    std::string routing = argv[1];

    Ptr<Instance::Manager> manager = shippingInstanceManager();

    // L1
    buildnwaytree(manager,1, 200, "Customer","c","Port","t-1","Truck segment");
    buildnwaytree(manager,1, 200, "Customer","c","Port","t-2","Truck segment");

    // L2
    Ptr<Instance> root = manager->instanceNew("root","Customer");
    Ptr<Instance> seg = manager->instanceNew("t-1->root","Truck segment");
    Ptr<Instance> segr = manager->instanceNew("root->t-1","Truck segment");
    seg->attributeIs("source","t-1");
    segr->attributeIs("source","root");
    seg->attributeIs("return segment","root->t-1");
    seg->attributeIs("Capacity","200");
    seg->attributeIs("length","10.0");
    segr->attributeIs("Capacity","200");
    segr->attributeIs("length","10.0");

    seg = manager->instanceNew("t-2->root","Truck segment");
    segr = manager->instanceNew("root->t-2","Truck segment");
    seg->attributeIs("source","t-2");
    segr->attributeIs("source","root");
    seg->attributeIs("return segment","root->t-2");
    seg->attributeIs("Capacity","300");
    seg->attributeIs("length","10.1");
    segr->attributeIs("Capacity","300");
    segr->attributeIs("length","10.1");

    // Setup fleet/conn
    Ptr<Instance> fleet = manager->instanceNew("myFleet","Fleet");
    fleet->attributeIs("Truck, speed","10");
    fleet->attributeIs("Truck, capacity","10");
    fleet->attributeIs("Plane, speed","10");
    fleet->attributeIs("Plane, capacity","100");
    Ptr<Instance> conn = manager->instanceNew("myConn","Conn");
    conn->attributeIs("routing", routing);

    assigninjectionparams(manager,1,200,"c", "root", 24, 10);

    std::cout << std::endl << "Run network for 30hrs" << std::endl;

    uint32_t t1_recv_last=0;
    uint32_t t2_recv_last=0;
    for(double t = 30; t <= 30; t+=30){
        manager->simulationManager()->virtualTimeIs(t);
        std::cout << "Shipments Received: " << root->attribute("Shipments Received") << ", Average Latency: " << root->attribute("Average Latency") << ", t-2 received: " << atoi(manager->instance("t-2->root")->attribute("Shipments Routed").c_str())-t2_recv_last << ", " << ", t-1 received: " << atoi(manager->instance("t-1->root")->attribute("Shipments Routed").c_str())-t1_recv_last << std::endl;
        t1_recv_last=atoi(manager->instance("t-1->root")->attribute("Shipments Routed").c_str());
        t2_recv_last=atoi(manager->instance("t-2->root")->attribute("Shipments Routed").c_str());
    }

    std::cout << std::endl << "Add large customer. Run network for 30 hours" << std::endl;

    Ptr<Instance> bigC = manager->instanceNew("BIGC","Customer");
    Ptr<Instance> bigS = manager->instanceNew("BIGS","Plane segment");
    Ptr<Instance> bigSR = manager->instanceNew("BIGSR","Plane segment");
    bigS->attributeIs("return segment","BIGSR");
    bigS->attributeIs("source","BIGC");
    bigSR->attributeIs("source","t-1");
    bigS->attributeIs("length","1.0");
    bigSR->attributeIs("length","1.0");
    bigS->attributeIs("Capacity","10");
    bigSR->attributeIs("Capacity","10");
    bigC->attributeIs("Transfer Rate","480");
    bigC->attributeIs("Shipment Size","10");
    bigC->attributeIs("Destination","root");

    conn->attributeIs("routing", "none");
    conn->attributeIs("routing", routing);
    manager->simulationManager()->virtualTimeIs(60);
    std::cout << "Shipments Received: " << root->attribute("Shipments Received") << ", Average Latency: " << root->attribute("Average Latency") << ", t-2 received: " << atoi(manager->instance("t-2->root")->attribute("Shipments Routed").c_str())-t2_recv_last << ", t-1 received: " << atoi(manager->instance("t-1->root")->attribute("Shipments Routed").c_str())-t1_recv_last << std::endl;
    t1_recv_last=atoi(manager->instance("t-1->root")->attribute("Shipments Routed").c_str());
    t2_recv_last=atoi(manager->instance("t-2->root")->attribute("Shipments Routed").c_str());

    std::cout << std::endl << "Observe congestion and re-route. Run network for 30 hours" << std::endl;
    for(uint32_t i =10; i <= 100; i+=5){
    conn->attributeIs("routing", "none");
    conn->attributeIs("routing", routing);
    manager->simulationManager()->virtualTimeIs(60+i);
    std::cout << "Shipments Received: " << root->attribute("Shipments Received") << ", Average Latency: " << root->attribute("Average Latency") << ", t-2 received: " << atoi(manager->instance("t-2->root")->attribute("Shipments Routed").c_str())-t2_recv_last << ", t-1 received: " << atoi(manager->instance("t-1->root")->attribute("Shipments Routed").c_str())-t1_recv_last << std::endl;
    t1_recv_last=atoi(manager->instance("t-1->root")->attribute("Shipments Routed").c_str());
    t2_recv_last=atoi(manager->instance("t-2->root")->attribute("Shipments Routed").c_str());
    }

    std::cout << std::endl;
}
