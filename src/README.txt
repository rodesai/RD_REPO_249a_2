user: rpdesai
user: bartho

-------------------------------------------------------------------------------
Exceptions
Exceptions that are generated in the engine and rep layers are caught and logged instead of percolating to the client. We decided upon this strategy as there was no particular error for which the client could provide automated response (e.g. a retry of a certin action). Instead, actions producing errors are simply logged and ignored in the case of incorrect arguments or aborted in the case of an internal error.

Exceptions were derived from Fwk::Exception. Two exceptions were sufficiently generic as to warrant their own exception types: argument exceptions and entity exist exceptions. The remaining exceptions utilized Fwk-provided exceptions.

Any errors identified within the rep layer were logged instead of throwing an error. This is for the same reason that errors produced in the engine layer are caught in the rep layer--it was determined that the client would not benefit from the exception information.

-------------------------------------------------------------------------------
Activity Manager

We chose not to implement the real and virtual time activities seperately. Instead, both managers are instances of a single Manager class, Activity::Manager. The conversion from real to virtual time is performed by an activity called RealToVirtualTimeActivity, that is registered with the real-time manager and executes hourly. Every time it is executed, RealToVirtualTimeActivity sleeps for the scaling factor (20 milliseconds) and then calls the virtual-time manager to execute. After this, it schedules itself to run again the next hour. This effectively scales the execution time.


The real time is advanced by calling timeIs from the SimulationManager:

	manager->simulationManager()->timeIs(t);

Optionally, virtual time can be advanced directly by calling virtualTimeIs from the SimulationManager:

	manager->simulationManager()->virtualTimeIs(t);

Note that real and virtual time are kept synchronous. That is, if virtual time is advanced explicitly, advancing real time will not cause a delay until the real time has moved past its scaled virtual time.

-------------------------------------------------------------------------------
Routing

Routing is performed by refering to a routing table. Whenever a location wishes to forward a shipment, it consults the routing table to find the segment that it should route the package to. This routing table is pre-computed before the simulation round stops. While there is a drawback to this approach that routes cannot be changed mid-shipment while the simulation is running, we felt that the simplicity of this approach was desirable. The routing table is generated when the client sets a routing algorithm using the following call:

Ptr<Instance> conn = manager->instance("TheConn");
conn->attributeIs("routing",<algorithm>);

For the base credit of the assignment, we support 2 routing algorithms: minHops and minDistance. minHops minimizes the number of locations a shipment visits by executing a BFS traversal of the network for each location to calculate the routing table. minDistance minimizes the distance each shipment visits by executing a Dijkstra traversal of the network for each location to calculate the routing table. 

The routing table is stored as a map from tuples of customer names (start location and end location) to segment IDs. 

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
Group-Constructed Simulation (verification.cpp)

Network Diagram:

        (c2)
         |
         1
         |
        (p3)
       /  |
     10   1
    /     |
  (p1)-1-(p2)
    \     |
     1    10
      \   |
       \  |
       (c1)

Through verification.cpp, we can run 4 simulations exercising the 2 routing algorithms (minHops, minDistance) with high and low network capacity. 
The simulation injects 1 shipment of 100 packages per hour. Each segment is a truck segment. Trucks have capacity 50 and speed 10. The high capacity
simulation assigns capacity 2 to each segment. Low capacity simulation assigns capacity 1 to each segment. Each simulation runs for 240 hours.

To run the simulations with high capacity, run:

$ ./verification high

To run with low capacity, run 

$ ./verification low

We print the output of each below

High:
>>>>>>>>>>>>>>>>>>>>>>>>
$ ./verification high
MIN HOPS ROUTING: 
Shipments Received: 238, Average Latency: 1.20

MIN DISTANCE ROUTING: 
Shipments Received: 239, Average Latency: 0.40

The results for Shipments Received and latency make sense:

With MIN HOPS routing, the following path is taken:
 c1->p2->p3->c2
 This path takes 1.2 hours (12 miles) / (10 miles/hour)
 The first shipment arrives after the second hour.* Subsequent shipments arrive at a rate of 1 shipment per hour, 
   giving a total of 238 shipments.

With MIN DISTANCE routing, the following path is taken:
 c1->p1->p2-p3->c2
 This path takes 0.4 hours (4 miles) / (10 miles/hour)
 The first shipment arrives after the first hour. Subsequent shipments arrive at a rate of 1 shipment per hour, 
   giving a total of 238 shipments.

*This is because injection activities start running at time 1

Low:
>>>>>>>>>>>>>>>>>>>>>>>>
./verification low
MIN HOPS ROUTING: 
Shipments Received: 119, Average Latency: 61.40

MIN DISTANCE ROUTING: 
Shipments Received: 239, Average Latency: 0.80

The results for Shipments Received and latency make sense: 
 
With MIN HOPS routing, the following path is taken:
 c1->p2->p3->c2
 This path takes 1.2 hours (12 miles) / (10 miles/hour). Each shipment takes 2 traversals of the path, so latency
   _for traveling_ for 1 shipment is 2.4 hours. Because shipping is slower than injection, shipments get queued. 
   Thus the actual average latency is far higher, 61.4 hours. The average latency grows over time because queues 
   grow larger and larger.
 The first shipment arrives after the second hour. Subsequent shipments arrive at a rate of 1 shipment every 2 hours,                         
   giving a total of 119 shipments.

With MIN DISTANCE routing, the following path is taken:
 c1->p1->p2-p3->c2
 This path takes 0.4 hours (4 miles) / (10 miles/hour). Each shipment takes 2 traversals of the path for a total of 0.8 hours.
   Since the shipping time is faster than injection, queues dont build up and the average latency is consistent at 0.8 hours.
 The first shipment arrives after the first hour. Subsequent shipments arrive at a rate of 1 shipment per hour,
   giving a total of 238 shipments.

*This is because injection activities start running at time 1

-------------------------------------------------------------------------------
100-Source Simulation (100customers.cpp)

The spec was a bit unclear about what speed to run the customers at for the non-random case. We aimed to saturate the network.

$ ./experiment
@30 Shipments Received: 19700, Average Latency: 0.45
@60 Shipments Received: 39700, Average Latency: 0.45
@90 Shipments Received: 59600, Average Latency: 0.45
@120 Shipments Received: 79600, Average Latency: 0.45
@150 Shipments Received: 99600, Average Latency: 0.45
@180 Shipments Received: 119600, Average Latency: 0.45

Average shipments received: 3237.84
Average shipments refused: 0

Random case:

$ ./experiment random
@30 Shipments Received: 5559, Average Latency: 8.17
@60 Shipments Received: 11323, Average Latency: 15.64
@90 Shipments Received: 17050, Average Latency: 23.07
@120 Shipments Received: 22820, Average Latency: 30.54
@150 Shipments Received: 28576, Average Latency: 38.01
@180 Shipments Received: 34344, Average Latency: 45.46

Average shipments received: 931.586
Average shipments refused: 1508.03

In the non-random case, the network is running at but not over capacity. Therefore, we don't see any shipment refusal and latency is consistent over time.
In the random case, the network is running over capacity. This is apparent in that many shipments are refused and the average latency is constantly growing

-------------------------------------------------------------------------------
Extra Credit Experiment

Network Diagram:


                (root)
        (t3)____|    |____
     5.1 |   5.1       10 |
        (t2)             (t1)________ 1
   [c1-c200 connected to t1 and t2]  |
   (c1) (c2) (c3) ... (c199) (c200) (BC)

For the extra credit portion of the assignment, we attempted to devise a routing scheme that would allow the network to re-route packages in response to traffic
changes, to avoid over-congesting a segment. Our experiment uses the network drawn above. There are 200 customers, each connected to 2 ports, t1 and t2. t1 connects
directly to a customer called root. t2 connects to root via another port called t3. Segment lengths are shown in the diagram. Finally, another customer BC is only 
connected to t1. At first, the simulation runs with only customers c1-c200 injecting shipments destined for root, at a rate that saturates the path (c*->t1->root). By
design, this path is the only path routed through. Then, after some time, we setup BC to inject a large load of shipments into the network. Under our 2 base 
schemes, minHops and minDistance, the path c*->t1->root will become congested and the package delivery latencies will continuously rise. We will devise an alternate
routing scheme that tries to revise its routes based on current conditions.

The new routing scheme is called minTime. It tries to minimize the amount of time that shipments spend being delivered. Its estimate for the time uses both the carrier speeds /
segment lengths AND a measured statistic called queueTime, that measures the time that packages spend in segment queues. minTime uses this measurement when assigning segments
weights when executing its dijkstra's traversal of the network. Also, to avoid dramatic load shifts between routes, the portion of the segment weight from the measured queue time
is multiplied by a random factor between 0 and 1. As routes are calculated per-customer, re-calculating the random factor for each customer allows the router to reduce the amount 
that load is shifted.

This routing scheme is implemented via the MinTimeTraversal in Engine.h. Just like MinHopsTraversal and MinDistanceTraversal, MinTimeTraversal works by ordering segents a certain way.
Its ordering is done as described above.

Finally, in order to be adaptive, routes must be re-calculated periodically. This could be done via an activity. In our simulation this was done by the client.

We ran this simulation using both our original routing algorithms and the MinTime algorithm. The simulation source code is in adaptive.cpp

We present the results below:

MIN HOPS

Run network for 30hrs
@30 Shipments Received: 5600, Average Latency: 2.00, t-2 received: 0, t-1 received: 5800

Add large customer. Run network for 30 hours
@60Shipments Received: 11600, Average Latency: 2.63, t-2 received: 0, t-1 received: 6598

Observe congestion and re-route every 6 hours. Run network for 90 hours
@66 Shipments Received: 12800, Average Latency: 2.84, t-2 received: 0, t-1 received: 1320
@72 Shipments Received: 14000, Average Latency: 3.06, t-2 received: 0, t-1 received: 1320
@78 Shipments Received: 15200, Average Latency: 3.29, t-2 received: 0, t-1 received: 1320
@84 Shipments Received: 16400, Average Latency: 3.52, t-2 received: 0, t-1 received: 1320
@90 Shipments Received: 17600, Average Latency: 3.76, t-2 received: 0, t-1 received: 1320
@96 Shipments Received: 18800, Average Latency: 4.01, t-2 received: 0, t-1 received: 1320
@102 Shipments Received: 20000, Average Latency: 4.26, t-2 received: 0, t-1 received: 1320
@108 Shipments Received: 21200, Average Latency: 4.51, t-2 received: 0, t-1 received: 1320
@114 Shipments Received: 22400, Average Latency: 4.76, t-2 received: 0, t-1 received: 1320
@120 Shipments Received: 23600, Average Latency: 5.01, t-2 received: 0, t-1 received: 1320
@126 Shipments Received: 24800, Average Latency: 5.27, t-2 received: 0, t-1 received: 1320
@132 Shipments Received: 26000, Average Latency: 5.53, t-2 received: 0, t-1 received: 1320
@138 Shipments Received: 27200, Average Latency: 5.79, t-2 received: 0, t-1 received: 1320
@144 Shipments Received: 28400, Average Latency: 6.05, t-2 received: 0, t-1 received: 1320
@150 Shipments Received: 29600, Average Latency: 6.31, t-2 received: 0, t-1 received: 1320

MIN DISTANCE

Run network for 30hrs
@30 Shipments Received: 5600, Average Latency: 2.00, t-2 received: 0, t-1 received: 5800

Add large customer. Run network for 30 hours
@60Shipments Received: 11600, Average Latency: 2.63, t-2 received: 0, t-1 received: 6598

Observe congestion and re-route every 6 hours. Run network for 90 hours
@66 Shipments Received: 12800, Average Latency: 2.84, t-2 received: 0, t-1 received: 1320
@72 Shipments Received: 14000, Average Latency: 3.06, t-2 received: 0, t-1 received: 1320
@78 Shipments Received: 15200, Average Latency: 3.29, t-2 received: 0, t-1 received: 1320
@84 Shipments Received: 16400, Average Latency: 3.52, t-2 received: 0, t-1 received: 1320
@90 Shipments Received: 17600, Average Latency: 3.76, t-2 received: 0, t-1 received: 1320
@96 Shipments Received: 18800, Average Latency: 4.01, t-2 received: 0, t-1 received: 1320
@102 Shipments Received: 20000, Average Latency: 4.26, t-2 received: 0, t-1 received: 1320
@108 Shipments Received: 21200, Average Latency: 4.51, t-2 received: 0, t-1 received: 1320
@114 Shipments Received: 22400, Average Latency: 4.76, t-2 received: 0, t-1 received: 1320
@120 Shipments Received: 23600, Average Latency: 5.01, t-2 received: 0, t-1 received: 1320
@126 Shipments Received: 24800, Average Latency: 5.27, t-2 received: 0, t-1 received: 1320
@132 Shipments Received: 26000, Average Latency: 5.53, t-2 received: 0, t-1 received: 1320
@138 Shipments Received: 27200, Average Latency: 5.79, t-2 received: 0, t-1 received: 1320
@144 Shipments Received: 28400, Average Latency: 6.05, t-2 received: 0, t-1 received: 1320
@150 Shipments Received: 29600, Average Latency: 6.31, t-2 received: 0, t-1 received: 1320

MIN TIME

Run network for 30hrs
@30 Shipments Received: 5600, Average Latency: 2.00, t-2 received: 0, t-1 received: 5800

Add large customer. Run network for 30 hours
@60Shipments Received: 11600, Average Latency: 2.63, t-2 received: 0, t-1 received: 6598

Observe congestion and re-route every 6 hours. Run network for 90 hours
@66 Shipments Received: 13299, Average Latency: 2.78, t-2 received: 995, t-1 received: 325
@72 Shipments Received: 14738, Average Latency: 2.70, t-2 received: 199, t-1 received: 1121
@78 Shipments Received: 15938, Average Latency: 2.69, t-2 received: 0, t-1 received: 1320
@84 Shipments Received: 17262, Average Latency: 2.68, t-2 received: 980, t-1 received: 340
@90 Shipments Received: 18698, Average Latency: 2.63, t-2 received: 196, t-1 received: 1124
@96 Shipments Received: 19898, Average Latency: 2.62, t-2 received: 0, t-1 received: 1320
@102 Shipments Received: 21222, Average Latency: 2.61, t-2 received: 980, t-1 received: 340
@108 Shipments Received: 22658, Average Latency: 2.57, t-2 received: 196, t-1 received: 1124
@114 Shipments Received: 23858, Average Latency: 2.57, t-2 received: 0, t-1 received: 1320
@120 Shipments Received: 25181, Average Latency: 2.57, t-2 received: 985, t-1 received: 335
@126 Shipments Received: 26618, Average Latency: 2.54, t-2 received: 197, t-1 received: 1123
@132 Shipments Received: 27818, Average Latency: 2.54, t-2 received: 0, t-1 received: 1320
@138 Shipments Received: 29144, Average Latency: 2.54, t-2 received: 970, t-1 received: 350
@144 Shipments Received: 30578, Average Latency: 2.51, t-2 received: 194, t-1 received: 1126
@150 Shipments Received: 31778, Average Latency: 2.51, t-2 received: 0, t-1 received: 1320

Both minDistance and minHops fail to utilize the route via t-2, as can be seen by the fact that
t-2 received no shipments. Consequently, the average package latency grows over time.

The minTime simulation keeps latencies consistent by offloading traffic to t-2 when t-1 gets congested.
When the congestion on t-1 has cleared, shipments are routed back through t-1 to utilize the faster route.

