#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <vector>
#include <map>
#include <exception>

#include "Ptr.h"
#include "PtrInterface.h"
#include "Instance.h"

namespace Shipping {

typedef string SegmentID;
typedef string LocationID;


class ArgumentException : public exception {
	virtual const char* message() const throw() {
		return "Argument exception.";
	}
};


class Mile : public Ordinal<Mile, uint64_t> {
public:
	Mile(uint64_t num) : Ordinal<Mile, uint64_t>(num) {
	}
};


class MilePerHour : public Nominal<MilePerHour, uint64_t> {
public:
	MilePerHour(uint64_t num) : Nominal<MilePerHour, uint64_t>(num) {
	}
};


class Dollar : public Ordinal<Dollar, float> {
public:
	Dollar(float num) : Ordinal<Dollar, float>(num) {
		if (num < 0) throw ArgumentException;
	}
};


class DollarPerMile : public Ordinal<DollarPerMile, uint64_t> {
public:
	DollarPerMile(uint64_t num) : Ordinal<DollarPerMile, uint64_t>(num) {
	}
};


class Hour : public Ordinal<Hour, float> {
public:
	Hour(float num) : Ordinal<Hour, float>(num) {
		if (num < 0) throw ArgumentException;
	}
};


class Difficulty : public Nominal<Difficulty, float> {
public:
	Difficulty(float num) : Nominal<Difficulty, float>(num) {
		if (num < 1.0 || num > 5.0) throw ArgumentException;
	}
};


class PackageNum : public Ordinal<PackageNum, uint64_t> {
public:
	PackageNum(uint64_t num) : Ordinal<PackageNum, uint64_t>(num) {
	}
};


class ShippingEngineReactor {
public:
	void onSegmentDel() {
		// remove segment from source
		// remove segment from reverse segment
	}
};


class StatsReactor {
public:
	void onSegmentNew() {
		// increment counts
	};
	void onSegmentDel() {
	}
		// decrement counts
	void onLocationNew() {
		// increment counts
	};
	void onLocationDel() {
		// decrement counts
	}
private:
	Stats* stats;
};


class ShippingEngine {
public:
	// accessor
	inline Segment* segement(SegmentID sid) const { return segmentMap_[sid]; }
	inline Location* location(LocationID lid) const { return locationMap_[lid]; }

	// mutators
	void segmentIs(Segment* s, SegmentID name) { segmentMap_[name] = s; }
	void segmentDel();
	void locationIs(location* s, locationID name) { locationMap_[name] = s; }
	void locationDel();
private:
	// TODO: will have to use FWK map for smart pointers
	typedef map<LocationID, Location*> LocationMap;
	LocationMap locationMap_;
	typedef map<SegmentID, Segment*> SegmentMap;
	SegmentMap segmentMap_;
};


class Segment {
public:
	enum ExpediteSupport {
		no_ = 0,
		yes_ = 1
	}

	enum EntityType {
		truckSegment_ = 0,
		boatSegment_ = 1,
		planeSegment_ = 2
	}

	// accesors
	static inline ExpediteSupport yes() { return yes_; }
	static inline ExpediteSupport no() { return no_; }
	static inline EntityType truckSegment() { return truckSegment_; }
	static inline EntityType boatSegment() { return boatSegment_; }
	static inline EntityType planeSegment() { return planeSegment_; }	

	inline LocationID source() const { return source_; }
	inline Mile length() const { return length_; }
	inline SegmentID returnSegment() const { return returnSegment_; }
	inline Difficulty difficulty() const { return difficulty_; }
	inline ExpediteSupport expediteSupport() const { return expediteSupport_; }

	// mutators
	void sourceIs(LocationID s){
		// remove segment from previous source, if exists
		// set source
		// add segment to new source
	}
	void lengthIs(Mile l) { length_ = l; }
	void returnSegmentIs(SegmentID s) { returnSegment_ = s; }
	void difficultyIs(Difficulty d) { difficulty_ = d; }
	void expediteSupportIs(ExpediteSupport es) { expediteSupport_ = es; }

private:
	LocationID source_;
	Mile length_; // TODO: use ordinate value type? also, cannot be negative
	SegmentID returnSegment_;
	Difficulty difficulty_; //TODO: create class to limit between 1.0 and 5.0
	ExpediteSupport expediteSupport_;
};


class Location {
public:
	enum EntityType {
		customer_ = 0,
		port_ = 1,
		truckTerminal_ = 2,
		boatTerminal_ = 3,
		planeTerminal_ = 4
	}

	// accessors
	static inline EntityType customer() { return customer_; } 
	static inline EntityType port() { return port_; } 
	static inline EntityType truckTerminal() { return truckTerminal_; } 
	static inline EntityType boatTerminal() { return boatTerminal_; } 
	static inline EntityType planeTerminal() { return planeTerminal_; } 

	inline EntityType entityType() const { return entityType_; }
	inline SegmentID segment(int index) const { return segments_[index]; }

	// mutators
	void entityTypeIs(EntityType et) { entityType_ = et; }
	// note: per the instructions, segments_ is read-only

private:
	friend class Segment;
	void segmentEnq(SegmentID s) {
		// TODO: add segment to list
	}
	void segmentDeq(SegmentID s) {
		// TODO: remove segment from list
	}
	EntityType entityType_;
	vector<SegmentID> segments_;
};


class Path {
public:
	// accessors
	Dollar cost() const { return cost_; }
	Hour time() const { return time_; }
	Segment::ExpediteSupport expedited() const { return expedited_; }
	class PathElement {
	public:
		// accessors
		inline LocationID source() const { return source_; }
		inline SegmentID segment() const { return segment_; }

		// mutators
		void sourceIs(SourceID s) const { source_ = s; }
		void segmentIs(SegmentID s) const { segment_ = s; }
	private:
		LocationID source_;
		SegmentID segment_;
	};
private:
	Dollar cost_;
	Hour time_;
	Segment::ExpediteSupport expedited_;
	typedef vector<PathElement*> LocationVector;
	LocationVector locations_;
};


class Conn {
public:
	vector<Path*> connect();
	vector<Path*> explore();
};


class Fleet {
public:
	enum Mode {
		truck_ = 0,
		boat_ = 1,
		plane_ = 2
	}

	// accessors
	static inline Mode truck() { return truck_; }
	static inline Mode boat() { return boat_; }
	static inline Mode plane() { return plane_; }

	inline Mode mode() { return mode_; }
	inline MilePerHour speed() { return speed_; }
	inline DollarPerHour cost() { return cost_; }
	inline PackageNum capacity() { return capacity; }

	// mutators
	void mode(Mode m) { mode_ = m; }
	void speedIs(MilePerHour s) { speed_ = s; }
	void costIs(DollarPerMile c) { cost_ = c; }
	void capacityIs(MaxPackages c) { capacity_ = c; }
private:
	Mode mode_;
	MilePerHour speed_; // TODO: use ordinate value type
	DollarPerMile cost_;
	PackageNum capacity_;
};


class Stats {
	// TODO: this needs to be updated via notification
public:
	// accessors
	inline int locationCount(Location::EntityType et) const
		{ return locationCount_[et]; }
	inline int locationCount() const
		{ return locationCountTotal_; } // TODO: not sure if this is required
	inline int segmentCount(Segment::EntityType et) const
		{ return segmentCount_[et]; }
	inline int segmentCount() const
		{ return segmentCountTotal_; } // TODO: not sure if this is required
	inline float expeditePercentage() const
		{ return expediteSegmentCount_ * 1.0 / segmentCountTotal_;}
private:
	map<Location::EntityType, int> locationCount_;
	map<Segment::EntityType, int> segmentCount_;
	int locationCountTotal_ = 0;
	int segmentCountTotal_ = 0;
	int expediteSegmentCount_ = 0;
};


} /* end namespace */

#endif
