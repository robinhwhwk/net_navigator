#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Helper function to perform HTTP GET requests
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t totalSize = size * nmemb;
    s->append((char*)contents, totalSize);
    return totalSize;
}

std::string httpGet(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    return readBuffer;
}

// Get my location details (MyIP, longitude, latitude, city)
std::tuple<std::string, std::pair<double, double>, std::string> getMyLoc() {
    std::string url = "https://ipapi.is/json/";
    std::string response = httpGet(url);
    auto data = json::parse(response);

    try {
        std::string myIP = data["ip"];
        double lon = data["location"]["longitude"];
        double lat = data["location"]["latitude"];
        std::string city = data["location"]["city"];
        return {myIP, {lon, lat}, city};
    } catch (json::exception& e) {
        std::cerr << "Error: " << e.what() << " Not Found" << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Get target location details (IP, longitude, latitude, city)
std::tuple<std::string, std::pair<double, double>, std::string> getTargetLoc(const std::string& IP) {
    std::string url = "https://ipapi.is/" + IP + "/json/";
    std::string response = httpGet(url);
    auto data = json::parse(response);

    try {
        double lon = data["longitude"];
        double lat = data["latitude"];
        std::string city = data["city"];
        return {IP, {lon, lat}, city};
    } catch (json::exception& e) {
        std::cerr << "Error: " << e.what() << " Not Found" << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Get locations for a list of IP addresses
std::vector<std::tuple<std::string, std::pair<double, double>, std::string>> getLoc(const std::vector<std::string>& ipList) {
    std::vector<std::tuple<std::string, std::pair<double, double>, std::string>> locationList;

    for (const auto& ipAddress : ipList) {
        std::string url = "https://ipapi.is/" + ipAddress + "/json/";
        std::string response = httpGet(url);
        auto data = json::parse(response);

        // Check if IP is private IP
        if (data.contains("error") && data["error"] == true) {
            continue;
        }

        double lon = data["longitude"];
        double lat = data["latitude"];
        if (lon == 0 || lat == 0) {
            continue; // Skip if longitude or latitude is invalid
        }
        std::string city = data["city"];
        locationList.push_back({ipAddress, {lon, lat}, city});
    }

    return locationList;
}

int main() {
    // Example usage of getMyLoc
    auto [myIP, myCoordinates, myCity] = getMyLoc();
    std::cout << "My IP: " << myIP << ", Location: (" << myCoordinates.first << ", " << myCoordinates.second << "), City: " << myCity << std::endl;

    // Example usage of getTargetLoc
    std::string targetIP = "8.8.8.8";
    auto [targetIPAddr, targetCoordinates, targetCity] = getTargetLoc(targetIP);
    std::cout << "Target IP: " << targetIPAddr << ", Location: (" << targetCoordinates.first << ", " << targetCoordinates.second << "), City: " << targetCity << std::endl;

    // Example usage of getLoc with a list of IP addresses
    std::vector<std::string> ipList = {"8.8.8.8", "8.8.4.4"};
    auto locations = getLoc(ipList);
    for (const auto& [ip, coordinates, city] : locations) {
        std::cout << "IP: " << ip << ", Location: (" << coordinates.first << ", " << coordinates.second << "), City: " << city << std::endl;
    }

    return 0;
}
