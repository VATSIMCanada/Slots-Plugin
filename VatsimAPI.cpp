#include "pch.h"
#include "vatsimAPI.h"
#include "SituPlugin.h"
#include "CSiTRadar.h"
#include "json.hpp"
#include "curl/curl.h"

using json = nlohmann::json;

string CDataHandler::url1;
int CDataHandler::refreshInterval;
string CDataHandler::tagLabel;
string CDataHandler::vatsimJson3URL;
bool CDataHandler::firstSlotPull = TRUE;
clock_t CDataHandler::timeSlotUpdate = clock();
clock_t CDataHandler::oldTime = clock();

static size_t write_data(void* buffer, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)buffer, size * nmemb);
    return size * nmemb;
}

void setup_curl_modern(CURL* h, const char* url, string* response) {
    curl_easy_setopt(h, CURLOPT_URL, url);
    curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(h, CURLOPT_WRITEDATA, response);
    curl_easy_setopt(h, CURLOPT_FOLLOWLOCATION, 1L);

    // --- STATIC NATIVE WINDOWS SSL ---
    // No cert file needed because it's baked into your DLL 
    // and using the Windows OS trust store
    curl_easy_setopt(h, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NATIVE_CA);
    curl_easy_setopt(h, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NO_REVOKE);

    curl_easy_setopt(h, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(h, CURLOPT_SSL_VERIFYHOST, 2L);
    // ---------------------------------

    curl_easy_setopt(h, CURLOPT_TIMEOUT, 10L);
}

void CDataHandler::GetVatsimAPIurlData() {
    CURL* vatsimStatus = curl_easy_init();
    string vatsimURLs;
    if (vatsimStatus) {
        setup_curl_modern(vatsimStatus, "https://status.vatsim.net/", &vatsimURLs);
        if (curl_easy_perform(vatsimStatus) == CURLE_OK) {
            size_t pos = vatsimURLs.find("json3=");
            if (pos != string::npos) {
                vatsimURLs = vatsimURLs.substr(pos + 6);
                CDataHandler::vatsimJson3URL = vatsimURLs.substr(0, vatsimURLs.find("\n") - 1);
            }
        }
        curl_easy_cleanup(vatsimStatus);
    }
}

void CDataHandler::GetVatsimAPIData(void* args) {
    CAsync* data = (CAsync*)args;
    CDataHandler::url1 = "https://bookings.vatcan.ca/api/event/" + CSiTRadar::eventCode;
    CDataHandler::refreshInterval = 60;
    CDataHandler::tagLabel = "EVT";

    string cidString;
    json cidJson;
    CDataHandler::timeSlotUpdate = clock();

    // --- PART 1: Fetch VATCAN Slots ---
    if (((CDataHandler::timeSlotUpdate - CDataHandler::oldTime) / CLOCKS_PER_SEC > 300) || CDataHandler::firstSlotPull) {
        CURL* curl = curl_easy_init();
        if (curl) {
            setup_curl_modern(curl, CDataHandler::url1.c_str(), &cidString);
            CURLcode res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            if (res == CURLE_OK) {
                try {
                    cidJson = json::parse(cidString);
                    if (cidJson.contains("error")) {
                        string error = cidJson.at("error");
                        data->Plugin->DisplayUserMessage("VATCAN Slot Manager", "API Error", error.c_str(), true, true, true, true, true);
                        CSiTRadar::amendStatus = 2;
                    }
                    else {
                        data->Plugin->DisplayUserMessage("VATCAN Slot Manager", "Update Successful", "Slot times parsed", true, false, false, false, false);
                    }
                }
                catch (exception& e) {
                    data->Plugin->DisplayUserMessage("VATCAN Slot Manager", "Parse Error", (string("Slots failed: ") + e.what()).c_str(), true, true, true, true, true);
                }
            }
            else {
                string errStr = curl_easy_strerror(res);
                data->Plugin->DisplayUserMessage("VATCAN Slot Manager", "Network Error", (string("Connect failed: ") + errStr).c_str(), true, true, true, true, true);
            }
        }
        CDataHandler::oldTime = clock();
        CDataHandler::firstSlotPull = FALSE;
    }

    // --- PART 2: Fetch VATSIM Data File ---
    string responseString;
    CURL* curl1 = curl_easy_init();
    if (curl1 && !CDataHandler::vatsimJson3URL.empty()) {
        setup_curl_modern(curl1, CDataHandler::vatsimJson3URL.c_str(), &responseString);
        CURLcode res = curl_easy_perform(curl1);
        curl_easy_cleanup(curl1);

        if (res == CURLE_OK) {
            try {
                auto jsonArray = json::parse(responseString);
                if (!cidJson.empty()) {
                    for (auto& pilots : cidJson) {
                        int cid = pilots["cid"];
                        string slot = pilots["slot"];
                        CSiTRadar::slotTime[cid] = slot;
                    }
                }
                if (jsonArray.contains("pilots") && !jsonArray["pilots"].empty()) {
                    for (auto& array : jsonArray["pilots"]) {
                        string apiCallsign = array["callsign"];
                        int apiCID = array["cid"];
                        CSiTRadar::mAcData[apiCallsign].CID = to_string(apiCID);
                        if (CSiTRadar::slotTime.count(apiCID)) {
                            CSiTRadar::mAcData[apiCallsign].slotTime = CSiTRadar::slotTime[apiCID];
                            CSiTRadar::mAcData[apiCallsign].hasCTP = TRUE;
                        }
                        else {
                            CSiTRadar::mAcData[apiCallsign].slotTime = "";
                            CSiTRadar::mAcData[apiCallsign].hasCTP = FALSE;
                        }
                    }
                }
                string timeStamp = jsonArray["general"]["update_timestamp"];
                data->Plugin->DisplayUserMessage("VATCAN Slot Manager", "Update Successful", (string("CIDs fetched at ") + timeStamp).c_str(), true, false, false, false, false);
            }
            catch (exception& e) {
                data->Plugin->DisplayUserMessage("VATCAN Slot Manager", "Parse Error", (string("VATSIM Data failed: ") + e.what()).c_str(), true, true, true, true, true);
            }
        }
    }

    // --- PART 3: Fetch NATTRACK ---
    string NATTrackResponse;
    CURL* curlNATTrack = curl_easy_init();
    if (curlNATTrack) {
        setup_curl_modern(curlNATTrack, "https://nattrak.vatsim.net/api/plugins", &NATTrackResponse);
        if (curl_easy_perform(curlNATTrack) == CURLE_OK) {
            try {
                auto jsonNATTrack = json::parse(NATTrackResponse);
                for (auto& array : jsonNATTrack) {
                    string natCallsign = array["callsign"];
                    if (CSiTRadar::mAcData.count(natCallsign)) {
                        auto& d = CSiTRadar::mAcData[natCallsign];
                        if (!array["status"].is_null()) d.TAG_ITEM_NAT_STATUS = array["status"];
                        if (!array["nat"].is_null()) d.TAG_ITEM_NAT_NAT = array["nat"];
                        if (!array["fix"].is_null()) d.TAG_ITEM_NAT_FIX = array["fix"];
                        if (!array["level"].is_null()) d.TAG_ITEM_NAT_LEVEL = array["level"];
                        if (!array["mach"].is_null()) d.TAG_ITEM_NAT_MACH = array["mach"];
                        if (!array["estimating_time"].is_null()) d.TAG_ITEM_NAT_ESTTIME = array["estimating_time"];
                        if (!array["clearance_issued"].is_null()) d.TAG_ITEM_NAT_CLR = array["clearance_issued"];
                        if (!array["extra_info"].is_null()) d.TAG_ITEM_NAT_EXTRA = array["extra_info"];
                    }
                }
                data->Plugin->DisplayUserMessage("VATCAN Slot Manager", "Update Successful", "NAT Track Updated", true, false, false, false, false);
            }
            catch (...) {}
        }
        curl_easy_cleanup(curlNATTrack);
    }

    CSiTRadar::canAmend = TRUE;
    CSiTRadar::amendStatus = 0;
    delete args;
}

void CDataHandler::AmendFlightPlans(void* args) {

    CAsync* data = (CAsync*)args;
    string oldRemarks;
    string newRemarks;
    int countFP = 0;

    struct tm gmt;
    time_t t = time(0);
    gmtime_s(&gmt, &t);

    char timeStr[50];
    strftime(timeStr, 50, "%H%MZ", &gmt);

    for (CFlightPlan flightPlan = data->Plugin->FlightPlanSelectFirst(); flightPlan.IsValid();
        flightPlan = data->Plugin->FlightPlanSelectNext(flightPlan)) {
        oldRemarks = flightPlan.GetFlightPlanData().GetRemarks();

        // if the callsign has not been correlated with a CID, don't try to amend the Flightplan yet, needs update from vatsim status
        if (CSiTRadar::mAcData[flightPlan.GetCallsign()].CID == "") { continue; }

        if (flightPlan.GetFlightPlanData().IsReceived()) {

            if (CSiTRadar::mAcData[flightPlan.GetCallsign()].hasCTP == TRUE) {
                oldRemarks = flightPlan.GetFlightPlanData().GetRemarks();

                if (oldRemarks.find("CTP SLOT") == string::npos) {

                    newRemarks = (string)"CTP SLOT / " + timeStr + " " + oldRemarks;
                    flightPlan.GetFlightPlanData().SetRemarks(newRemarks.c_str());
                }
            }
            // If someone adds CTP SLOT to their remarks, but isn't on the list, then flag this in the remarks
            else {
                if (oldRemarks.find("CTP SLOT") != string::npos && oldRemarks.find("CTP MISMATCH") == string::npos) {
                    newRemarks = (string)"CTP MISMATCH / NON EVENT / " + timeStr + " " + oldRemarks;

                    flightPlan.GetFlightPlanData().SetRemarks(newRemarks.c_str());
                }

                else if (oldRemarks.find("NON EVENT") == string::npos) {
                    newRemarks = (string)"NON EVENT / " + timeStr + " " + oldRemarks;

                    flightPlan.GetFlightPlanData().SetRemarks(newRemarks.c_str());
                }
            }
            flightPlan.GetFlightPlanData().AmendFlightPlan();
        }
        countFP++;
    }
    data->Plugin->DisplayUserMessage("Slot Helper", "Success", (to_string(countFP) + (string)" Flight Plans Updated").c_str(), true, false, false, false, false);

    delete args;
}