#include <iostream>
#include <time.h>

#include "logging.h"
#include "activity/Activity.h"

namespace Activity{

Activity::Activity(string name, ManagerPtr manager) : 
    NamedInterface(name), status_(Activity::uninit()), nextTime_(0.0), notifiee_(NULL),
    manager_(manager), priority_(1)
{}

void Activity::priorityIs(Priority priority){
    priority_=priority;
}

void Activity::statusIs(Activity::Status status){
    status_ = status;
    if (notifiee_ != NULL) {
        notifiee_->onStatus();
    }
}

void Activity::nextTimeIs(Time t){
    nextTime_ = t;
    if (notifiee_ != NULL) {
        notifiee_->onNextTime();
    }
}

void Activity::lastNotifieeIs(Activity::Notifiee* n){
    notifiee_ = n;
    n->notifierIs(this);
}

ActivityPtr Manager::activityNew() {
    std::stringstream s;
    s << "act-auto-name-"<< activityName_;
    activityName_++;
    return activityNew(s.str());
}

ActivityPtr Manager::activityNew(const string& name) {
    ActivityPtr activity = activities_[name];
    if (activity != NULL) {
        cerr << "Activity already exists!" << endl;
        return NULL;
    }
    activity = new Activity(name, this);
    activities_[name] = activity;
    return activity;
}

ActivityPtr Manager::activity(const string& name) const {
    map<string, ActivityPtr>::const_iterator it = activities_.find(name);
    if(it != activities_.end() ) {
        return (*it).second;
    }
    // dont throw an exception (accessor)
    return NULL; 
}

void Manager::activityDel(const string& name) {
    activities_.erase(name);
}

void Manager::lastActivityIs(ActivityPtr activity) {
    scheduledActivities_.push(activity);
}

void Manager::nowIs(Time t) {

    DEBUG_LOG << std::endl;
    DEBUG_LOG << "==================================" << std::endl;
    DEBUG_LOG << "NOW: " << t.value() << std::endl;
    DEBUG_LOG << "==================================" <<std::endl;
    DEBUG_LOG << std::endl;

    //find the most recent activites to run and run them in order
    while (!scheduledActivities_.empty()) {
        //figure out the next activity to run
        ActivityPtr nextToRun = scheduledActivities_.top();
        //if the next time is greater than the specified time, break
        //the loop
        if (nextToRun->nextTime() > t) {
                break;
        }
        now_ = nextToRun->nextTime();
        //run the minimum time activity and remove it from the queue
        scheduledActivities_.pop();
        if(nextToRun->status() == Activity::nextTimeScheduled()){
            nextToRun->statusIs(Activity::executing());
            nextToRun->statusIs(Activity::free());
        }
    }
    //syncrhonize the time
    now_ = t;
}
}
