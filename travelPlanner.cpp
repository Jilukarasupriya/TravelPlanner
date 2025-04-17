#define _USE_MATH_DEFINES  // Added to make M_PI available

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <cmath>  // M_PI should now be defined after adding the define above
#include <stdexcept>
#include <algorithm>
#include <limits>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <queue>
#include <functional>

using namespace std;

// As a fallback, define PI ourselves in case _USE_MATH_DEFINES doesn't work
#ifndef M_PI
const double M_PI = 3.14159265358979323846;
#endif

// ---------- ENUM AND STRUCTS ----------
enum TransportMode { FLIGHT, TRAIN, BUS, CAR };

string modeToString(TransportMode mode) {
    switch (mode) {
        case FLIGHT: return "Flight";
        case TRAIN: return "Train";
        case BUS: return "Bus";
        case CAR: return "Car";
        default: return "Unknown";
    }
}

TransportMode stringToMode(const string& modeStr) {
    string lowerMode = modeStr;
    transform(lowerMode.begin(), lowerMode.end(), lowerMode.begin(), ::tolower);
    if (lowerMode == "flight") return FLIGHT;
    if (lowerMode == "train") return TRAIN;
    if (lowerMode == "bus") return BUS;
    if (lowerMode == "car") return CAR;
    throw invalid_argument("Invalid transport mode: " + modeStr);
}

struct City {
    string country;
    string name;
    double latitude;
    double longitude;
};

struct Edge {
    int to;
    TransportMode mode;
    double distance;
    double time;
    double cost;
    double hopPenalty; // Penalty to discourage excessive segments
};

struct Segment {
    string fromCountry;
    string fromCity;
    string toCountry;
    string toCity;
    TransportMode mode;
    double cost;
    double time;
    double distance;
};

struct PathResult {
    vector<Segment> segments;
    double totalCost = 0;
    double totalTime = 0;
    double totalDistance = 0;
};

// Helper function to trim whitespace from strings
void trim(string &s) {
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch) { 
        return !isspace(ch); 
    }));
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) { 
        return !isspace(ch); 
    }).base(), s.end());
}

// Generic CSV loader
template<typename T>
vector<T> loadCSV(const string& filename, function<T(const string&)> parseRow) {
    vector<T> data;
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Could not open file: " + filename);
    }

    string line;
    while (getline(file, line)) {
        try {
            data.push_back(parseRow(line));
        } catch (const exception& e) {
            cerr << "Error processing line: " << line << " - " << e.what() << endl;
        }
    }
    return data;
}

// ---------- GRAPH CLASS ----------
class Graph {
public:
    void addCity(const string& country, const string& name, double lat, double lon) {
        string key = country + "," + name;
        if (cityToId.count(key)) return;
        cityToId[key] = static_cast<int>(cities.size());  // Fixed: explicit cast from size_t to int
        cities.push_back({country, name, lat, lon});
        adj.emplace_back();
    }

    void addEdge(const string& fromCountry, const string& fromCity, 
                 const string& toCountry, const string& toCity, TransportMode mode) {
        string fromKey = fromCountry + "," + fromCity;
        string toKey = toCountry + "," + toCity;

        int fromId = getCityId(fromKey);
        int toId = getCityId(toKey);

        double distance = haversine(fromId, toId);
        double speed = modeSpeed(mode);
        double cost = distance * modeCostPerKm(mode);
        double time = distance / speed;
        double hopPenalty = (mode == FLIGHT) ? 100.0 : 200.0; // Lower penalty for flights

        adj[static_cast<size_t>(fromId)].push_back({toId, mode, distance, time, cost, hopPenalty});  // Fixed: explicit cast
    }

    const vector<Edge>& getEdges(int cityId) const {
        return adj[static_cast<size_t>(cityId)];  // Fixed: explicit cast
    }

    int getCityId(const string& key) const {
        if (!cityToId.count(key)) throw invalid_argument("City not found: " + key);
        return cityToId.at(key);
    }

    City getCity(int id) const {
        return cities[static_cast<size_t>(id)];  // Fixed: explicit cast
    }

    double haversine(int id1, int id2) const {
        const City& c1 = cities[static_cast<size_t>(id1)];  // Fixed: explicit cast
        const City& c2 = cities[static_cast<size_t>(id2)];  // Fixed: explicit cast

        const double R = 6371.0; // Earth radius in km
        double lat1 = c1.latitude * M_PI / 180.0;
        double lon1 = c1.longitude * M_PI / 180.0;
        double lat2 = c2.latitude * M_PI / 180.0;
        double lon2 = c2.longitude * M_PI / 180.0;
        double dlat = lat2 - lat1;
        double dlon = lon2 - lon1;

        double a = pow(sin(dlat / 2), 2) +
                   cos(lat1) * cos(lat2) * pow(sin(dlon / 2), 2);
        double c = 2 * asin(sqrt(a));

        return R * c;
    }

    int cityCount() const {
        return static_cast<int>(cities.size());  // Fixed: explicit cast
    }

    void printAllCities() const {
        cout << "Available cities:\n";
        for (const auto& city : cities) {
            cout << "- " << city.name << ", " << city.country << "\n";
        }
    }

    vector<string> findCities(const string& query) const {
        vector<string> results;
        string lowercaseQuery;
        transform(query.begin(), query.end(), back_inserter(lowercaseQuery), ::tolower);

        for (const auto& city : cities) {
            string lowercaseCity = city.name + ", " + city.country;
            transform(lowercaseCity.begin(), lowercaseCity.end(), lowercaseCity.begin(), ::tolower);

            if (lowercaseCity.find(lowercaseQuery) != string::npos) {
                results.push_back(city.country + "," + city.name);
            }
        }
        return results;
    }

private:
    vector<City> cities;
    unordered_map<string, int> cityToId;
    vector<vector<Edge>> adj;

    double modeSpeed(TransportMode mode) const {
        switch (mode) {
            case FLIGHT: return 900.0;   // Increased for realism
            case TRAIN: return 120.0;
            case BUS: return 60.0;
            case CAR: return 70.0;
        }
        return 1.0;
    }

    double modeCostPerKm(TransportMode mode) const {
        switch (mode) {
            case FLIGHT: return 0.12;    // Reduced to favor flights
            case TRAIN: return 0.10;
            case BUS: return 0.08;       // Increased to reduce short hops
            case CAR: return 0.12;
        }
        return 1.0;
    }
};

// Helper function to parse cities from CSV
City parseCityLine(const string& line) {
    stringstream ss(line);
    string country, city, latStr, lonStr;

    getline(ss, country, ',');
    getline(ss, city, ',');
    getline(ss, latStr, ',');
    getline(ss, lonStr, ',');

    // Clean city names
    city.erase(remove(city.begin(), city.end(), '\"'), city.end());
    city.erase(remove(city.begin(), city.end(), '\''), city.end());
    trim(city);
    trim(country);

    return City{
        country,
        city,
        stod(latStr),
        stod(lonStr)
    };
}

// Function to load cities from CSV file
void loadCities(Graph& g, const string& filename) {
    auto cities = loadCSV<City>(filename, parseCityLine);
    for (const auto& city : cities) {
        g.addCity(city.country, city.name, city.latitude, city.longitude);
    }
}

// Modified findShortestPath to respect specified transport mode
PathResult findShortestPath(const Graph& graph, const string& startKey, const string& endKey, TransportMode preferredMode = FLIGHT, bool enforceMode = false) {
    int start = graph.getCityId(startKey);
    int end = graph.getCityId(endKey);

    using QueueElement = pair<double, int>;
    priority_queue<QueueElement, vector<QueueElement>, greater<QueueElement>> pq;

    vector<double> dist(static_cast<size_t>(graph.cityCount()), numeric_limits<double>::max());  // Fixed: explicit cast
    vector<int> prev(static_cast<size_t>(graph.cityCount()), -1);  // Fixed: explicit cast
    vector<TransportMode> modeUsed(static_cast<size_t>(graph.cityCount()), FLIGHT);  // Fixed: explicit cast
    vector<double> hops(static_cast<size_t>(graph.cityCount()), numeric_limits<double>::max());  // Fixed: explicit cast

    dist[static_cast<size_t>(start)] = 0;  // Fixed: explicit cast
    hops[static_cast<size_t>(start)] = 0;  // Fixed: explicit cast
    pq.push({0, start});

    while (!pq.empty()) {
        auto top = pq.top();
        auto currentDist = top.first;
        auto u = top.second;
        pq.pop();

        if (u == end) break;
        if (currentDist > dist[static_cast<size_t>(u)]) continue;  // Fixed: explicit cast

        for (const Edge& edge : graph.getEdges(u)) {
            if (enforceMode && edge.mode != preferredMode) continue; // Skip edges that don't match the preferred mode
            int v = edge.to;
            double newDist = dist[static_cast<size_t>(u)] + edge.distance + edge.hopPenalty;  // Fixed: explicit cast
            double newHops = hops[static_cast<size_t>(u)] + 1;  // Fixed: explicit cast

            if (newDist < dist[static_cast<size_t>(v)] || (newDist == dist[static_cast<size_t>(v)] && newHops < hops[static_cast<size_t>(v)])) {  // Fixed: explicit casts
                dist[static_cast<size_t>(v)] = newDist;  // Fixed: explicit cast
                hops[static_cast<size_t>(v)] = newHops;  // Fixed: explicit cast
                prev[static_cast<size_t>(v)] = u;  // Fixed: explicit cast
                modeUsed[static_cast<size_t>(v)] = edge.mode;  // Fixed: explicit cast
                pq.push({newDist, v});
            }
        }
    }

    PathResult result;
    if (dist[static_cast<size_t>(end)] == numeric_limits<double>::max()) {  // Fixed: explicit cast
        return result;
    }

    vector<int> path;
    for (int at = end; at != -1; at = prev[static_cast<size_t>(at)]) {  // Fixed: explicit cast
        path.push_back(at);
    }
    reverse(path.begin(), path.end());

    for (size_t i = 0; i < path.size() - 1; ++i) {
        int from = path[i];
        int to = path[i+1];
        TransportMode mode = modeUsed[static_cast<size_t>(to)];  // Fixed: explicit cast

        const vector<Edge>& edges = graph.getEdges(from);
        auto it = find_if(edges.begin(), edges.end(), [to, mode](const Edge& e) {
            return e.to == to && e.mode == mode;
        });

        if (it != edges.end()) {
            const Edge& edge = *it;
            City fromCity = graph.getCity(from);
            City toCity = graph.getCity(to);

            result.segments.push_back({
                fromCity.country, fromCity.name,
                toCity.country, toCity.name,
                edge.mode,
                edge.cost,
                edge.time,
                edge.distance
            });

            result.totalCost += edge.cost;
            result.totalTime += edge.time;
            result.totalDistance += edge.distance;
        }
    }

    return result;
}

// Function to find path through specified stops with preferred modes
PathResult findPathWithStops(const Graph& graph, const vector<string>& cityKeys, const vector<TransportMode>& modes) {
    PathResult finalResult;

    for (size_t i = 0; i < cityKeys.size() - 1; ++i) {
        TransportMode mode = modes[i];
        PathResult segmentResult = findShortestPath(graph, cityKeys[i], cityKeys[i + 1], mode, true);

        if (segmentResult.segments.empty()) {
            throw runtime_error("No route found between " + cityKeys[i] + " and " + cityKeys[i + 1] + " using " + modeToString(mode));
        }

        finalResult.segments.insert(finalResult.segments.end(), 
                                   segmentResult.segments.begin(), 
                                   segmentResult.segments.end());
        finalResult.totalCost += segmentResult.totalCost;
        finalResult.totalTime += segmentResult.totalTime;
        finalResult.totalDistance += segmentResult.totalDistance;
    }

    return finalResult;
}

void generateRoutes(Graph& graph) {
    for (int i = 0; i < graph.cityCount(); ++i) {
        City from = graph.getCity(i);
        for (int j = 0; j < graph.cityCount(); ++j) {
            if (i == j) continue;
            City to = graph.getCity(j);
            double dist = graph.haversine(i, j);

            // Add flights for long distances
            if (dist > 1000) {
                graph.addEdge(from.country, from.name, to.country, to.name, FLIGHT);
            }
            // Add other modes for shorter distances
            else if (dist < 500) {
                graph.addEdge(from.country, from.name, to.country, to.name, BUS);
                graph.addEdge(from.country, from.name, to.country, to.name, TRAIN);
                graph.addEdge(from.country, from.name, to.country, to.name, CAR);
            }
        }
    }
}

string selectCity(const Graph& graph, const string& prompt) {
    string query;
    cout << prompt;
    getline(cin, query);
    trim(query);

    vector<string> matches = graph.findCities(query);

    if (matches.empty()) {
        throw runtime_error("No cities found matching: " + query);
    } else if (matches.size() == 1) {
        return matches[0];
    } else {
        cout << "Multiple matches found:\n";
        for (size_t i = 0; i < matches.size(); ++i) {
            size_t commaPos = matches[i].find(',');
            string country = matches[i].substr(0, commaPos);
            string city = matches[i].substr(commaPos + 1);
            cout << "  " << (i+1) << ") " << city << ", " << country << "\n";
        }

        cout << "Enter your choice (1-" << matches.size() << "): ";
        int choice;
        cin >> choice;
        cin.ignore();

        if (choice < 1 || choice > static_cast<int>(matches.size())) {  // Fixed: explicit cast
            throw runtime_error("Invalid choice");
        }

        return matches[static_cast<size_t>(choice - 1)];  // Fixed: explicit cast
    }
}

int main() {
    try {
        Graph graph;

        cout << "Loading cities data...\n";
        loadCities(graph, "cities.csv");

        cout << "Generating routes...\n";
        generateRoutes(graph);

        cout << "Travel Planner\n";
        cout << "==============\n\n";

        string startKey, endKey;
        startKey = selectCity(graph, "Enter starting city or country: ");

        int numStops;
        cout << "Enter the number of stops (0 for direct route): ";
        cin >> numStops;
        cin.ignore();

        vector<string> cityKeys = {startKey};
        vector<TransportMode> modes;

        // Collect intermediate stops and transport modes
        for (int i = 0; i < numStops; ++i) {
            string prompt = "Enter stop #" + to_string(i + 1) + " city or country: ";
            string stopKey = selectCity(graph, prompt);
            cityKeys.push_back(stopKey);

            string modeStr;
            cout << "Enter transport mode to reach this stop (Flight/Train/Bus/Car): ";
            getline(cin, modeStr);
            trim(modeStr);
            modes.push_back(stringToMode(modeStr));
        }

        endKey = selectCity(graph, "Enter destination city or country: ");
        cityKeys.push_back(endKey);

        // Get transport mode for the final leg
        if (numStops > 0 || !modes.empty()) {
            string modeStr;
            cout << "Enter transport mode from last stop to destination (Flight/Train/Bus/Car): ";
            getline(cin, modeStr);
            trim(modeStr);
            modes.push_back(stringToMode(modeStr));
        }

        size_t startComma = startKey.find(',');
        size_t endComma = endKey.find(',');
        string startCity = startKey.substr(startComma + 1);
        string startCountry = startKey.substr(0, startComma);
        string endCity = endKey.substr(endComma + 1);
        string endCountry = endKey.substr(0, endComma);

        cout << "\nPlanning trip from " << startCity << ", " << startCountry 
             << " to " << endCity << ", " << endCountry;
        if (numStops > 0) {
            cout << " with " << numStops << " stop(s)";
        }
        cout << "\n";

        PathResult result;
        if (numStops == 0 && modes.empty()) {
            // Direct route without stops
            result = findShortestPath(graph, startKey, endKey);
        } else {
            // Route with specified stops and modes
            result = findPathWithStops(graph, cityKeys, modes);
        }

        if (result.segments.empty()) {
            cout << "\nNo route found between the specified cities.\n";
        } else {
            cout << "\nOptimal Travel Plan:\n";
            cout << "------------------------------------------------------------\n";
            cout << left << setw(25) << "From" << setw(25) << "To" 
                 << setw(10) << "Mode" << setw(10) << "Distance" 
                 << setw(10) << "Time" << setw(10) << "Cost" << "\n";
            cout << "------------------------------------------------------------\n";

            for (const auto& seg : result.segments) {
                cout << left << setw(25) << (seg.fromCity + ", " + seg.fromCountry.substr(0, 15))
                     << setw(25) << (seg.toCity + ", " + seg.toCountry.substr(0, 15))
                     << setw(10) << modeToString(seg.mode)
                     << setw(10) << fixed << setprecision(1) << seg.distance << "km"
                     << setw(10) << fixed << setprecision(1) << seg.time << "hrs"
                     << "$" << fixed << setprecision(2) << seg.cost << "\n";
            }

            cout << "------------------------------------------------------------\n";
            cout << right << setw(60) << "Total: " 
                 << fixed << setprecision(1) << result.totalDistance << "km | "
                 << fixed << setprecision(1) << result.totalTime << "hrs | $"
                 << fixed << setprecision(2) << result.totalCost << "\n";
        }

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}