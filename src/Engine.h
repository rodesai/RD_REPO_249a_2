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

class Segment : public Fwk::PtrInterface<Segment>{

public:

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

    // accesors
    static inline ExpediteSupport yes() { return yes_; }
    static inline ExpediteSupport no() { return no_; }
    static inline EntityType truckSegment() { return truckSegment_; }
    static inline EntityType boatSegment() { return boatSegment_; }
    static inline EntityType planeSegment() { return planeSegment_; }

    inline EntityType entityType() const { return entityType_; }
    inline LocationID source() { return source_; }
    inline Mile length() const { return length_; }
    inline Segment::Ptr returnSegment() { return returnSegment_; }
    inline Difficulty difficulty() const { return difficulty_; }
    inline ExpediteSupport expediteSupport() const { return expediteSupport_; }

    // mutators
    void sourceIs(LocationID source){
        // remove segment from previous source, if exists
        // set source
        // add segment to new source
    }
    void lengthIs(Mile l) { length_ = l; }
    void returnSegmentIs(Segment::Ptr s) { returnSegment_ = s; }
    void difficultyIs(Difficulty d) { difficulty_ = d; }
    void expediteSupportIs(ExpediteSupport es) { expediteSupport_ = es; }

    Segment::Ptr SegmentIs(SegmentID id, EntityType type){
        return new Segment(id, type);
    }

private:

    Segment(SegmentID id, EntityType type) : length_(0), difficulty_(1.0), expediteSupport_(no_), id_(id), entityType_(type){}

    Mile length_;

    Difficulty difficulty_;

    ExpediteSupport expediteSupport_;

    SegmentID id_;

    EntityType entityType_;

    Segment::Ptr returnSegment_;

    LocationID source_;
};


class Location : public Fwk::PtrInterface<Location>{

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

    Location(EntityType type): entityType_(type){}

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

    LocationID id_;
};


class Customer : public Location{

public: 

    typedef Fwk::Ptr<Customer> Ptr;
    typedef Fwk::Ptr<Customer const> PtrConst;

    // Instantiator
    Customer::Ptr CustomerIs();

private:

    // Constructor
    Customer() : Location(Location::customer()){}
};


class Port : public Location{

public:

    typedef Fwk::Ptr<Port> Ptr;
    typedef Fwk::Ptr<Port const> PtrConst;

    // Instantiator
    Port::Ptr PortIs();

private:

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
    // Constructor
    TruckTerminal() : Terminal(Location::truckTerminal(),Segment::truckSegment()){}
};


class BoatTerminal : public Terminal{

public:

    typedef Fwk::Ptr<BoatTerminal> Ptr;
    typedef Fwk::Ptr<BoatTerminal const> PtrConst;

    // Instantiator
    BoatTerminal::Ptr BoatTerminalIs();

private:
    // Constructor
    BoatTerminal() : Terminal(Location::boatTerminal(),Segment::boatSegment()){}
};


class PlaneTerminal : public Terminal{

public:

    typedef Fwk::Ptr<PlaneTerminal> Ptr;
    typedef Fwk::Ptr<PlaneTerminal const> PtrConst;

    // Instantiator
    PlaneTerminal::Ptr PlaneTerminalIs();

private:

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

    typedef Fwk::Ptr<ShippingEngine> Ptr;
    typedef Fwk::Ptr<ShippingEngine const> PtrConst;

    // accessor
    Segment::Ptr segement(SegmentID sid) { return segmentMap_[sid]; }
    Location::Ptr location(LocationID lid) { return locationMap_[lid]; }
    Conn::Ptr conn(ConnID cid) const;
    Stats::Ptr stats(StatsID sid) const;
    Fleet::Ptr fleet(FleetID fid) const;

    // mutators
    void segmentIs(Segment::Ptr s, SegmentID name) { segmentMap_[name] = s; }
    void segmentDel();
    void locationIs(Location::Ptr s, LocationID name) { locationMap_[name] = s; }
    void locationDel();

private:

    typedef std::map<LocationID, Location::Ptr> LocationMap;
    LocationMap locationMap_;

    typedef std::map<SegmentID, Segment::Ptr> SegmentMap;
    SegmentMap segmentMap_;
};

class ShippingEngineReactor : public Fwk::PtrInterface<ShippingEngineReactor>{

public:

    typedef Fwk::Ptr<ShippingEngineReactor> Ptr;
    typedef Fwk::Ptr<ShippingEngineReactor const> PtrConst;

    void onSegmentDel(Segment::Ptr segment) {
        // remove segment from source
        // remove segment from reverse segment
    }

private:
    ShippingEngine::Ptr owner_;
};


class StatsReactor : public Fwk::PtrInterface<Stats>{

public:

    typedef Fwk::Ptr<StatsReactor> Ptr;
    typedef Fwk::Ptr<StatsReactor const> PtrConst;

    void onSegmentNew(SegmentID segment) {
        // increment counts
    }

    void onSegmentDel(Segment::Ptr segment) {
        // decrement counts
    }

    void onLocationNew(LocationID location) {
        // increment counts
    }

    void onLocationDel(Location::Ptr location) {
        // decrement counts
    }

private:

    Stats::Ptr owner_;
};

} /* end namespace */

#endif
