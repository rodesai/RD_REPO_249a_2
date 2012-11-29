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
        seg->attributeIs("Capacity","10");
        seg->attributeIs("length","10.0");
        segr->attributeIs("Capacity","10");
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

    bool random = false;
    if(argc > 1 && string(argv[1]) == "random") 
        random = true;

    Ptr<Instance::Manager> manager = shippingInstanceManager();

    // L1
    buildnwaytree(manager,1, 10, "Customer","c","Truck terminal","t-1","Truck segment");
    buildnwaytree(manager,11,20, "Customer","c","Truck terminal","t-2","Truck segment");
    buildnwaytree(manager,21,30, "Customer","c","Truck terminal","t-3","Truck segment");
    buildnwaytree(manager,31,40, "Customer","c","Truck terminal","t-4","Truck segment");
    buildnwaytree(manager,41,50, "Customer","c","Truck terminal","t-5","Truck segment");
    buildnwaytree(manager,51,60, "Customer","c","Truck terminal","t-6","Truck segment");
    buildnwaytree(manager,61,70, "Customer","c","Truck terminal","t-7","Truck segment");
    buildnwaytree(manager,71,80, "Customer","c","Truck terminal","t-8","Truck segment");
    buildnwaytree(manager,81,90, "Customer","c","Truck terminal","t-9","Truck segment");
    buildnwaytree(manager,91,100,"Customer","c","Truck terminal","t-10","Truck segment");

    // L2
    buildnwaytree(manager,1, 10, "Truck terminal","t","Truck terminal","t","Truck segment");

    // L3
    Ptr<Instance> root = manager->instanceNew("root","Customer");
    Ptr<Instance> seg = manager->instanceNew("t->root","Truck segment");
    Ptr<Instance> segr = manager->instanceNew("root->t","Truck segment");
    seg->attributeIs("source","t");
    segr->attributeIs("source","root");
    seg->attributeIs("return segment","root->t");
    seg->attributeIs("Capacity","100");
    seg->attributeIs("length","10.0");
    segr->attributeIs("Capacity","100");
    segr->attributeIs("length","10.0");

    // Setup fleet/conn
    Ptr<Instance> fleet = manager->instanceNew("myFleet","Fleet");
    fleet->attributeIs("Truck, speed","7");
    fleet->attributeIs("Truck, capacity","100");
    Ptr<Instance> conn = manager->instanceNew("myConn","Conn");
    conn->attributeIs("routing", "minHops");

    if(!random)
        assigninjectionparams(manager,1,100,"c", "root", 16, 100);
    else
        assigninjectionparams(manager,1,100,"c", "root", 16, 1,1000);

    manager->simulationManager()->virtualTimeIs(30.0);
    std::cout << "Shipments Received: " << root->attribute("Shipments Received") << ", Average Latency: " << root->attribute("Average Latency") << std::endl;
    manager->simulationManager()->virtualTimeIs(60.0);
    std::cout << "Shipments Received: " << root->attribute("Shipments Received") << ", Average Latency: " << root->attribute("Average Latency") << std::endl;
    manager->simulationManager()->virtualTimeIs(90.0);
    std::cout << "Shipments Received: " << root->attribute("Shipments Received") << ", Average Latency: " << root->attribute("Average Latency") << std::endl;
    manager->simulationManager()->virtualTimeIs(120.0);
    std::cout << "Shipments Received: " << root->attribute("Shipments Received") << ", Average Latency: " << root->attribute("Average Latency") << std::endl;
    manager->simulationManager()->virtualTimeIs(150.0);
    std::cout << "Shipments Received: " << root->attribute("Shipments Received") << ", Average Latency: " << root->attribute("Average Latency") << std::endl;
    manager->simulationManager()->virtualTimeIs(180.0);
    std::cout << "Shipments Received: " << root->attribute("Shipments Received") << ", Average Latency: " << root->attribute("Average Latency") << std::endl;
    manager->simulationManager()->virtualTimeIs(210.0);
    std::cout << "Shipments Received: " << root->attribute("Shipments Received") << ", Average Latency: " << root->attribute("Average Latency") << std::endl;
    manager->simulationManager()->virtualTimeIs(240.0);
    std::cout << "Shipments Received: " << root->attribute("Shipments Received") << ", Average Latency: " << root->attribute("Average Latency") << std::endl;
}
