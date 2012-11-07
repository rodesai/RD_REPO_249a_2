#ifndef ENGINE_H
#define ENGINE_H

#include <stdlib.h>
#include <stdint.h>

#include <string>
#include <vector>
#include <set>
#include <map>
#include <exception>
#include <iostream>

#include "fwk/Ptr.h"
#include "fwk/NamedInterface.h"

#include "Nominal.h"

namespace Shipping {

typedef std::string EntityID;

class ArgumentException : public std::exception {
    virtual const char* message() const throw() {
        return "Argument exception.";
    }
};

// Primitive Types

class Mile : public Ordinal<Mile, double> {
public:
    Mile(double num) : Ordinal<Mile, double>(num) {
        if (num < 0.0 && num != -1.0) throw ArgumentException();
    }
    Mile() : Ordinal<Mile,double>(defaultValue_){}
    static Mile defaultValue(){ return defaultValue_; }
private:
    static const double defaultValue_ = 1.0;
};


class MilePerHour : public Nominal<MilePerHour, double> {
public:
    MilePerHour(double num) : Nominal<MilePerHour, double>(num) {
        if(num < 0) throw ArgumentException();
    }
    MilePerHour() : Nominal<MilePerHour, double>(defaultValue_) {}
    static MilePerHour defaultValue(){ return defaultValue_; }
private:
    static const double defaultValue_ = 1.0;
};


class Dollar : public Ordinal<Dollar, double> {
public:
    Dollar(double num) : Ordinal<Dollar, double>(num) {
        if (num < 0 && num != -1) throw ArgumentException();
    }
    Dollar() : Ordinal<Dollar,double>(defaultValue_){}
    static Dollar defaultValue(){ return defaultValue_; }
private:
    static const double defaultValue_ = 1.0;
};


class DollarPerMile : public Nominal<DollarPerMile, double> {
public:
    DollarPerMile(double num) : Nominal<DollarPerMile, double>(num) {
        if(num < 0.0) throw ArgumentException();
    }
    DollarPerMile() : Nominal<DollarPerMile, double>(defaultValue_) {}
    static DollarPerMile defaultValue(){ return defaultValue_; }
private:
    static const double defaultValue_ = 1.0;
};


class Hour : public Ordinal<Hour, double> {
public:
    Hour(double num) : Ordinal<Hour, double>(num) {
        if (num < 0 && num != -1) throw ArgumentException();
    }
    Hour() : Ordinal<Hour,double>(defaultValue_){}
    static Hour defaultValue(){ return defaultValue_; }
private:
    static const double defaultValue_ = 1.0;
};


class Difficulty : public Nominal<Difficulty, double> {
public:
    Difficulty(double num) : Nominal<Difficulty, double>(num) {
        if (num < 1.0 || num > 5.0) throw ArgumentException();
    }
    static Difficulty defaultValue(){ return defaultValue_; }
private:
    static const double defaultValue_ = 1.0;
};


class PackageNum : public Ordinal<PackageNum, uint64_t> {
public:
    PackageNum(uint64_t num) : Ordinal<PackageNum, uint64_t>(num) {}
    PackageNum() : Ordinal<PackageNum, uint64_t>(defaultValue_) {}
    static PackageNum defaultValue(){ return defaultValue_; }
private:
    static const uint64_t defaultValue_ = 1;
};

class TransportMode : public Ordinal<TransportMode,uint8_t> {
    enum Mode{
        truck_=0,
        boat_=1,
        plane_=2
    };
public:
    static Mode truck(){ return truck_; }
    static Mode boat(){ return boat_; }
    static Mode plane(){ return plane_; }
    TransportMode(Mode m) : Ordinal<TransportMode,uint8_t>(m){}
};

// Client Types
class Segment;
class Location;
class ShippingNetwork;
class Path;
class Conn;
class Fleet;
class Stats;

// Reactors
class SegmentReactor;
class ShippingNetworkReactor;
class StatsReactor;

// Pointers
typedef Fwk::Ptr<Segment> SegmentPtr;
typedef Fwk::Ptr<Location> LocationPtr;
typedef Fwk::Ptr<ShippingNetwork> ShippingNetworkPtr;
typedef Fwk::Ptr<Path> PathPtr;
typedef Fwk::Ptr<Conn> ConnPtr;
typedef Fwk::Ptr<Fleet> FleetPtr;
typedef Fwk::Ptr<Stats> StatsPtr;
typedef Fwk::Ptr<SegmentReactor> SegmentReactorPtr;
typedef Fwk::Ptr<ShippingNetworkReactor> ShippingNetworkReactorPtr;
typedef Fwk::Ptr<StatsReactor> StatsReactorPtr;

// Const Pointers
typedef Fwk::Ptr<Segment const> SegmentPtrConst;
typedef Fwk::Ptr<Location const > LocationPtrConst;
typedef Fwk::Ptr<ShippingNetwork const> ShippingNetworkPtrConst;
typedef Fwk::Ptr<Path const> PathPtrConst;
typedef Fwk::Ptr<Conn const> ConnPtrConst;
typedef Fwk::Ptr<Fleet const> FleetPtrConst;
typedef Fwk::Ptr<Stats const> StatsPtrConst;
typedef Fwk::Ptr<SegmentReactor const> SegmentReactorPtrConst;
typedef Fwk::Ptr<ShippingNetworkReactor const> ShippingNetworkReactorPtrConst;
typedef Fwk::Ptr<StatsReactor const> StatsReactorPtrConst;

class Location : public Fwk::NamedInterface {

public:

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

    uint32_t segmentCount() const; 
    SegmentPtr segment(uint32_t index) const; 

    inline EntityType entityType() const { return entityType_; }

protected: 

    Location(EntityID name, EntityType type): Fwk::NamedInterface(name), entityType_(type){}

private:

    // Factory Class
    friend class ShippingNetwork;
    // Internal Reactor Classes 
    friend class SegmentReactor;

    // mutators
    void entityTypeIs(EntityType et);
    // note: per the instructions, segments_ is read-only

    void segmentIs(SegmentPtr segment);

    void segmentDel(SegmentPtr segment);

    EntityType entityType_;
    typedef std::vector<SegmentPtr> SegmentList;
    SegmentList segments_;
};


class Segment : public Fwk::NamedInterface {

public:

    // Class Types

    enum ExpediteSupport {
        expediteUnsupported_ = 0,
        expediteSupported_ = 1,
        expediteUnspecified_ = 2,
    };

    // Notifiee Class

    class Notifiee : public virtual Fwk::NamedInterface::Notifiee {
    public:
        // Events
        virtual void onSource(){}
        virtual void onReturnSegment(){}
        virtual void onExpediteSupport(){}
        void notifierIs(SegmentPtr notifier){ notifier_=notifier; }
        SegmentPtr notifier() const { return notifier_; }
    protected:
        Notifiee(){}
        SegmentPtr notifier_;
    };
    typedef Fwk::Ptr<Segment::Notifiee> NotifieePtr;
    typedef Fwk::Ptr<Segment::Notifiee const> NotifieePtrConst;

    // Accesors

    // constant accesors
    static inline ExpediteSupport expediteSupported() { return expediteSupported_; }
    static inline ExpediteSupport expediteUnsupported() { return expediteUnsupported_; }
    static inline ExpediteSupport expediteUnspecified() { return expediteUnspecified_; }

    inline TransportMode mode() const { return mode_; }
    inline LocationPtr source() const { return source_; }
    inline Mile length() const { return length_; }
    inline SegmentPtr returnSegment() const { return returnSegment_; }
    inline Difficulty difficulty() const { return difficulty_; }
    inline ExpediteSupport expediteSupport() const { return expediteSupport_; }

    // mutators
    void sourceIs(EntityID source);
    void lengthIs(Mile l);
    void returnSegmentIs(EntityID segment); 
    void difficultyIs(Difficulty d); 
    void expediteSupportIs(ExpediteSupport es);
    void notifieeIs(Segment::Notifiee* notifiee);

private:

    // Factory Class
    friend class ShippingNetwork;
    // Internal Reactor Classes
    friend class ShippingNetworkReactor;
    friend class SegmentReactor;

    Segment(ShippingNetworkPtrConst network, EntityID name, TransportMode mode) : Fwk::NamedInterface(name), length_(1.0), difficulty_(1.0), expediteSupport_(expediteUnsupported_), mode_(mode), network_(network){}

    void sourceIs(LocationPtr source);
    void returnSegmentIs(SegmentPtr returnSegment);

    // attributes
    Mile length_;
    Difficulty difficulty_;
    ExpediteSupport expediteSupport_;
    TransportMode mode_;
    SegmentPtr returnSegment_;
    LocationPtr source_;

    typedef std::vector<Segment::NotifieePtr> NotifieeList;
    NotifieeList notifieeList_;

    ShippingNetworkPtrConst network_;
};

class Fleet : public Fwk::NamedInterface {

public:

    MilePerHour speed(TransportMode segmentType) const; 
    PackageNum capacity(TransportMode segmentType) const; 
    DollarPerMile cost(TransportMode segmentType) const; 

    void speedIs(TransportMode m, MilePerHour s);
    void capacityIs(TransportMode m, PackageNum p);
    void costIs(TransportMode m, DollarPerMile d);
private:
 
    // Factory Class
    friend class ShippingNetwork;

    Fleet(std::string name) : NamedInterface(name){};

    typedef std::map<TransportMode,MilePerHour> SpeedMap;
    SpeedMap speed_;

    typedef std::map<TransportMode,PackageNum> CapacityMap;
    CapacityMap capacity_;

    typedef std::map<TransportMode,DollarPerMile> CostMap;
    CostMap cost_;
};

class Path : public Fwk::PtrInterface<Path>{

public:

    enum Expedited{
        expeditedPath_=0,
        unexpeditedPath_
    };
    static Expedited expeditedPath(){ return expeditedPath_; }
    static Expedited unexpeditedPath(){ return unexpeditedPath_; }

    class PathElement;
    typedef Fwk::Ptr<PathElement> PathElementPtr;
    typedef Fwk::Ptr<PathElement const> PathElementPtrConst;
    class PathElement : public Fwk::PtrInterface<Path::PathElement> {
    public:
        // accessors
        inline LocationPtr source() const { return segment_->source(); }
        inline SegmentPtr segment() const { return segment_; }
        // mutators
        void segmentIs(SegmentPtr s); 
        // Constructor
        static PathElementPtr PathElementIs(SegmentPtr segment);
    private:
        PathElement(SegmentPtr segment) : segment_(segment){}
        SegmentPtr segment_;
    };

    typedef std::vector<PathElementPtr> PathList;

    // accessors
    Dollar cost() const { return cost_; }
    Hour time() const { return time_; }
    Mile distance() const{ return distance_; }
    Expedited expedited() const { return expedited_; }
    PathElementPtr pathElement(uint32_t index) const;
    uint32_t pathElementCount() const; 
    LocationPtr lastLocation() const; ;
    LocationPtr location(LocationPtr location) const;

    // mutators

    void pathElementEnq(PathElementPtr element,Dollar cost_,Hour time_,Mile distance_);

    static PathPtr PathIs(Expedited expedited);
    
private:

    Dollar cost_;
    Hour time_;
    Mile distance_;
    Expedited expedited_;

    std::set<EntityID> locations_;
    PathList path_;

    Path(Expedited expedited);
};


class Conn : public Fwk::NamedInterface {

public:

    typedef std::set<EntityID> LocationSet;
    typedef std::vector<PathPtr> PathList;

    /* Constraint Classes */

    class Constraint;
    typedef Fwk::Ptr<Conn::Constraint> ConstraintPtr;
    typedef Fwk::Ptr<Conn::Constraint const> ConstraintPtrConst;
    class Constraint : public Fwk::PtrInterface<Conn::Constraint>{
    public:
        enum EvalOutput{
            fail_=0,
            pass_=1
        };
        static EvalOutput fail(){ return fail_; }
        static EvalOutput pass(){ return pass_; }
        void pathIs(PathPtr path){ path_=path; }
        void nextIs(ConstraintPtr next){ nxt_=next; }
        PathPtr path() const { return path_; }
        ConstraintPtr next() const { return nxt_; }
        virtual EvalOutput evalOutput()=0;
    protected:
        Constraint() : path_(NULL),nxt_(NULL){}
        PathPtr path_;
        ConstraintPtr nxt_;
    };

    PathList paths(ConstraintPtr constraints,EntityID start) const;
    PathList paths(ConstraintPtr constraints,EntityID start,EntityID end) const;

    class DistanceConstraint : public Conn::Constraint{
    public:
        EvalOutput evalOutput(){
            if( !path_ || path_->distance() > distance_ ) 
                return Constraint::fail();
            return Constraint::pass();
        }
        static ConstraintPtr DistanceConstraintIs(Mile distance){
            return new DistanceConstraint(distance);
        }
    private:
        DistanceConstraint(Mile distance) : Constraint(), distance_(distance){}
        Mile distance_;
    };

    class CostConstraint : public Constraint{
    public:
        EvalOutput evalOutput(){
            if( !path_ || path_->cost() > cost_ ) 
                return Constraint::fail();
            return Constraint::pass();
        }
        static ConstraintPtr CostConstraintIs(Dollar cost){
            return new CostConstraint(cost);
        }
    private:
        CostConstraint(Dollar cost) : Constraint(), cost_(cost){}
        Dollar cost_;
    };

    class TimeConstraint : public Constraint{
    public:
        EvalOutput evalOutput(){
            if( !path_ || path_->time() > time_ )
                return Constraint::fail();
            return Constraint::pass();
        }
        static ConstraintPtr TimeConstraintIs(Hour time){
            return new TimeConstraint(time);
        } 
    private:
        TimeConstraint(Hour time) : Constraint(), time_(time) {}
        Hour time_;
    };

    class ExpediteConstraint : public Constraint{
    public:
        EvalOutput evalOutput(){
            if( !path_ || path_->expedited() != expedited_ )
                return Constraint::fail();
            return Constraint::pass();
        }
        static ConstraintPtr ExpediteConstraintIs(Path::Expedited expedited){
            return new ExpediteConstraint(expedited);
        }
    private:
        ExpediteConstraint(Path::Expedited expedited) : Constraint(), expedited_(expedited){}
        Path::Expedited expedited_;
    };

private:

    // Graph Traversal
    PathList paths(FleetPtr fleet,ConstraintPtr constraints,LocationPtr start,LocationPtr endpoint) const ;
    // Helper Functions for paths
    bool validSegment(SegmentPtr segment) const;
    void pathElementEnque(SegmentPtr segment, PathPtr path, FleetPtr fleet) const;
    PathPtr copyPath(PathPtr path, Path::Expedited expedited, FleetPtr fleet) const;

    // Factory Class
    friend class ShippingNetwork;

    Conn(std::string name,ShippingNetworkPtrConst shippingNetwork, FleetPtr fleet) : NamedInterface(name), shippingNetwork_(shippingNetwork), fleet_(fleet){}

    ShippingNetworkPtrConst shippingNetwork_;
    FleetPtr fleet_;
};

class Stats : public Fwk::NamedInterface {

    // TODO: this needs to be updated via notification

public:

    // accessors
    uint32_t locationCount(Location::EntityType et) const; 
    uint32_t segmentCount(TransportMode et) const;
    inline double expeditePercentage() const {
        if (totalSegmentCount_ == 0)
            return 0;
        return ((double)expediteSegmentCount_ * 100.0) / ((double)totalSegmentCount_);
    }

private:

    // Factory Class
    friend class ShippingNetwork;
    // Internal Reactor Class
    friend class SegmentReactor;
    friend class StatsReactor;

    Stats(std::string name) : NamedInterface(name){
        expediteSegmentCount_=0;
        totalSegmentCount_=0;
    }

    // mutators
    void locationCountIncr(Location::EntityType type);
    void locationCountDecr(Location::EntityType type);
    void segmentCountIncr(TransportMode type);
    void segmentCountDecr(TransportMode type);
    void expediteSegmentCountIncr();
    void expediteSegmentCountDecr();
    void totalSegmentCountIncr();
    void totalSegmentCountDecr();

    typedef std::map<Location::EntityType, uint32_t> LocationCountMap;
    LocationCountMap locationCount_;

    typedef std::map<TransportMode, uint32_t> SegmentCountMap;   
    SegmentCountMap segmentCount_;

    uint32_t expediteSegmentCount_;
    uint32_t totalSegmentCount_;
};

class ShippingNetwork : public Fwk::NamedInterface {

public:

    // notifiee class
    class Notifiee : public virtual Fwk::NamedInterface::Notifiee {
    public:
        // Segments
        virtual void onSegmentNew(EntityID segmentID){}
        virtual void onSegmentDel(SegmentPtr segment){}
        // Locations
        virtual void onLocationNew(EntityID locationID){}
        virtual void onLocationDel(LocationPtr location){}
        void notifierIs(ShippingNetworkPtr notifier){ notifier_=notifier; }
        ShippingNetworkPtr notifier() const { return notifier_; }
    protected:
        Notifiee() {}
        ShippingNetworkPtr notifier_;
    };
    typedef Fwk::Ptr<ShippingNetwork::Notifiee> NotifieePtr;
    typedef Fwk::Ptr<ShippingNetwork::Notifiee const> NotifieePtrConst;

    // accessor
    SegmentPtr segment(EntityID name) const; 
    LocationPtr location(EntityID name) const;
    ConnPtrConst conn(EntityID name) const; 
    StatsPtrConst stats(EntityID name) const; 
    FleetPtr fleet(EntityID name) const;

    // mutators
    // Instance Creators
    // These instance creators create an instance, add it to the map, 
    // and set up any needed reactors;
    SegmentPtr SegmentNew(EntityID name, TransportMode mode); 
    SegmentPtr segmentDel(EntityID name);
    LocationPtr LocationNew(EntityID name, Location::EntityType entityType);
    LocationPtr locationDel(EntityID name);
    // These instance creators should only create instance on first call
    ConnPtr ConnNew(EntityID name);
    ConnPtr connDel(EntityID name);
    StatsPtr StatsNew(EntityID name);
    StatsPtr statsDel(EntityID name);
    FleetPtr FleetNew(EntityID name);
    FleetPtr fleetDel(EntityID name);
    void notifieeIs(ShippingNetwork::NotifieePtr notifiee);
    static ShippingNetworkPtr ShippingNetworkIs(EntityID name);

private:

    ShippingNetwork(EntityID name) : Fwk::NamedInterface(name){}

    // attributes
    typedef std::map<EntityID, LocationPtr> LocationMap;
    LocationMap locationMap_;

    typedef std::map<EntityID, SegmentPtr> SegmentMap;
    SegmentMap segmentMap_;

    typedef std::map<EntityID,ConnPtr> ConnMap;
    ConnMap conn_;
    ConnPtr connPtr_;

    typedef std::map<EntityID,FleetPtr> FleetMap;
    FleetMap fleet_;
    FleetPtr fleetPtr_;

    typedef std::map<EntityID,StatsPtr> StatMap;
    StatMap stat_;
    StatsPtr statPtr_;

    // notifiees
    typedef std::vector<ShippingNetwork::NotifieePtr> NotifieeList;
    NotifieeList notifieeList_;
};

class SegmentReactor : public Segment::Notifiee {

public:

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
    void onReturnSegment();  

    void onExpediteSupport();

private:

    // Factory Class
    friend class ShippingNetwork;

    SegmentReactor(ShippingNetworkPtr network,StatsPtr stats);

    LocationPtr currentSource_;
    SegmentPtr  currentReturnSegment_;

    ShippingNetworkPtr network_;
    StatsPtr stats_;
};

class ShippingNetworkReactor : public ShippingNetwork::Notifiee{

public:

    void onSegmentDel(SegmentPtr segment); 

    void onLocationDel(LocationPtr location);

    static ShippingNetworkReactorPtr ShippingNetworkReactorIs(){
        return new ShippingNetworkReactor();
    }

private:

    // Factory Class
    friend class ShippingNetwork;

    ShippingNetworkReactor();
};


class StatsReactor : public ShippingNetwork::Notifiee{

public:

    void onSegmentNew(EntityID segmentID);
    void onSegmentDel(SegmentPtr segment);
    void onLocationNew(EntityID locationID);
    void onLocationDel(LocationPtr location);

private:

    // Factory Class
    friend class ShippingNetwork;

    StatsReactor(StatsPtr stats);

    StatsPtr stats_;
};

} /* end namespace */

#endif
