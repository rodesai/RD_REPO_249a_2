#ifndef ENGINE_H
#define ENGINE_H

#include <stdint.h>

#include <string>
#include <vector>
#include <map>
#include <exception>

#include "Ptr.h"
#include "PtrInterface.h"

#include "Nominal.h"

namespace Shipping {

typedef std::string ConnID;
typedef std::string FleetID;
typedef std::string StatsID;
typedef std::string SegmentID;
typedef std::string LocationID;

class ArgumentException : public std::exception {
    virtual const char* message() const throw() {
        return "Argument exception.";
    }
};

// Primitive Types

class Mile : public Ordinal<Mile, uint64_t> {
public:
    Mile(uint64_t num) : Ordinal<Mile, uint64_t>(num) {}
};


class MilePerHour : public Nominal<MilePerHour, uint64_t> {
public:
    MilePerHour(uint64_t num) : Nominal<MilePerHour, uint64_t>(num) {}
};


class Dollar : public Ordinal<Dollar, float> {
public:
    Dollar(float num) : Ordinal<Dollar, float>(num) {
        if (num < 0) throw ArgumentException();
    }
};


class DollarPerMile : public Nominal<DollarPerMile, uint64_t> {
public:
    DollarPerMile(uint64_t num) : Nominal<DollarPerMile, uint64_t>(num) {}
};


class Hour : public Ordinal<Hour, float> {
public:
    Hour(float num) : Ordinal<Hour, float>(num) {
        if (num < 0) throw ArgumentException();
    }
};


class Difficulty : public Nominal<Difficulty, float> {
public:
    Difficulty(float num) : Nominal<Difficulty, float>(num) {
        if (num < 1.0 || num > 5.0) throw ArgumentException();
    }
};


class PackageNum : public Ordinal<PackageNum, uint64_t> {
public:
    PackageNum(uint64_t num) : Ordinal<PackageNum, uint64_t>(num) {}
};

// Core Types

class Segment;
class SegmentReactor;

class Location : public Fwk::PtrInterface<Location>{

public:

    typedef Fwk::Ptr<Location> Ptr;
    typedef Fwk::Ptr<Location const> PtrConst;

    enum SegmentSourceOK{
        yes_=0,
        no_=1
    };

    enum EntityType {
        customer_ = 0,
        port_ = 1,
        truckTerminal_ = 2,
        boatTerminal_ = 3,
        planeTerminal_ = 4
    };

    // accessors
    static inline EntityType customer() { return customer_; } 
    static inline EntityType port() { return port_; } 
    static inline EntityType truckTerminal() { return truckTerminal_; } 
    static inline EntityType boatTerminal() { return boatTerminal_; } 
    static inline EntityType planeTerminal() { return planeTerminal_; } 
    static inline SegmentSourceOK yes(){ return yes_; }
    static inline SegmentSourceOK no(){ return no_; }

    SegmentID segment(uint32_t index) const { return segments_[index]; }

    inline EntityType entityType() const { return entityType_; }

    // mutators
    void entityTypeIs(EntityType et) { entityType_ = et; }
    // note: per the instructions, segments_ is read-only

    virtual SegmentSourceOK segmentSourceOK(Segment*) const = 0;

protected: 

    Location(EntityType type): entityType_(type){}

private:

    friend class SegmentReactor;

    void segmentEnq(SegmentID s){
        // add segment
    }

    void segmentDeq(SegmentID s) {
        // TODO: remove segment from list
    }

    EntityType entityType_;

    typedef std::vector<SegmentID> SegmentList;
    SegmentList segments_;

    LocationID id_;
};


class Segment : public Fwk::PtrInterface<Segment>{

public:

    // Class Types

    typedef Fwk::Ptr<Segment> Ptr;
    typedef Fwk::Ptr<Segment const> PtrConst;

    enum ExpediteSupport {
        no_ = 0,
        yes_ = 1
    };

    enum EntityType {
        truckSegment_ = 0,
        boatSegment_ = 1,
        planeSegment_ = 2
    };

    // Notifiees

    class Notifiee : public Fwk::PtrInterface<Segment::Notifiee>{

    public:

        typedef Fwk::Ptr<Segment::Notifiee> Ptr;
        typedef Fwk::Ptr<Segment::Notifiee const> PtrConst;

        // Events
        virtual void onSource(){}
        virtual void onReverseSegment(){}

        void notifierIs(Segment::Ptr notifier){ notifier_=notifier; }
        Segment::Ptr notifier() const { return notifier_; }

    protected:
        Notifiee(){}
    private:
        Segment::Ptr notifier_;
    };

    // accesors
    static inline ExpediteSupport yes() { return yes_; }
    static inline ExpediteSupport no() { return no_; }
    static inline EntityType truckSegment() { return truckSegment_; }
    static inline EntityType boatSegment() { return boatSegment_; }
    static inline EntityType planeSegment() { return planeSegment_; }

    inline EntityType entityType() const { return entityType_; }
    inline Location::Ptr source() { return source_; }
    inline Mile length() const { return length_; }
    inline Segment::Ptr returnSegment() { return returnSegment_; }
    inline Difficulty difficulty() const { return difficulty_; }
    inline ExpediteSupport expediteSupport() const { return expediteSupport_; }

    Segment::Notifiee::Ptr notifiee(){ return notifiee_; }

    // mutators
    void sourceIs(Location::Ptr source);
    void lengthIs(Mile l) { length_ = l; }
    void returnSegmentIs(Segment::Ptr s) { returnSegment_ = s; }
    void difficultyIs(Difficulty d) { difficulty_ = d; }
    void expediteSupportIs(ExpediteSupport es) { expediteSupport_ = es; }

    void notifieeIs(Segment::Notifiee::Ptr notifiee){
        notifiee->notifierIs(this);
        notifiee_ = notifiee;
    }

private:

    friend class ShippingEngine;

    Segment(SegmentID id, EntityType type) : length_(0), difficulty_(1.0), expediteSupport_(no_), id_(id), entityType_(type){}

    // attributes
    Mile length_;
    Difficulty difficulty_;
    ExpediteSupport expediteSupport_;
    SegmentID id_;
    EntityType entityType_;
    Segment::Ptr returnSegment_;
    Location::Ptr source_;

    Segment::Notifiee::Ptr notifiee_;
};



class Customer : public Location{

public: 

    typedef Fwk::Ptr<Customer> Ptr;
    typedef Fwk::Ptr<Customer const> PtrConst;

    SegmentSourceOK segmentSourceOK(Segment* segment) const{
        return yes_;
    }

private:

    friend class ShippingEngine;

    // Constructor
    Customer() : Location(Location::customer()){}
};


class Port : public Location{

public:

    typedef Fwk::Ptr<Port> Ptr;
    typedef Fwk::Ptr<Port const> PtrConst;

    SegmentSourceOK segmentSourceOK(Segment* segment) const{
        return yes_;
    }

private:

    friend class ShippingEngine;

    // Constructor
    Port() : Location(Location::port()){}
};


class Terminal : public Location{

protected:

    // Constructor
    Terminal(Location::EntityType terminalType, Segment::EntityType segmentType) : Location(terminalType),segmentType_(segmentType){}

    SegmentSourceOK segmentSourceOK(Segment* segment) const {
        if( segment->entityType() == segmentType_) return yes_;
        return no_;
    }

private:

    Segment::EntityType segmentType_;
};


class TruckTerminal : public Terminal{

public:

    typedef Fwk::Ptr<TruckTerminal> Ptr;
    typedef Fwk::Ptr<TruckTerminal const> PtrConst;

    // Instantiator
    TruckTerminal::Ptr TruckTerminalIs();

private:

    friend class ShippingEngine;

    // Constructor
    TruckTerminal() : Terminal(Location::truckTerminal(),Segment::truckSegment()){}
};


class BoatTerminal : public Terminal{

public:

    typedef Fwk::Ptr<BoatTerminal> Ptr;
    typedef Fwk::Ptr<BoatTerminal const> PtrConst;

private:

    friend class ShippingEngine;

    // Constructor
    BoatTerminal() : Terminal(Location::boatTerminal(),Segment::boatSegment()){}
};


class PlaneTerminal : public Terminal{

public:

    typedef Fwk::Ptr<PlaneTerminal> Ptr;
    typedef Fwk::Ptr<PlaneTerminal const> PtrConst;

private:

    friend class ShippingEngine;

    // Constructor
    PlaneTerminal() : Terminal(planeTerminal_,Segment::planeSegment()){}
};


class Path : public Fwk::PtrInterface<Path> {

public:

    typedef Fwk::Ptr<Path> Ptr;
    typedef Fwk::Ptr<Path const> PtrConst;

    class PathElement : public PtrInterface<PathElement>{

    public:

        typedef Fwk::Ptr<PathElement> Ptr;
        typedef Fwk::Ptr<PathElement const> PtrConst;

        // accessors
        inline LocationID source() const { return source_; }
        inline SegmentID segment() const { return segment_; }

        // mutators
        void sourceIs(LocationID s) { source_ = s; }
        void segmentIs(SegmentID s) { segment_ = s; }
    private:

        LocationID source_;
        SegmentID segment_;
    };


    typedef std::vector<PathElement::Ptr> LocationVector;
    typedef std::vector<PathElement::Ptr>::iterator LocationIterator;

    // accessors
    Dollar cost() const { return cost_; }
    Hour time() const { return time_; }
    Segment::ExpediteSupport expedited() const { return expedited_; }
    LocationIterator pathIterator(); 

private:

    Dollar cost_;
    Hour time_;
    Segment::ExpediteSupport expedited_;

    LocationVector locations_;
};


class Conn : public Fwk::PtrInterface<Conn>{

public:

    typedef Fwk::Ptr<Conn> Ptr;
    typedef Fwk::Ptr<Conn const> PtrConst;

    std::vector<Path::Ptr> connect() const;
    std::vector<Path::Ptr> explore() const;
};


class Fleet : public Fwk::PtrInterface<Fleet>{

public:

    typedef Fwk::Ptr<Fleet> Ptr;
    typedef Fwk::Ptr<Fleet const> PtrConst;

    MilePerHour speed(Segment::EntityType segmentType);
    PackageNum capacity(Segment::EntityType segmentType);
    DollarPerMile cost(Segment::EntityType segmentType);

private:

    typedef std::map<Segment::EntityType,MilePerHour> SpeedMap;
    SpeedMap speed_;

    typedef std::map<Segment::EntityType,PackageNum> CapacityMap;
    CapacityMap capacity_;
   
    typedef std::map<Segment::EntityType,DollarPerMile> CostMap;
    CostMap cost_;
};


class Stats : public Fwk::PtrInterface<Stats>{

    // TODO: this needs to be updated via notification

public:

    typedef Fwk::Ptr<Stats> Ptr;
    typedef Fwk::Ptr<Stats const> PtrConst;

    // accessors
    inline int locationCount(Location::EntityType et) 
        { return locationCount_[et]; }
    inline int segmentCount(Segment::EntityType et) 
        { return segmentCount_[et]; }
    inline float expeditePercentage() const
        { return expediteSegmentCount_ * 1.0 / segmentCountTotal_;}
private:
    std::map<Location::EntityType, int> locationCount_;
    std::map<Segment::EntityType, int> segmentCount_;
    int expediteSegmentCount_;
    int segmentCountTotal_;
};

class ShippingEngine : public Fwk::PtrInterface<Stats>{

public:

    // class types

    typedef Fwk::Ptr<ShippingEngine> Ptr;
    typedef Fwk::Ptr<ShippingEngine const> PtrConst;

    // notifiee

    class Notifiee : public Fwk::PtrInterface<ShippingEngine::Notifiee>{
    public:

        typedef Fwk::Ptr<ShippingEngine::Notifiee> Ptr;
        typedef Fwk::Ptr<ShippingEngine::Notifiee const> PtrConst;

        // Segments
        virtual void onSegmentNew(SegmentID segmentID){}
        virtual void onSegmentDel(Segment::Ptr segment){}

        // Locations
        virtual void onCustomerNew(LocationID locationID){}
        virtual void onCustomerDel(Customer::Ptr customer){}
        virtual void onPortNew(LocationID locationID){}
        virtual void onPortDel(Port::Ptr port){}
        virtual void onPlaneTerminalNew(LocationID locationID){}
        virtual void onPlaneTerminalDel(PlaneTerminal::Ptr planeTerminal){}
        virtual void onBoatTerminalNew(LocationID locationID){}
        virtual void onBoatTerminalDel(BoatTerminal::Ptr boatTerminal){}
        virtual void onTruckTerminalNew(LocationID locationID){}
        virtual void onTruckTerminalDel(TruckTerminal::Ptr truckTerminal){}
 
    protected:

        Notifiee() {}

    private:

        ShippingEngine::Ptr shippingEngine_;
    };

    // accessor

    Segment::Ptr segement(SegmentID sid) { return segmentMap_[sid]; }
    Location::Ptr location(LocationID lid) { return locationMap_[lid]; }
    Conn::Ptr conn(ConnID cid) const;
    Stats::Ptr stats(StatsID sid) const;
    Fleet::Ptr fleet(FleetID fid) const;

    // mutators

    // Instance Creators
    // These instance creators create an instance, add it to the map, 
    // and set up any needed reactors;

    Segment::Ptr SegmentNew(SegmentID name, Segment::EntityType entityType); 
    Segment::Ptr segmentDel();

    Location::Ptr CustomerNew(LocationID name);
    Location::Ptr customerDel();

    Location::Ptr PortNew(LocationID name);
    Location::Ptr portDel();

    Location::Ptr TruckTerminalNew(LocationID name);
    Location::Ptr truckTerminalDel();

    Location::Ptr BoatTerminalNew(LocationID name);
    Location::Ptr boatTerminalDel();

    Location::Ptr PlaneTerminalNew(LocationID name);
    Location::Ptr planeTerminalDel();

    // These instance creators should only create instance on first call

    Conn::Ptr ConnNew(ConnID name);
    Conn::Ptr connDel(ConnID name);

    Stats::Ptr StatsNew(StatsID name);
    Stats::Ptr statsDel(StatsID name);

    Fleet::Ptr FleetNew(FleetID name);
    Fleet::Ptr fleetDel(FleetID name);

    void notifieeIs(Segment::Notifiee::Ptr notifiee);

private:

    // attributes
    typedef std::map<LocationID, Location::Ptr> LocationMap;
    LocationMap locationMap_;
    typedef std::map<SegmentID, Segment::Ptr> SegmentMap;
    SegmentMap segmentMap_;

    // notifiees
    typedef std::vector<ShippingEngine::Notifiee::Ptr> NotifieeList;
    NotifieeList notifieees_;
};

class SegmentReactor : public Segment::Notifiee{

public:

    typedef Fwk::Ptr<SegmentReactor> Ptr;
    typedef Fwk::Ptr<SegmentReactor const> PtrConst;

    /* Remove owner from the current source
     * Add owner to the new source
     * Reset current source to the new source
     */
    void onSource();

    /* Remove owner from current reverse segment
     * Add owner to the new reverse segment
     * Reset the current reverse segment to the 
     *   new reverse segment
     */ 
    void onReverseSegment();  

    static SegmentReactor::Ptr SegmentReactorIs(){
        return new SegmentReactor();
    }

private:

    SegmentReactor(){
        currentSource_=NULL;
        currentReverse_=NULL;
    }

    Location::Ptr currentSource_;
    Segment::Ptr  currentReverse_;
};

class ShippingEngineReactor : public ShippingEngine::Notifiee{

public:

    typedef Fwk::Ptr<ShippingEngineReactor> Ptr;
    typedef Fwk::Ptr<ShippingEngineReactor const> PtrConst;

    void onSegmentDel(Segment::Ptr segment) {
        // remove segment from source
        // remove segment from reverse segment
    }

    static ShippingEngineReactor::Ptr ShippingEngineReactorIs(){
        return new ShippingEngineReactor();
    }

private:

    ShippingEngineReactor(){}
};


class StatsReactor : public ShippingEngine::Notifiee{

public:

    typedef Fwk::Ptr<StatsReactor> Ptr;
    typedef Fwk::Ptr<StatsReactor const> PtrConst;

    void onSegmentNew(SegmentID segmentID);
    void onSegmentDel(Segment::Ptr segment);
    void onCustomerNew(LocationID locationID);
    void onCustomerDel(Customer::Ptr customer);
    void onPortNew(LocationID locationID);
    void onPortDel(Port::Ptr port);
    void onPlaneTerminalNew(LocationID locationID);
    void onPlaneTerminalDel(PlaneTerminal::Ptr planeTerminal);
    void onBoatTerminalNew(LocationID locationID);
    void onBoatTerminalDel(BoatTerminal::Ptr boatTerminal);
    void onTruckTerminalNew(LocationID locationID);
    void onTruckTerminalDel(TruckTerminal::Ptr truckTerminal);

    static StatsReactor::Ptr StatsReactorIs(){
        return new StatsReactor();
    }

private:

    StatsReactor(){}
};

} /* end namespace */

#endif
