#include <stdlib.h>
#include <iostream>
#include <stack>
#include "engine/Engine.h"

using namespace Shipping;

/*
 * Location 
 *
 */

void Location::entityTypeIs(Location::EntityType et){
    entityType_=et;
}

void Location::segmentIs(std::string segmentID){

    // Make sure this segment is not already listed
    std::vector<std::string>::iterator it;
    for ( it=segments_.begin() ; it < segments_.end(); it++ ){
        if(*it == segmentID) return;
    }

    // Add the segment to the end of the list
    segments_.push_back(segmentID);
}

/* segmentDel(): Remove a Segment from this Location 
 */
void Location::segmentDel(std::string segmentID){
    // Find this segment and erase it from the list
    std::vector<std::string>::iterator it;
    for ( it=segments_.begin() ; it < segments_.end(); it++ ){
        if(*it == segmentID){
            segments_.erase(it);
            return;
        }
    }
}

/*
 * Segment 
 *
 */

void Segment::sourceIs(Location::Ptr source){

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

void Segment::returnSegmentIs(Segment::Ptr returnSegment){

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

void Segment::expediteSupportIs(Segment::ExpediteSupport expediteSupport){

    if(expediteSupport_==expediteSupport){
        return;
    }

    expediteSupport_=expediteSupport;

    // Call Notifiees
    Segment::NotifieeList::iterator it;
    for ( it=notifieeList_.begin(); it < notifieeList_.end(); it++ ){
        try{
            (*it)->onExpediteSupport();
        }
        catch(...){
            // ERROR: maybe we should log something here
        }
    }
}

void Segment::notifieeIs(Segment::Notifiee* notifiee){

    // Ensure idempotency
    std::vector<Segment::Notifiee::Ptr>::iterator it;
    for ( it=notifieeList_.begin(); it < notifieeList_.end(); it++ ){
        if( (*it) == notifiee ) return;
    }

    // Register this notiee
    notifiee->notifierIs(this);
    notifieeList_.push_back(notifiee);
}

void Segment::lengthIs(Mile length){
    length_=length;
}

void Segment::difficultyIs(Difficulty difficulty){
    difficulty_=difficulty;
}

/*
 * Path 
 *
 */

void Path::PathElement::segmentIs(Segment::Ptr segment){
    segment_=segment;
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

void Stats::expediteSegmentCountDecr(){
    if(expediteSegmentCount_ > 0) expediteSegmentCount_--; 
}

void Stats::expediteSegmentCountIncr(){
    expediteSegmentCount_++;
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

ShippingNetwork::Ptr ShippingNetwork::ShippingNetworkIs(EntityID name){

    // Construct the network
    ShippingNetwork::Ptr retval = new ShippingNetwork(name);

    // Initialize Singletons (fleet info, stats, conn objects)
    retval->connPtr_ = new Conn("The Conn");
    retval->fleetPtr_ = new Fleet("The Fleet");
    retval->statPtr_ = new Stats("The Stat");

    // Setup my reactors
    retval->notifieeIs(new StatsReactor(retval->statPtr_));
    retval->notifieeIs(new ShippingNetworkReactor());

    return retval;
}

void ShippingNetwork::notifieeIs(ShippingNetwork::Notifiee::Ptr notifiee){

    // Ensure idempotency
    ShippingNetwork::NotifieeList::iterator it;
    for ( it=notifieeList_.begin(); it < notifieeList_.end(); it++ ){
        if( (*it) == notifiee ) return;
    }

    // Register this notiee
    notifiee->notifierIs(this);
    notifieeList_.push_back(notifiee);
}

Segment::Ptr ShippingNetwork::SegmentNew(EntityID name, TransportMode entityType){

    // If Segment with this name already exists, just return it
    Segment::Ptr existing = segment(name);
    if(existing) return existing;

    // Create a New Segment
    Segment::Ptr retval(new Segment(name,entityType));
    segmentMap_[name]=retval;

    // Setup Reactor
    retval->notifieeIs(new SegmentReactor(this,statPtr_));

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

Segment::Ptr ShippingNetwork::segmentDel(EntityID name){

    Segment::Ptr retval;

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

Location::Ptr ShippingNetwork::LocationNew(EntityID name, Location::EntityType entityType){
    
    // If Segment with this name already exists, just return it
    Location::Ptr existing = location(name);
    if(existing) return existing;

    // Create a New Segment
    Location::Ptr retval(new Location(name,entityType));
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

Location::Ptr ShippingNetwork::locationDel(EntityID name){

    Location::Ptr retval;

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

Stats::Ptr ShippingNetwork::StatsNew(EntityID name){
    stat_[name] = statPtr_;
    return statPtr_;
}

Stats::Ptr ShippingNetwork::statsDel(EntityID name){
    StatMap::iterator it = stat_.find(name);
    if(it == stat_.end())
        return NULL;
    stat_.erase(it);
    return it->second;
}

Conn::Ptr ShippingNetwork::ConnNew(EntityID name){
    conn_[name] = connPtr_;
    return connPtr_;
}

Conn::Ptr ShippingNetwork::connDel(EntityID name){
    ConnMap::iterator it = conn_.find(name);
    if(it == conn_.end())
        return NULL;
    conn_.erase(it);
    return it->second;
}

Fleet::Ptr ShippingNetwork::FleetNew(EntityID name){
    fleet_[name]=fleetPtr_;
    return fleetPtr_;
}

Fleet::Ptr ShippingNetwork::fleetDel(EntityID name){
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

SegmentReactor::SegmentReactor(ShippingNetwork::Ptr network, Stats::Ptr stats){
    currentSource_ = NULL;
    currentReturnSegment_ = NULL;
    network_=network;
    stats_=stats;
}

void SegmentReactor::onSource(){
    // Remove the notifier from the old source
    if(currentSource_){
        currentSource_->segmentDel(notifier_->name());
    }
    // Update the source reference
    currentSource_ = notifier_->source();
    // Add the notifier to the new source
    if(currentSource_){
        currentSource_->segmentIs(notifier_->name());
    }
}

void SegmentReactor::onReturnSegment(){

    /* Remove this segment from old return segment if it still thinks
     * this reactor's segment is its return segment
     */
    if(currentReturnSegment_ && currentReturnSegment_->returnSegment() == notifier_){
        currentReturnSegment_->returnSegmentIs(NULL);
    }

    /* Update return segment ref */
    currentReturnSegment_ = notifier_->returnSegment();

    /* Update new return segment to set this segment as its return segment
     * if this segment is not already its return segment
     */
    if(currentReturnSegment_ && currentReturnSegment_->returnSegment() != notifier_){
        currentReturnSegment_->returnSegmentIs(notifier_);
    }
}

void SegmentReactor::onExpediteSupport(){
    if(notifier_->expediteSupport() == Segment::expediteSupported()){
        stats_->expediteSegmentCountIncr();
    }
    else if(notifier_->expediteSupport() == Segment::expediteUnsupported()){
        stats_->expediteSegmentCountDecr();
    }
}

/*
 * ShippingNetworkReactor
 * 
 */

ShippingNetworkReactor::ShippingNetworkReactor(){}

void ShippingNetworkReactor::onSegmentDel(Segment::Ptr segment){
    // Clean up this Segment's source
    segment->sourceIs(NULL);
    // Clean up this Segment's return segment
    segment->returnSegmentIs(NULL);
}

void ShippingNetworkReactor::onLocationDel(Location::Ptr location){
    // Clean up this Location from all its Segments
    for(uint32_t i = 0;i < location->segmentCount(); i++){
        Segment::Ptr segment = notifier_->segment(location->segmentID(i));
        segment->sourceIs(NULL);
    }
}

/*
 * StatsReactor
 *
 */

StatsReactor::StatsReactor(Stats::Ptr stats){
    stats_=stats;
}

void StatsReactor::onSegmentNew(EntityID segmentID){
    Segment::Ptr segment = notifier_->segment(segmentID);
    if(segment){
        stats_->segmentCountIncr(segment->mode());
        stats_->totalSegmentCountIncr();
        if(segment->expediteSupport() == segment->expediteSupported()){
            stats_->expediteSegmentCountIncr();
        }
    }
}

void StatsReactor::onSegmentDel(Segment::Ptr segment){
    stats_->segmentCountDecr(segment->mode());
    stats_->totalSegmentCountDecr();
    if(segment->expediteSupport() == segment->expediteSupported()){
        stats_->expediteSegmentCountDecr();
    }
}

void StatsReactor::onLocationNew(EntityID locationID){
    Location::Ptr location = notifier_->location(locationID);
    stats_->locationCountIncr(location->entityType());    
}

void StatsReactor::onLocationDel(Location::Ptr location){
    stats_->locationCountDecr(location->entityType());
}

/*
 * Conn
 * 
 */
Conn::PathList Conn::paths(ConstraintList constraints,LocationSet endpoints,
                       Location::Ptr start, ShippingNetwork* network,Fleet* fleet){

    Conn::PathList retval;

    std::stack<Path::Ptr> pathStack;

    // Load Starting Paths
    for(uint32_t i = 1; i <= start->segmentCount(); i++){
        Path::Ptr path = Path::PathIs(fleet);
        std::cout << "Adding segment: " << start->segmentID(i) << std::endl;
        Path::PathElement::Ptr startElement = Path::PathElement::PathElementIs(network->segment(start->segmentID(i)));
        path->pathElementEnq(startElement);
        pathStack.push(path);
    }

    std::cout << "Starting stack size: " << pathStack.size() << std::endl;
    std::cout << "Starting location: " << start->name() << std::endl;

    while(pathStack.size() > 0){

        sleep(1);

        Path::Ptr currentPath = pathStack.top();
        pathStack.pop();

        std::cout << "Visiting location: " << currentPath->lastLocation()->name() << std::endl;
        std::cout << "path: " << currentPath.ptr() << std::endl;

        /* Evaluate Current Path */

        // Evaluate constraints
        Conn::Constraint::EvalOutput evalOutput = Conn::Constraint::pass();
        for(ConstraintList::iterator it = constraints.begin(); it < constraints.end(); it++){
            (*it)->pathIs(currentPath);
            evalOutput = (*it)->evalOutput();
            if(evalOutput == Conn::Constraint::fail()){
                break;
            }
        }
        if(evalOutput==Conn::Constraint::fail()){
            std::cout << "Failed to pass constraints, discarding path" << std::endl;
            continue;
        }

        if( endpoints.size() == 0 ){
            retval.push_back(currentPath);
        }
        else if( endpoints.count(currentPath->lastLocation()->name()) != 0 ){
            std::cout << "Found an endpoint, terminate path" << std::endl;
            retval.push_back(currentPath);
            continue;
        }

        // Iterate over next set of segments
        for(uint32_t i = 1; i <= currentPath->lastLocation()->segmentCount(); i++){

            EntityID segmentID = currentPath->lastLocation()->segmentID(i);
            Segment::Ptr segment = network->segment(segmentID);
            Location::Ptr destination = segment->returnSegment()->source();

            std::cout << "Destination of potential path: " << destination->name() << std::endl;

            // If the segment has a valid end point that doesnt cause a cycle, push onto stack
            if( destination
                && !(currentPath->location(destination))){
                Path::Ptr pathCopy = Path::PathIs(currentPath);
                pathCopy->pathElementEnq(Path::PathElement::PathElementIs(segment));
                pathStack.push(pathCopy);
            }
            else{
                std::cout << "Found a looped path, discard" << std::endl;
            }
        }
    }

    return retval;
}

std::vector<Path::Ptr> Conn::connect(Location::Ptr start, Location::Ptr end, ShippingNetwork* network, Fleet* fleet){
    Conn::LocationSet ls;
    ls.insert(end->name());
    return paths(Conn::ConstraintList(),ls,start,network,fleet);
}

/*
 * Fleet
 *
 */

void Fleet::speedIs(TransportMode m, MilePerHour s){
    speed_[m]=s;
}

void Fleet::capacityIs(TransportMode m, PackageNum p){
    capacity_[m]=p;
}

void Fleet::costIs(TransportMode m, DollarPerMile d){
    cost_[m]=d;
}

/*
 * Path
 *
 */

Path::Ptr Path::PathIs(Fleet::Ptr fleet){
    return new Path(fleet.ptr());
}

Path::Ptr Path::PathIs(Path::Ptr path){
    return new Path(path.ptr());
}

Path::Path(Path* path) : cost_(0),time_(0),distance_(0),expedited_(Segment::expediteSupported()){
    fleet_ = path->fleet();
    for(uint32_t i = 0; i < path->pathElementCount(); i++){
        Path::PathElement::Ptr elementCpy = Path::PathElement::PathElementIs(path->pathElement(i));
        pathElementEnq(elementCpy);
    }
}

Path::Path(Fleet* fleet) : cost_(0),time_(0),distance_(0),expedited_(Segment::expediteSupported()){
    fleet_ = fleet; 
}

Path::PathElement::Ptr Path::PathElement::PathElementIs(Segment::Ptr segment){
    return new Path::PathElement(segment);
}

Path::PathElement::Ptr Path::PathElement::PathElementIs(Path::PathElement::Ptr pathElement){
    return new Path::PathElement(pathElement.ptr());
}

Path::PathElement::Ptr Path::pathElement(uint32_t index){
    if(index >= path_.size()) return NULL;
    return path_[index];
}

void Path::pathElementEnq(Path::PathElement::Ptr element){
    /* Add Element */
    path_.push_back(element);
    /* Update Metadata */
    Difficulty difficulty = element->segment()->difficulty();
    Mile length = element->segment()->length();
    //TODO
    TransportMode mode = element->segment()->mode();
    // Update cost
    cost_ = cost_.value() + difficulty.value()*length.value()*(fleet_->cost(element->segment()->mode())).value();
    // Update time
    time_ = time_.value() + length.value()*(fleet_->speed(mode)).value();
    // Update distance
    distance_ = distance_.value() + length.value();
    // Update expedite status
    if(expedited_ == Segment::expediteSupported() && element->segment()->expediteSupport() == Segment::expediteUnsupported()){
        expedited_ = Segment::expediteUnsupported();
    }

    std::cout << "Adding source to segment set: " << element->segment()->source()->name() << " ptr: " << element->segment()->source().ptr() << std::endl;
    locations_.insert(element->segment()->source()->name());
    std::cout << "Adding dst to segment set: " << element->segment()->returnSegment()->source()->name() << " ptr: " << element->segment()->returnSegment()->source().ptr() << std::endl;
    locations_.insert(element->segment()->returnSegment()->source()->name());
}

Location::Ptr Path::lastLocation(){
    if(path_.size() == 0) return NULL;
    Segment::Ptr lastReturnSegment = path_.back()->segment()->returnSegment();
    if(!lastReturnSegment) return NULL;
    return lastReturnSegment->source();
}

Location::Ptr Path::location(Location::Ptr location){
    if(locations_.count(location->name()) == 0){ return NULL; }
    return location;
}
