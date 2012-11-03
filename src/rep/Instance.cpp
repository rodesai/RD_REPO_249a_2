#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#include "include/Instance.h"
#include "include/Engine.h"

namespace Shipping {

using namespace std;

// strings
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
    ManagerImpl() {
        shippingNetwork_ = new Shipping::ShippingNetwork();
    };
    Ptr<Instance> instanceNew(const string& name, const string& type);
    Ptr<Instance> instance(const string& name);
    void instanceDel(const string& name);
    Shipping::ShippingNetwork::Ptr shippingNetwork()
        { return shippingNetwork_; }
private:
    map<string,Ptr<Instance> > instance_;
    Shipping::ShippingNetwork::Ptr shippingNetwork_;
};

Ptr<Instance> ManagerImpl::instance(const string& name) {
    map<string,Ptr<Instance> >::const_iterator t = instance_.find(name);
    return t == instance_.end() ? NULL : (*t).second;
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
    string attribute(const string& name) {
        int i = segmentNumber(name);
        if (i != 0) {
            return location_->segment(i).name();
        }
        return "";
    }
    void attributeIs(const string& name, const string& v) {}
private:
    Ptr<ManagerImpl> manager_;
    Shipping::Location::Ptr location_;
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
        location_ = mamager->shippingNetwork()->TruckTerminalNew(name);
    }
};


class BoatTerminalRep : public LocationRep {
public:
    BoatTerminalRep(const string& name, ManagerImpl *manager) :
        LocationRep(name, manager) {
        location_ = mamager->shippingNetwork()->BoatTerminalNew(name);
    }
};


class PlaneTerminalRep : public LocationRep {
public:
    PlaneTerminalRep(const string& name, ManagerImpl *manager) :
        LocationRep(name, manager) {
        location_ = mamager->shippingNetwork()->PlaneTerminalNew(name);
    }
};


class CustomerRep : public LocationRep {
public:
    CustomerRep(const string& name, ManagerImpl *manager) :
        LocationRep(name, manager) {
        location_ = mamager->shippingNetwork()->CustomerNew(name);
    }
};


class PortRep : public LocationRep {
public:
    PortRep(const string& name, ManagerImpl *manager) :
        LocationRep(name, manager) {
        location_ = mamager->shippingNetwork()->PortNew(name);
    }
};


// TODO: should i be using a larger int type?
void streamInt(stringstream ss, int i) {
    ss.precision(0);
    ss << i;
}

void streamFloat(stringstream ss, float f) {
    ss.precision(2);
    ss << f;
}


class SegmentRep : public Instance {
public:
    SegmentRep(const string& name, ManagerImpl* manager) :
        Instance(name), manager_(manager) {
        // TODO: Nothing else to do?
        manager_ = manager;
    }

    // Instance method
    string attribute(const string& name) {
        stringstream ss;
        if (name == "source") {
            return segment_->source().name();
        } else if (name == "return segment") {
            return segment_->returnSegment().name();
        } else if (name == "length") {
            streamFloat(ss, segment_->length());
        } else if (name == "difficulty") {
            streamFloat(ss, segment_->difficulty());
        } else if (name == "expedite support") {
            return segment->expediteSupport() == 
                Shipping::Segment::expediteSupported() ?
                "yes" : "no";
        }

        return ss.str();
    }
    void attributeIs(const string& name, const string& v) {
        if (name == "source") {
            Location::Ptr source = manager_[v];
            if (source) segment_->sourceIs(source);
        } else if (name == "return segment") {
            Segment::Ptr rs = manager_[v];
            if (rs) segment_->returnSegmentIs(name);
        } else if (name == "length") {
            segment_->lengthIs(Shipping::Mile(atoi(v)));
        } else if (name == "difficulty") {
            segment_->difficutlyIs(Shipping::Difficulty(atof(v)));
        } else if (name == "expedite support" && v == "yes") {
            segment_->expediteSupportIs(Shipping::Segment::yes());
        }
    }
private:
    Ptr<ManagerImpl> manager_;
    int segmentNumber(const string& name);
    Shipping::Segment::Ptr segment_;
};


class TruckSegmentRep : public SegmentRep {
public:
    TruckSegmentRep(const string& name, ManagerImpl *manager) :
        SegmentRep(name, manager) {
        segment_ = mamager->shippingNetwork()->SegmentNew(name,
            Shipping::Segment::truckSegment());
    }
};


class BoatSegmentRep : public SegmentRep {
public:
    BoatSegmentRep(const string& name, ManagerImpl *manager) :
        SegmentRep(name, manager) {
        segment_ = mamager->shippingNetwork()->SegmentNew(name,
            Shipping::Segment::boatSegment());

    }
};


class PlaneSegmentRep : public SegmentRep {
public:
    PlaneSegmentRep(const string& name, ManagerImpl *manager) :
        SegmentRep(name, manager) {
        segment_ = mamager->shippingNetwork()->SegmentNew(name,
            Shipping::Segment::planeSegment());
    }
};


class FleetRep : public Instance {
public:
    FleetRep(const string& name, ManagerImpl* manager) :
        Instance(name), manager_(manager) {
        // TODO: do I need the manager?
        manager_ = manager;
        fleet_ = mamager->shippingNetwork()->FleetNew(name);
    }

    // Instance method
    string attribute(const string& name) {
        stringstream ss;
        FleetAttribute fa = fleetAttribute(name);
        // TODO: need to set precision differently for float and int
        if (fa.attribute == speedStr) {
            streamInt(ss, fleet_->speed(fa.mode));
        } else if (fa.attribute == capacityStr) {
            streamInt(ss, fleet_->capacity(fa.mode));
        } else if (fa.attribute == costStr) {
            streamFloat(ss, fleet_->cost(fa.mode));
        }
        return ss.str();
    }
    void attributeIs(const string& name, const string& v) {
        FleetAttribute fa = fleetAttribute(name);
        if (fa.attribute == speedStr) {
            fleet_->speedIs(fa.mode, v);
        } else if (fa.attribute == capacityStr) {
            fleet_->capacityIs(fa.mode, v);
        } else if (fa.attribute == costStr) {
            fleet_->costIs(fa.mode, v);
        }
    }
private:
    static const string speedStr = "speed";
    static const string capacityStr = "capacity";
    static const string costStr = "cost";
    typedef struct FleetAttribute_t {
        string& attribute;
        Shipping::Fleet::Mode mode;
    } FleetAttribute;
    FleetAttribute fleetAttribute(string& str) {
        FleetAttribute result = {"", Shipping::Fleet::boat()};
        string& namePtr = strtok(name, ", ");
        if (namePtr == "Boat") {
            result.mode = Shipping::Fleet::boat();
        } else if (namePtr == "Truck") {
            result.mode = Shipping::Fleet::truck();
        } else if (namePtr == "Plane") {
            result.mode = Shipping::Fleet::plane();
        } else {
            return result
        }
        result.attribute = strtok(NULL, ", ");
        return result;
    }
    Ptr<ManagerImpl> manager_;
    Shipping::Fleet::Ptr fleet_;
};


class StatsRep : public Instance {
public:
    StatsRep(const string& name, ManagerImpl* manager) :
        Instance(name), manager_(manager) {
        // TODO: Nothing else to do?
        // TODO: do I need the manager?
        manager_ = manager;
        stats_ = mamager->shippingNetwork()->StatsNew(name);
    }

    // Instance method
    string attribute(const string& name) {
        // TODO: convert output to string
        std::stringstream ss;

        // return location count
        if (name == truckTerminalStr) {
            streamInt(ss, stats_->locationCount(
                Shipping::Location::truckTerminal()));
        } else if (name == customerStr) {
            streamInt(ss, stats_->locationCount(
                Shipping::Location::customer()));            
        } else if (name == portStr) {
            streamInt(ss, stats_->locationCount(
                Shipping::Location::port()));            
        } else if (name == planeTerminalStr) {
            streamInt(ss, stats_->locationCount(
                Shipping::Location::planeTerminal()));            
        } else if (name == boatTerminalStr) {
            streamInt(ss, stats_->locationCount(
                Shipping::Location::boatTerminal()));            
        }

        // return segment stats
        else if (name == boatSegmentStr) {
            streamInt(ss, stats_->segmentCount(
                Shipping::Segment::boatSegment()));
        } else if (name == truckSegmentStr) {
            streamInt(ss, stats_->segmentCount(
                Shipping::Segment::truckSegment()));
        } else if (name == planeSegmentStr) {
            streamInt(ss, stats_->segmentCount(
                Shipping::Segment::planeSegment()));
        }

        // expedite percentage
        else if (name == "expedite percentage") {
            streamFloat(ss, stats_->expeditePercentage());
        }

        return ss.str()
    }
    void attributeIs(const string& name, const string& v) {
        // nothing to do here
    }
private:
    Ptr<ManagerImpl> manager_;
    Shipping::Stats::Ptr stats_;
};

class ConnRep : public Instance {
public:
    ConnRep(const string& name, ManagerImpl* manager) :
        Instance(name), manager_(manager) {
        // TODO: do I need the manager?
        manager_ = manager;
        conn_ = Shipping::ShippingNetwork::ConnNew(name);
    }

    // Instance method
    string attribute(const string& name) {
        stringstream ss;
        bool explore = false;
        std::vector<Shipping::Path::Ptr> paths;
        string& namePtr = strtok(name, ", :");
        if (namePtr == "connect") {
            namePtr = strtok(NULL, ", :");
            string& loc1 = namePtr;
            string& loc2 = (namePtr, ", :");
            paths = conn_->connect(loc1, loc2);
        } else if (namePtr = "explore") {
            explore = true;
            Shipping::Mile distance = -1;
            Shipping::Hour hour = -1;
            Shipping::Dollar cost = -1;
            Shipping::Segment::ExpediteSupport es =
                Shipping::Segment::unspecified();
            namePtr = strtok(NULL, ", :");
            string& loc = namePtr;
            paths = conn_->explore(loc, distance, cost, hour, es);
        }

        uint numPaths = paths.size()
        for (int i = 0; i < numPaths; i++) {
            Shipping::Path path = paths[i]
            if (!explore) {
                streamFloat(ss, path->cost());
                ss << " ";
                streamFloat(ss, path->time());
                ss << " " << segment->expediteSupport() == 
                    Shipping::Segment::expediteSupported() ?
                    "yes" : "no";
                ss << "; ";
            }

            Shipping::Path::LocationIterator li = path->pathIterator();
            for (; li != li.end(); i++) {
                ss << li->source().name()
                if (li + 1 != li.end()) {
                    ss << "(" << li->segment().name() << ":";
                    streamFloat(ss, li->segment().length());
                    ss << ":" li->segment().returnSegment().name() << ") ";
                }
            }

            ss << "\n";
        }

        return ss.str();
    }
    void attributeIs(const string& name, const string& v) {
        // nothing to do here
    }
private:
    Ptr<ManagerImpl> manager_;
    Shipping::Stats::Ptr stats_;
    string locationString(Shipping::Path p) {
        stringstream ss;

    }
};

ManagerImpl::ManagerImpl() {
}

Ptr<Instance> ManagerImpl::instanceNew(const string& name,
    const string& type) {
    // do not create an instance if the name already exists
    if (!instance_[name]) return NULL;

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
