#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#include "rep/Instance.h"
#include "engine/Engine.h"
#include "logging.h"

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
    enum InstanceType {
        customer_ = 0,
        port_,
        truckTerminal_,
        boatTerminal_,
        planeTerminal_,
        truckSegment_,
        boatSegment_,
        planeSegment_,
        stats_,
        conn_,
        fleet_,
    };

    static inline InstanceType customer() { return customer_; }
    static inline InstanceType port() { return port_; }
    static inline InstanceType truckTerminal() { return truckTerminal_; }
    static inline InstanceType boatTerminal() { return boatTerminal_; }
    static inline InstanceType planeTerminal() { return planeTerminal_; }
    static inline InstanceType truckSegment() { return truckSegment_; }
    static inline InstanceType boatSegment() { return boatSegment_; }
    static inline InstanceType planeSegment() { return planeSegment_; }
    static inline InstanceType stats() { return stats_; }
    static inline InstanceType conn() { return conn_; }
    static inline InstanceType fleet() { return fleet_; }

    ManagerImpl();
    Ptr<Instance> instanceNew(const string& name, const string& type);
    Ptr<Instance> instance(const string& name);
    void nowIs(Time t){
        if(t > realTimeManager_->now())
            realTimeManager_->nowIs(t);
    }
    void instanceDel(const string& name);
    ShippingNetworkPtr shippingNetwork()
        { return shippingNetwork_; }
private:

    class RealToVirtualTimeActivity : public Activity::Activity::Notifiee{
    public:
        // public constructor ok in private class
        RealToVirtualTimeActivity(Activity::ManagerPtr realManager, Activity::ManagerPtr virtualManager, uint64_t usPerHour): 
            Activity::Activity::Notifiee(), realManager_(realManager), virtualManager_(virtualManager), usPerHour_(usPerHour)
        {}
        // use to execute when 
        virtual void onStatus(){
            Activity::Activity::Status status = notifier_->status();
            if(status == Activity::Activity::executing()){
                // sleep for the required time
                usleep(usPerHour_);
                // execute the virtual manager at correct time
                virtualManager_->nowIs(notifier_->nextTime());
            }
            if(status == Activity::Activity::free()){
                // reschedule notifying activity for next hour
                notifier_->statusIs(Activity::Activity::nextTimeScheduled());
                notifier_->nextTimeIs(notifier_->nextTime().value()+1.0);
                realManager_->lastActivityIs(notifier_);
            }
        } 
    private:
        Activity::ManagerPtr realManager_;
        Activity::ManagerPtr virtualManager_;
        uint64_t usPerHour_;
    };
    typedef Fwk::Ptr<RealToVirtualTimeActivity> R2VTimeActivityPtr;

    Ptr<Instance> fleetInstance_;
    Ptr<Instance> connInstance_;
    Ptr<Instance> statsInstance_;
    typedef struct InstanceMapElem_t {
        InstanceType type;
        Ptr<Instance> ptr;
    } InstanceMapElem;
    map<string, InstanceMapElem> instance_;
    ShippingNetworkPtr shippingNetwork_;

    Activity::ManagerPtr realTimeManager_;
    R2VTimeActivityPtr r2vTimeActivity_;
};

Ptr<Instance> ManagerImpl::instance(const string& name) {
    map<string,InstanceMapElem>::const_iterator t = instance_.find(name);
    if (t == instance_.end())
        return NULL;
    return (*t).second.ptr;
}

class BaseRep : public Instance {
public:
    string attribute(const string& name){
        try{
            return attributeImpl(name);
        }
        catch(ArgumentException e){
            std::cerr << e.message() << std::endl;
        }
        catch(...){
            std::cerr << "Error reading attribute" << std::endl;
        }
        return "";
    }
    void attributeIs(const string& name, const string& v){
        try{
            attributeIsImpl(name,v);
        }
        catch(ArgumentException e){
            std::cerr << e.message() << std::endl;
        }
        catch(...){
            std::cerr << "Error writing attribute" << std::endl; 
        }
    }
    virtual string attributeImpl(const string& name)=0;
    virtual void attributeIsImpl(const string& name, const string& v)=0;
    BaseRep(const string& name) : Instance(name){}
};

class LocationRep : public BaseRep {
public:

    LocationRep(const string& name, ManagerImpl* manager) :
        BaseRep(name), manager_(manager) {
        manager_ = manager;
    }

    // Instance method
    LocationPtr representee() { return representee_; }
    string attributeImpl(const string& name) {
        int i = segmentNumber(name);
        SegmentPtr sp;
        if ((sp = representee_->segment(i))) {
            return sp->name();
        }
        fprintf(stderr, "Location does not have segment at index %d.\n", i);
        return "";
    }
    void attributeIsImpl(const string& name, const string& v) {}
protected:
    Ptr<ManagerImpl> manager_;
    LocationPtr representee_;
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
            Location::EntityType::truckTerminal());
    }
};


class BoatTerminalRep : public LocationRep {
public:
    BoatTerminalRep(const string& name, ManagerImpl *manager) :
        LocationRep(name, manager) {
        representee_ = manager->shippingNetwork()->LocationNew(name,
            Location::EntityType::boatTerminal());
    }
};


class PlaneTerminalRep : public LocationRep {
public:
    PlaneTerminalRep(const string& name, ManagerImpl *manager) :
        LocationRep(name, manager) {
        representee_ = manager->shippingNetwork()->LocationNew(name,
            Location::EntityType::planeTerminal());
    }
};


class CustomerRep : public LocationRep {
public:
    CustomerRep(const string& name, ManagerImpl *manager) :
        LocationRep(name, manager) {
        representee_ = manager->shippingNetwork()->LocationNew(name,
            Location::EntityType::customer());
    }
};


class PortRep : public LocationRep {
public:
    PortRep(const string& name, ManagerImpl *manager) :
        LocationRep(name, manager) {
        representee_ = manager->shippingNetwork()->LocationNew(name,
            Location::EntityType::port());
    }
};


// int based types
string PackageNumToStr(PackageNum x) {
    stringstream s;
    s << x.value();
    return s.str();
}

string MileToStr(Mile x) {
    stringstream s;
    s.precision(2);
    s << fixed << x.value();
    return s.str();
}

string MilePerHourToStr(MilePerHour x) {
    stringstream s;
    s.precision(2);
    s << fixed << x.value();
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


class SegmentRep : public BaseRep {
public:
    SegmentRep(const string& name, ManagerImpl* manager) :
        BaseRep(name), manager_(manager) {
        manager_ = manager;
    }

    // Instance method
    SegmentPtr representee() { return representee_; }
    string attributeImpl(const string& name) {
        if (name == "source") {
            if (representee_->source())
                return representee_->source()->name();
            else return "";
        } else if (name == "return segment") {
            if (representee_->returnSegment())
                return representee_->returnSegment()->name();
            else return "";
        } else if (name == "length") {
            return MileToStr(representee_->length());
        } else if (name == "difficulty") {
            return DifficultyToStr(representee_->difficulty());
        } else if (name == "expedite support") {
            return (representee_->mode(PathMode::expedited()) == 
                            PathMode::expedited()) ?
                "yes" : "no";
        }
        fprintf(stderr, "Invalid attribute input: %s.\n", name.data());
        return "";
    }
    void attributeIsImpl(const string& name, const string& v) {
        if (name == "source") {
            // remove source if string is empty
            if (v == "") {
                representee_->sourceIs("");
                return;
            }
            Ptr<LocationRep> sr = dynamic_cast<LocationRep *> (manager_->instance(v).ptr());
            if (!sr) {
                fprintf(stderr, "Source does not exist: %s.\n", v.data());
                return;
            }
            if (!sourceOK(sr->representee()->entityType())) {
                fprintf(stderr, "Source of incompatible type: %s.\n", v.data());
                return;
            }
            representee_->sourceIs(v);
        } else if (name == "return segment") {
            // remove return segment if string is empty
            if (v == "") {
                representee_->returnSegmentIs("");
                return;
            }
            Ptr<SegmentRep> sr = dynamic_cast<SegmentRep *> (manager_->instance(v).ptr());
            if (!sr) {
                fprintf(stderr, "Segment does not exist: %s.\n", v.data());
                return;
            }
            if (sr->representee_->transportMode() != representee_->transportMode()){
                fprintf(stderr, "Segment of incompatible return type: %s.\n", v.data());
                return;
            }
            representee_->returnSegmentIs(v);
        } else if (name == "length") {
            representee_->lengthIs(Mile(atof(v.data())));
        } else if (name == "difficulty") {
            representee_->difficultyIs(Difficulty(atof(v.data())));
        } else if (name == "expedite support") {
            if (v == "yes")
                representee_->modeIs(PathMode::expedited());
            else if (v == "no")
                representee_->modeDel(PathMode::expedited());
        } else {
            fprintf(stderr, "Invalid input for attributeIs: %s and %s.\n", name.data(), v.data());
        }
    }
protected:
    virtual bool sourceOK(Location::EntityType et) = 0;
    Ptr<ManagerImpl> manager_;
    int segmentNumber(const string& name);
    SegmentPtr representee_;
};


class TruckSegmentRep : public SegmentRep {
public:
    TruckSegmentRep(const string& name, ManagerImpl *manager) :
        SegmentRep(name, manager) {
        representee_ = manager->shippingNetwork()->SegmentNew(name,
            TransportMode::truck(),PathMode::unexpedited());
    }
protected:
    bool sourceOK(Location::EntityType et) {
        if (et == Location::EntityType::truckTerminal() ||
            et == Location::EntityType::customer() ||
            et == Location::EntityType::port())
            return true;
        return false;
    }
};


class BoatSegmentRep : public SegmentRep {
public:
    BoatSegmentRep(const string& name, ManagerImpl *manager) :
        SegmentRep(name, manager) {
        representee_ = manager->shippingNetwork()->SegmentNew(name,
            TransportMode::boat(),PathMode::unexpedited());

    }
protected:
    bool sourceOK(Location::EntityType et) {
        if (et == Location::EntityType::boatTerminal() ||
            et == Location::EntityType::customer() ||
            et == Location::EntityType::port())
            return true;
        return false;
    }
};


class PlaneSegmentRep : public SegmentRep {
public:
    PlaneSegmentRep(const string& name, ManagerImpl *manager) :
        SegmentRep(name, manager) {
        representee_ = manager->shippingNetwork()->SegmentNew(name,
            TransportMode::plane(),PathMode::unexpedited());
    }
protected:
    bool sourceOK(Location::EntityType et) {
        if (et == Location::EntityType::planeTerminal() ||
            et == Location::EntityType::customer() ||
            et == Location::EntityType::port())
            return true;
        return false;
    }
};


class FleetRep : public BaseRep {
public:
    FleetRep(const string& name, ManagerImpl* manager) :
        BaseRep(name), manager_(manager) {
        manager_ = manager;
        fleet_ = manager->shippingNetwork()->FleetNew(name);
    }

    // Instance method
    string attributeImpl(const string& name) {
        stringstream ss;
        FleetAttribute fa = fleetAttribute(name);
        if (fa.attribute == speedStr) {
            return MilePerHourToStr(fleet_->speed(fa.mode));
        } else if (fa.attribute == capacityStr) {
            return PackageNumToStr(fleet_->capacity(fa.mode));
        } else if (fa.attribute == costStr) {
            return DollarPerMileToStr(fleet_->cost(fa.mode));
        } else {
            fprintf(stderr, "Invalid attribute input: %s.\n", name.data());
        }
        return ss.str();
    }
    void attributeIsImpl(const string& name, const string& v) {
        FleetAttribute fa = fleetAttribute(name);
        if (fa.attribute == speedStr) {
            fleet_->speedIs(fa.mode, MilePerHour(atof(v.data())));
        } else if (fa.attribute == capacityStr) {
            fleet_->capacityIs(fa.mode, PackageNum(atoi(v.data())));
        } else if (fa.attribute == costStr) {
            fleet_->costIs(fa.mode, DollarPerMile(atof(v.data())));
        } else {
            fprintf(stderr, "Invalid input for attributeIs: %s and %s.\n", name.data(), v.data());
        }
    }
private:
    typedef struct FleetAttribute_t {
        string attribute;
        TransportMode mode;
    } FleetAttribute;
    FleetAttribute fleetAttribute(const string& str) {
        FleetAttribute result = {"", TransportMode::boat()};

        char* tokenString = strdup(str.data());
        char* namePtr = strtok(tokenString, ", ");
        if (strcmp(namePtr, "Boat") == 0) {
            result.mode = TransportMode::boat();
        } else if (strcmp(namePtr, "Truck") == 0) {
            result.mode = TransportMode::truck();
        } else if (strcmp(namePtr, "Plane") == 0) {
            result.mode = TransportMode::plane();
        } else {
            delete tokenString;
            return result;
        }
        result.attribute = string(strtok(NULL, ", "));
        delete tokenString;
        return result;
    }
    Ptr<ManagerImpl> manager_;
    FleetPtr fleet_;
};


class StatsRep : public BaseRep {
public:
    StatsRep(const string& name, ManagerImpl* manager) :
        BaseRep(name), manager_(manager) {
        manager_ = manager;
        stats_ = manager->shippingNetwork()->StatsNew(name);
    }

    // Instance method
    string attributeImpl(const string& name) {
        std::stringstream ss;

        // return location count
        if (name == truckTerminalStr) {
            ss << stats_->locationCount(
                Location::EntityType::truckTerminal());
        } else if (name == customerStr) {
            ss << stats_->locationCount(
                Location::EntityType::customer());            
        } else if (name == portStr) {
            ss << stats_->locationCount(
                Location::EntityType::port());            
        } else if (name == planeTerminalStr) {
            ss << stats_->locationCount(
                Location::EntityType::planeTerminal());            
        } else if (name == boatTerminalStr) {
            ss << stats_->locationCount(
                Location::EntityType::boatTerminal());            
        }

        // return segment stats
        else if (name == boatSegmentStr) {
            ss << stats_->segmentCount(
                TransportMode::boat());
        } else if (name == truckSegmentStr) {
            ss << stats_->segmentCount(
                TransportMode::truck());
        } else if (name == planeSegmentStr) {
            ss << stats_->segmentCount(
                TransportMode::plane());
        }

        // expedite percentage
        else if (name == "expedite percentage") {
            ss.precision(2);
            uint32_t totalCount = stats_->totalSegmentCount();
            uint32_t expeditedCount = stats_->segmentCount(PathMode::expedited());
            double percentage = 0.0;
            if(totalCount>0){ 
                percentage =  100.0 * ((double)expeditedCount)/((double)stats_->totalSegmentCount());
            }
            ss << fixed << percentage;
        }

        else {
            fprintf(stderr, "Invalid stats attribute input.\n");
        }

        return ss.str();
    }
    void attributeIsImpl(const string& name, const string& v) {
        // nothing to do here
    }
private:
    Ptr<ManagerImpl> manager_;
    StatsPtr stats_;
};

class ConnRep : public BaseRep {
public:
    ConnRep(const string& name, ManagerImpl* manager) :
        BaseRep(name), manager_(manager) {
        manager_ = manager;
        conn_ = manager->shippingNetwork()->ConnNew(name);
    }

    // Instance method
    string attributeImpl(const string& name) {
        // create types useful for parsing
        stringstream ss;
        bool explore = false;
        std::vector<PathPtr> paths;
        size_t expeditedIndex=0;
        char* tokenString = strdup(name.data());
        char* namePtr = strtok(tokenString, ", :");

        // parse string and submit query
        DEBUG_LOG << "Submitting query.\n";
        if (strcmp(namePtr, "connect") == 0) {
            // parse start and end location; return if either is empty
            namePtr = strtok(NULL, ", :");
            Ptr<LocationRep> loc1 = dynamic_cast<LocationRep*> (manager_->instance(namePtr).ptr());
            namePtr = strtok(NULL, ", :");
            Ptr<LocationRep> loc2 = dynamic_cast<LocationRep*> (manager_->instance(namePtr).ptr());
            delete tokenString;
            if (!loc1 && !loc2) {
                fprintf(stderr, "Could not find both locations.\n");
                return "";
            }

            // create pathselector object, run expedited query
            Conn::PathSelector selector(NULL,loc1->representee(), loc2->representee());
            selector.modeIs(PathMode::expedited());
            paths = conn_->paths(selector);

            // submit unexpedited query, add to original paths list
            expeditedIndex=paths.size();
            selector = Conn::PathSelector(NULL,loc1->representee(), loc2->representee());
            selector.modeIs(PathMode::unexpedited());
            std::vector<PathPtr> unexpeditedpaths = conn_->paths(selector);
            paths.insert(paths.end(),unexpeditedpaths.begin(),unexpeditedpaths.end());

        } else if (strcmp(namePtr, "explore") == 0) {
            explore = true;

            // parse start location and constraints; return if null location
            namePtr = strtok(NULL, ", :");
            Ptr<LocationRep> loc = dynamic_cast<LocationRep*> (manager_->instance(namePtr).ptr());
            bool expedited = false;
            Conn::ConstraintPtr constraints = parseConstraints(namePtr, expedited);
            delete tokenString;
            if (!loc) {
                fprintf(stderr, "Invalid location name %s.\n", namePtr);
                return "";
            }

            // create pathselector object, run expedited query
            Conn::PathSelector selector(constraints,loc->representee());
            selector.modeIs(PathMode::expedited());
            paths = conn_->paths(selector);

            // add unexpedited paths if no constraint on expedite support
            expeditedIndex = paths.size();
            if (!expedited) {
                selector = Conn::PathSelector(constraints,loc->representee());
                selector.modeIs(PathMode::unexpedited());
                std::vector<PathPtr> unexpeditedPaths = conn_->paths(selector);
                paths.insert(paths.end(), unexpeditedPaths.begin(), unexpeditedPaths.end());
            }
        }

        // output paths
        DEBUG_LOG << "Reading path.\n";
        unsigned int numPaths = paths.size();
        DEBUG_LOG << numPaths << " path(s) found.\n";
        std::set<string> pathStrings;
        for (size_t i = 0; i < numPaths; i++) {
            PathPtr path = paths[i];

            // output cost/time if not explore
            if (!explore) {
                ss << DollarToStr(path->cost()) << " ";
                ss << HourToStr(path->time()) << " ";
                ss << (i < expeditedIndex ?
                    "yes" : "no");
                ss << "; ";
            }

            uint32_t numLocs = path->pathElementCount().value();
            DEBUG_LOG << numLocs << " location(s) in path.\n";
            for (uint32_t j = 0; j < numLocs; j++) {
                ss << path->pathElement(j)->source()->name();
                ss << "(" << path->pathElement(j)->segment()->name() << ":";
                ss << MileToStr(path->pathElement(j)->segment()->length());
                ss << ":" << path->pathElement(j)->segment()->returnSegment()->name() << ") ";
            }

            ss << path->lastLocation()->name() << "\n";
            pathStrings.insert(ss.str());
            ss.str(std::string());
        }

        // return unique set of paths
        for (std::set<string>::iterator it=pathStrings.begin(); it!=pathStrings.end(); it++)
            ss << *it;
        return ss.str();
    }
    void attributeIsImpl(const string& name, const string& v) {
        // nothing to do here
    }
private:
    Conn::ConstraintPtr parseConstraints(char* s, bool& expedited) {
        Conn::ConstraintPtr result = NULL, lastPtr = NULL, newPtr = NULL;
        while ((s = strtok(NULL, ": "))) {
            if (strcmp(s, "distance") == 0) {
                s = strtok(NULL, ": ");
                newPtr = Conn::DistanceConstraintIs(Mile(atof(s)));
            } else if (strcmp(s, "cost") == 0) {
                s = strtok(NULL, ": ");
                newPtr = Conn::CostConstraintIs(Dollar(atof(s)));
            } else if (strcmp(s, "time") == 0) {
                s = strtok(NULL, ": ");
                newPtr = Conn::TimeConstraintIs(Hour(atof(s)));
            } else if (strcmp(s, "expedited") == 0) {
                expedited=true; 
            } else {
                fprintf(stderr, "Invalid explore constraint.\n");
                break;
            }
            if (!result) result = newPtr;
            else lastPtr->nextIs(newPtr);
            lastPtr = newPtr;
        }
        return result;
    }
    Ptr<ManagerImpl> manager_;
    ConnPtr conn_;
};

ManagerImpl::ManagerImpl() {
    fleetInstance_ = NULL;
    connInstance_ = NULL;
    statsInstance_ = NULL;
    shippingNetwork_ = ShippingNetwork::ShippingNetworkIs("ShippingNetwork");
    realTimeManager_ = Activity::Manager::ManagerIs();
    r2vTimeActivity_ = new RealToVirtualTimeActivity(realTimeManager_,shippingNetwork_->manager(),1000000);
    // Setup activity
    Activity::ActivityPtr activityPtr = realTimeManager_->activityNew("r2vtime_activity");
    activityPtr->nextTimeIs(0.0);
    activityPtr->lastNotifieeIs(r2vTimeActivity_.ptr());
    realTimeManager_->lastActivityIs(activityPtr);
    // Update the expedited costs by their multipliers
    FleetPtr fleet = shippingNetwork_->FleetNew("_repfleet");
    fleet->costMultiplierIs(PathMode::unexpedited(),1.0);
    fleet->speedMultiplierIs(PathMode::unexpedited(),1.0);
    fleet->costMultiplierIs(PathMode::expedited(),1.5);
    fleet->speedMultiplierIs(PathMode::expedited(),1.3);
}

Ptr<Instance> ManagerImpl::instanceNew(const string& name,
    const string& type) {
    try{
    // do not name anything the empty string
    if (name == "") {
        fprintf(stderr, "Invalid name.\n");
        return NULL;
    }

    // do not create an instance if the name already exists
    if (instance(name)) {
        fprintf(stderr, "Instance already exists with name %s.\n", name.data());
        return NULL;
    }

    Ptr<Instance> inst;
    InstanceType instType;

    // Location Types
    if (type == truckTerminalStr) {
        inst = new TruckTerminalRep(name, this);
        instType = truckTerminal_;
    } else if (type == customerStr) {
        inst = new CustomerRep(name, this);
        instType = customer_;
    } else if (type == portStr) {
        inst = new PortRep(name, this);
        instType = port_;
    } else if (type == boatTerminalStr) {
        inst = new BoatTerminalRep(name, this);
        instType = boatTerminal_;
    } else if (type == planeTerminalStr) {
        inst = new PlaneTerminalRep(name, this);
        instType = planeTerminal_;
    }

    // Segment Types
    else if (type == planeSegmentStr) {
        inst = new PlaneSegmentRep(name, this);
        instType = planeSegment_;
    } else if (type == truckSegmentStr) {
        inst = new TruckSegmentRep(name, this);
        instType = truckSegment_;
    } else if (type == boatSegmentStr) {
        inst = new BoatSegmentRep(name, this);
        instType = boatSegment_;
    }

    // Conn Type
    else if (type == "Conn") {
        if (connInstance_) return connInstance_;
        inst = new ConnRep(name, this);
        connInstance_ = inst;
        instType = conn_;
    }

    // Fleet Type
    else if (type == "Fleet") {
        if (fleetInstance_) return fleetInstance_;
        inst = new FleetRep(name, this);
        fleetInstance_ = inst;
        instType = fleet_;
    }

    // Stats Type
    else if (type == "Stats") {
        if (statsInstance_) return statsInstance_;
        inst = new StatsRep(name, this);
        statsInstance_ = inst;
        instType = stats_;
    }

    else {
        fprintf(stderr, "Invalid instance new.\n");
        return NULL;
    }

    InstanceMapElem ime = {instType, inst};
    instance_[name] = ime;
    return inst;

    }
    catch(...){
        std::cerr << "Error caught from rep layer" << std::endl;
        return NULL;
    }
}

void ManagerImpl::instanceDel(const string& name) {
    try{
    map<string,InstanceMapElem>::const_iterator t = instance_.find(name);
    if (t == instance_.end()) {
        fprintf(stderr, "Instance does not exist with name: %s.\n", name.data());
        return;
    }
    Ptr<Instance> inst = (*t).second.ptr;
    InstanceType instType = (*t).second.type;
    if (instType == customer_ || instType == port_ || instType == truckTerminal_ ||
        instType == boatTerminal_ || instType == planeTerminal_) {
        shippingNetwork_->locationDel(name);
    } else if (instType == boatSegment_ || instType == truckSegment_ ||
        instType == planeSegment_) {
        shippingNetwork_->segmentDel(name);
    }

    fprintf(stderr, "Type cannot be deleted.\n");
    instance_.erase(name);
    }
    catch(...){
        std::cerr << "Error caught from rep layer" << std::endl;
    }
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

