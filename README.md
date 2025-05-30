# Global Travel Planner

A web interface for a C++ travel planner. Plan multi-city journeys using flights, trains, buses, and cars,
and get optimized routes with distance, time, and cost estimations.

## Features
- Find the most efficient path between cities
- Multi-modal support: flights, trains, buses, and cars
- Cost and time estimation for each segment and total trip
- Interactive web UI with city autocomplete and dynamic stops

## Prerequisites
- Node.js (>=12) and npm
- g++ compiler with C++17 support
- (Optional) make (for using the Makefile)

## Installation & Running
1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd <repository-directory>
   ```
   
3. Install Node.js dependencies:
   ```bash
   npm install
   ```
   
4. Build the C++ planner and start the server:
   ```bash
   npm start
   ```
   
   This runs the build script (g++ -std=c++17 travelPlanner.cpp -o travelPlanner) and then starts the server.
5. Open your browser and navigate to:
   ```
   http://localhost:3000
   ```

Alternatively, you can build and run manually:
```bash
make           # Build the C++ executable via Makefile
node server.js # Start the Express server
```

## Usage
- Enter a starting city and destination
- Specify the number of stops (0 for direct route)
- For each stop and the final leg, choose a transport mode (Flight, Train, Bus, Car)
- Click *Plan Trip* to view the itinerary and summary

## Data Files
- *cities.csv*: Defines the graph nodes with geographic coordinates
- *routes.csv*: Contains sample schedules and operators (not used by the current planner)

## License
This project is licensed under the ISC License. See [package.json](package.json) for details.
