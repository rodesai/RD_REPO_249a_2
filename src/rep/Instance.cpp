#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#include "rep/Instance.h"
#include "engine/Engine.h"

namespace Shipping {

using namespace std;

// strings
static const string speedStr = "speed";
static const string capacityStr = "capacity";
static const string costStr = "cost";
static const string truckTerminalStr = "Truck terminal";
static const string customerStr = "Customer";
static const string portStr = "Port";
static const string boatTerminalStr = "Boat terminal";
static const string planeTerminalStr = "Plane terminal";
static const string planeSegmentStr = "Plane segment";
static const string truckSegmentStr = "Truck segment";
static const string boatSegmentStr = "Boat segment";
static const string segmentStr = "segment";
static const int segmentStrlen = segmentStr.length();


class ManagerImpl : public Instance::Manager {
public:
    ManagerImpl();
    Ptr<Instance> instanceNew(const string& name, const string& type);
    Ptr<Instance> instance(const string& name);
    void instanceDel(const string& name);
    Shipping::ShippingNetworkPtr shippingNetwork()
        { return shippingNetwork_; }
private:
    map<string,Ptr<Instance> > instance_;
    Shipping::ShippingNetworkPtr shippingNetwork_;
};

Ptr<Instance> ManagerImpl::instance(const string& name) {
    map<string,Ptr<Instance> >::const_iterator t = instance_.find(name);
    if (t == instance_.end())
        return NULL;
    return (*t).second;
}

void ManagerImpl::instanceDel(const string& name) {
}

class LocationRep : public Instance {
public:

    LocationRep(const string& name, ManagerImpl* manager) :
        Instance(name), manager_(manager) {
        // TODO: Nothing else to do?
        // TODO: what is this for?
        manager_ = manager;
    }

    // Instance method
    Shipping::LocationPtr representee() { return representee_; }
    string attribute(const string& name) {
        int i = segmentNumber(name);
        if (i != 0) {
            return representee_->segmentID(i);
        }
        return "";
    }
    void attributeIs(const string& name, const string& v) {}
protected:
    Ptr<ManagerImpl> manager_;
    Shipping::LocationPtr representee_;
    int segmentNumber(const string& name) {
        if (name.substr(0, segmentStrlen) == segmentStr) {
            const char* t = name.c_str() + segmentStrlen;
            return atoi(t);
        }
        return 0;
    }
};


class TruckTerminalRep : public LocationRep {
public:
    TruckTerminalRep(const string& name, ManagerImpl *manager) :
        LocationRep(name, manager) {
        representee_ = manager->shippingNetwork()->LocationNew(name,
            Shipping::Location::truckTerminal());
    }
};


class BoatTerminalRep : public LocationRep {
public:
    BoatTerminalRep(const string& name, ManagerImpl *manager) :
        LocationRep(name, manager) {
        representee_ = manager->shippingNetwork()->LocationNew(name,
            Shipping::Location::boatTerminal());
    }
};


class PlaneTerminalRep : public LocationRep {
public:
    PlaneTerminalRep(const string& name, ManagerImpl *manager) :
        LocationRep(name, manager) {
        representee_ = manager->shippingNetwork()->LocationNew(name,
            Shipping::Location::planeTerminal());
    }
};


class CustomerRep : public LocationRep {
public:
    CustomerRep(const string& name, ManagerImpl *manager) :
        LocationRep(name, manager) {
        representee_ = manager->shippingNetwork()->LocationNew(name,
            Shipping::Location::customer());
    }
};


class PortRep : public LocationRep {
public:
    PortRep(const string& name, ManagerImpl *manager) :
        LocationRep(name, manager) {
        representee_ = manager->shippingNetwork()->LocationNew(name,
            Shipping::Location::port());
    }
};


// TODO: are these the correct types?
// TODO: should we move these into the types?
// int based types
string MileToStr(Mile x) {
    stringstream s;
    s << x.value();
    return s.str();
}

string PackageNumToStr(PackageNum x) {
    stringstream s;
    s << x.value();
    return s.str();
}

string MilePerHourToStr(MilePerHour x) {
    stringstream s;
    s << x.value();
    return s.str();
}

string DollarPerMileToStr(DollarPerMile x) {
    stringstream s;
    s.precision(2);
    s << fixed << x.value();
    return s.str();
}

// float based types
string DifficultyToStr(Difficulty x) {
    stringstream s;
    s.precision(2);
    s << fixed << x.value();
    return s.str();
}

string DollarToStr(Dollar x) {
    stringstream s;
    s.precision(2);
    s << fixed << x.value();
    return s.str();
}

string HourToStr(Hour x) {
    stringstream s;
    s.precision(2);
    s << fixed << x.value();
    return s.str();
}


class SegmentRep : public Instance {
public:
    SegmentRep(const string& name, ManagerImpl* manager) :
        Instance(name), manager_(manager) {
        // TODO: Nothing else to do?
        manager_ = manager;
    }

    // Instance method
    Shipping::SegmentPtr representee() { return representee_; }
    string attribute(const string& name) {
        if (name == "source" && representee_->source()) {
            return representee_->source()->name();
        } else if (name == "return segment" && representee_->returnSegment()) {
            return representee_->returnSegment()->name();
        } else if (name == "length") {
            return MileToStr(representee_->length());
        } else if (name == "difficulty") {
            return DifficultyToStr(representee_->difficulty());
        } else if (name == "expedite support") {
            return (representee_->expediteSupport() == 
                            Shipping::Segment::expediteSupported()) ?
                "yes" : "no";
        }

        return "";
    }
    void attributeIs(const string& name, const string& v) {
        if (name == "source") {
            Ptr<LocationRep> sr = dynamic_cast<LocationRep *> (manager_->instance(v).ptr());
            if (sr) {
                LocationPtr loc = sr->representee();
                if (sourceOK(loc->entityType())) {
                    representee_->sourceIs(loc);
                }
            }
        } else if (name == "return segment") {
            Ptr<SegmentRep> sr= dynamic_cast<SegmentRep *> (manager_->instance(v).ptr());
            if (sr) representee_->returnSegmentIs(sr->representee());
        } else if (name == "length") {
            representee_->lengthIs(Shipping::Mile(atoi(v.data())));
        } else if (name == "difficulty") {
            representee_->difficultyIs(Shipping::Difficulty(atof(v.data())));
        } else if (name == "expedite support" && v == "yes") {
            representee_->expediteSupportIs(Shipping::Segment::expediteSupported());
        }
    }
protected:
    // TODO: why do I need to put the "return false" in here?
    virtual bool sourceOK(Location::EntityType et) { return false; }
    Ptr<ManagerImpl> manager_;
    int segmentNumber(const string& name);
    Shipping::SegmentPtr representee_;
};

// TODO: check that segments are right
class TruckSegmentRep : public SegmentRep {
public:
    TruckSegmentRep(const string& name, ManagerImpl *manager) :
        SegmentRep(name, manager) {
        representee_ = manager->shippingNetwork()->SegmentNew(name,
            Shipping::Segment::truckSegment());
    }
protected:
    bool sourceOK(Location::EntityType et) {
        if (et == Location::truckTerminal() ||
            et == Location::customer() ||
            et == Location::port())
            return true;
        return false;
    }
};


class BoatSegmentRep : public SegmentRep {
public:
    BoatSegmentRep(const string& name, ManagerImpl *manager) :
        SegmentRep(name, manager) {
        representee_ = manager->shippingNetwork()->SegmentNew(name,
            Shipping::Segment::boatSegment());

    }
protected:
    bool sourceOK(Location::EntityType et) {
        if (et == Location::boatTerminal() ||
            et == Location::customer() ||
            et == Location::port())
            return true;
        return false;
    }
};


class PlaneSegmentRep : public SegmentRep {
public:
    PlaneSegmentRep(const string& name, ManagerImpl *manager) :
        SegmentRep(name, manager) {
        representee_ = manager->shippingNetwork()->SegmentNew(name,
            Shipping::Segment::planeSegment());
    }
protected:
    bool sourceOK(Location::EntityType et) {
        if (et == Location::planeTerminal() ||
            et == Location::customer() ||
            et == Location::port())
            return true;
        return false;
    }
};


class FleetRep : public Instance {
public:
    FleetRep(const string& name, ManagerImpl* manager) :
        Instance(name), manager_(manager) {
        // TODO: do I need the manager?
        manager_ = manager;
        fleet_ = manager->shippingNetwork()->FleetNew(name);
    }

    // Instance method
    string attribute(const string& name) {
        stringstream ss;
        FleetAttribute fa = fleetAttribute(name);
        if (fa.attribute == speedStr) {
            return MilePerHourToStr(fleet_->speed(fa.mode));
        } else if (fa.attribute == capacityStr) {
            return PackageNumToStr(fleet_->capacity(fa.mode));
        } else if (fa.attribute == costStr) {
            return DollarPerMileToStr(fleet_->cost(fa.mode));
        }
        return ss.str();
    }
    void attributeIs(const string& name, const string& v) {
        FleetAttribute fa = fleetAttribute(name);
        if (fa.attribute == speedStr) {
            fleet_->speedIs(fa.mode, MilePerHour(atof(v.data())));
        } else if (fa.attribute == capacityStr) {
            fleet_->capacityIs(fa.mode, PackageNum(atoi(v.data())));
        } else if (fa.attribute == costStr) {
            fleet_->costIs(fa.mode, DollarPerMile(atof(v.data())));
        }
    }
private:
    typedef struct FleetAttribute_t {
        string attribute;
        Shipping::Fleet::Mode mode;
    } FleetAttribute;
    FleetAttribute fleetAttribute(const string& str) {
        FleetAttribute result = {"", Shipping::Fleet::boat()};

        // TODO: this doesn't seem efficient
        char* tokenString = strdup(str.data());
        char* namePtr = strtok(tokenString, ", ");
        if (strcmp(namePtr, "Boat") == 0) {
            result.mode = Shipping::Fleet::boat();
        } else if (strcmp(namePtr, "Truck") == 0) {
            result.mode = Shipping::Fleet::truck();
        } else if (strcmp(namePtr, "Plane") == 0) {
            result.mode = Shipping::Fleet::plane();
        } else {
            delete tokenString;
            return result;
        }
        result.attribute = string(strtok(NULL, ", "));
        delete tokenString;
        return result;
    }
    Ptr<ManagerImpl> manager_;
    Shipping::FleetPtr fleet_;
};


class StatsRep : public Instance {
public:
    StatsRep(const string& name, ManagerImpl* manager) :
        Instance(name), manager_(manager) {
        // TODO: Nothing else to do?
        // TODO: do I need the manager?
        manager_ = manager;
        stats_ = manager->shippingNetwork()->StatsNew(name);
    }

    // Instance method
    string attribute(const string& name) {
        // TODO: convert output to string
        std::stringstream ss;

        // return location count
        if (name == truckTerminalStr) {
            ss << stats_->locationCount(
                Shipping::Location::truckTerminal());
        } else if (name == customerStr) {
            ss << stats_->locationCount(
                Shipping::Location::customer());            
        } else if (name == portStr) {
            ss << stats_->locationCount(
                Shipping::Location::port());            
        } else if (name == planeTerminalStr) {
            ss << stats_->locationCount(
                Shipping::Location::planeTerminal());            
        } else if (name == boatTerminalStr) {
            ss << stats_->locationCount(
                Shipping::Location::boatTerminal());            
        }

        // return segment stats
        else if (name == boatSegmentStr) {
            ss << stats_->segmentCount(
                Shipping::Segment::boatSegment());
        } else if (name == truckSegmentStr) {
            ss << stats_->segmentCount(
                Shipping::Segment::truckSegment());
        } else if (name == planeSegmentStr) {
            ss << stats_->segmentCount(
                Shipping::Segment::planeSegment());
        }

        // expedite percentage
        else if (name == "expedite percentage") {
            ss.precision(2);
            ss << fixed << stats_->expeditePercentage();
        }

        return ss.str();
    }
    void attributeIs(const string& name, const string& v) {
        // nothing to do here
    }
private:
    Ptr<ManagerImpl> manager_;
    Shipping::StatsPtr stats_;
};

class ConnRep : public Instance {
public:
    ConnRep(const string& name, ManagerImpl* manager) :
        Instance(name), manager_(manager) {
        // TODO: do I need the manager?
        manager_ = manager;
        conn_ = manager->shippingNetwork()->ConnNew(name);
    }

    // Instance method
    string attribute(const string& name) {
        stringstream ss;
        bool explore = false;
        std::vector<Shipping::PathPtr> paths;

        // TODO: is there a better way to tokenize?
        char* tokenString = strdup(name.data());
        char* namePtr = strtok(tokenString, ", :");
        if (strcmp(namePtr, "connect") == 0) {
            namePtr = strtok(NULL, ", :");
            Ptr<LocationRep> loc1 = dynamic_cast<LocationRep*> (manager_->instance(namePtr).ptr());
            namePtr = strtok(NULL, ", :");
            Ptr<LocationRep> loc2 = dynamic_cast<LocationRep*> (manager_->instance(namePtr).ptr());
            delete tokenString;
            if (!loc1 && !loc2) return "";
            paths = conn_->connect(loc1->representee(), loc2->representee());
        } else if (strcmp(namePtr, "explore") == 0) {
            explore = true;
            Shipping::Mile distance = -1;
            Shipping::Hour hour = -1;
            Shipping::Dollar cost = -1;
            Shipping::Segment::ExpediteSupport es =
                Shipping::Segment::expediteUnspecified();
            namePtr = strtok(NULL, ", :");
            Ptr<LocationRep> loc = dynamic_cast<LocationRep*> (manager_->instance(namePtr).ptr());
            delete tokenString;
            if (!loc) return "";
            paths = conn_->explore(loc->representee(), distance, cost, hour, es);
        }

        unsigned int numPaths = paths.size();
        for (int i = 0; i < numPaths; i++) {
            Shipping::PathPtr path = paths[i];
            if (!explore) {
                // TODO: the setting of precision here is inconsistent
                ss << DollarToStr(path->cost()) << " ";
                ss << HourToStr(path->time()) << " ";
                ss << (path->expedited() == 
                    Shipping::Segment::expediteSupported() ?
                    "yes" : "no");
                ss << "; ";
            }

            // TODO: This is incomplete since the paths aren't ready

            // Shipping::Path::LocationIterator li = path->pathIterator();
            // // TODO: is this where the iterator stops?
            // for (; li != NULL; i++) {
            //     ss << li->source().name()
            //     if (li + 1 != li.end()) {
            //         ss << "(" << li->segment()->name() << ":";
            //         ss << MileToStr(li->segment().length());
            //         ss << ":" li->segment().returnSegment()->name() << ") ";
            //     }
            // }

            ss << "\n";
        }

        return ss.str();
    }
    void attributeIs(const string& name, const string& v) {
        // nothing to do here
    }
private:
    Ptr<ManagerImpl> manager_;
    Shipping::ConnPtr conn_;
};

ManagerImpl::ManagerImpl() {
    // TODO: Don't think I need to set this name to something the client can read
    shippingNetwork_ =
        Shipping::ShippingNetwork::ShippingNetworkIs("ShippingNetwork");
}

Ptr<Instance> ManagerImpl::instanceNew(const string& name,
    const string& type) {
    // do not create an instance if the name already exists
    if (instance_[name]) return NULL;

    // Location Types
    if (type == truckTerminalStr) {
        Ptr<TruckTerminalRep> t = new TruckTerminalRep(name, this);
        instance_[name] = t;
        return t;
    } else if (type == customerStr) {
        Ptr<CustomerRep> t = new CustomerRep(name, this);
        instance_[name] = t;
        return t;
    } else if (type == portStr) {
        Ptr<PortRep> t = new PortRep(name, this);
        instance_[name] = t;
        return t;
    } else if (type == boatTerminalStr) {
        Ptr<BoatTerminalRep> t = new BoatTerminalRep(name, this);
        instance_[name] = t;
        return t;
    } else if (type == planeTerminalStr) {
        Ptr<PlaneTerminalRep> t = new PlaneTerminalRep(name, this);
        instance_[name] = t;
        return t;
    }

    // Segment Types
    else if (type == planeSegmentStr) {
        Ptr<PlaneSegmentRep> t = new PlaneSegmentRep(name, this);
        instance_[name] = t;
        return t;
    } else if (type == truckSegmentStr) {
        Ptr<TruckSegmentRep> t = new TruckSegmentRep(name, this);
        instance_[name] = t;
        return t;
    } else if (type == boatSegmentStr) {
        Ptr<BoatSegmentRep> t = new BoatSegmentRep(name, this);
        instance_[name] = t;
        return t;
    }

    // Conn Type
    else if (type == "Conn") {
        Ptr<ConnRep> t = new ConnRep(name, this);
        instance_[name] = t;
        return t;
    }

    // Fleet Type
    else if (type == "Fleet") {
        Ptr<FleetRep> t = new FleetRep(name, this);
        instance_[name] = t;
        return t;
    }

    // Stats Type
    else if (type == "Stats") {
        Ptr<StatsRep> t = new StatsRep(name, this);
        instance_[name] = t;
        return t;
    }

    return NULL;
}

}

/*
 * This is the entry point for your library.
 * The client program will call this function to get a handle
 * on the Instance::Manager object, and from there will use
 * that object to interact with the middle layer (which will
 * in turn interact with the engine layer).
 */
Ptr<Instance::Manager> shippingInstanceManager() {
    return new Shipping::ManagerImpl();
}
