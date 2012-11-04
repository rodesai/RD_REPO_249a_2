#include <stdlib.h>
#include <iostream>
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

void Segment::returnSegmentRm(){

    std::cout << "IN " << name() << " rm" << std::endl;

    returnSegment_=NULL;
}

void Segment::returnSegmentSet(Segment::Ptr returnSegment){
    returnSegment_=returnSegment;
}

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

void Stats::segmentCountDecr(Segment::EntityType type){
    if(segmentCount_.count(type) == 0){
        segmentCount_[type]=0;
    }
    else if(segmentCount_[type] > 0){
        segmentCount_[type] = segmentCount_[type]-1;
    }
}

void Stats::segmentCountIncr(Segment::EntityType type){
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

Segment::Ptr ShippingNetwork::SegmentNew(EntityID name, Segment::EntityType entityType){

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
        stats_->segmentCountIncr(segment->entityType());
        stats_->totalSegmentCountIncr();
        if(segment->expediteSupport() == segment->expediteSupported()){
            stats_->expediteSegmentCountIncr();
        }
    }
}

void StatsReactor::onSegmentDel(Segment::Ptr segment){
    stats_->segmentCountDecr(segment->entityType());
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
    return retval;
/*
    std::stack<Path::Ptr> pathStack;

    // Load Starting Paths
    for(uint32_t i = 0; i < start->segmentCount(); i++){
        Path::Ptr path = PathIs();
        Path::PathElement::Ptr start = PathElementIs(start->segment(i));
        path->elementEnq(start);
        pathStack.push(path);
    }

    while(pathStack.size() > 0){

        Path::Ptr currentPath = pathStack.top();

        // Evaluate Current Path
         
 
        // If the last location is null, ignore this path
        if(!currentPath->lastLocation()) continue;

        // If the path is cyclic, ignore this path
        

        // Evaluate constraints
        Conn::Constraint::EvalOutput evalOutput;
        for(ConstraintList::iterator it = constraints.begin(): it < constraints.end(); it++){
            it->pathIs(currentPath);
            evalOutput = it->evalOutput();
            if(evalOutput == Conn::Constraint::fail()) break;
        }
        if(evalOutput==Conn::Constraint::fail()) continue;

        if( endpoints.size() == 0 ){
            retval.push_back(path);
        }
        else if( endpoints.count(currentPath->lastLocation()->name()) != 0 ){
            retval.push_back(path);
            continue;
        }

        if(

        for(uint32_t i = 0; i < currentPath->
    }
*/
}

/*
 * Fleet
 *
 */

void Fleet::speedIs(Mode m, MilePerHour s){
    speed_[m]=s;
}

void Fleet::capacityIs(Mode m, PackageNum p){
    capacity_[m]=p;
}

void Fleet::costIs(Mode m, DollarPerMile d){
    cost_[m]=d;
}
