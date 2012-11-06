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

/*
 * Segment 
 *
 */

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
    std::vector<Segment::NotifieePtr>::iterator it;
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

void Path::PathElement::segmentIs(SegmentPtr segment){
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

ShippingNetworkPtr ShippingNetwork::ShippingNetworkIs(EntityID name){

    // Construct the network
    ShippingNetworkPtr retval = new ShippingNetwork(name);

    // Initialize Singletons (fleet info, stats, conn objects)
    retval->fleetPtr_ = new Fleet("The Fleet");
    retval->statPtr_ = new Stats("The Stat");
    retval->connPtr_ = new Conn("The Conn",retval,retval->fleetPtr_);

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

SegmentPtr ShippingNetwork::SegmentNew(EntityID name, TransportMode entityType){

    // If Segment with this name already exists, just return it
    SegmentPtr existing = segment(name);
    if(existing) return existing;

    // Create a New Segment
    SegmentPtr retval(new Segment(this,name,entityType));
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
    if(existing) return existing;

    // Create a New Segment
    LocationPtr retval(new Location(name,entityType));
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
        currentSource_->segmentDel(notifier_);
    }
    // Update the source reference
    currentSource_ = notifier_->source();
    // Add the notifier to the new source
    if(currentSource_){
        currentSource_->segmentIs(notifier_);
    }
}

void SegmentReactor::onReturnSegment(){

    /* Remove this segment from old return segment if it still thinks
     * this reactor's segment is its return segment
     */
    if(currentReturnSegment_ && currentReturnSegment_->returnSegment() == notifier_){
        currentReturnSegment_->returnSegmentIs((SegmentPtr)NULL);
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

void ShippingNetworkReactor::onSegmentDel(SegmentPtr segment){
    // Clean up this Segment's source
    segment->sourceIs((LocationPtr)NULL);
    // Clean up this Segment's return segment
    segment->returnSegmentIs((SegmentPtr)NULL);
}

void ShippingNetworkReactor::onLocationDel(LocationPtr location){
    // Clean up this Location from all its Segments
    for(uint32_t i = 0;i < location->segmentCount(); i++){
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
    SegmentPtr segment = notifier_->segment(segmentID);
    if(segment){
        stats_->segmentCountIncr(segment->mode());
        stats_->totalSegmentCountIncr();
        if(segment->expediteSupport() == segment->expediteSupported()){
            stats_->expediteSegmentCountIncr();
        }
    }
}

void StatsReactor::onSegmentDel(SegmentPtr segment){
    stats_->segmentCountDecr(segment->mode());
    stats_->totalSegmentCountDecr();
    if(segment->expediteSupport() == segment->expediteSupported()){
        stats_->expediteSegmentCountDecr();
    }
}

void StatsReactor::onLocationNew(EntityID locationID){
    LocationPtr location = notifier_->location(locationID);
    stats_->locationCountIncr(location->entityType());    
}

void StatsReactor::onLocationDel(LocationPtr location){
    stats_->locationCountDecr(location->entityType());
}

/*
 * Conn
 * 
 */

Conn::PathList Conn::paths(ConstraintPtr constraints,EntityID start, EntityID end) {

    LocationPtr startPtr = shippingNetwork_->location(start);
    LocationPtr endPtr = shippingNetwork_->location(end);
    if(startPtr){
        return paths(fleet_,constraints,startPtr,endPtr);
    }
    return PathList();
}

Conn::PathList Conn::paths(ConstraintPtr constraints,EntityID start) {

    LocationPtr startPtr = shippingNetwork_->location(start);
    if(startPtr){
        return paths(fleet_,constraints,startPtr,NULL);
    }
    return PathList();
}

bool validSegment(SegmentPtr segment){
    return (segment && segment->source() && segment->returnSegment() && segment->returnSegment()->source());
}

Conn::PathList Conn::paths(FleetPtr fleet, ConstraintPtr constraints,LocationPtr start, LocationPtr endpoint) const {

    Conn::PathList retval;

    std::stack<PathPtr> pathStack;

    // Load Starting Paths
    for(uint32_t i = 1; i <= start->segmentCount(); i++){
        if(validSegment(start->segment(i))){
            PathPtr path = Path::PathIs(fleet);
            DEBUG_LOG << "Adding segment: " << start->segment(i)->name() << std::endl;
            Path::PathElementPtr startElement = Path::PathElement::PathElementIs(start->segment(i));
            path->pathElementEnq(startElement);
            pathStack.push(path);
        }
    }

    DEBUG_LOG << "Starting stack size: " << pathStack.size() << std::endl;
    DEBUG_LOG << "Starting location: " << start->name() << std::endl;

    while(pathStack.size() > 0){

        PathPtr currentPath = pathStack.top();
        pathStack.pop();

        DEBUG_LOG << "Visiting location: " << currentPath->lastLocation()->name() << std::endl;
        DEBUG_LOG << "path: " << currentPath.ptr() << std::endl;

        /* Evaluate Current Path */

        // Evaluate constraints
        Conn::Constraint::EvalOutput evalOutput = Conn::Constraint::pass();
        ConstraintPtr constraint = constraints;
        while(constraint){ 
            constraint->pathIs(currentPath);
            evalOutput = constraint->evalOutput();
            if(evalOutput == Conn::Constraint::fail()){
                break;
            }
            constraint=constraint->next();
        }
        if(evalOutput==Conn::Constraint::fail()){
            DEBUG_LOG << "Failed to pass constraints, discarding path" << std::endl;
            continue;
        }

        // Should we output the segment?
        if( !endpoint || endpoint->name() == currentPath->lastLocation()->name() ){
            retval.push_back(currentPath);
        }

        /* Iterate over next set of segments
         */
        // Only continue iterating if we havent the endpoint (THIS IS AN OPTIMIZATION)
        if(!endpoint || endpoint->name() != currentPath->lastLocation()->name()){
        for(uint32_t i = 1; i <= currentPath->lastLocation()->segmentCount(); i++){
            SegmentPtr segment = currentPath->lastLocation()->segment(i); 
            if(validSegment(segment)){
                LocationPtr destination = segment->returnSegment()->source();
                DEBUG_LOG << "Destination of potential path: " << destination->name() << std::endl;
                // If the segment has a valid end point that doesnt cause a cycle, push onto stack
                if( destination
                    && !(currentPath->location(destination))){
                    PathPtr pathCopy = Path::PathIs(currentPath);
                    pathCopy->pathElementEnq(Path::PathElement::PathElementIs(segment));
                    pathStack.push(pathCopy);
                }
                else{
                    DEBUG_LOG << "Found a looped path, discard" << std::endl;
                }
            }
        }
        }
    }

    return retval;
}

/*
std::vector<PathPtr> Conn::connect(LocationPtr start, LocationPtr end, ShippingNetwork* network, Fleet* fleet) const {
    Conn::LocationSet ls;
    ls.insert(end->name());
    return paths(NULL,ls,start,network,fleet);
}

std::vector<PathPtr> explore(LocationPtr start,
                                       Mile distance, Dollar cost, Hour time, Segment::ExpediteSupport expedited
                                       ShippingNetwork* network, Fleet* fleet) const;
*/
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

PathPtr Path::PathIs(FleetPtr fleet){
    return new Path(fleet.ptr());
}

PathPtr Path::PathIs(PathPtr path){
    return new Path(path.ptr());
}

Path::Path(Path* path) : cost_(0),time_(0),distance_(0),expedited_(Segment::expediteSupported()){
    fleet_ = path->fleet();
    for(uint32_t i = 0; i < path->pathElementCount(); i++){
        Path::PathElementPtr elementCpy = Path::PathElement::PathElementIs(path->pathElement(i));
        pathElementEnq(elementCpy);
    }
}

Path::Path(Fleet* fleet) : cost_(0),time_(0),distance_(0),expedited_(Segment::expediteSupported()){
    fleet_ = fleet; 
}

Path::PathElementPtr Path::PathElement::PathElementIs(SegmentPtr segment){
    return new Path::PathElement(segment);
}

Path::PathElementPtr Path::PathElement::PathElementIs(Path::PathElementPtr pathElement){
    return new Path::PathElement(pathElement.ptr());
}

Path::PathElementPtr Path::pathElement(uint32_t index){
    if(index >= path_.size()) return NULL;
    return path_[index];
}

void Path::pathElementEnq(Path::PathElementPtr element){

    /* Add Element */
    path_.push_back(element);
    /* Update Metadata */
    Difficulty difficulty = element->segment()->difficulty();
    Mile length = element->segment()->length();
    //TODO
    TransportMode mode = element->segment()->mode();
    // Update cost
    cost_ = cost_.value() + difficulty.value()*length.value()*(fleet_->cost(mode)).value();
    // Update time
    time_ = time_.value() + (length.value())/((fleet_->speed(mode)).value());
    // Update distance
    distance_ = distance_.value() + length.value();
    // Update expedite status
    if(expedited_ == Segment::expediteSupported() && element->segment()->expediteSupport() == Segment::expediteUnsupported()){
        expedited_ = Segment::expediteUnsupported();
    }

    DEBUG_LOG << "Adding source to segment set: " << element->segment()->source()->name() << " ptr: " << element->segment()->source().ptr() << std::endl;
    locations_.insert(element->segment()->source()->name());
    DEBUG_LOG << "Adding dst to segment set: " << element->segment()->returnSegment()->source()->name() << " ptr: " << element->segment()->returnSegment()->source().ptr() << std::endl;
    locations_.insert(element->segment()->returnSegment()->source()->name());
}

LocationPtr Path::lastLocation(){
    if(path_.size() == 0) return NULL;
    SegmentPtr lastReturnSegment = path_.back()->segment()->returnSegment();
    if(!lastReturnSegment) return NULL;
    return lastReturnSegment->source();
}

LocationPtr Path::location(LocationPtr location){
    if(locations_.count(location->name()) == 0){ return NULL; }
    return location;
}
