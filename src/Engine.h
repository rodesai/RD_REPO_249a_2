#ifndef ENGINE_H
#define ENGINE_H

#include <stdint.h>

#include <string>
#include <vector>
#include <map>
#include <exception>

#include "fwk/Ptr.h"
#include "fwk/NamedInterface.h"

#include "Nominal.h"

namespace Shipping {


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

class Segment : public Fwk::NamedInterface {

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

    class Notifiee : public virtual Fwk::NamedInterface::Notifiee {

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

    Segment(string name, EntityType type) : Fwk::NamedInterface(name), length_(0), difficulty_(1.0), expediteSupport_(no_), entityType_(type){}

    // attributes
    Mile length_;
    Difficulty difficulty_;
    ExpediteSupport expediteSupport_;
    EntityType entityType_;
    Segment::Ptr returnSegment_;
    Location::Ptr source_;
    Segment::Notifiee::Ptr notifiee_;
};


class Location : public Fwk::NamedInterface {

public:

    typedef Fwk::Ptr<Location> Ptr;
    typedef Fwk::Ptr<Location const> PtrConst;

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

    Segment::Ptr segment(uint32_t index) const { return segments_[index]; }

    inline EntityType entityType() const { return entityType_; }

    // mutators
    void entityTypeIs(EntityType et) { entityType_ = et; }
    // note: per the instructions, segments_ is read-only

protected: 

    Location(string name, EntityType type): Fwk::NamedInterface(name), entityType_(type){}

    virtual void segmentEnq(Segment::Ptr s){
        // add segment
    }

private:

    void segmentDeq(Segment::Ptr s) {
        // TODO: remove segment from list
    }

    EntityType entityType_;

    typedef std::vector<Segment::Ptr> SegmentList;
    SegmentList segments_;

};


class Customer : public Location{

public: 

    typedef Fwk::Ptr<Customer> Ptr;
    typedef Fwk::Ptr<Customer const> PtrConst;

private:

    friend class ShippingEngine;

    // Constructor
    Customer() : Location(Location::customer()){}
};


class Port : public Location{

public:

    typedef Fwk::Ptr<Port> Ptr;
    typedef Fwk::Ptr<Port const> PtrConst;

private:

    friend class ShippingEngine;

    // Constructor
    Port() : Location(Location::port()){}
};


class Terminal : public Location{

protected:

    // Constructor
    Terminal(Location::EntityType terminalType, Segment::EntityType segmentType) : Location(terminalType),segmentType_(segmentType){}

private:

    // Add a segment
    void segmentEnq(Segment::Ptr s){
        if(segmentType_ != s->entityType()){
            // ERROR!
        }
        Location::segmentEnq(s);
    }

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

class Path : public Fwk::NamedInterface {

public:

    typedef Fwk::Ptr<Path> Ptr;
    typedef Fwk::Ptr<Path const> PtrConst;

    class PathElement : public NamedInterface {

    public:

        typedef Fwk::Ptr<PathElement> Ptr;
        typedef Fwk::Ptr<PathElement const> PtrConst;

        // accessors
        inline Location::Ptr source() const { return source_; }
        inline Segment::Ptr segment() const { return segment_; }

        // mutators
        void sourceIs(Location::Ptr s) { source_ = s; }
        void segmentIs(Segment::Ptr s) { segment_ = s; }
    private:

        Location::Ptr source_;
        Segment::Ptr segment_;
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


class Conn : public Fwk::NamedInterface {

public:

    typedef Fwk::Ptr<Conn> Ptr;
    typedef Fwk::Ptr<Conn const> PtrConst;

    std::vector<Path::Ptr> connect(Location::Ptr start, Location::Ptr end) const;
    std::vector<Path::Ptr> explore(Location::Ptr start, Location::Ptr end,
        Mile distance, Dollar cost, Hour time,
        Segment::ExpediteSupport expedited) const;
};


class Fleet : public Fwk::NamedInterface {

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


class Stats : public Fwk::NamedInterface {

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

class ShippingEngine : public Fwk::NamedInterface {

public:

    // class types

    typedef Fwk::Ptr<ShippingEngine> Ptr;
    typedef Fwk::Ptr<ShippingEngine const> PtrConst;

    // notifiee

    class Notifiee : public virtual Fwk::NamedInterface::Notifiee {
    public:

        typedef Fwk::Ptr<ShippingEngine::Notifiee> Ptr;
        typedef Fwk::Ptr<ShippingEngine::Notifiee const> PtrConst;

        // Segments
        virtual void onSegmentNew(Segment::Ptr segment){}
        virtual void onSegmentDel(Segment::Ptr segment){}

        // Locations
        virtual void onCustomerNew(Location::Ptr location){}
        virtual void onCustomerDel(Customer::Ptr customer){}
        virtual void onPortNew(Location::Ptr port){}
        virtual void onPortDel(Port::Ptr port){}
        virtual void onPlaneTerminalNew(Location::Ptr planeTerminal){}
        virtual void onPlaneTerminalDel(PlaneTerminal::Ptr planeTerminal){}
        virtual void onBoatTerminalNew(Location::Ptr boatTerminal){}
        virtual void onBoatTerminalDel(BoatTerminal::Ptr boatTerminal){}
        virtual void onTruckTerminalNew(Location::Ptr truckTerminal){}
        virtual void onTruckTerminalDel(TruckTerminal::Ptr truckTerminal){}
 
    protected:

        Notifiee() {}

    private:

        ShippingEngine::Ptr shippingEngine_;
    };

    // accessor

    Segment::Ptr segement(string name) { return segmentMap_[name]; }
    Location::Ptr location(string name) { return locationMap_[name]; }
    Conn::Ptr conn(string cid) const;
    Stats::Ptr stats(string sid) const;
    Fleet::Ptr fleet(string fid) const;

    // mutators

    // Instance Creators
    // These instance creators create an instance, add it to the map, 
    // and set up any needed reactors;

    Segment::Ptr SegmentNew(string name, Segment::EntityType entityType); 
    Segment::Ptr segmentDel();

    Location::Ptr CustomerNew(string name);
    Location::Ptr customerDel();

    Location::Ptr PortNew(string name);
    Location::Ptr portDel();

    Location::Ptr TruckTerminalNew(string name);
    Location::Ptr truckTerminalDel();

    Location::Ptr BoatTerminalNew(string name);
    Location::Ptr boatTerminalDel();

    Location::Ptr PlaneTerminalNew(string name);
    Location::Ptr planeTerminalDel();

    // These instance creators should only create instance on first call

    Conn::Ptr ConnNew(string name);
    Conn::Ptr connDel(string name);

    Stats::Ptr StatsNew(string name);
    Stats::Ptr statsDel(string name);

    Fleet::Ptr FleetNew(string name);
    Fleet::Ptr fleetDel(string name);

    void notifieeIs(Segment::Notifiee::Ptr notifiee);

private:

    // attributes
    typedef std::map<string, Location::Ptr> LocationMap;
    LocationMap locationMap_;
    typedef std::map<string, Segment::Ptr> SegmentMap;
    SegmentMap segmentMap_;

    // notifiees
    typedef std::vector<ShippingEngine::Notifiee::Ptr> NotifieeList;
    NotifieeList notifieees_;
};

class SegmentReactor : public Segment::Notifiee {

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
        currentSource_ = NULL;
        currentReverse_ = NULL;
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

    void onSegmentNew(Segment::Ptr segment);
    void onSegmentDel(Segment::Ptr segment);
    void onCustomerNew(Customer::Ptr customer);
    void onCustomerDel(Customer::Ptr customer);
    void onPortNew(Port::Ptr port);
    void onPortDel(Port::Ptr port);
    void onPlaneTerminalNew(PlaneTerminal::Ptr planeTerminal);
    void onPlaneTerminalDel(PlaneTerminal::Ptr planeTerminal);
    void onBoatTerminalNew(BoatTerminal::Ptr boatTerminal);
    void onBoatTerminalDel(BoatTerminal::Ptr boatTerminal);
    void onTruckTerminalNew(TruckTerminal::Ptr truckTerminal);
    void onTruckTerminalDel(TruckTerminal::Ptr truckTerminal);

    static StatsReactor::Ptr StatsReactorIs(){
        return new StatsReactor();
    }

private:

    StatsReactor(){}
};

} /* end namespace */

#endif
