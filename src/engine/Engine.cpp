#include <stdlib.h>
#include <iostream>
#include <stack>
#include "engine/Engine.h"
#include "logging.h"

using namespace Shipping;

/*
 * Location 
 *
 */

Location::Location(EntityID name, EntityType type): Fwk::NamedInterface(name), entityType_(type){
}

SegmentCount Location::segmentCount() const { 
    return segments_.size(); 
}

SegmentPtr Location::segment(uint32_t index) const {
    if (index < 1 || index > segments_.size())
        return NULL;
    return segments_[index-1];
}


void Location::entityTypeIs(Location::EntityType et){
    entityType_=et;
}

void Location::segmentIs(SegmentPtr segment){

    // Make sure this segment is not already listed
    std::vector<SegmentPtr>::iterator it;
    for ( it=segments_.begin() ; it < segments_.end(); it++ ){
        if(*it == segment) return;
    }

    // Add the segment to the end of the list
    segments_.push_back(segment);
}

/* segmentDel(): Remove a Segment from this Location 
 */
void Location::segmentDel(SegmentPtr segment){
    // Find this segment and erase it from the list
    std::vector<SegmentPtr>::iterator it;
    for ( it=segments_.begin() ; it < segments_.end(); it++ ){
        if(*it == segment){
            segments_.erase(it);
            return;
        }
    }
}

void Location::notifieeIs(Location::Notifiee* notifiee){
    // Ensure idempotency
    std::vector<Location::NotifieePtr>::iterator it;
    for ( it=notifieeList_.begin(); it < notifieeList_.end(); it++ ){
        if( (*it) == notifiee ) return;
    }

    // Register this notifiee
    notifiee->notifierIs(this);
    notifieeList_.push_back(notifiee);
}

void Location::shipmentIs(ShipmentPtr shipment) {
    // Call Notifiees if not destination
    // TODO: this shouldn't occur at all
    if (entityType_ == EntityType::customer())
        return;
    Location::NotifieeList::iterator it;
    for ( it=notifieeList_.begin(); it < notifieeList_.end(); it++ ){
        try{
            (*it)->onShipment(shipment);
        }
        catch(...){
            // ERROR: maybe we should log something here
        }
    }
}

void LocationReactor::onShipment(ShipmentPtr shipment) {
    EntityID nextSegmentName = network_->conn()->nextHop(notifier_->name(), shipment->destination()->name());
    SegmentPtr segment = network_->segment(nextSegmentName);
    if (!segment) {
        DEBUG_LOG << "Cannot find next hop: <" << nextSegmentName << "> to connect " << notifier_->name() << " and " << shipment->destination()->name() << ".\n";
        throw EntityExistsException();
    }
    segment->shipmentIs(shipment);
}

/*
 * Customer 
 *
 */

void Customer::transferRateIs(ShipmentPerDay spd){
    // TODO: NOT IDEMPOTENT
    transferRate_ = spd; 

    // Call Notifiees
    Customer::NotifieeList::iterator it;
    for ( it=notifieeList_.begin(); it < notifieeList_.end(); it++ ){
        try{
            (*it)->onTransferRate();
        }
        catch(...){
            // ERROR: maybe we should log something here
        }
    }
}

Time Customer::nextShipmentTime() const {
    // TODO: add different ranges of times
    if (transferRate_.value() <= 0.00005)
        // TODO: this is not the right error
        throw EntityExistsException();
    double HoursPerShipment = 24 / transferRate_.value();
    // TODO: casting takes floor, right?
    int daysBeforeToday = manager_->now().value() / 24;
    int hoursToday = manager_->now().value() - daysBeforeToday * 24;
    int shipmentsToday = hoursToday / HoursPerShipment;
    return Time(daysBeforeToday * 24 + (shipmentsToday + 1) * HoursPerShipment);
}

void Customer::shipmentSize(PackageNum pn) {
    shipmentSize_ = pn; 

    // Call Notifiees
    Customer::NotifieeList::iterator it;
    for ( it=notifieeList_.begin(); it < notifieeList_.end(); it++ ){
        try{
            (*it)->onShipmentSize();
        }
        catch(...){
            // ERROR: maybe we should log something here
        }
    }

}
void Customer::destinationIs(LocationPtr lp){
    if (lp->entityType() != EntityType::customer()) {
        throw ArgumentException();
    }
    destination_ = lp;


    // Call Notifiees
    Customer::NotifieeList::iterator it;
    for ( it=notifieeList_.begin(); it < notifieeList_.end(); it++ ){
        try{
            (*it)->onDestination();
        }
        catch(...){
            // ERROR: maybe we should log something here
        }
    }
}

Customer::Customer(EntityID name, EntityType type) : Location(name, type), destination_(NULL){
    shipmentsReceived_ = 0;
    totalLatency_ = Hour(0);
    totalCost_ = Dollar(0);
    transferRate_ = ShipmentPerDay(0);

}

void Customer::notifieeIs(Customer::Notifiee* notifiee){
    // Ensure idempotency
    std::vector<Customer::NotifieePtr>::iterator it;
    for ( it=notifieeList_.begin(); it < notifieeList_.end(); it++ ){
        if( (*it) == notifiee ) return;
    }

    // Register this notifiee
    notifiee->notifierIs(this);
    notifieeList_.push_back(notifiee);
}

void CustomerReactor::onTransferRate() {
    transferRateSet_ = true;
    checkAndCreateInjectActivity();
}

void CustomerReactor::onShipmentSize() {
    shipmentSizeSet_ = true;
    checkAndCreateInjectActivity();
}

void CustomerReactor::onDestination() {
    destinationSet_ = true;
    checkAndCreateInjectActivity();
}

void Customer::shipmentIs(ShipmentPtr shipment) {
    Customer::NotifieeList::iterator it;
    for ( it=notifieeList_.begin(); it < notifieeList_.end(); it++ ){
        try{
            (*it)->onShipment(shipment);
        }
        catch(...){
            // ERROR: maybe we should log something here
        }
    }
}

void CustomerReactor::onShipment(ShipmentPtr shipment) {
    DEBUG_LOG << "Shipment has arrived at customer.\n";

    // if shipment is arriving at destination, udpate stats
    if (shipment->destination()->name() == notifier_->name()) {
        DEBUG_LOG << "  Customer is destination; udating stats...\n";
        // TODO: these are probably not in the right units
        CustomerPtr cust = dynamic_cast<Customer*> (shipment->destination().ptr());
        cust->totalLatency_ = Hour(cust->totalLatency_.value() + manager_->now().value() - shipment->startTime().value());
        cust->totalCost_ = cust->totalCost_ + shipment->cost();
        cust->shipmentsReceived_ ++;
        return;
    }

    // otherwise, if arriving at the source, forward activity to segment
    else if (shipment->source()->name() == notifier_->name()) {
        DEBUG_LOG << "  Customer is source; preparing shipment...\n";
        EntityID nextSegmentName = network_->conn()->nextHop(notifier_->name(), shipment->destination()->name());

        std::cout << "DEBUGGING: remove the following code\n";
        nextSegmentName = "seg1";

        SegmentPtr segment = network_->segment(nextSegmentName);
        if (!segment) {
            DEBUG_LOG << "Cannot find next hop: <" << nextSegmentName << "> to connect " << notifier_->name() << " and " << shipment->destination()->name() << ".\n";
            throw EntityExistsException();
        }
        segment->shipmentIs(shipment);
        return;
    }

    // shipment ended up at wrong customer
    // TODO: this is certainly the wrong error type
    throw EntityExistsException();
}

void CustomerReactor::checkAndCreateInjectActivity() {
    CustomerPtr cust = dynamic_cast<Customer*>(const_cast<Location*> (notifier_.ptr()));

    if (transferRateSet_ && shipmentSizeSet_ && destinationSet_) {
        DEBUG_LOG << "Criteria fully specified. Setting up shipment injection activity...\n";

        // retrieve / create activity
        Activity::ActivityPtr ia = manager_->activity(notifier_->name());
        if (!ia) {
            ia = manager_->activityNew(notifier_->name());
            InjectActivityReactor* iar = new InjectActivityReactor();
            ia->lastNotifieeIs(iar);
            iar->managerIs(manager_);
            iar->sourceIs(cust);

            // TODO: do I need to call manager_->lastActivityIs(ia)?
        }

        try {
            ia->nextTimeIs(cust->nextShipmentTime());
            manager_->lastActivityIs(ia);
        } catch (...) {
            // TODO: log?
        }
    }
}

void InjectActivityReactor::onStatus() {
    DEBUG_LOG << "Inject activity notified.\n";

    // create new shipment

    // TODO: better name?
    ShipmentPtr shipment = new Shipment("name");
    shipment->loadIs(source_->shipmentSize());
    shipment->sourceIs(source_);
    shipment->destinationIs(source_->destination());
    shipment->startTimeIs(manager_->now());

    // add shipment to location
    source_->shipmentIs(shipment);

    // reschedule activity
    try {
        notifier_->nextTimeIs(source_->nextShipmentTime());
    } catch (...) {
        // TODO: log?
    }
}


/*
 * Segment 
 *
 */

ModeCount Segment::modeCount() const{
    return mode_.size();
}

PathMode Segment::mode(uint16_t index) const{
    std::set<PathMode>::const_iterator it;
    int i = 0;
    it = mode_.begin();
    while (it != mode_.end()){
        if(i == index) return *it;
        i++;
        *it++;
    }
    return PathMode::undef();
}

PathMode Segment::mode(PathMode mode) const{
    if(mode_.count(mode) != 0) return mode;
    return PathMode::undef();
}

void Segment::sourceIs(EntityID source){
    sourceIs(network_->location(source));
}

void Segment::returnSegmentIs(EntityID segment){
    returnSegmentIs(network_->segment(segment));
}

void Segment::sourceIs(LocationPtr source){

    // Ensure idempotency
    if(source == source_){
        return;
    }

    // Set Source
    source_=source;

    // Call Notifiees
    Segment::NotifieeList::iterator it;
    for ( it=notifieeList_.begin(); it < notifieeList_.end(); it++ ){
        try{
            (*it)->onSource();
        }
        catch(...){
            // ERROR: maybe we should log something here
        }
    }
}

void Segment::returnSegmentIs(SegmentPtr returnSegment){

    // Ensure idempotency
    if(returnSegment == returnSegment_){
        return;
    }

    // Set Source
    returnSegment_=returnSegment;

    // Call Notifiees
    Segment::NotifieeList::iterator it;
    for ( it=notifieeList_.begin(); it < notifieeList_.end(); it++ ){
        try{
            (*it)->onReturnSegment();
        }
        catch(...){
            // ERROR: maybe we should log something here
        }
    }
}

void Segment::transportModeIs(TransportMode transportMode){
    transportMode_ = transportMode;
}

void Segment::modeIs(PathMode mode){
    if(mode_.count(mode) != 0){
        return;
    }
    mode_.insert(mode);
    // Call Notifiees
    Segment::NotifieeList::iterator it;
    for ( it=notifieeList_.begin(); it < notifieeList_.end(); it++ ){
        try{
            (*it)->onMode(mode);
        }
        catch(...){
            // ERROR: maybe we should log something here
        }
    }
}

PathMode Segment::modeDel(PathMode mode){
    if(mode_.count(mode) == 0){
        return PathMode::undef();
    }
    mode_.erase(mode);
    Segment::NotifieeList::iterator it;
    for ( it=notifieeList_.begin(); it < notifieeList_.end(); it++ ){
        try{
            (*it)->onModeDel(mode);
        }
        catch(...){
            // ERROR: maybe we should log something here
        }
    }
    return mode;
}

void Segment::notifieeIs(Segment::Notifiee* notifiee){

    // Ensure idempotency
    std::vector<Segment::NotifieePtr>::iterator it;
    for ( it=notifieeList_.begin(); it < notifieeList_.end(); it++ ){
        if( (*it) == notifiee ) return;
    }

    // Register this notifiee
    notifiee->notifierIs(this);
    notifieeList_.push_back(notifiee);
}

void Segment::lengthIs(Mile length){
    length_=length;
}

void Segment::capacityIs(ShipmentNum capacity){
    capacity_=capacity;
}

void Segment::difficultyIs(Difficulty difficulty){
    difficulty_=difficulty;
}

void Segment::shipmentIs(ShipmentPtr shipment) {
    DEBUG_LOG << "Shipment has arrived at segment "<< this->name() << "\n";

    // add subshipments to queue
    // TODO: better name?
    SubshipmentPtr s = new Subshipment("name");
    s->shipmentIs(shipment);
    s->shipmentOrderIs(Subshipment::other());
    s->remainingLoadIs(shipment->load());
    subshipmentEnqueue(s);

    // notify segment reactor
    Segment::NotifieeList::iterator it;
    for ( it=notifieeList_.begin(); it < notifieeList_.end(); it++ ){
        try{
            (*it)->onShipment(shipment);
        }
        catch(...){
            // ERROR: maybe we should log something here
        }
    }
}

SubshipmentPtr Segment::subshipmentDequeue(PackageNum capacity) {
    // return null if queue is empty
    if (subshipmentQueue_.empty())
        return NULL;

    // remove subshipment if remaining packages can be delivered at once
    SubshipmentPtr lastSubshipment = subshipmentQueue_.front();
    if (capacity >= lastSubshipment->remainingLoad()) {
        subshipmentQueue_.pop();
        lastSubshipment->shipmentOrderIs(Subshipment::last());
        return lastSubshipment;
    }

    // otherwise return partial shipment
    lastSubshipment->remainingLoadIs(lastSubshipment->remainingLoad() - capacity);
    SubshipmentPtr result = new Subshipment("name");
    result->shipmentIs(lastSubshipment->shipment());
    result->shipmentOrderIs(Subshipment::other());
    result->remainingLoadIs(capacity);
    return result;

}

Hour Segment::carrierLatency() const {
    // TODO: can make this more general (include difficulty, expedite)
    return Hour(length_.value() * network_->fleet()->speed(transportMode_).value());
}

PackageNum Segment::carrierCapacity() const {
    return network_->fleet()->capacity(transportMode_);
}

Dollar Segment::carrierCost() const {
    // TODO: can make this more general
    return Dollar(length_.value() * network_->fleet()->cost(transportMode_).value());
}

/*
 * Stats
 *
 */

uint32_t Stats::locationCount(Location::EntityType et) const {
    Stats::LocationCountMap::const_iterator pos = locationCount_.find(et);
    if(pos == locationCount_.end()){
        return 0;
    }
    return pos->second;
}

uint32_t Stats::segmentCount(TransportMode et) const {
    Stats::SegmentCountMap::const_iterator pos = segmentCount_.find(et);
    if(pos == segmentCount_.end()){
        return 0;
    }
    return pos->second;
}

uint32_t Stats::segmentCount(PathMode et) const {
    Stats::PathModeCountMap::const_iterator pos = modeCount_.find(et);
    if(pos == modeCount_.end()){
        return 0;
    }
    return pos->second;
}

void Stats::locationCountDecr(Location::EntityType type){
    if(locationCount_.count(type) == 0){
        locationCount_[type]=0;
    }
    else if(locationCount_[type] > 0){
        locationCount_[type] = locationCount_[type]-1;
    }
}

void Stats::locationCountIncr(Location::EntityType type){
    if(locationCount_.count(type) == 0){
        locationCount_[type]=0;
    }
    locationCount_[type]=locationCount_[type]+1;
}

void Stats::segmentCountDecr(TransportMode type){
    if(segmentCount_.count(type) == 0){
        segmentCount_[type]=0;
    }
    else if(segmentCount_[type] > 0){
        segmentCount_[type] = segmentCount_[type]-1;
    }
}

void Stats::segmentCountIncr(TransportMode type){
    if(segmentCount_.count(type) == 0){
        segmentCount_[type]=0;
    }
    segmentCount_[type]=segmentCount_[type]+1;
}

void Stats::segmentCountDecr(PathMode type){
    if(modeCount_.count(type) == 0){
        modeCount_[type]=0;
    }
    else if(modeCount_[type] > 0){
        modeCount_[type] = modeCount_[type]-1;
    }
}

void Stats::segmentCountIncr(PathMode type){
    if(modeCount_.count(type) == 0){
        modeCount_[type]=0;
    }
    modeCount_[type]=modeCount_[type]+1;
}

void Stats::totalSegmentCountDecr(){
    if(totalSegmentCount_ > 0) totalSegmentCount_--;
}

void Stats::totalSegmentCountIncr(){
    totalSegmentCount_++;
}

/*
 * ShippingNetwork 
 *
 */

SegmentPtr ShippingNetwork::segment(EntityID name) const {
    ShippingNetwork::SegmentMap::const_iterator pos = segmentMap_.find(name);
    if(pos == segmentMap_.end()){
        return NULL;
    }
    return pos->second;
}

uint32_t ShippingNetwork::locationCount() const {
    return locationMap_.size();
}

LocationPtr ShippingNetwork::location(int32_t index) {
    if(locationIteratorPos_ == -1 || index < locationIteratorPos_){
        locationIteratorPos_=0;
        locationIterator_ = locationMap_.begin();
    }
    while(locationIterator_ != locationMap_.end()){
        if(locationIteratorPos_==index){
            return locationIterator_->second;
        }
        locationIteratorPos_++;
        locationIterator_++;
    }
    return NULL;
}

LocationPtr ShippingNetwork::location(EntityID name) const {
    ShippingNetwork::LocationMap::const_iterator pos = locationMap_.find(name);
    if(pos == locationMap_.end()){
        return NULL;
    }
    return pos->second;
}

ConnPtrConst ShippingNetwork::conn(EntityID name) const {
    ShippingNetwork::ConnMap::const_iterator pos = conn_.find(name);
    if(pos == conn_.end()){
        return NULL;
    }
    return pos->second;
}

FleetPtr ShippingNetwork::fleet(EntityID name) const {
    ShippingNetwork::FleetMap::const_iterator pos = fleet_.find(name);
    if(pos == fleet_.end()){
        return NULL;
    }
    return pos->second;
}

StatsPtrConst ShippingNetwork::stats(EntityID name) const {
    ShippingNetwork::StatMap::const_iterator pos = stat_.find(name);
    if(pos == stat_.end()){
        return NULL;
    }
    return pos->second;
}

ShippingNetworkPtr ShippingNetwork::ShippingNetworkIs(EntityID name){

    // Construct the network
    ShippingNetworkPtr retval = new ShippingNetwork(name);

    // Initialize Singletons (fleet info, stats, conn objects)
    retval->fleetPtr_ = new Fleet("The Fleet");
    retval->statPtr_ = new Stats("The Stat");
    retval->connPtr_ = new Conn("The Conn",retval,retval->fleetPtr_);
    retval->connPtr_->notifieeIs(new RoutingReactor(retval));

    // Setup my reactors
    retval->notifieeIs(new StatsReactor(retval->statPtr_));
    retval->notifieeIs(new ShippingNetworkReactor());

    return retval;
}

void ShippingNetwork::notifieeIs(ShippingNetwork::NotifieePtr notifiee){
    // Ensure idempotency
    ShippingNetwork::NotifieeList::iterator it;
    for ( it=notifieeList_.begin(); it < notifieeList_.end(); it++ ){
        if( (*it) == notifiee ) return;
    }

    // Register this notiee
    notifiee->notifierIs(this);
    notifieeList_.push_back(notifiee);
}

SegmentPtr ShippingNetwork::SegmentNew(EntityID name, TransportMode entityType, PathMode pathMode){

    // If Segment with this name already exists, just return it
    SegmentPtr existing = segment(name);
    if(existing){
        throw EntityExistsException();
    }

    // Create a New Segment
    SegmentPtr retval(new Segment(this,name,entityType,pathMode));
    segmentMap_[name]=retval;

    // Setup Reactor
    // TODO: is it an issue that I'm not using Ptr?
    SegmentReactor* sr = new SegmentReactor(this,statPtr_);
    sr->manager_ = manager_;
    retval->notifieeIs(sr);

    // Issue Notifications
    ShippingNetwork::NotifieeList::iterator it;
    for(it=notifieeList_.begin(); it < notifieeList_.end(); it++){
        try{
            (*it)->onSegmentNew(name);
        }
        catch(...){
            // ERROR: Maybe we should log something
        }
    }

    return retval;
}

SegmentPtr ShippingNetwork::segmentDel(EntityID name){

    SegmentPtr retval;

    ShippingNetwork::SegmentMap::iterator segmentPos = segmentMap_.find(name);
    if(segmentPos == segmentMap_.end())
        return NULL;
    
    // erase the entry
    retval = segmentPos->second;
    segmentMap_.erase(segmentPos);

    // Issue Notifications
    ShippingNetwork::NotifieeList::iterator it;
    for(it=notifieeList_.begin(); it < notifieeList_.end(); it++){
        try{
            (*it)->onSegmentDel(retval);
        }
        catch(...){
            // ERROR: Maybe we should log something
        }
    }
    
    return retval;
}

LocationPtr ShippingNetwork::LocationNew(EntityID name, Location::EntityType entityType){

    // If Segment with this name already exists, just return it
    LocationPtr existing = location(name);
    if(existing){
        throw EntityExistsException();
    }

    // Create a New Segment
    LocationPtr retval;
    if (entityType == Location::EntityType::customer()) {
        retval = new Customer(name, entityType);
        CustomerPtr cust = (dynamic_cast<Customer*> (retval.ptr()));
        cust->manager_ = manager_;

        // create CustomerReactor
        // TODO: is this pointer a problem?
        CustomerReactor* notifiee = new CustomerReactor();
        notifiee->manager_ = manager_;
        notifiee->network_ = this;
        cust->notifieeIs(notifiee);
    } else {
        retval = new Location(name,entityType);
        LocationReactor* notifiee = new LocationReactor();
        notifiee->network_ = this;
        retval->notifieeIs(notifiee);
    }
    locationMap_[name]=retval;

    // Issue Notifications
    ShippingNetwork::NotifieeList::iterator it;
    for(it=notifieeList_.begin(); it < notifieeList_.end(); it++){
        try{
            (*it)->onLocationNew(name);
        }
        catch(...){
            // ERROR: Maybe we should log something
        }
    }

    return retval;
}

LocationPtr ShippingNetwork::locationDel(EntityID name){

    LocationPtr retval;

    ShippingNetwork::LocationMap::iterator locationPos = locationMap_.find(name);
    if(locationPos == locationMap_.end())
        return NULL;
    
    // erase the entry
    retval = locationPos->second;
    locationMap_.erase(locationPos);

    // Issue Notifications
    ShippingNetwork::NotifieeList::iterator it; 
    for(it=notifieeList_.begin(); it < notifieeList_.end(); it++){
        try{
            (*it)->onLocationDel(retval);
        }   
        catch(...){
            // ERROR: Maybe we should log something
        }
    }   

    return retval;
}

StatsPtr ShippingNetwork::StatsNew(EntityID name){
    if(stat_.count(name) != 0){
        throw EntityExistsException();
    }
    stat_[name] = statPtr_;
    return statPtr_;
}

StatsPtr ShippingNetwork::statsDel(EntityID name){
    StatMap::iterator it = stat_.find(name);
    if(it == stat_.end())
        return NULL;
    stat_.erase(it);
    return it->second;
}

ConnPtr ShippingNetwork::ConnNew(EntityID name){
    if(conn_.count(name) != 0){
        throw EntityExistsException();
    }
    conn_[name] = connPtr_;
    return connPtr_;
}

ConnPtr ShippingNetwork::connDel(EntityID name){
    ConnMap::iterator it = conn_.find(name);
    if(it == conn_.end())
        return NULL;
    conn_.erase(it);
    return it->second;
}

FleetPtr ShippingNetwork::FleetNew(EntityID name){
    if(fleet_.count(name) != 0){
        throw EntityExistsException();
    }
    fleet_[name]=fleetPtr_;
    return fleetPtr_;
}

FleetPtr ShippingNetwork::fleetDel(EntityID name){
    FleetMap::iterator it = fleet_.find(name);
    if(it == fleet_.end())
        return NULL;
    fleet_.erase(it);
    return it->second;
}

/*
 * SegmentReactor
 * 
 */

SegmentReactor::SegmentReactor(ShippingNetworkPtr network, StatsPtr stats){
    currentSource_ = NULL;
    currentReturnSegment_ = NULL;
    network_=network;
    stats_=stats;
}

void SegmentReactor::onSource(){
    // Remove the notifier from the old source
    if(currentSource_){
        currentSource_->segmentDel(notifier());
    }
    // Update the source reference
    currentSource_ = notifier()->source();
    // Add the notifier to the new source
    if(currentSource_){
        currentSource_->segmentIs(notifier());
    }
}

void SegmentReactor::onReturnSegment(){

    /* Remove this segment from old return segment if it still thinks
     * this reactor's segment is its return segment
     */
    if(currentReturnSegment_ && currentReturnSegment_->returnSegment() == notifier()){
        currentReturnSegment_->returnSegmentIs((SegmentPtr)NULL);
    }

    /* Update return segment ref */
    currentReturnSegment_ = notifier()->returnSegment();

    /* Update new return segment to set this segment as its return segment
     * if this segment is not already its return segment
     */
    if(currentReturnSegment_ && currentReturnSegment_->returnSegment() != notifier()){
        currentReturnSegment_->returnSegmentIs(notifier());
    }
}

void SegmentReactor::onMode(PathMode mode){
    stats_->segmentCountIncr(mode);
}

void SegmentReactor::onModeDel(PathMode mode){
    stats_->segmentCountDecr(mode);
}

void SegmentReactor::onShipment(ShipmentPtr shipment) {
    DEBUG_LOG << "Segment reactor notified of new shipment.\n";

    // wait for other carriers to finish if none are available
    SegmentPtr segment = const_cast<Segment*> (notifier_.ptr());
    if (segment->carriersUsed() == segment->capacity().value()) {
        segment->shipmentsRefusedInc();
    }

    // otherwise, create a new forward activity and execute immediately
    else {
        DEBUG_LOG << "Creating new ForwardActivityReactor...\n";
        // generate activity name
        std::stringstream s;
        s << segment->name() << "-" << segment->carriersUsed();

        // create new activity and activity reactor
        Activity::ActivityPtr fa = manager_->activityNew(s.str());
        ForwardActivityReactor* far = new ForwardActivityReactor();
        far->managerIs(manager_);
        far->segmentIs(segment);
        fa->nextTimeIs(Time(manager_->now().value() + notifier_->carrierLatency().value()));
        fa->lastNotifieeIs(far);
        segment->carriersUsedInc();
        manager_->lastActivityIs(fa);
    }
}


void ForwardActivityReactor::onStatus() {
    DEBUG_LOG << "ForwardActivityReactor notified at " << manager_->now().value() << ".\n";

    // deliver shipment if the shipment is complete
    if (subshipment_) {
        DEBUG_LOG << "  Delivering subshipment...\n";
        subshipment_->shipment()->costInc(segment_->carrierCost());
        if (subshipment_->shipmentOrder() == Subshipment::last()) {
            DEBUG_LOG << "  Shipment is complete.\n";
            segment_->returnSegment()->source()->shipmentIs(subshipment_->shipment());
        }
        // TODO: delete subshipment?
    }

    // pick up next shipment if there is one
    subshipment_ = segment_->subshipmentDequeue(segment_->carrierCapacity());

    // TODO: add carriers if capacity increases
    // delete activity if there is no remaining subshipment or there are too many carriers
    if (!subshipment_ || segment_->carriersUsed() > segment_->capacity().value()) {
        manager_->activityDel(notifier_->name());
        segment_->carriersUsedDec();
        return; 
    }

    // sleep for required time
    notifier_->nextTimeIs(Time(manager_->now().value() + segment_->carrierLatency().value()));
}

/*
 * ShippingNetworkReactor
 * 
 */

ShippingNetworkReactor::ShippingNetworkReactor(){}

void ShippingNetworkReactor::onSegmentDel(SegmentPtr segment){
    // Clean up this Segment's source
    segment->sourceIs((LocationPtr)NULL);
    // Clean up this Segment's return segment
    segment->returnSegmentIs((SegmentPtr)NULL);
}

void ShippingNetworkReactor::onLocationDel(LocationPtr location){
    // Clean up this Location from all its Segments
    for(uint32_t i = 1;i <= location->segmentCount().value(); i++){
        SegmentPtr segment = location->segment(i);
        segment->sourceIs((LocationPtr)NULL);
    }
}

/*
 * StatsReactor
 *
 */

StatsReactor::StatsReactor(StatsPtr stats){
    stats_=stats;
}

void StatsReactor::onSegmentNew(EntityID segmentID){
    SegmentPtr segment = notifier()->segment(segmentID);
    if(segment){
        stats_->totalSegmentCountIncr();
        stats_->segmentCountIncr(segment->transportMode());
        for(uint16_t i = 0; i < segment->modeCount().value(); i++){
            stats_->segmentCountIncr(segment->mode(i));
        }
    }
}

void StatsReactor::onSegmentDel(SegmentPtr segment){
    stats_->totalSegmentCountDecr();
    stats_->segmentCountDecr(segment->transportMode());
    for(uint16_t i = 0; i < segment->modeCount().value(); i++){
        stats_->segmentCountDecr(segment->mode(i));
    }
}

void StatsReactor::onLocationNew(EntityID locationID){
    LocationPtr location = notifier()->location(locationID);
    stats_->locationCountIncr(location->entityType());    
}

void StatsReactor::onLocationDel(LocationPtr location){
    stats_->locationCountDecr(location->entityType());
}

/*
 * Conn
 * 
 */

    class DistanceConstraint : public Conn::Constraint{
    public:
        EvalOutput evalOutput(){
            if( !path_ || path_->distance() > distance_ )
                return Conn::Constraint::fail();
            return Conn::Constraint::pass();
        }
        static Conn::ConstraintPtr DistanceConstraintIs(Mile distance){
            return new DistanceConstraint(distance);
        }
    private:
        DistanceConstraint(Mile distance) : Conn::Constraint(), distance_(distance){}
        Mile distance_;
    };

    class CostConstraint : public Conn::Constraint{
    public:
        EvalOutput evalOutput(){
            if( !path_ || path_->cost() > cost_ )
                return Conn::Constraint::fail();
            return Conn::Constraint::pass();
        }
        static Conn::ConstraintPtr CostConstraintIs(Dollar cost){
            return new CostConstraint(cost);
        }
    private:
        CostConstraint(Dollar cost) : Conn::Constraint(), cost_(cost){}
        Dollar cost_;
    };

    class TimeConstraint : public Conn::Constraint{
    public:
        EvalOutput evalOutput(){
            if( !path_ || path_->time() > time_ )
                return Conn::Constraint::fail();
            return Conn::Constraint::pass();
        }
        static Conn::ConstraintPtr TimeConstraintIs(Hour time){
            return new TimeConstraint(time);
        }
    private:
        TimeConstraint(Hour time) : Conn::Constraint(), time_(time) {}
        Hour time_;
    };

Conn::ConstraintPtr Conn::DistanceConstraintIs(Mile distance){
    return DistanceConstraint::DistanceConstraintIs(distance);
}

Conn::ConstraintPtr Conn::TimeConstraintIs(Hour time){
    return TimeConstraint::TimeConstraintIs(time);
}

Conn::ConstraintPtr Conn::CostConstraintIs(Dollar cost){
    return CostConstraint::CostConstraintIs(cost);
}

Conn::PathSelectorPtr Conn::PathSelector::PathSelectorIs(Type type, ConstraintPtr constraints, LocationPtr start, LocationPtr end){
    if(start == NULL){
        throw new ArgumentException();
    }
    if(type == Conn::PathSelector::connect() && end == NULL){
        throw new ArgumentException();
    }
    if(type == Conn::PathSelector::explore() && end != NULL){
        throw new ArgumentException();
    }
    if(type == Conn::PathSelector::spantree() && end != NULL){
        throw new ArgumentException();
    }

    return new PathSelector(type, constraints,start,end);
}

void Conn::PathSelector::modeIs(PathMode mode){
    pathModes_.insert(mode);
}

PathMode Conn::PathSelector::modeDel(PathMode mode){
    if(pathModes_.count(mode) == 0) return PathMode::undef();
    pathModes_.erase(mode);
    return mode;
}

void Conn::notifieeIs(Conn::NotifieePtr notifiee){
    // Ensure idempotency
    std::vector<Conn::NotifieePtr>::iterator it;
    for ( it=notifieeList_.begin(); it < notifieeList_.end(); it++ ){
        if( (*it) == notifiee ) return;
    }

    // Register this notifiee
    notifiee->notifierIs(this);
    notifieeList_.push_back(notifiee);
}

EntityID Conn::nextHop(EntityID source, EntityID dest) const {
    RoutingTable::const_iterator iter = nextHop_.find(pair<EntityID,EntityID>(source,dest));
    if(iter == nextHop_.end()) return "";
    return iter->second;
}

void Conn::endLocationTypeIs(Location::EntityType type){
    endLocationType_.insert(type);
}

void Conn::routingIs(RoutingAlgorithm routingAlgorithm){
    if(routingAlgorithm_==routingAlgorithm) return;
    routingAlgorithm_=routingAlgorithm;
    // Call Notifiees
    Conn::NotifieeList::iterator it;
    for ( it=notifieeList_.begin(); it < notifieeList_.end(); it++ ){
        try{
            (*it)->onRouting();
        }
        catch(...){
            // ERROR: maybe we should log something here
        }
    } 
}
 
Conn::PathList Conn::paths(Conn::PathSelectorPtr selector) const {
    Conn::PathSelector::Type type = selector->type();
    LocationPtr startPtr = selector->start();
    LocationPtr endPtr = selector->end();
    /* Check Pre-Conditions */
    // Make Sure endPtr is valid Location in my network
    if(type == Conn::PathSelector::connect() &&
        (shippingNetwork_->location(endPtr->name()) != endPtr)
    ){
        return PathList();
    }
    // Make sure startPtr is valid Location in my network
    if(shippingNetwork_->location(startPtr->name()) != startPtr) {
        return PathList();
    }

    MinHopTraversal traversal = MinHopTraversal();
    ((Conn*) this)->traversalOrderIs(&traversal);
    return paths(selector->type(), std::set<Location::EntityType>(), selector->modes(),
                    priority_queue<PathPtr,vector<PathPtr>,TraversalCompare>(TraversalCompare(this)), 
                    selector->constraints(), startPtr, endPtr); 
}

bool Conn::validSegment(SegmentPtr segment) const{
    return (segment && segment->source() && segment->returnSegment() && segment->returnSegment()->source());
}

PathPtr Conn::pathElementEnque(Path::PathElementPtr pathElement, PathPtr path, FleetPtr fleet) const{
    /* Update Metrics */
    Dollar cost;
    Hour time;
    cost = (pathElement->segment()->length()).value() 
           * (fleet->cost(pathElement->segment()->transportMode())).value() 
           * (fleet->costMultiplier(pathElement->elementMode())).value()
           * (pathElement->segment()->difficulty()).value();
    time = (pathElement->segment()->length()).value() 
           / ( (fleet->speed(pathElement->segment()->transportMode()).value()) * ((fleet->speedMultiplier(pathElement->elementMode())).value()));
    path->pathElementEnq(pathElement,cost,time,pathElement->segment()->length());
    return path;
}

PathPtr Conn::copyPath(PathPtr path, FleetPtr fleet) const {
    PathPtr copy = Path::PathIs(path->firstLocation());
    for(uint32_t i = 0; i < path->pathElementCount().value(); i++){
        pathElementEnque(path->pathElement(i), copy, fleet);
    }
    return copy;
}

Conn::Constraint::EvalOutput Conn::checkConstraints(ConstraintPtr constraints, PathPtr path) const {
    Conn::ConstraintPtr constraint = constraints;
    while(constraint){
        constraint->pathIs(path);
        if(constraint->evalOutput() == Conn::Constraint::fail()){
            return Conn::Constraint::fail();
        }
        constraint=constraint->next();
    }
    return Conn::Constraint::pass();
}

std::set<PathMode> Conn::modeIntersection(SegmentPtr segment,std::set<PathMode> modes) const {
    std::set<PathMode> retval;
    std::set<PathMode>::const_iterator it;
    for(it = modes.begin(); it != modes.end(); it++){
        if(segment->mode(*it) == *it) retval.insert(*it);
    }
    return retval;
}

Conn::PathList Conn::paths(Conn::PathSelector::Type type, std::set<Location::EntityType> endLocationTypes, std::set<PathMode> modes, 
                             priority_queue<PathPtr,vector<PathPtr>,TraversalCompare> pathContainer, ConstraintPtr constraints,
                             LocationPtr start, LocationPtr endpoint) const {

    Conn::PathList retval;

    DEBUG_LOG << "Starting location: " << start->name() << std::endl;

    // Setup 
    std::set<EntityID> visited;
    pathContainer.push(Path::PathIs(start));
    // Traverse
    while(pathContainer.size() > 0){

        PathPtr currentPath = pathContainer.top();
        pathContainer.pop();

        DEBUG_LOG << "Visiting location: " << currentPath->lastLocation()->name() << std::endl;

        // Evaluate constraints
        if(checkConstraints(constraints,currentPath)==Conn::Constraint::fail()){
            DEBUG_LOG << "Failed to pass constraints, discarding path" << std::endl;
            continue;
        }

        /* Special Processing for different Query Types */

        // Spanning Tree
        if(type == Conn::PathSelector::spantree()){
            if(visited.count(currentPath->lastLocation()->name()) > 0)
                continue;
            visited.insert(currentPath->lastLocation()->name());
            if(currentPath->pathElementCount() > 0){ 
                retval.push_back(currentPath);
            }
        }

        // Explore
        if(type == Conn::PathSelector::explore()){
            if(currentPath->pathElementCount() > 0) 
                retval.push_back(currentPath);
        }

        // Connect
        if(type == Conn::PathSelector::connect()){
            // Only output if we have reached the endpoint
            // Also, if enpoint is reached then stop traversing
            if(endpoint->name() == currentPath->lastLocation()->name()){
                if(currentPath->pathElementCount() > 0)
                     retval.push_back(currentPath);
                continue;
            }
        }

        if(endLocationTypes.count(currentPath->lastLocation()->entityType()) > 0){
            continue;
        }

        // Continue traversal
        for(uint32_t i = 1; i <= currentPath->lastLocation()->segmentCount().value(); i++){
            SegmentPtr segment = currentPath->lastLocation()->segment(i); 
            if( validSegment(segment)                                                 // Valid Segment
                && segment->returnSegment()->source()                                 // AND Valid Return Segment
                && !(currentPath->location(segment->returnSegment()->source()))       // AND Not a Loop
              )
              {
                  std::set<PathMode> overlap = modeIntersection(segment,modes);  
                  DEBUG_LOG << "Segment Modes: " << segment->modeCount().value() << ", Transport Modes: " << modes.size() << ", Overlap Size: " << overlap.size() << std::endl;               
                  for(std::set<PathMode>::const_iterator it = overlap.begin(); it != overlap.end(); it++){
                      PathPtr pathCopy = copyPath(currentPath,fleet_);
                      pathElementEnque(Path::PathElement::PathElementIs(segment,*it),pathCopy,fleet_);
                      pathContainer.push(pathCopy);
                  }
            }
        }
    }
    return retval;
}

/*
 * Fleet
 *
 */

MilePerHour Fleet::speed(TransportMode m) const {
    Fleet::SpeedMap::const_iterator pos = speed_.find(m);
    if(pos == speed_.end()){
        return MilePerHour();
    }
    return pos->second;
}

void Fleet::speedIs(TransportMode m, MilePerHour s){
    speed_[m]=s;
}

void Fleet::speedMultiplierIs(PathMode mode, Multiplier m){
    speedMultiplier_[mode]=m;
}

Multiplier Fleet::speedMultiplier(PathMode m) const{
    Fleet::SpeedMultiplierMap::const_iterator pos = speedMultiplier_.find(m);
    if(pos == speedMultiplier_.end()){
        return Multiplier();
    }
    return pos->second;
}

PackageNum Fleet::capacity(TransportMode m) const {
    Fleet::CapacityMap::const_iterator pos = capacity_.find(m);
    if(pos == capacity_.end()){
        return PackageNum();
    }
    return pos->second;
}

void Fleet::capacityIs(TransportMode m, PackageNum p){
    capacity_[m]=p;
}

DollarPerMile Fleet::cost(TransportMode m) const {
    Fleet::CostMap::const_iterator pos = cost_.find(m);
    if(pos == cost_.end()){
        return DollarPerMile();
    }
    return pos->second;
}

void Fleet::costMultiplierIs(PathMode mode, Multiplier m){
    costMultiplier_[mode]=m;
}

Multiplier Fleet::costMultiplier(PathMode m) const{
    Fleet::CostMultiplierMap::const_iterator pos = costMultiplier_.find(m);
    if(pos == costMultiplier_.end()){
        return Multiplier();
    }
    return pos->second;
}

void Fleet::costIs(TransportMode m, DollarPerMile d){
    cost_[m]=d;
}

/*
 * Path
 *
 */

PathPtr Path::PathIs(LocationPtr firstLocation){
    return new Path(firstLocation);
}

Path::Path(LocationPtr firstLocation) : cost_(0),time_(0),distance_(0),firstLocation_(firstLocation), lastLocation_(firstLocation){
    locations_.insert(firstLocation->name());
}

Path::PathElementPtr Path::PathElement::PathElementIs(SegmentPtr segment, PathMode elementMode){
    return new Path::PathElement(segment,elementMode);
}

Path::PathElementPtr Path::pathElement(uint32_t index) const{
    if(index >= path_.size()) return NULL;
    return path_[index];
}

PathElementCount Path::pathElementCount() const {
    return path_.size();
}

void Path::pathElementEnq(Path::PathElementPtr element, Dollar cost, Hour time,Mile distance){
    /* Add Element */
    path_.push_back(element);
    /* Update Metadata */
    cost_ = cost_.value() + cost.value();
    time_ = time_.value() + time.value(); 
    distance_ = distance_.value() + distance.value();
    lastLocation_ = element->segment()->returnSegment()->source(); 
    locations_.insert(element->segment()->source()->name());
    locations_.insert(element->segment()->returnSegment()->source()->name());
}

LocationPtr Path::location(LocationPtr location) const{
    if(locations_.count(location->name()) == 0){ return NULL; }
    return location;
}

void Path::PathElement::segmentIs(SegmentPtr segment){
    segment_=segment;
}

/* Routing Reactor */
void RoutingReactor::onRouting(){
    Conn::RoutingAlgorithm algo = notifier()->routing();
    notifier()->nextHopClear();
    if(algo == Conn::minHops()){
        initMinHopsRoutingTable();
    }
    else if(algo == Conn::minDistance()){
        initMinDistanceRoutingTable();
    }
}

void RoutingReactor::initMinHopsRoutingTable(){
    // Iterate over each location and entries for it to the route table
    uint32_t index;
    for(index = 0; index < network_->locationCount(); index++){
        LocationPtr location; 
        std::set<PathMode> modes;
        Conn::PathList paths;
        Conn::PathSelector::Type pathType;
        Conn::MinHopTraversal traversal;
        pathType = Conn::PathSelector::spantree();
        modes.insert(PathMode::unexpedited());
        location = network_->location(index);
        notifier()->traversalOrderIs(&traversal);
        paths = notifier()->paths(pathType,notifier()->endLocationTypes(),modes,
                                  priority_queue<PathPtr,vector<PathPtr>,Conn::TraversalCompare>(Conn::TraversalCompare(notifier())),
                                  NULL,location,NULL);
        // Add each path to routing table
        for(uint32_t i = 0;i < paths.size(); i++){
            PathPtr path = paths[i];
            EntityID endLocation = path->lastLocation()->name();
            EntityID segment = path->pathElement(0)->segment()->name();
            notifier()->nextHopIs(location->name(),endLocation,segment);
        }
    }
}

void RoutingReactor::initMinDistanceRoutingTable(){
    // Iterate over each location and entries for it to the route table
    uint32_t index;
    for(index = 0; index < network_->locationCount(); index++){
        LocationPtr location;
        std::set<PathMode> modes;
        Conn::PathList paths;
        Conn::PathSelector::Type pathType;
        Conn::MinDistanceTraversal traversal;
        pathType = Conn::PathSelector::spantree();
        modes.insert(PathMode::unexpedited());
        location = network_->location(index);
        notifier()->traversalOrderIs(&traversal);
        paths = notifier()->paths(pathType,notifier()->endLocationTypes(),modes,
                                  priority_queue<PathPtr,vector<PathPtr>,Conn::TraversalCompare>(Conn::TraversalCompare(notifier())),
                                  NULL,location,NULL);
        // Add each path to routing table
        for(uint32_t i = 0;i < paths.size(); i++){
            PathPtr path = paths[i];
            EntityID endLocation = path->lastLocation()->name();
            EntityID segment = path->pathElement(0)->segment()->name();
            notifier()->nextHopIs(location->name(),endLocation,segment);
        }
    }
}
