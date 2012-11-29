user: rpdsai
user: bartho

-------------------------------------------------------------------------------
Exceptions
Exceptions that are generated in the engine and rep layers are caught and logged instead of percolating to the client. We decided upon this strategy as there was no particular error for which the client could provide automated response (e.g. a retry of a certin action). Instead, actions producing errors are simply logged and ignored in the case of incorrect arguments or aborted in the case of an internal error.

Exceptions were derived from Fwk::Exception. Two exceptions were sufficiently generic as to warrant their own exception types: argument exceptions and entity exist exceptions. The remaining exceptions utilized Fwk-provided exceptions.

Any errors identified within the rep layer were logged instead of throwing an error. This is for the same reason that errors produced in the engine layer are caught in the rep layer--it was determined that the client would not benefit from the exception information.

-------------------------------------------------------------------------------
Activity Manager

- how is it structured?
- how do we use it?
- how do we switch from real to virtual?

-------------------------------------------------------------------------------
Routing Table
Before the routing information can be generated, a user must make a call to the connection instance (see below).

The routing table is stored as a map from tuples of customer names (start location and end location) to segment ID's. Thus, when delivering a shipment to a customer location, the program iteratively looks up the "next hop" until arriving at the destination.

-------------------------------------------------------------------------------
Routing Algorithms
We implemented two different algorithms, which the user can select using the following:

	connPtr->attributeIs("routing",[algorithm name]).

minHops: This algorithm minimizes the number of hops in between the source and the destination.

minDistance: This algorithm minimizes the distance between the source and the destination.

-------------------------------------------------------------------------------
Fleet Schedules
Multiple fleet schedules are implemented by allowing multiple fleets with different start times, which are specified with an hour of the day value. For instance, to set up different fleet behavior between 12AM and 12PM, one creates two fleets and establishes two different start times. Upon setting a start time, an activity is schedule to run daily to switch the active fleet to this fleet.

An example is provided below.

	// create two fleets
	Ptr<Instance> fleet1 = manager->instanceNew("fleet1", "Fleet");
	Ptr<Instance> fleet2 = manager->instanceNew("fleet2", "Fleet");

	// set up criteria for each
	fleet1->attributeIs("Truck, speed", "60");
	fleet2->attributeIs("Truck, speed", "90");
	...

	// set up start times for each
	fleet1->attributeIs("Start Time", "0");
	fleet2->attributeIs("Start Time", "12");

To accomodate this, the ShippingNetwork class was modified such that the fleet attribute can now contain multiple fleets. Further, a new attribute, activeFleet, was added to specify which fleet is currently active.

-------------------------------------------------------------------------------
Shipment Activities

Shipment activities are outlined in the PDF contained with the source code: ShipmentActivityDiagram.pdf. These activities generally follow the protocol outlined by the assignment.

One addition was the DeliveryActivity and DeliveryActivityReactor. An issue we encountered during testing and simulation occurred when a carrier (represented by a ForwardActivityReactor) delivered a shipment and returned to pick up a new shipment. If a shipment arrived at the same time but was processed before, it was counted as a refused shipment. That is, if a carrier from segment S dropped off a shipment at time t and a shipment was enqueued at segment S also at time t, whether the shipment was refused depended on the arbitrary ordering of the activities.

Thus, our activites were extended to include a priority value. Shipment delivery activities are enqueued with no time delay but with low priority to ensure that the activities of any soon-to-be available carriers are executed first.

-------------------------------------------------------------------------------
Forwarding Shipments on Segments
Shipments are queued up for forwarding accross segments according to a first-in, first-out policy. Carriers, represented abstractly by ForwardActivityReactor, will carry as many shipments while not exceeding their own capacity. There is no method of batching or dallying; carriers are sent immediately after they pick up shipments.

-------------------------------------------------------------------------------
Shipment Rate
Customers send shipments at the rate specified by the user. This rate defaults to zero, which means that the customer is not sending any shipments. If the user specifies a shipment rate with the remaining two criteria for shipments, then the CustomerReactor will initiate shipment scheduling.

If the rate of shipments changes, then the customer will continue to send shipments at that rate, regardless of how many shipments are already sent on that day. For instance, if a customer sends packages at a rate of 20 packages per day, and the rate is reduced to 10 packages halfway through the day, the customer continues to send packages for the rest of the day, having sent roughly 15 packages by the end of the day.

-------------------------------------------------------------------------------
Group-Constructed Simulation (verification.cpp)

-------------------------------------------------------------------------------
100-Source Simulation (100customers.cpp)
