#ifndef ENGINE_H
#define ENGINE_H

#include <stdlib.h>
#include <stdint.h>

#include <string>
#include <vector>
#include <set>
#include <map>
#include <exception>

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

class Mile : public Ordinal<Mile, uint64_t> {
public:
    Mile(int64_t num) : Ordinal<Mile, uint64_t>(num) {
        if (num < 0 && num != -1) throw ArgumentException();
    }
};


class MilePerHour : public Nominal<MilePerHour, uint64_t> {
public:
    enum {min_ = 0,max_ = 10000};
    MilePerHour(uint64_t num) : Nominal<MilePerHour, uint64_t>(num) {
        if(num < min_ || num > max_) throw ArgumentException();
    }
    MilePerHour() : Nominal<MilePerHour, uint64_t>(max()) {}
    static MilePerHour max(){ return max_; }
    static MilePerHour min(){ return min_; }
};


class Dollar : public Ordinal<Dollar, float> {
public:
    Dollar(float num) : Ordinal<Dollar, float>(num) {
        if (num < 0 && num != -1) throw ArgumentException();
    }
};


class DollarPerMile : public Nominal<DollarPerMile, uint64_t> {
public:
    enum {min_ = 0,max_ = 1000000};
    DollarPerMile(uint64_t num) : Nominal<DollarPerMile, uint64_t>(num) {
        if(num < min_ || num > max_) throw ArgumentException();
    }
    DollarPerMile() : Nominal<DollarPerMile, uint64_t>(max()) {}
    static DollarPerMile max(){ return max_; }
    static DollarPerMile min(){ return min_; }
};


class Hour : public Ordinal<Hour, float> {
public:
    Hour(float num) : Ordinal<Hour, float>(num) {
        if (num < 0 && num != -1) throw ArgumentException();
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
    enum {min_ = 0,max_ = 1000000};
    PackageNum(uint64_t num) : Ordinal<PackageNum, uint64_t>(num) {
        if(num < min_ || num > max_) throw ArgumentException();
    }
    PackageNum() : Ordinal<PackageNum, uint64_t>(max()) {}
    static PackageNum max(){ return max_; }
    static PackageNum min(){ return min_; }
};

// Core Types

class Segment;
class SegmentReactor;
class ShippingNetwork;
class Fleet;
class Location : public Fwk::NamedInterface {

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

    uint32_t segmentCount() const { return segments_.size(); }
    EntityID segmentID(uint32_t index) const {
        // TODO: it would be better to return a null pointer instead of an empty string
        if (index < 1 || index > segments_.size())
            return "";
        return segments_[index - 1];
    }

    inline EntityType entityType() const { return entityType_; }

protected: 

    Location(EntityID name, EntityType type): Fwk::NamedInterface(name), entityType_(type){}

private:

    // Factory Class
    friend class ShippingNetwork;
    // Needs access to private mutators to add/remove segments
    friend class SegmentReactor;

    // mutators
    void entityTypeIs(EntityType et);
    // note: per the instructions, segments_ is read-only

    void segmentIs(EntityID segmentID);

    void segmentDel(EntityID segmentID);

    EntityType entityType_;
    typedef std::vector<EntityID> SegmentList;
    SegmentList segments_;
};


class Segment : public Fwk::NamedInterface {

public:

    // Class Types

    typedef Fwk::Ptr<Segment> Ptr;
    typedef Fwk::Ptr<Segment const> PtrConst;

    enum ExpediteSupport {
        expediteUnsupported_ = 0,
        expediteSupported_ = 1,
        expediteUnspecified_ = 2,
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
        virtual void onReturnSegment(){}
        virtual void onExpediteSupport(){}

        void notifierIs(Segment::Ptr notifier){ notifier_=notifier; }
        Segment::Ptr notifier() const { return notifier_; }

    protected:
        Notifiee(){}
        Segment::Ptr notifier_;
    };

    // Accesors

    // constant accesors
    static inline ExpediteSupport expediteSupported() { return expediteSupported_; }
    static inline ExpediteSupport expediteUnsupported() { return expediteUnsupported_; }
    static inline ExpediteSupport expediteUnspecified() { return expediteUnspecified_; }
    static inline EntityType truckSegment() { return truckSegment_; }
    static inline EntityType boatSegment() { return boatSegment_; }
    static inline EntityType planeSegment() { return planeSegment_; }

    inline EntityType entityType() const { return entityType_; }
    inline Location::Ptr source() const { return source_; }
    inline Mile length() const { return length_; }
    inline Segment::Ptr returnSegment() const { return returnSegment_; }
    inline Difficulty difficulty() const { return difficulty_; }
    inline ExpediteSupport expediteSupport() const { return expediteSupport_; }

    // mutators
    void sourceIs(Location::Ptr source);
    void lengthIs(Mile l);
    void returnSegmentIs(Segment::Ptr s); 
    void difficultyIs(Difficulty d); 
    void expediteSupportIs(ExpediteSupport es);
    void notifieeIs(Segment::Notifiee* notifiee);

private:

    friend class ShippingNetwork;
    friend class SegmentReactor;

    Segment(EntityID name, EntityType type) : Fwk::NamedInterface(name), length_(0), difficulty_(1.0), expediteSupport_(expediteUnsupported_), entityType_(type){}

    void returnSegmentRm();
    void returnSegmentSet(Segment::Ptr returnSegment);

    // attributes
    Mile length_;
    Difficulty difficulty_;
    ExpediteSupport expediteSupport_;
    EntityType entityType_;
    Segment::Ptr returnSegment_;
    Location::Ptr source_;

    typedef std::vector<Segment::Notifiee::Ptr> NotifieeList;
    NotifieeList notifieeList_;
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
        inline Location::Ptr source() const { return segment_->source(); }
        inline Segment::Ptr segment() const { return segment_; }

        // mutators
        void segmentIs(Segment::Ptr s); 

    private:

        Segment::Ptr segment_;
    };


    typedef std::vector<PathElement::Ptr> LocationVector;
    typedef std::vector<PathElement::Ptr>::iterator LocationIterator;

    // accessors
    Dollar cost() const { return cost_; }
    Hour time() const { return time_; }
    Mile distance() const{ return distance_; }
    Segment::ExpediteSupport expedited() const { return expedited_; }
    LocationIterator pathIterator(); 

private:

    Dollar cost_;
    Hour time_;
    Mile distance_;
    Segment::ExpediteSupport expedited_;

    LocationVector locations_;
};


class Conn : public Fwk::NamedInterface {

public:

    typedef Fwk::Ptr<Conn> Ptr;
    typedef Fwk::Ptr<Conn const> PtrConst;

    typedef std::set<EntityID> LocationSet;
    typedef std::vector<Path::Ptr> PathList;

    std::vector<Path::Ptr> connect(Location::Ptr start, Location::Ptr end) const { return std::vector<Path::Ptr>(); }
    std::vector<Path::Ptr> explore(Location::Ptr start,
        Mile distance, Dollar cost, Hour time,
        Segment::ExpediteSupport expedited) const { return std::vector<Path::Ptr>(); }

    /* Constraint Classes */

    class Constraint : public Fwk::PtrInterface<Conn::Constraint>{
    public:
        typedef Fwk::Ptr<Conn::Constraint> Ptr;
        typedef Fwk::Ptr<Conn::Constraint const> PtrConst;
        enum EvalOutput{
            fail_=0,
            pass_=1
        };
        EvalOutput fail(){ return fail_; }
        EvalOutput pass(){ return pass_; }
        void pathIs(Path::Ptr path){ path_=path; }
        Path::Ptr path(){ return path_; }
        virtual EvalOutput evalOutput()=0;
    protected:
        Constraint(){}
        Path::Ptr path_;
    };

    typedef std::vector<Constraint::Ptr> ConstraintList;

    class DistanceConstraint : public Conn::Constraint{
    public:
        EvalOutput evalOutput(){
            if( !path_ || path_->distance() > distance_ ) 
                return Constraint::fail();
            return Constraint::pass();
        }
        Constraint::Ptr DistanceConstraintIs(Mile distance){
            return new DistanceConstraint(distance);
        }
    private:
        DistanceConstraint(Mile distance) : distance_(distance){}
        Mile distance_;
    };

    class CostConstraint : public Constraint{
    public:
        EvalOutput evalOutput(){
            if( !path_ || path_->cost() > cost_ ) 
                return Constraint::fail();
            return Constraint::pass();
        }
        Constraint::Ptr CostConstraintIs(Dollar cost){
            return new CostConstraint(cost);
        }
    private:
        CostConstraint(Dollar cost) : cost_(cost){}
        Dollar cost_;
    };

    class TimeConstraint : public Constraint{
    public:
        EvalOutput evalOutput(){
            if( !path_ || path_->time() > time_ )
                return Constraint::fail();
            return Constraint::pass();
        }
        Constraint::Ptr TimeConstraintIs(Hour time){
            return new TimeConstraint(time);
        } 
    private:
        TimeConstraint(Hour time) : time_(time){}
        Hour time_;
    };

    class ExpediteConstraint : public Constraint{
    public:
        EvalOutput evalOutput(){
            if( !path_ || path_->expedited() != expediteSupport_ )
                return Constraint::fail();
            return Constraint::pass();
        }
        Constraint::Ptr ExpediteConstraintIs(Segment::ExpediteSupport expediteSupport){
            return new ExpediteConstraint(expediteSupport);
        }
    private:
        ExpediteConstraint(Segment::ExpediteSupport expediteSupport) : expediteSupport_(expediteSupport){}
        Segment::ExpediteSupport expediteSupport_;
    };

    PathList paths(ConstraintList constraints,LocationSet endpoints,Location::Ptr start,
                    ShippingNetwork* network,Fleet* fleet);

private:
   
    friend class ShippingNetwork;

    Conn(std::string name) : NamedInterface(name){}
};


class Fleet : public Fwk::NamedInterface {

public:

    enum Mode {
        boat_ = 1,
        plane_ = 2,
        truck_ = 3
    };

    typedef Fwk::Ptr<Fleet> Ptr;
    typedef Fwk::Ptr<Fleet const> PtrConst;

    static inline Mode boat() { return boat_; }
    static inline Mode truck() { return truck_; }
    static inline Mode plane() { return plane_; }

    MilePerHour speed(Mode segmentType) { return speed_[segmentType]; }
    PackageNum capacity(Mode segmentType) { return capacity_[segmentType]; }
    DollarPerMile cost(Mode segmentType) { return cost_[segmentType]; }

    void speedIs(Mode m, MilePerHour s);
    void capacityIs(Mode m, PackageNum p);
    void costIs(Mode m, DollarPerMile d);
private:

    friend class ShippingNetwork;

    Fleet(std::string name) : NamedInterface(name){};

    typedef std::map<Mode,MilePerHour> SpeedMap;
    SpeedMap speed_;

    typedef std::map<Mode,PackageNum> CapacityMap;
    CapacityMap capacity_;
   
    typedef std::map<Mode,DollarPerMile> CostMap;
    CostMap cost_;
};


class Stats : public Fwk::NamedInterface {

    // TODO: this needs to be updated via notification

public:

    typedef Fwk::Ptr<Stats> Ptr;
    typedef Fwk::Ptr<Stats const> PtrConst;

    // accessors
    inline uint32_t locationCount(Location::EntityType et) 
        { return locationCount_[et]; }
    inline uint32_t segmentCount(Segment::EntityType et) 
        { return segmentCount_[et]; }
    inline float expeditePercentage() const {
        if (totalSegmentCount_ == 0)
            return 0;
        return expediteSegmentCount_ * 100.0 / totalSegmentCount_;
    }

private:

    friend class ShippingNetwork;
    friend class SegmentReactor;
    friend class StatsReactor;

    Stats(std::string name) : NamedInterface(name){
        expediteSegmentCount_=0;
        totalSegmentCount_=0;
    }

    // mutators
    void locationCountIncr(Location::EntityType type);
    void locationCountDecr(Location::EntityType type);
    void segmentCountIncr(Segment::EntityType type);
    void segmentCountDecr(Segment::EntityType type);
    void expediteSegmentCountIncr();
    void expediteSegmentCountDecr();
    void totalSegmentCountIncr();
    void totalSegmentCountDecr();

    typedef std::map<Location::EntityType, uint32_t> LocationCountMap;
    LocationCountMap locationCount_;

    typedef std::map<Segment::EntityType, uint32_t> SegmentCountMap;   
    SegmentCountMap segmentCount_;

    uint32_t expediteSegmentCount_;
    uint32_t totalSegmentCount_;
};

class ShippingNetwork : public Fwk::NamedInterface {

public:

    // class types

    typedef Fwk::Ptr<ShippingNetwork> Ptr;
    typedef Fwk::Ptr<ShippingNetwork const> PtrConst;

    // notifiee

    class Notifiee : public virtual Fwk::NamedInterface::Notifiee {
    public:

        typedef Fwk::Ptr<ShippingNetwork::Notifiee> Ptr;
        typedef Fwk::Ptr<ShippingNetwork::Notifiee const> PtrConst;

        // Segments
        virtual void onSegmentNew(EntityID segmentID){}
        virtual void onSegmentDel(Segment::Ptr segment){}

        // Locations
        virtual void onLocationNew(EntityID locationID){}
        virtual void onLocationDel(Location::Ptr location){}

        void notifierIs(ShippingNetwork::Ptr notifier){ notifier_=notifier; }
        ShippingNetwork::Ptr notifier() const { return notifier_; }
 
    protected:

        Notifiee() {}

        ShippingNetwork::Ptr notifier_;
    };

    // accessor

    Segment::Ptr segment(EntityID name) { return segmentMap_[name]; }
    Location::Ptr location(EntityID name) { return locationMap_[name]; }
    Conn::Ptr conn(EntityID name) { return conn_[name]; }
    Stats::Ptr stats(EntityID name) { return stat_[name]; }
    Fleet::Ptr fleet(EntityID name) { return fleet_[name]; }

    // mutators

    // Instance Creators
    // These instance creators create an instance, add it to the map, 
    // and set up any needed reactors;

    Segment::Ptr SegmentNew(EntityID name, Segment::EntityType entityType); 
    Segment::Ptr segmentDel(EntityID name);

    Location::Ptr LocationNew(EntityID name, Location::EntityType entityType);
    Location::Ptr locationDel(EntityID name);

    // These instance creators should only create instance on first call

    Conn::Ptr ConnNew(EntityID name);
    Conn::Ptr connDel(EntityID name);

    Stats::Ptr StatsNew(EntityID name);
    Stats::Ptr statsDel(EntityID name);

    Fleet::Ptr FleetNew(EntityID name);
    Fleet::Ptr fleetDel(EntityID name);

    void notifieeIs(ShippingNetwork::Notifiee::Ptr notifiee);

    static ShippingNetwork::Ptr ShippingNetworkIs(EntityID name);

private:

    ShippingNetwork(EntityID name) : Fwk::NamedInterface(name){}

    // attributes
    typedef std::map<EntityID, Location::Ptr> LocationMap;
    LocationMap locationMap_;

    typedef std::map<EntityID, Segment::Ptr> SegmentMap;
    SegmentMap segmentMap_;

    typedef std::map<EntityID,Conn::Ptr> ConnMap;
    ConnMap conn_;
    Conn::Ptr connPtr_;

    typedef std::map<EntityID,Fleet::Ptr> FleetMap;
    FleetMap fleet_;
    Fleet::Ptr fleetPtr_;

    typedef std::map<EntityID,Stats::Ptr> StatMap;
    StatMap stat_;
    Stats::Ptr statPtr_;

    // notifiees
    typedef std::vector<ShippingNetwork::Notifiee::Ptr> NotifieeList;
    NotifieeList notifieeList_;
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
    void onReturnSegment();  

    void onExpediteSupport();

private:

    friend class ShippingNetwork;

    SegmentReactor(ShippingNetwork::Ptr network,Stats::Ptr stats);

    Location::Ptr currentSource_;
    Segment::Ptr  currentReturnSegment_;

    ShippingNetwork::Ptr network_;
    Stats::Ptr stats_;
};

class ShippingNetworkReactor : public ShippingNetwork::Notifiee{

public:

    typedef Fwk::Ptr<ShippingNetworkReactor> Ptr;
    typedef Fwk::Ptr<ShippingNetworkReactor const> PtrConst;

    void onSegmentDel(Segment::Ptr segment); 

    void onLocationDel(Location::Ptr location);

    static ShippingNetworkReactor::Ptr ShippingNetworkReactorIs(){
        return new ShippingNetworkReactor();
    }

private:

    friend class ShippingNetwork;

    ShippingNetworkReactor();
};


class StatsReactor : public ShippingNetwork::Notifiee{

public:

    typedef Fwk::Ptr<StatsReactor> Ptr;
    typedef Fwk::Ptr<StatsReactor const> PtrConst;

    void onSegmentNew(EntityID segmentID);
    void onSegmentDel(Segment::Ptr segment);
    void onLocationNew(EntityID locationID);
    void onLocationDel(Location::Ptr location);

private:

    friend class ShippingNetwork;

    StatsReactor(Stats::Ptr stats);

    Stats::Ptr stats_;
};

} /* end namespace */

#endif
