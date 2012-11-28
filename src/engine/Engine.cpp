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

Location::Location(EntityID name, EntityType type): 
    Fwk::NamedInterface(name), entityType_(type){}

int uniqueInt = 0;

string uniqueName() {
    stringstream s;
    s << uniqueInt;
    uniqueInt += 1;
    return s.str();
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
    if (spd == transferRate_)
        return;

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

    Time shipmentFrequency = 24.0/(static_cast<double>(transferRate().value()));
    return manager_->now().value()+shipmentFrequency.value();

/*    // If we have already sent the full load for today then dont send any more 
    if(shipmentsSentToday_ >= transferRate().value()){
        return (static_cast<uint32_t>(manager_->now().value())/24)*24.0 + 24.0;
    }

    // Shipments remaining to send
    uint64_t shipmentsLeft = transferRate().value()-shipmentsSentToday_;
    // Hours left in the day
    Time hoursLeft = (static_cast<uint32_t>(manager_->now().value())/24)*24.0 + 24.0 - manager_->now().value();
    Time nextShipmentOffset = hoursLeft.value()/shipmentsLeft;
    return manager_->now().value() + nextShipmentOffset.value();
*/
}

void Customer::shipmentSizeIs(PackageNum pn) {
    if (shipmentSize_ == pn)
        return;

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
void Customer::destinationIs(LocationPtr lp) {
    // ignore request if nothing has changed
    if (destination_)
        if (lp->name() == destination_->name())
            return;

    // ignore the request if the destination is not a customer
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

    DEBUG_LOG << "Shipment " << shipment->name() << " arrived at customer " << notifier()->name() 
              << " from customer " << shipment->source()->name() << " @ " << manager_->now().value() << std::endl;

    // if shipment is arriving at destination, udpate stats
    if (shipment->destination()->name() == notifier_->name()) {
        CustomerPtr cust = dynamic_cast<Customer*> (shipment->destination().ptr());
        DEBUG_LOG << "  Customer is destination; updating stats: \n";
        DEBUG_LOG << "     latency: " << Hour(manager_->now().value() - shipment->startTime().value()).value() << std::endl;
        cust->totalLatency_ = Hour(cust->totalLatency_.value() + manager_->now().value() - shipment->startTime().value());
        cust->totalCost_ = cust->totalCost_ + shipment->cost();
        cust->shipmentsReceived_ ++;
        return;
    }

    // otherwise, if arriving at the source, forward activity to segment
    else if (shipment->source()->name() == notifier_->name()) {
        EntityID nextSegmentName = network_->conn()->nextHop(notifier_->name(), shipment->destination()->name());
        DEBUG_LOG << "Customer forwarding shipment to: " << nextSegmentName << std::endl;
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
    // TODO: if transferRate is changed, we should update this
    if (!(transferRateSet_ && shipmentSizeSet_ && destinationSet_)) return;

    DEBUG_LOG << "Criteria fully specified. Setting up shipment injection activity...\n";
    CustomerPtr cust = dynamic_cast<Customer*>(const_cast<Location*> (notifier_.ptr()));
  
    // Clear the old activity
    Activity::ActivityPtr activity = manager_->activity(notifier_->name());
    if(activity){
        activity->statusIs(Activity::Activity::cancelled());
        manager_->activityDel(activity->name());
    }

    activity = manager_->activityNew(notifier_->name());
    InjectActivityReactor* iar = new InjectActivityReactor();
    iar->managerIs(manager_);
    iar->sourceIs(cust);
    activity->lastNotifieeIs(iar);
    activity->nextTimeIs(cust->nextShipmentTime());
    activity->statusIs(Activity::Activity::nextTimeScheduled());
    manager_->lastActivityIs(activity);
}

void InjectActivityReactor::onStatus() {
    if (notifier_->status() == Activity::Activity::executing()) {
        ShipmentPtr shipment = new Shipment(uniqueName());
        DEBUG_LOG << "Creating shipment " << shipment->name() << " for source " << source_->name() << " @ " << manager_->now().value() << "\n";
        shipment->loadIs(source_->shipmentSize());
        shipment->sourceIs(source_);
        shipment->destinationIs(source_->destination());
        shipment->startTimeIs(manager_->now());
        // add shipment to location
        source_->shipmentIs(shipment);
    }
    else if(notifier_->status() == Activity::Activity::free()){
        DEBUG_LOG << source_->name() << " next shipment @ " << source_->nextShipmentTime().value() << ".\n";
        notifier_->nextTimeIs(source_->nextShipmentTime());
        notifier_->statusIs(Activity::Activity::nextTimeScheduled());
        source_->manager()->lastActivityIs(notifier_);
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
    // ensure idempotency
    if (capacity_ == capacity)
        return;

    capacity_ = capacity;

    // Call Notifiees
    Segment::NotifieeList::iterator it;
    for ( it=notifieeList_.begin(); it < notifieeList_.end(); it++ ){
        try{
            (*it)->onCapacity();
        }
        catch(...){
            // ERROR: maybe we should log something here
        }
    }
}

void Segment::difficultyIs(Difficulty difficulty){
    difficulty_=difficulty;
}

void Segment::shipmentIs(ShipmentPtr shipment) {

    DEBUG_LOG << "Shipment " << shipment->name() << " arrived at segment " << this->name() << std::endl; 

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
    SubshipmentPtr lastSubshipment = subshipmentQueue_.front();

    // remove subshipment if remaining packages can be delivered at once
    if (capacity >= lastSubshipment->remainingLoad()) {
        subshipmentQueue_.pop();
        return lastSubshipment;
    }

    // otherwise return partial shipment
    SubshipmentPtr result = new Subshipment("name");
    lastSubshipment->remainingLoadIs(lastSubshipment->remainingLoad() - capacity);
    result->shipmentIs(lastSubshipment->shipment());
    result->remainingLoadIs(capacity);
    return result;

}

Hour Segment::carrierLatency() const {
    // TODO: can make this more general (include difficulty, expedite)
    return Hour(length_.value() / network_->activeFleet()->speed(transportMode_).value());
}

PackageNum Segment::carrierCapacity() const {
    return network_->activeFleet()->capacity(transportMode_);
}

Dollar Segment::carrierCost() const {
    // TODO: can make this more general
    return Dollar(length_.value() * network_->activeFleet()->cost(transportMode_).value());
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

ShippingNetworkPtr ShippingNetwork::ShippingNetworkIs(EntityID name, ManagerPtr manager){

    // Construct the network
    ShippingNetworkPtr retval = new ShippingNetwork(name,manager);

    // Initialize Singletons (fleet info, stats, conn objects)
    retval->fleetPtr_ = retval->createFleetAndReactor("The Fleet");
    retval->statPtr_ = new Stats("The Stat");
    retval->connPtr_ = new Conn("The Conn",retval);
    retval->connPtr_->notifieeIs(new RoutingReactor(retval));

    // Setup my reactors
    retval->notifieeIs(new StatsReactor(retval->statPtr_));
    retval->notifieeIs(new ShippingNetworkReactor());

    return retval;
}

void ShippingNetwork::activeFleetIs(FleetPtr fleet) {
    // TODO: should we take the entityID as a parameter instead?
    DEBUG_LOG << "Active fleet is now " << fleet->name() << "\n";
    fleetPtr_ = fleet;
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

// TODO: this function could be absorbed into the constructor for Fleet
FleetPtr ShippingNetwork::createFleetAndReactor(EntityID name) {
    FleetPtr fleet = new Fleet(name);
    FleetReactor* fr = new FleetReactor();
    fr->managerIs(manager_);
    fr->networkIs(this);
    fleet->notifieeIs(fr);
    return fleet;
}

FleetPtr ShippingNetwork::FleetNew(EntityID name){
    // if entity name already exists, throw error
    if(fleet_.count(name) != 0){
        throw EntityExistsException();
    }

    // if no fleet is defined for user, return the default
    if(fleet_.size() == 0) {
        DEBUG_LOG << "Returning default for " << name << "\n";
        fleet_[name] = fleetPtr_;
    }

    // otherwise, create a new fleet
    else fleet_[name] = createFleetAndReactor(name);

    return fleet_[name];
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

void SegmentReactor::onCapacity() {
    // start up activities as needed
    startupFAR();
}

void SegmentReactor::onShipment(ShipmentPtr shipment) {

    DEBUG_LOG << "Segment reactor notified of new shipment.\n";

    // increase reject count if there are no available carriers
    SegmentPtr segment = const_cast<Segment*> (notifier_.ptr());
    if (segment->carriersUsed() >= segment->capacity().value()) {
        DEBUG_LOG << "Segment " << notifier()->name() << " refusing shipment " << shipment->name() << std::endl;
        segment->shipmentsRefusedInc();
    }

    // otherwise, start up a new FAR
    else {
        startupFAR();
    }
}

void SegmentReactor::startupFAR() {
    SegmentPtr segment = const_cast<Segment*> (notifier_.ptr());
    while (segment->carriersUsed() < segment->capacity().value() && !segment->subshipmentQueue_.empty()) {
        DEBUG_LOG << "Creating new ForwardActivityReactor...\n";
        // create new activity and activity reactor
        Activity::ActivityPtr fa = manager_->activityNew();
        ForwardActivityReactor* far = new ForwardActivityReactor();
        far->managerIs(manager_);
        far->segmentIs(segment);
        fa->lastNotifieeIs(far);

        // increment the number of carriers used
        segment->carriersUsedInc();

        // schedule activity to pick up new subshipment
        fa->statusIs(Activity::Activity::free());
    }

    // DEBUGGING
    if (segment->carriersUsed() >= segment->capacity().value())
        DEBUG_LOG << "Using all " << segment->carriersUsed()<< " carriers.\n";
    if (segment->subshipmentQueue_.empty())
        DEBUG_LOG << "No more subshipments.\n";
}

void ForwardActivityReactor::onStatus() {
    if (notifier_->status() == Activity::Activity::executing()) {
        DEBUG_LOG << "Delivering subshipment at time " << manager_->now().value() << "\n";
        subshipment_->shipment()->costInc(segment_->carrierCost());
        segment_->deliveryMap_[subshipment_->shipment()->name()] += subshipment_->remainingLoad().value();

        if (segment_->deliveryMap_[subshipment_->shipment()->name()] == subshipment_->shipment()->load().value()) {
            DEBUG_LOG << "  Shipment " << subshipment_->shipment()->name() << " is complete.\n";
            segment_->returnSegment()->source()->shipmentIs(subshipment_->shipment());
            segment_->deliveryMap_.erase(segment_->deliveryMap_.find(subshipment_->shipment()->name()));
        }
    }

    else if (notifier_->status() == Activity::Activity::free()) {
        // reschedule activity if there is another subshipment left and not exceeding carriers
        if (segment_->carriersUsed() <= segment_->capacity().value()) {
            subshipment_ = segment_->subshipmentDequeue(segment_->carrierCapacity());

            if (subshipment_) {
                DEBUG_LOG << "  Picking up new subshipment for shipment "<< subshipment_->shipment()->name()<<"\n";
                notifier_->statusIs(Activity::Activity::nextTimeScheduled());
                notifier_->nextTimeIs(Time(manager_->now().value() + segment_->carrierLatency().value()));
                DEBUG_LOG << "  Shipment to be delivered at time " << (double)notifier_->nextTime().value() << ".\n";
                manager_->lastActivityIs(notifier_);
                if (segment_->deliveryMap_.find(subshipment_->shipment()->name()) == segment_->deliveryMap_.end()) {
                    DEBUG_LOG << "  Shipment is starting.\n";
                    segment_->shipmentsReceivedInc();
                    segment_->deliveryMap_[subshipment_->shipment()->name()] = 0;
                }

                return;
            }
        }

        // otherwise, delete activity
        manager_->activityDel(notifier_->name());
        segment_->carriersUsedDec();
    }
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

void Conn::supportedRouteModeIs(uint32_t index, PathMode mode){
    supportedRouteModes_[index].insert(mode);
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
    DEBUG_LOG << "Time for segment is " << time.value() << "\n";
    DEBUG_LOG << "Transport mode speed is " << fleet->speed(pathElement->segment()->transportMode()).value() << "\n";
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

        if( currentPath->pathElementCount() > 0 && 
            endLocationTypes.count(currentPath->lastLocation()->entityType()) > 0
          )
        {
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
                      PathPtr pathCopy = copyPath(currentPath,shippingNetwork_->activeFleet());
                      pathElementEnque(Path::PathElement::PathElementIs(segment,*it),pathCopy,shippingNetwork_->activeFleet());
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

void Fleet::notifieeIs(Fleet::Notifiee* notifiee){

    // Ensure idempotency
    std::vector<Fleet::NotifieePtr>::iterator it;
    for ( it=notifieeList_.begin(); it < notifieeList_.end(); it++ ){
        if( (*it) == notifiee ) return;
    }

    // Register this notifiee
    notifiee->notifierIs(this);
    notifieeList_.push_back(notifiee);
}

void Fleet::startTimeIs(Time startTime){
    if (startTime_ == startTime && startTimeSet_)
        return;

    startTimeSet_ = true;
    startTime_ = startTime;

    // Call Notifiees
    Fleet::NotifieeList::iterator it;
    for ( it=notifieeList_.begin(); it < notifieeList_.end(); it++ ){
        try{
            (*it)->onStartTime();
        }
        catch(...){
            // ERROR: maybe we should log something here
        }
    }    
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

void FleetReactor::onStartTime() {
    DEBUG_LOG << "Fleet reactor notified of start time set for "<< notifier_->name()<< ".\n";

    // if activity exists already, clear the old activity
    Activity::ActivityPtr activity = manager_->activity(notifier_->name());
    if (activity) {
        activity->statusIs(Activity::Activity::cancelled());
        manager_->activityDel(activity->name());
    }

    // create new activity
    activity = manager_->activityNew(notifier_->name());
    FleetChangeActivityReactor* fcar = new FleetChangeActivityReactor();
    fcar->managerIs(manager_);
    fcar->networkIs(const_cast<ShippingNetwork*> (network_.ptr()));
    fcar->fleetIs(const_cast<Fleet*> (notifier_.ptr()));
    activity->lastNotifieeIs(fcar);

    // find the next time the schedule would change
    float currTime = manager_->now().value();
    uint32_t days = currTime / 24;
    if (currTime - days * 24 > notifier_->startTime().value())
        days += 1;
    DEBUG_LOG << "Change scheduled for hour: " << days * 24 + notifier_->startTime().value() << "\n";

    // set the activity
    activity->nextTimeIs(Time(days * 24 + notifier_->startTime().value()));
    activity->statusIs(Activity::Activity::nextTimeScheduled());
    manager_->lastActivityIs(activity);
}

void FleetChangeActivityReactor::onStatus() {
    if (notifier_->status() == Activity::Activity::executing()) {
        DEBUG_LOG << "Changing fleet to " << fleet_->name() << "\n";
        network_->activeFleetIs(fleet_);
    }

    else if(notifier_->status() == Activity::Activity::free()){
        // schedule the activity one day later
        notifier_->nextTimeIs(Time(manager_->now().value() + 24));
        notifier_->statusIs(Activity::Activity::nextTimeScheduled());
        manager_->lastActivityIs(notifier_);
    }
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
        Conn::MinHopTraversal traversal;
        initRoutingTable(&traversal);
    }
    else if(algo == Conn::minDistance()){
        Conn::MinDistanceTraversal traversal;
        initRoutingTable(&traversal);
    }
}

void RoutingReactor::initRoutingTable(Conn::TraversalOrder* traversal){
    // Iterate over each location and entries for it to the route table
    uint32_t index;
    for(index = 0; index < network_->locationCount(); index++){
        LocationPtr location = network_->location(index);
        // Use to merge paths across path modes
        std::map<EntityID,PathPtr> pathsUsed;
        Conn::PathList finalPaths;
        Conn::ModeCollection::iterator modeIt;
        for(modeIt = notifier()->supportedRouteModes_.begin(); modeIt != notifier()->supportedRouteModes_.end(); modeIt++){
            std::set<PathMode> modes;
            Conn::PathList paths;
            Conn::PathSelector::Type pathType;
            pathType = Conn::PathSelector::spantree();
            modes = modeIt->second;
            notifier()->traversalOrderIs(traversal);
            paths = notifier()->paths(pathType,notifier()->endLocationTypes(),modes,
                                  priority_queue<PathPtr,vector<PathPtr>,Conn::TraversalCompare>(Conn::TraversalCompare(notifier())),
                                  NULL,location,NULL);
            // Merge Paths
            for(uint32_t i = 0; i < paths.size(); i++){
                PathPtr path = paths[i];
                if( pathsUsed.count(path->lastLocation()->name()) == 0
                    || notifier()->traversalOrder()->compare(path,pathsUsed[path->lastLocation()->name()])
                  )
                {
                    pathsUsed[path->lastLocation()->name()] = path;
                }
            }
        }
        // Convert Paths Used to a routing table
        std::map<EntityID,PathPtr>::iterator pathsUsedIt;
        for(pathsUsedIt = pathsUsed.begin(); pathsUsedIt != pathsUsed.end(); pathsUsedIt++){
            DEBUG_LOG << "Found path from " << location->name() << "to " << pathsUsedIt->first << std::endl;
            notifier()->nextHopIs(location->name(),pathsUsedIt->first,pathsUsedIt->second->pathElement(0)->segment()->name());
        }
    }
}

