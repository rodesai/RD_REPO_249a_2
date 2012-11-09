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

uint32_t Location::segmentCount() const { 
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

/*
 * Segment 
 *
 */

uint16_t Segment::modeCount() const{
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

void Segment::difficultyIs(Difficulty difficulty){
    difficulty_=difficulty;
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
    if(existing) return existing;

    // Create a New Segment
    SegmentPtr retval(new Segment(this,name,entityType,pathMode));
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
    SegmentPtr segment = notifier()->segment(segmentID);
    if(segment){
        stats_->totalSegmentCountIncr();
        stats_->segmentCountIncr(segment->transportMode());
        for(uint16_t i = 0; i < segment->modeCount(); i++){
            stats_->segmentCountIncr(segment->mode(i));
        }
    }
}

void StatsReactor::onSegmentDel(SegmentPtr segment){
    stats_->totalSegmentCountDecr();
    stats_->segmentCountDecr(segment->transportMode());
    for(uint16_t i = 0; i < segment->modeCount(); i++){
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

void Conn::PathSelector::modeIs(PathMode mode){
    pathModes_.insert(mode);
}

PathMode Conn::PathSelector::modeDel(PathMode mode){
    if(pathModes_.count(mode) == 0) return PathMode::undef();
    pathModes_.erase(mode);
    return mode;
}
 
Conn::PathList Conn::paths(Conn::PathSelector selector) const {
    LocationPtr startPtr = selector.start();
    LocationPtr endPtr = selector.end();
    // Make Sure endPtr is valid Location in my network
    if(endPtr && shippingNetwork_->location(endPtr->name()) != endPtr){
        return PathList();
    }
    // Make sure startPtr is valid Location in my network
    if(!startPtr || (startPtr && shippingNetwork_->location(startPtr->name()) != startPtr)) {
        return PathList();
    }
    return paths(selector.modes(), selector.constraints(),startPtr,endPtr); 
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
           * (fleet->costMultiplier(pathElement->mode())).value()
           * (pathElement->segment()->difficulty()).value();
    time = (pathElement->segment()->length()).value() 
           / ( (fleet->speed(pathElement->segment()->transportMode()).value()) * ((fleet->speedMultiplier(pathElement->mode())).value()));
    path->pathElementEnq(pathElement,cost,time,pathElement->segment()->length());
    return path;
}

PathPtr Conn::copyPath(PathPtr path, FleetPtr fleet) const {
    PathPtr copy = Path::PathIs(path->firstLocation());
    for(uint32_t i = 0; i < path->pathElementCount(); i++){
        pathElementEnque(path->pathElement(i), copy, fleet);
    }
    return copy;
}

Conn::Constraint::EvalOutput Conn::checkConstraints(Conn::ConstraintPtr constraints, PathPtr path) const {
    ConstraintPtr constraint = constraints;
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

Conn::PathList Conn::paths(std::set<PathMode> modes, ConstraintPtr constraints,LocationPtr start, LocationPtr endpoint) const {

    Conn::PathList retval;

    DEBUG_LOG << "Starting location: " << start->name() << std::endl;

    // Setup 
    std::stack<PathPtr> pathStack;
    pathStack.push(Path::PathIs(start));
    // Traverse
    while(pathStack.size() > 0){

        PathPtr currentPath = pathStack.top();
        pathStack.pop();

        DEBUG_LOG << "Visiting location: " << currentPath->lastLocation()->name() << std::endl;

        // Evaluate constraints
        if(checkConstraints(constraints,currentPath)==Conn::Constraint::fail()){
            DEBUG_LOG << "Failed to pass constraints, discarding path" << std::endl;
            continue;
        }

        // Should we output the path?
        if(    (!endpoint || endpoint->name() == currentPath->lastLocation()->name()) // We have reached an endpoint
            && (currentPath->pathElementCount() > 0)                                  // Don't ouput 0-hop path (Instructor's Requirement)
          ){
            DEBUG_LOG << "Output path" << std::endl;
            retval.push_back(currentPath);
        }

        // Stop traversing if we have hit the endpoint (THIS IS AN OPTIMIZATION)
        if(endpoint && endpoint == currentPath->lastLocation()) continue;

        // Continue traversal
        for(uint32_t i = 1; i <= currentPath->lastLocation()->segmentCount(); i++){
            SegmentPtr segment = currentPath->lastLocation()->segment(i); 
            if( validSegment(segment)                                                // Valid Segment
                && segment->returnSegment()->source()                                // AND Valid Return Segment
                && !(currentPath->location(segment->returnSegment()->source())))      // AND Not a Loop
              {
                  std::set<PathMode> overlap = modeIntersection(segment,modes);  
                  DEBUG_LOG << "Segment Modes: " << segment->modeCount() << ", Transport Modes: " << modes.size() << ", Overlap Size: " << overlap.size() << std::endl;               
                  for(std::set<PathMode>::const_iterator it = overlap.begin(); it != overlap.end(); it++){
                      PathPtr pathCopy = copyPath(currentPath,fleet_);
                      pathElementEnque(Path::PathElement::PathElementIs(segment,*it),pathCopy,fleet_);
                      pathStack.push(pathCopy);
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

Path::Path(LocationPtr firstLocation) : cost_(0),time_(0),distance_(0),firstLocation_(firstLocation), lastLocation_(firstLocation){}

Path::PathElementPtr Path::PathElement::PathElementIs(SegmentPtr segment, PathMode mode){
    return new Path::PathElement(segment,mode);
}

Path::PathElementPtr Path::pathElement(uint32_t index) const{
    if(index >= path_.size()) return NULL;
    return path_[index];
}

uint32_t Path::pathElementCount() const {
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
