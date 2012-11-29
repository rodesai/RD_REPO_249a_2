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
#include <queue>
#include <utility>

#include "fwk/Ptr.h"
#include "fwk/NamedInterface.h"

#include "Nominal.h"

#include "activity/Activity.h"

using namespace Activity;

namespace Shipping {

typedef std::string EntityID;

class ArgumentException : public Fwk::Exception {
public:
    ArgumentException() : Exception("Argument exception."){}
};

class EntityExistsException : public Fwk::Exception {
public:
    EntityExistsException() : Exception("Entity exists.") {}
};

class Multiplier : public Ordinal<Multiplier, double> {
public:
    Multiplier(double num) : Ordinal<Multiplier, double>(num) {
        if (num < 0.0) throw ArgumentException();
    }
    Multiplier() : Ordinal<Multiplier,double>(defaultValue_){}
    static Multiplier defaultValue(){ return defaultValue_; }
private:
    static const double defaultValue_ = 1.0;
};

class Mile : public Ordinal<Mile, double> {
public:
    Mile(double num) : Ordinal<Mile, double>(num) {
        if (num < 0.0) throw ArgumentException();
    }
    Mile() : Ordinal<Mile,double>(defaultValue_){}
    std::string str() {
        std::stringstream s;
        s.precision(2);
        s << std::fixed << value_;
        return s.str();
    }
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
    std::string str() {
        std::stringstream s;
        s.precision(2);
        s << std::fixed << value_;
        return s.str();
    }
    static MilePerHour defaultValue(){ return defaultValue_; }
private:
    static const double defaultValue_ = 1.0;
};

class Dollar : public Ordinal<Dollar, double> {
public:
    Dollar(double num) : Ordinal<Dollar, double>(num) {
        if (num < 0) throw ArgumentException();
    }
    Dollar() : Ordinal<Dollar,double>(defaultValue_){}
    std::string str() {
        std::stringstream s;
        s.precision(2);
        s << std::fixed << value_;
        return s.str();
    }
    static Dollar defaultValue(){ return defaultValue_; }
    Dollar operator+(const Dollar other) const {
        return Dollar(value_ + other.value());
    }
private:
    static const double defaultValue_ = 1.0;
};

class DollarPerMile : public Nominal<DollarPerMile, double> {
public:
    DollarPerMile(double num) : Nominal<DollarPerMile, double>(num) {
        if(num < 0.0) throw ArgumentException();
    }
    DollarPerMile() : Nominal<DollarPerMile, double>(defaultValue_) {}
    std::string str() {
        std::stringstream s;
        s.precision(2);
        s << std::fixed << value_;
        return s.str();
    }
    static DollarPerMile defaultValue(){ return defaultValue_; }
private:
    static const double defaultValue_ = 1.0;
};

class Hour : public Ordinal<Hour, double> {
public:
    Hour(double num) : Ordinal<Hour, double>(num) {
        if (num < 0) throw ArgumentException();
    }
    Hour() : Ordinal<Hour,double>(defaultValue_){}
    std::string str() {
        std::stringstream s;
        s.precision(2);
        s << std::fixed << value_;
        return s.str();
    }
    static Hour defaultValue(){ return defaultValue_; }
private:
    static const double defaultValue_ = 1.0;
};

class HourOfDay : public Ordinal<HourOfDay, double> {
public:
    HourOfDay(double num) : Ordinal<HourOfDay, double>(num) {
        if (num < 0 || num >= 24) throw ArgumentException();
    }
    HourOfDay() : Ordinal<HourOfDay,double>(defaultValue_){}
    std::string str() {
        std::stringstream s;
        s.precision(2);
        s << std::fixed << value_;
        return s.str();
    }
    static HourOfDay defaultValue(){ return defaultValue_; }
private:
    static const double defaultValue_ = 1.0;
};

class Difficulty : public Nominal<Difficulty, double> {
public:
    Difficulty(double num) : Nominal<Difficulty, double>(num) {
        if (num < 1.0 || num > 5.0) throw ArgumentException();
    }
    std::string str() {
        std::stringstream s;
        s.precision(2);
        s << std::fixed << value_;
        return s.str();
    }
    static Difficulty defaultValue(){ return defaultValue_; }
private:
    static const double defaultValue_ = 1.0;
};

class PackageNum : public Ordinal<PackageNum, int64_t> {
public:
    PackageNum(int64_t num) : Ordinal<PackageNum, int64_t>(num){
        if (num < 0) throw ArgumentException();
    }
    PackageNum() : Ordinal<PackageNum, int64_t>(defaultValue_) {}
    std::string str() {
        std::stringstream s;
        s << value_;
        return s.str();
    }
    static PackageNum defaultValue(){ return defaultValue_; }
    PackageNum operator+(const PackageNum other) const {
        return PackageNum(value_ + other.value());
    }
    PackageNum operator-(const PackageNum other) const {
        return PackageNum(value_ - other.value());
    }
private:
    static const int64_t defaultValue_ = 1;
};

class ShipmentNum : public Ordinal<ShipmentNum, int64_t> {
public:
    ShipmentNum(int64_t num) : Ordinal<ShipmentNum, int64_t>(num) {
        if (num < 0) throw ArgumentException();
    }
    ShipmentNum() : Ordinal<ShipmentNum, int64_t>(defaultValue_) {}
    std::string str() {
        std::stringstream s;
        s << value_;
        return s.str();
    }
    ShipmentNum operator++(int) {
        return ++value_;
    }
    ShipmentNum operator--(int) {
        return --value_;
    }
    static ShipmentNum defaultValue(){ return defaultValue_; }
private:
    static const int64_t defaultValue_ = 10;
};

class PathElementNum : public Ordinal<PathElementNum, int64_t> {
public:
    PathElementNum(int64_t num) : Ordinal<PathElementNum, int64_t>(num) {
        if (num < 0) throw ArgumentException();
    }
    PathElementNum() : Ordinal<PathElementNum, int64_t>(defaultValue_) {}
    std::string str() {
        std::stringstream s;
        s << value_;
        return s.str();
    }
    static PathElementNum defaultValue(){ return defaultValue_; }
private:
    static const int64_t defaultValue_ = 10;
};

class SubshipmentNum : public Ordinal<SubshipmentNum, int64_t> {
public:
    SubshipmentNum(int64_t num) : Ordinal<SubshipmentNum, int64_t>(num) {
        if (num < 0) throw ArgumentException();
    }
    SubshipmentNum() : Ordinal<SubshipmentNum, int64_t>(defaultValue_) {}
    std::string str() {
        std::stringstream s;
        s << value_;
        return s.str();
    }
    static SubshipmentNum defaultValue(){ return defaultValue_; }
private:
    static const int64_t defaultValue_ = 10;
};

class CarrierNum : public Ordinal<CarrierNum, int64_t> {
public:
    CarrierNum(int64_t num) : Ordinal<CarrierNum, int64_t>(num) {
        if (num < 0) throw ArgumentException();
    }
    CarrierNum() : Ordinal<CarrierNum, int64_t>(defaultValue_) {}
    std::string str() {
        std::stringstream s;
        s << value_;
        return s.str();
    }
    CarrierNum operator++(int) {
        return ++value_;
    }
    CarrierNum operator--(int) {
        return --value_;
    }
    static CarrierNum defaultValue(){ return defaultValue_; }
private:
    static const int64_t defaultValue_ = 10;
};

class LocationNum : public Ordinal<LocationNum, int64_t> {
public:
    LocationNum(int64_t num) : Ordinal<LocationNum, int64_t>(num) {
        if (num < 0) throw ArgumentException();
    }
    LocationNum() : Ordinal<LocationNum, int64_t>(defaultValue_) {}
    std::string str() {
        std::stringstream s;
        s << value_;
        return s.str();
    }
    static LocationNum defaultValue(){ return defaultValue_; }
private:
    static const int64_t defaultValue_ = 10;
};

class SegmentNum : public Ordinal<SegmentNum, int64_t> {
public:
    SegmentNum(int64_t num) : Ordinal<SegmentNum, int64_t>(num) {
        if (num < 0) throw ArgumentException();
    }
    SegmentNum() : Ordinal<SegmentNum, int64_t>(defaultValue_) {}
    std::string str() {
        std::stringstream s;
        s << value_;
        return s.str();
    }
    static SegmentNum defaultValue(){ return defaultValue_; }
private:
    static const int64_t defaultValue_ = 10;
};


class ShipmentPerDay : public Ordinal<ShipmentPerDay, int64_t> {
public:
    ShipmentPerDay(int64_t num) : Ordinal<ShipmentPerDay, int64_t>(num) {
        if (num < 0) throw ArgumentException();
    }
    ShipmentPerDay() : Ordinal<ShipmentPerDay, int64_t>(defaultValue_) {}
    std::string str() {
        std::stringstream s;
        s << value_;
        return s.str();
    }
    static ShipmentPerDay defaultValue(){ return defaultValue_; }
private:
    static const int64_t defaultValue_ = 0;
};

class TransportMode : public Ordinal<TransportMode,uint8_t> {
    enum Mode{
        truck_=0,
        boat_=1,
        plane_=2,
        undef_=255
    };
public:
    static TransportMode truck(){ return truck_; }
    static TransportMode boat(){ return boat_; }
    static TransportMode plane(){ return plane_; }
    static TransportMode undef(){ return undef_; }
    TransportMode(uint8_t m) : Ordinal<TransportMode,uint8_t>(m){}
};

class PathMode : public Ordinal<PathMode,uint8_t> {
    enum Mode{
        expedited_=0,
        unexpedited_=1,
        undef_=255
    };
public:
    static PathMode expedited(){ return expedited_; }
    static PathMode unexpedited(){ return unexpedited_; }
    static PathMode undef(){ return undef_; }
    PathMode(uint8_t m) : Ordinal<PathMode,uint8_t>(m){}
};

class ModeCount : public Ordinal<ModeCount,uint8_t>{
public:
    ModeCount(uint8_t m) : Ordinal<ModeCount,uint8_t>(m){}
};

// Client Types
class Shipment;
class Subshipment;
class Segment;
class Location;
class Customer;
class ShippingNetwork;
class Path;
class Conn;
class Fleet;
class Stats;

// Reactors
class LocationReactor;
class CustomerReactor;
class InjectActivityReactor;
class ForwardActivityReactor;
class SegmentReactor;
class ShippingNetworkReactor;
class StatsReactor;

// Pointers
typedef Fwk::Ptr<Shipment> ShipmentPtr;
typedef Fwk::Ptr<Subshipment> SubshipmentPtr;
typedef Fwk::Ptr<Segment> SegmentPtr;
typedef Fwk::Ptr<Location> LocationPtr;
typedef Fwk::Ptr<Customer> CustomerPtr;
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
    enum EntityType{
        customer_ = 0,
        port_ = 1,
        truckTerminal_ = 2,
        boatTerminal_ = 3,
        planeTerminal_ = 4
    };
    static EntityType customer(){ return customer_; }
    static EntityType port(){ return port_; }
    static EntityType truckTerminal(){ return truckTerminal_; }
    static EntityType boatTerminal(){ return boatTerminal_; }
    static EntityType planeTerminal(){ return planeTerminal_; }

    virtual void shipmentIs(ShipmentPtr shipment);
    SegmentNum segmentCount() const; 
    SegmentPtr segment(uint32_t index) const; 
    inline EntityType entityType() const { return entityType_; }

    class NotifieeConst : public virtual Fwk::NamedInterface::NotifieeConst {
    public:
        virtual void onShipment(ShipmentPtr shipment){}
        LocationPtrConst notifier() const { return notifier_; }
        void notifierIs(LocationPtrConst notifier) { notifier_ = notifier; }
    protected:
        NotifieeConst(){}
        virtual ~NotifieeConst(){}
        LocationPtrConst notifier_;
    };
    class Notifiee : public virtual NotifieeConst, public virtual Fwk::NamedInterface::Notifiee {
    public:
        LocationPtr notifier() const { return const_cast<Location*>(NotifieeConst::notifier().ptr()); }
    protected:
        Notifiee(){}
        virtual ~Notifiee(){}
    };
    typedef Fwk::Ptr<Location::Notifiee> NotifieePtr;
    typedef Fwk::Ptr<Location::Notifiee const> NotifieePtrConst;
    void notifieeIs(Location::Notifiee* notifiee);
protected: 
    Location(EntityID name, EntityType type);
private:
    friend class ShippingNetwork;
    friend class SegmentReactor;
    void entityTypeIs(EntityType et);
    void segmentIs(SegmentPtr segment);
    void segmentDel(SegmentPtr segment);
    EntityType entityType_;
    typedef std::vector<SegmentPtr> SegmentList;
    SegmentList segments_;

    typedef std::vector<Location::NotifieePtr> NotifieeList;
    NotifieeList notifieeList_;
};

class LocationReactor : public Location::Notifiee {
public:
    void onShipment(ShipmentPtr shipment);
private:
    friend class Location;
    friend class ShippingNetwork;
    ShippingNetworkPtrConst network_;
    void forwardShipmentToSegment(ShipmentPtr shipment);
};

class Customer : public Location{
public:
    // mutators
    void transferRateIs(ShipmentPerDay spd);
    void shipmentSizeIs(PackageNum pn);
    void destinationIs(LocationPtr lp);
    void shipmentIs(ShipmentPtr shipment);

    // accessors
    ShipmentPerDay transferRate() const { return transferRate_; }
    Time nextShipmentTime() const;
    PackageNum shipmentSize() const { return shipmentSize_; }
    LocationPtr destination() const { return destination_; }

    ShipmentNum shipmentsReceived() const { return shipmentsReceived_; }
    Hour totalLatency() const { return totalLatency_; }
    Dollar totalCost() const { return totalCost_; }

    // reactor
    class NotifieeConst : public virtual Fwk::NamedInterface::NotifieeConst {
    public:
        virtual void onTransferRate(){}
        virtual void onShipmentSize(){}
        virtual void onDestination(){}
        virtual void onShipment(ShipmentPtr shipment) {}
        LocationPtrConst notifier() const { return notifier_; }
        void notifierIs(LocationPtrConst notifier) { notifier_ = notifier; }
    protected:
        NotifieeConst(){}
        virtual ~NotifieeConst(){}
        LocationPtrConst notifier_;
    };
    class Notifiee : public virtual NotifieeConst, public virtual Fwk::NamedInterface::Notifiee {
    public:
        LocationPtr notifier() const { return const_cast<Location*>(NotifieeConst::notifier().ptr()); }
    protected:
        Notifiee(){}
        virtual ~Notifiee(){}
    };
    typedef Fwk::Ptr<Customer::Notifiee> NotifieePtr;
    typedef Fwk::Ptr<Customer::Notifiee const> NotifieePtrConst;
    void notifieeIs(Customer::Notifiee* notifiee);
protected:
    Customer(EntityID name, EntityType type);
private:
    friend class ShippingNetwork;
    friend class CustomerReactor;
    friend class InjectActivityReactor;
    ManagerPtr manager() const { return manager_; }
    ShipmentPerDay transferRate_;
    PackageNum shipmentSize_;
    LocationPtr destination_;
    ShipmentNum shipmentsSentToday_;
    ShipmentNum shipmentsReceived_;
    Hour totalLatency_;
    Dollar totalCost_;
    ManagerPtr manager_;

    typedef std::vector<Customer::NotifieePtr> NotifieeList;
    NotifieeList notifieeList_;
};


class CustomerReactor : public Customer::Notifiee{
public:
    void onTransferRate();
    void onShipmentSize();
    void onDestination();
    void onShipment(ShipmentPtr shipment);
    CustomerReactor() {
        transferRateSet_ = false;
        shipmentSizeSet_ = false;
        destinationSet_ = false;
    }
private:
    friend class ShippingNetwork;
    ShippingNetworkPtrConst network_;
    ManagerPtr manager_;
    void checkAndCreateInjectActivity();

    bool transferRateSet_;
    bool shipmentSizeSet_;
    bool destinationSet_;
};

class Shipment: public Fwk::NamedInterface {
public:
    // accessors
    inline PackageNum load() const { return load_; }
    inline LocationPtr destination() const { return destination_; }
    inline LocationPtr source() const { return source_; }
    inline Dollar cost() const { return cost_; }
    inline Activity::Time startTime() const { return startTime_; }
    inline Activity::Time queueTime() const { return queueTime_; }

    // mutators
    void loadIs(PackageNum load) { load_ = load; }
    void destinationIs(LocationPtr loc) { destination_ = loc; }
    void sourceIs(LocationPtr src) { source_ = src;}
    void startTimeIs(Activity::Time t) { startTime_ = t; }
    void costInc(Dollar cost) {cost_ = cost_ + cost;}
    void queueTimeIs(Activity::Time t) { queueTime_ = t; }

    // constructor
    Shipment(std::string name) : NamedInterface(name), cost_(0), startTime_(0), queueTime_(0) {}
private:
    PackageNum load_;
    LocationPtr destination_;
    LocationPtr source_;
    Dollar cost_;
    Activity::Time startTime_;
    Activity::Time queueTime_;
};

class InjectActivityReactor : public Activity::Activity::Notifiee {
public:
    void onStatus();
    void onNextTime(){};

    void sourceIs(CustomerPtr customer) { source_ = customer; } 
    void managerIs(ManagerPtr m) { manager_ = m; }

    inline CustomerPtr source() const { return source_; }
private:
    ManagerPtr manager_;
    CustomerPtr source_;
};

class ForwardActivityReactor : public Activity::Activity::Notifiee {
public:
    void onStatus();
    void onNextTime(){};

    inline ManagerPtr manager() { return manager_; }
    inline SegmentPtr segment() { return segment_; }
    inline SubshipmentPtr subshipment(uint32_t i) { return subshipments_[i]; }

    void managerIs(ManagerPtr m) { manager_ = m; }
    void segmentIs(SegmentPtr s) { segment_ = s; }
    void subshipmentIs(SubshipmentPtr s) { subshipments_.push_back(s); }
    ForwardActivityReactor(){};
private:
    SegmentPtr segment_;
    vector<SubshipmentPtr> subshipments_;
    ManagerPtr manager_;
};

class DeliveryActivityReactor : public Activity::Activity::Notifiee {
public:
    void onStatus(){
        if(notifier()->status() == Activity::Activity::executing()){
            location_->shipmentIs(shipment_);
        }
    }
    DeliveryActivityReactor(ShipmentPtr shipment, LocationPtr location): location_(location), shipment_(shipment){};
private:
    LocationPtr location_; 
    ShipmentPtr shipment_;
};

class Subshipment : public Fwk::NamedInterface {
public:
    enum ShipmentOrder {
        last_ = 0,
        first_ = 1,
        other_
    };

    Subshipment(string name) : Fwk::NamedInterface(name) {};

    static inline ShipmentOrder last() { return last_; }
    static inline ShipmentOrder other() { return other_; }
    static inline ShipmentOrder first() { return first_; }

    inline ShipmentPtr shipment() const { return shipment_; }
    inline ShipmentOrder shipmentOrder() const { return shipmentOrder_; }
    inline PackageNum remainingLoad() const { return remainingLoad_; }

    void shipmentIs(ShipmentPtr shipment) { shipment_ = shipment; }
    void shipmentOrderIs(ShipmentOrder so) { shipmentOrder_ = so; }
    void remainingLoadIs(PackageNum pn) { remainingLoad_ = pn; }
private:
    ShipmentPtr shipment_;
    ShipmentOrder shipmentOrder_;
    PackageNum remainingLoad_;
};

class Segment : public Fwk::NamedInterface {
public:

    class NotifieeConst : public virtual Fwk::NamedInterface::NotifieeConst {
    public:
        // Events
        virtual void onSource(){}
        virtual void onReturnSegment(){}
        virtual void onMode(PathMode mode){}
        virtual void onModeDel(PathMode mode){}
        virtual void onShipment(ShipmentPtr shipment){}
        virtual void onCapacity(){}
        SegmentPtrConst notifier() const { return notifier_; }
        void notifierIs(SegmentPtrConst notifier){ notifier_=notifier; }
    protected:
        NotifieeConst(){}
        virtual ~NotifieeConst(){}
        SegmentPtrConst notifier_;
    };
    typedef Fwk::Ptr<Segment::NotifieeConst> NotifieeConstPtr;
    typedef Fwk::Ptr<Segment::NotifieeConst const> NotifieeConstPtrConst;
    class Notifiee : public virtual NotifieeConst, public virtual Fwk::NamedInterface::Notifiee {
    public:
        SegmentPtr notifier() const { return const_cast<Segment*>(NotifieeConst::notifier().ptr()); }
    protected:
        Notifiee(){}
        virtual ~Notifiee(){}
    };
    typedef Fwk::Ptr<Segment::Notifiee> NotifieePtr;
    typedef Fwk::Ptr<Segment::Notifiee const> NotifieePtrConst;

    inline LocationPtr source() const { return source_; }
    inline Mile length() const { return length_; }
    inline ShipmentNum capacity() const { return capacity_; }
    inline ShipmentNum shipmentsRouted() const { return shipmentsRouted_; }
    inline ShipmentNum shipmentsReceived() const { return shipmentsReceived_; }
    inline ShipmentNum shipmentsRefused() const { return shipmentsRefused_; }
    inline SegmentPtr returnSegment() const { return returnSegment_; }
    inline Difficulty difficulty() const { return difficulty_; }
    inline TransportMode transportMode() const { return transportMode_; }
    inline CarrierNum carriersUsed() const { return carriersUsed_; }
    Hour carrierLatency() const;
    PackageNum carrierCapacity() const;
    Dollar carrierCost() const;
    PathMode mode(PathMode mode) const;
    ModeCount modeCount() const;
    PathMode mode(uint16_t) const;
    SubshipmentNum subshipmentQueueSize() const { return subshipmentQueue_.size(); }
    Activity::Time totalQueueTime(){ return totalQueueTime_; }
    Activity::Time queueTime(){ return queueTime_; }

    void shipmentIs(ShipmentPtr shipment);
    void shipmentsReceivedInc() { shipmentsReceived_++; }
    void shipmentsRefusedInc() { shipmentsRefused_++; }
    void shipmentsRoutedInc(){ shipmentsRouted_++; }
    void carriersUsedInc() { carriersUsed_ ++; }
    void carriersUsedDec() { carriersUsed_ --; }
    void sourceIs(EntityID source);
    void lengthIs(Mile l);
    void capacityIs(ShipmentNum sn);
    void returnSegmentIs(EntityID segment); 
    void difficultyIs(Difficulty d); 
    void notifieeIs(Segment::Notifiee* notifiee);
    void transportModeIs(TransportMode transportMode);
    void modeIs(PathMode mode);
    void totalQueueTimeIs(Activity::Time t){ totalQueueTime_=t; }
    void queueTimeIs(Activity::Time t){
        queueTime_=t;
    }
    PathMode modeDel(PathMode mode);
    void subshipmentEnqueue(SubshipmentPtr sp) { subshipmentQueue_.push(sp); }
    SubshipmentPtr subshipmentDequeue(PackageNum);
private:
    friend class ShippingNetwork;
    friend class ShippingNetworkReactor;
    friend class SegmentReactor;
    friend class ForwardActivityReactor;
    typedef map<string,uint32_t> DeliveryMap;
    DeliveryMap deliveryMap_;

    Segment(ShippingNetworkPtrConst network, EntityID name, TransportMode transportMode, PathMode mode) : 
        Fwk::NamedInterface(name), length_(1.0), difficulty_(1.0), transportMode_(transportMode), network_(network), totalQueueTime_(0), shipmentsRouted_(0), queueTime_(-1.0){
        mode_.insert(mode);
        shipmentsReceived_ = 0;
        shipmentsRefused_ = 0;
        carriersUsed_ = 0;
    }
    void sourceIs(LocationPtr source);
    void returnSegmentIs(SegmentPtr returnSegment);
    // attributes
    Mile length_;
    Difficulty difficulty_;
    TransportMode transportMode_;
    ShipmentNum capacity_;
    std::set<PathMode> mode_;
    SegmentPtr returnSegment_;
    LocationPtr source_;
    typedef std::vector<Segment::NotifieePtr> NotifieeList;
    NotifieeList notifieeList_;
    ShippingNetworkPtrConst network_;
    Activity::Time totalQueueTime_;
    ShipmentNum shipmentsRouted_;
    Activity::Time queueTime_;

    // for activity forwarding
    CarrierNum carriersUsed_;
    ShipmentNum shipmentsReceived_;
    ShipmentNum shipmentsRefused_;
    typedef queue<SubshipmentPtr> SubshipmentQueue;
    SubshipmentQueue subshipmentQueue_;
};

class Fleet : public Fwk::NamedInterface {
public:
    MilePerHour speed(TransportMode segmentType) const; 
    PackageNum capacity(TransportMode segmentType) const; 
    DollarPerMile cost(TransportMode segmentType) const; 
    Multiplier speedMultiplier(PathMode pathMode) const;
    Multiplier costMultiplier(PathMode pathMode) const;
    HourOfDay startTime() const { return startTime_; }
    void speedIs(TransportMode m, MilePerHour s);
    void capacityIs(TransportMode m, PackageNum p);
    void costIs(TransportMode m, DollarPerMile d);
    void speedMultiplierIs(PathMode pathMode, Multiplier m);
    void costMultiplierIs(PathMode pathMode, Multiplier m);
    void startTimeIs(HourOfDay t);

    class NotifieeConst : public virtual Fwk::NamedInterface::NotifieeConst {
    public:
        virtual void onStartTime(){}
        FleetPtrConst notifier() const { return notifier_; }
        void notifierIs(FleetPtrConst notifier){ notifier_=notifier; }
    protected:
        NotifieeConst(){}
        virtual ~NotifieeConst(){}
        FleetPtrConst notifier_;
    };
    typedef Fwk::Ptr<Fleet::NotifieeConst> NotifieeConstPtr;
    typedef Fwk::Ptr<Fleet::NotifieeConst const> NotifieeConstPtrConst;
    class Notifiee : public virtual NotifieeConst, public virtual Fwk::NamedInterface::Notifiee {
    public:
        FleetPtr notifier() const { return const_cast<Fleet*>(NotifieeConst::notifier().ptr()); }
    protected:
        Notifiee(){}
        virtual ~Notifiee(){}
    };
    typedef Fwk::Ptr<Fleet::Notifiee> NotifieePtr;
    typedef Fwk::Ptr<Fleet::Notifiee const> NotifieePtrConst;
    void notifieeIs(Fleet::Notifiee* notifiee);
private:
    friend class ShippingNetwork;
    Fleet(std::string name) : NamedInterface(name), startTime_(0), startTimeSet_(false){};
    typedef std::map<TransportMode,MilePerHour> SpeedMap;
    SpeedMap speed_;
    typedef std::map<TransportMode,PackageNum> CapacityMap;
    CapacityMap capacity_;
    typedef std::map<TransportMode,DollarPerMile> CostMap;
    CostMap cost_;
    typedef std::map<PathMode,Multiplier> SpeedMultiplierMap;
    SpeedMultiplierMap speedMultiplier_;
    typedef std::map<PathMode,Multiplier> CostMultiplierMap;
    CostMultiplierMap costMultiplier_;
    HourOfDay startTime_;
    bool startTimeSet_;
    typedef std::vector<Fleet::NotifieePtr> NotifieeList;
    NotifieeList notifieeList_;
};

class FleetReactor : public Fleet::Notifiee {
public:
    void onStartTime();
    void managerIs(ManagerPtr m) { manager_ = m; }
    void networkIs(ShippingNetworkPtr s) { network_ = s; }
    inline ManagerPtr manager() { return manager_; }
    inline ShippingNetworkPtr network() { return network_; }
private:
    ShippingNetworkPtr network_;
    ManagerPtr manager_;
};

class FleetChangeActivityReactor : public Activity::Activity::Notifiee {
public:
    void onStatus();
    void onNextTime(){};
    void fleetIs(FleetPtr f) { fleet_ = f; }
    void managerIs(ManagerPtr m) { manager_ = m; }
    void networkIs(ShippingNetworkPtr s) { network_ = s; }
    inline FleetPtr fleet() { return fleet_; }
    inline ManagerPtr manager() { return manager_; }
    inline ShippingNetworkPtr network() { return network_; }
private:
    FleetPtr fleet_;
    ManagerPtr manager_;
    ShippingNetworkPtr network_;
};

class Path : public Fwk::PtrInterface<Path>{
public:
    class PathElement;
    typedef Fwk::Ptr<PathElement> PathElementPtr;
    typedef Fwk::Ptr<PathElement const> PathElementPtrConst;
    class PathElement : public Fwk::PtrInterface<Path::PathElement> {
    public:
        inline LocationPtr source() const { return segment_->source(); }
        LocationPtr dest() const { return segment_->returnSegment()->source(); }
        inline SegmentPtr segment() const { return segment_; }
        inline PathMode elementMode() const { return elementMode_; }
        void segmentIs(SegmentPtr s); 
        static PathElementPtr PathElementIs(SegmentPtr segment, PathMode elementMode);
    private:
        PathElement(SegmentPtr segment, PathMode elementMode) : segment_(segment), elementMode_(elementMode){}
        SegmentPtr segment_;
        PathMode elementMode_;
    };
    typedef std::vector<PathElementPtr> PathList;
    // accessors
    Dollar cost() const { return cost_; }
    Hour time() const { return time_; }
    Mile distance() const{ return distance_; }
    LocationPtr firstLocation() const { return firstLocation_; }
    LocationPtr lastLocation() const { return lastLocation_; }
    PathElementPtr pathElement(uint32_t index) const;
    PathElementNum pathElementCount() const; 
    LocationPtr location(LocationPtr location) const;
    // mutators
    void pathElementEnq(PathElementPtr element,Dollar cost_,Hour time_,Mile distance_);
    static PathPtr PathIs(LocationPtr firstLocation);
private:
    Dollar cost_;
    Hour time_;
    Mile distance_;
    LocationPtr firstLocation_;
    LocationPtr lastLocation_;
    std::set<EntityID> locations_;
    PathList path_;
    Path(LocationPtr firstLocation);
};

class Conn : public Fwk::NamedInterface {
public:

    class NotifieeConst : public virtual Fwk::NamedInterface::NotifieeConst {
    public:
        virtual void onRouting() {}
        void notifierIs(ConnPtr notifier){ notifier_=notifier; }
        ConnPtrConst notifier() const { return notifier_; }
    protected:
        NotifieeConst() {}
        virtual ~NotifieeConst(){}
        ConnPtrConst notifier_;
    };
    typedef Fwk::Ptr<Conn::NotifieeConst> NotifieeConstPtr;
    typedef Fwk::Ptr<Conn::NotifieeConst const> NotifieeConstPtrConst;
    class Notifiee : public virtual NotifieeConst, public virtual Fwk::NamedInterface::Notifiee {
    public:
        ConnPtr notifier() const { return const_cast<Conn*>(NotifieeConst::notifier().ptr()); }
    protected:
        Notifiee() {}
        virtual ~Notifiee(){}
    };
    typedef Fwk::Ptr<Conn::Notifiee> NotifieePtr;
    typedef Fwk::Ptr<Conn::Notifiee const> NotifieePtrConst;

    typedef std::vector<PathPtr> PathList;
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
        void nextIs(Conn::ConstraintPtr next){ nxt_=next; }
        PathPtr path() const { return path_; }
        Conn::ConstraintPtr next() const { return nxt_; }
        virtual EvalOutput evalOutput()=0;
    protected:
        Constraint() : path_(NULL),nxt_(NULL){}
        virtual ~Constraint(){}
        PathPtr path_;
        ConstraintPtr nxt_;
    };

    class PathSelector;
    typedef Fwk::Ptr<PathSelector> PathSelectorPtr;
    typedef Fwk::Ptr<PathSelector const> PathSelectorPtrConst;
    class PathSelector : public Fwk::PtrInterface<PathSelector>{
    public:
        enum Type{
            explore_=0,
            connect_=1,
            spantree_=2
        };
        static Type explore(){ return explore_; }
        static Type connect(){ return connect_; }
        static Type spantree(){ return spantree_; }
        // Mutators
        void modeIs(PathMode mode);
        PathMode modeDel(PathMode mode);
        static PathSelectorPtr PathSelectorIs(Type type, ConstraintPtr constraints, LocationPtr start, LocationPtr end);
    private:
        friend class Conn;
        PathSelector(Type type, ConstraintPtr constraints, LocationPtr start, LocationPtr end)
            : type_(type), start_(start), end_(end), constraints_(constraints){}
        inline Type type() const { return type_; }
        inline LocationPtr start() const { return start_; }
        inline LocationPtr end() const { return end_; }
        inline ConstraintPtr constraints() const { return constraints_; }
        inline std::set<PathMode> modes(){ return pathModes_; }
        Type type_;
        LocationPtr start_;
        LocationPtr end_;
        ConstraintPtr constraints_;
        std::set<PathMode> pathModes_;
    };

    enum RoutingAlgorithm{
        minDistance_,
        minHops_,
        minTime_,
        none_
    };
    static RoutingAlgorithm minDistance(){ return minDistance_; }
    static RoutingAlgorithm minHops(){ return minHops_; }
    static RoutingAlgorithm minTime(){ return minTime_; }
    static RoutingAlgorithm none(){ return none_; }

    // Accessors
    PathList paths(PathSelectorPtr selector) const;
    EntityID nextHop(EntityID startLocation,EntityID targetLocation) const;
    RoutingAlgorithm routing() const { return routingAlgorithm_; }

    // Mutators
    void routingIs(RoutingAlgorithm routingAlgorithm);
    void notifieeIs(Conn::NotifieePtr notifiee);
    void endLocationTypeIs(Location::EntityType type);
    void supportedRouteModeIs(uint32_t index, PathMode mode);

    // Instantiating Mutators for Constraints
    static ConstraintPtr DistanceConstraintIs(Mile distance);
    static ConstraintPtr TimeConstraintIs(Hour time);
    static ConstraintPtr CostConstraintIs(Dollar cost);

private:

    friend class RoutingReactor;
    friend class ShippingNetwork;

    class TraversalCompare : public binary_function<PathPtr,PathPtr,bool>{
    public:
        TraversalCompare(ConnPtrConst conn) : conn_(conn){}
        bool operator()(PathPtr a, PathPtr b) const {
            return conn_->traversalOrder()->compare(a,b);
        }
    private:
        ConnPtrConst conn_;
    };
    class TraversalOrder{
    public:
        TraversalOrder(){}
        virtual bool compare(PathPtr a, PathPtr b) const = 0;
    };
    class MinHopTraversal : public TraversalOrder{
    public:
        MinHopTraversal(){}
        virtual bool compare(PathPtr a, PathPtr b) const {
            return (a->pathElementCount() > b->pathElementCount());
        }
    };
    class MinDistanceTraversal : public TraversalOrder{
    public:
        MinDistanceTraversal(){}
        virtual bool compare(PathPtr a, PathPtr b) const {
            return (a->distance() > b->distance());
        }
    };
    class MinTimeTraversal : public TraversalOrder{
    public:
        MinTimeTraversal(FleetPtr fleet) : fleet_(fleet) {}
        virtual bool compare(PathPtr a, PathPtr b) const {
            double wA = weight(a);
            double wB = weight(b);
            if(wA == wB){
                return (rand()%10)>4;
            }
            return wA>wB;
        }
    private:
        double weight(PathPtr p) const {
            double retval = 0;
            for(uint32_t i = 0; i < p->pathElementCount().value(); i++){
                SegmentPtr s = p->pathElement(i)->segment();
                if(s->shipmentsReceived() > 0){
                    double randomFactor = ((double)(rand()%1000))/1000.0;
                    retval += randomFactor*s->queueTime().value();
                }
                retval += s->length().value() / fleet_->speed(s->transportMode()).value();
            }
            return retval;
        }
        FleetPtr fleet_;
    };

    Conn(std::string name,ShippingNetworkPtrConst shippingNetwork) : NamedInterface(name), shippingNetwork_(shippingNetwork), routingAlgorithm_(none_){}

    PathList paths(Conn::PathSelector::Type type, std::set<Location::EntityType> endLocationTypes, std::set<PathMode> pathModes,
                    priority_queue<PathPtr,vector<PathPtr>,TraversalCompare> pathContainer, ConstraintPtr constraints,LocationPtr start,LocationPtr endpoint) const ;
    bool validSegment(SegmentPtr segment) const;
    PathPtr pathElementEnque(Path::PathElementPtr pathElement, PathPtr path, FleetPtr fleet) const;
    PathPtr copyPath(PathPtr path, FleetPtr fleet) const;
    Constraint::EvalOutput checkConstraints(ConstraintPtr constraints, PathPtr path) const;
    std::set<PathMode> modeIntersection(SegmentPtr segment,std::set<PathMode> pathModes) const;

    typedef std::pair<EntityID,EntityID> RoutingTableKey;
    typedef std::map<RoutingTableKey,EntityID> RoutingTable;
    typedef std::set<PathMode> ModeSet;
    typedef std::map<uint32_t,ModeSet> ModeCollection;

    TraversalOrder* traversalOrder() const { return traversalOrder_; }
    void traversalOrderIs(TraversalOrder* traversalOrder){ traversalOrder_=traversalOrder; }
    std::set<Location::EntityType> endLocationTypes() const { return endLocationType_; }

    // Next hop mutators
    void nextHopClear(){
        nextHop_.clear();
    }
    void nextHopIs(EntityID source, EntityID sink, EntityID next){
        RoutingTableKey key(source,sink);
        nextHop_.insert(pair<RoutingTableKey,EntityID>(key,next));
    }

    ShippingNetworkPtrConst shippingNetwork_;
    RoutingAlgorithm routingAlgorithm_;
    RoutingTable nextHop_;
    std::set<Location::EntityType> endLocationType_;
    TraversalOrder* traversalOrder_;
    typedef std::vector<Conn::NotifieePtr> NotifieeList;
    NotifieeList notifieeList_;
    ModeCollection supportedRouteModes_;
};

class Stats : public Fwk::NamedInterface {
public:
    LocationNum locationCount(Location::EntityType et) const; 
    SegmentNum segmentCount(TransportMode et) const;
    SegmentNum segmentCount(PathMode pm) const;
    SegmentNum totalSegmentCount() const { return totalSegmentCount_; }
private:
    friend class ShippingNetwork;
    friend class SegmentReactor;
    friend class StatsReactor;
    Stats(std::string name) : NamedInterface(name){
        totalSegmentCount_=0;
    }
    void locationCountIncr(Location::EntityType type);
    void locationCountDecr(Location::EntityType type);
    void segmentCountIncr(TransportMode type);
    void segmentCountDecr(TransportMode type);
    void segmentCountIncr(PathMode type);
    void segmentCountDecr(PathMode type);
    void totalSegmentCountIncr();
    void totalSegmentCountDecr();
    typedef std::map<Location::EntityType, uint32_t> LocationCountMap;
    LocationCountMap locationCount_;
    typedef std::map<TransportMode, uint32_t> SegmentCountMap;   
    SegmentCountMap segmentCount_;
    typedef std::map<PathMode, uint32_t> PathModeCountMap;
    PathModeCountMap modeCount_;
    uint32_t totalSegmentCount_;
};

class ShippingNetwork : public Fwk::NamedInterface {
public:

    class NotifieeConst : public virtual Fwk::NamedInterface::NotifieeConst {
    public:
        virtual void onSegmentNew(EntityID segmentID){}
        virtual void onSegmentDel(SegmentPtr segment){}
        virtual void onLocationNew(EntityID locationID){}
        virtual void onLocationDel(LocationPtr location){}
        void notifierIs(ShippingNetworkPtrConst notifier){ notifier_=notifier; }
        ShippingNetworkPtrConst notifier() const { return notifier_; }
    protected:
        NotifieeConst() {}
        virtual ~NotifieeConst(){}
        ShippingNetworkPtrConst notifier_;
    };
    typedef Fwk::Ptr<ShippingNetwork::NotifieeConst> NotifieeConstPtr;
    typedef Fwk::Ptr<ShippingNetwork::NotifieeConst const> NotifieeConstPtrConst;
    class Notifiee : public virtual NotifieeConst, public virtual Fwk::NamedInterface::Notifiee {
    public:
        ShippingNetworkPtr notifier() const { return const_cast<ShippingNetwork*>(NotifieeConst::notifier().ptr()); }
    protected:
        Notifiee() {}
        virtual ~Notifiee(){}
    };
    typedef Fwk::Ptr<ShippingNetwork::Notifiee> NotifieePtr;
    typedef Fwk::Ptr<ShippingNetwork::Notifiee const> NotifieePtrConst;

    ManagerPtr manager() const { return manager_; }
    SegmentPtr segment(EntityID name) const; 
    LocationPtr location(EntityID name) const;
    LocationNum locationCount() const;
    LocationPtr location(int32_t index) ;
    ConnPtrConst conn(EntityID name) const;
    ConnPtr conn() const { return connPtr_; }
    StatsPtrConst stats(EntityID name) const; 
    FleetPtr fleet(EntityID name) const;
    FleetPtr activeFleet() const { return fleetPtr_; }
    SegmentPtr SegmentNew(EntityID name, TransportMode mode, PathMode pathMode); 
    SegmentPtr segmentDel(EntityID name);
    LocationPtr LocationNew(EntityID name, Location::EntityType entityType);
    LocationPtr locationDel(EntityID name);
    ConnPtr ConnNew(EntityID name);
    ConnPtr connDel(EntityID name);
    StatsPtr StatsNew(EntityID name);
    StatsPtr statsDel(EntityID name);
    FleetPtr FleetNew(EntityID name);
    FleetPtr fleetDel(EntityID name);
    void activeFleetIs(FleetPtr fleet);
    void notifieeIs(ShippingNetwork::NotifieePtr notifiee);
    static ShippingNetworkPtr ShippingNetworkIs(EntityID name, ManagerPtr manager);

private:
    FleetPtr createFleetAndReactor(EntityID name);
    ShippingNetwork(EntityID name, ManagerPtr manager) : Fwk::NamedInterface(name){
        manager_=manager;
        locationIteratorPos_=-1;
    }
    ManagerPtr manager_;
    typedef std::map<EntityID, LocationPtr> LocationMap;
    LocationMap locationMap_;
    LocationMap::const_iterator locationIterator_;
    int32_t locationIteratorPos_;
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

class RoutingReactor : public Conn::Notifiee {
public:
    /* Setup a routing table 
     * based on the current routing mechanism
     */
    void onRouting();
private:
    friend class ShippingNetwork;
    RoutingReactor(ShippingNetworkPtr network) : network_(network){}
    void initRoutingTable(Conn::TraversalOrder*);
    ShippingNetworkPtr network_;
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
    void onMode(PathMode mode);
    void onModeDel(PathMode mode);
    void onShipment(ShipmentPtr shipment);
    void onCapacity();
private:
    // Factory Class
    friend class ShippingNetwork;
    SegmentReactor(ShippingNetworkPtr network,StatsPtr stats);
    void startupFAR();
    LocationPtr currentSource_;
    SegmentPtr  currentReturnSegment_;
    ShippingNetworkPtr network_;
    StatsPtr stats_;
    ManagerPtr manager_;
};

class ShippingNetworkReactor : public ShippingNetwork::Notifiee{
public:
    void onSegmentDel(SegmentPtr segment); 
    void onLocationDel(LocationPtr location);
    static ShippingNetworkReactorPtr ShippingNetworkReactorIs(){
        return new ShippingNetworkReactor();
    }
private:
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
    friend class ShippingNetwork;
    StatsReactor(StatsPtr stats);
    StatsPtr stats_;
};

} /* end namespace */

#endif
