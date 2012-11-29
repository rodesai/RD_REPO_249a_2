#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <ostream>
#include <iostream>
#include <string>
#include "Instance.h"

using namespace std;

void buildnwaytree(Ptr<Instance::Manager> manager, uint32_t fanout_start,uint32_t fanout_end, string leaftype, string leafbasename, string roottype, string rootname, string segmenttype, string capacity){
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
        seg->attributeIs("Capacity",capacity);
        seg->attributeIs("length","15.0");
        segr->attributeIs("Capacity",capacity);
        segr->attributeIs("length","15.0");
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
        //std::cout << shipmentsize << std::endl;
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
    buildnwaytree(manager,1, 10, "Customer","c","Truck terminal","t-1","Truck segment","1");
    buildnwaytree(manager,11,20, "Customer","c","Truck terminal","t-2","Truck segment","1");
    buildnwaytree(manager,21,30, "Customer","c","Truck terminal","t-3","Truck segment","1");
    buildnwaytree(manager,31,40, "Customer","c","Truck terminal","t-4","Truck segment","1");
    buildnwaytree(manager,41,50, "Customer","c","Truck terminal","t-5","Truck segment","1");
    buildnwaytree(manager,51,60, "Customer","c","Truck terminal","t-6","Truck segment","1");
    buildnwaytree(manager,61,70, "Customer","c","Truck terminal","t-7","Truck segment","1");
    buildnwaytree(manager,71,80, "Customer","c","Truck terminal","t-8","Truck segment","1");
    buildnwaytree(manager,81,90, "Customer","c","Truck terminal","t-9","Truck segment","1");
    buildnwaytree(manager,91,100,"Customer","c","Truck terminal","t-10","Truck segment","1");

    // L2
    buildnwaytree(manager,1, 10, "Truck terminal","t","Truck terminal","t","Truck segment","10");

    // L3
    Ptr<Instance> root = manager->instanceNew("root","Customer");
    Ptr<Instance> seg = manager->instanceNew("t->root","Truck segment");
    Ptr<Instance> segr = manager->instanceNew("root->t","Truck segment");
    seg->attributeIs("source","t");
    segr->attributeIs("source","root");
    seg->attributeIs("return segment","root->t");
    seg->attributeIs("Capacity","100");
    seg->attributeIs("length","15.0");
    segr->attributeIs("Capacity","100");
    segr->attributeIs("length","15.0");

    // Setup fleet/conn
    Ptr<Instance> fleet = manager->instanceNew("myFleet","Fleet");
    fleet->attributeIs("Truck, speed","100");
    fleet->attributeIs("Truck, capacity","100");
    Ptr<Instance> conn = manager->instanceNew("myConn","Conn");
    conn->attributeIs("routing", "minHops");

    if(!random)
        assigninjectionparams(manager,1,100,"c", "root", 160, 100);
    else
        assigninjectionparams(manager,1,100,"c", "root", 160, 1,1000);

    manager->simulationManager()->virtualTimeIs(30);
    std::cout << "Shipments Received: " << root->attribute("Shipments Received") << ", Average Latency: " << root->attribute("Average Latency") << std::endl;
    manager->simulationManager()->virtualTimeIs(60);
    std::cout << "Shipments Received: " << root->attribute("Shipments Received") << ", Average Latency: " << root->attribute("Average Latency") << std::endl;
    manager->simulationManager()->virtualTimeIs(90);
    std::cout << "Shipments Received: " << root->attribute("Shipments Received") << ", Average Latency: " << root->attribute("Average Latency") << std::endl;
    manager->simulationManager()->virtualTimeIs(120);
    std::cout << "Shipments Received: " << root->attribute("Shipments Received") << ", Average Latency: " << root->attribute("Average Latency") << std::endl;
    manager->simulationManager()->virtualTimeIs(150);
    std::cout << "Shipments Received: " << root->attribute("Shipments Received") << ", Average Latency: " << root->attribute("Average Latency") << std::endl;
    manager->simulationManager()->virtualTimeIs(180);
    std::cout << "Shipments Received: " << root->attribute("Shipments Received") << ", Average Latency: " << root->attribute("Average Latency") << std::endl;

    std::cout << std::endl;
    
    uint32_t shipmentsRcvd=0;
    uint32_t shipmentsRfsd=0;

    for(uint32_t i = 1; i <= 100;i++){
        uint32_t ti = (i-1)/10 + 1;
        stringstream sids; sids <<"c-"<<i<<"->t-"<<ti;
        std::string segmentid = sids.str();
        shipmentsRcvd+=atoi(manager->instance(segmentid)->attribute("Shipments Received").c_str());
        shipmentsRfsd+=atoi(manager->instance(segmentid)->attribute("Shipments Refused").c_str());
    }

    for(uint32_t i=1; i <=10; i++){
        stringstream sids; sids<< "t-" << i << "->t";
        shipmentsRcvd+=atoi(manager->instance(sids.str())->attribute("Shipments Received").c_str());
        shipmentsRfsd+=atoi(manager->instance(sids.str())->attribute("Shipments Refused").c_str());
    }

    shipmentsRcvd+=atoi(manager->instance("t->root")->attribute("Shipments Received").c_str());
    shipmentsRfsd+=atoi(manager->instance("t->root")->attribute("Shipments Refused").c_str());

    std::cout << "Average shipments received: " << ((double)shipmentsRcvd)/111.0 << std::endl;
    std::cout << "Average shipments refused: " << ((double)shipmentsRfsd)/111.0 << std::endl;
}
