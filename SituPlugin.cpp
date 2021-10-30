#include "pch.h"
#include "SituPlugin.h"
#include "CSiTRadar.h"
#include "json.hpp"
#include "vatsimAPI.h"

using json = nlohmann::json;

const int TAG_ITEM_CTP_SLOT = 5000;
const int TAG_ITEM_CTP_CTOT = 5001;
const int TAG_ITEM_NAT_STATUS = 5002;
const int TAG_ITEM_NAT_NAT = 5003;
const int TAG_ITEM_NAT_FIX = 5004;
const int TAG_ITEM_NAT_LEVEL = 5005;
const int TAG_ITEM_NAT_MACH = 5006;
const int TAG_ITEM_NAT_ESTTIME = 5007;
const int TAG_ITEM_NAT_CLR = 5008;
const int TAG_ITEM_NAT_EXTRA = 5009;

SituPlugin::SituPlugin()
	: EuroScopePlugIn::CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
		"VATCAN Bookings",
		"1.1.5-amendfp",
		"VATCAN",
		"Attribution-NonCommercial 4.0 International (CC BY-NC 4.0)")
{
    RegisterTagItemType("Slot", TAG_ITEM_CTP_SLOT);
    RegisterTagItemType("CTOT", TAG_ITEM_CTP_CTOT);
    RegisterTagItemType("status", TAG_ITEM_NAT_STATUS);
    RegisterTagItemType("nat", TAG_ITEM_NAT_NAT);
    RegisterTagItemType("fix", TAG_ITEM_NAT_FIX);
    RegisterTagItemType("level", TAG_ITEM_NAT_LEVEL);
    RegisterTagItemType("mach", TAG_ITEM_NAT_MACH);
    RegisterTagItemType("estimating_time", TAG_ITEM_NAT_ESTTIME);
    RegisterTagItemType("clearance_issued", TAG_ITEM_NAT_CLR);
    RegisterTagItemType("extra_info", TAG_ITEM_NAT_EXTRA);

    CSiTRadar::eventCode = "Enter Code";

    try {

        std::ifstream settings_file(".\\vatcan_bookings.json");
        if (settings_file.is_open()) {
            json j = json::parse(settings_file);

            CSiTRadar::eventCode = j["evtCode"];
            CDataHandler::tagLabel = j["evtLabel"];
        }
        else {
            
            CSiTRadar::eventCode = "Enter Code";
            CDataHandler::tagLabel = "EVT";
        }
    }
    catch (std::ifstream::failure e) {

    };
}

SituPlugin::~SituPlugin()
{
}

EuroScopePlugIn::CRadarScreen* SituPlugin::OnRadarScreenCreated(const char* sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated)
{
    return new CSiTRadar;
}

void SituPlugin::OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan,
    EuroScopePlugIn::CRadarTarget RadarTarget,
    int ItemCode,
    int TagData,
    char sItemString[16],
    int* pColorCode,
    COLORREF* pRGB,
    double* pFontSize) {

    if (ItemCode == TAG_ITEM_CTP_SLOT) {
        if (CSiTRadar::mAcData[FlightPlan.GetCallsign()].hasCTP) {
            strcpy_s(sItemString, 16,"X");
        }
        else {
            strcpy_s(sItemString, 16, "");
        }
    }
    if (ItemCode == TAG_ITEM_CTP_CTOT) {
        strncpy_s(sItemString, 16, CSiTRadar::mAcData[FlightPlan.GetCallsign()].slotTime.c_str(), 15);
    }
    if (ItemCode == TAG_ITEM_NAT_STATUS) {
        strncpy_s(sItemString, 16, CSiTRadar::mAcData[FlightPlan.GetCallsign()].TAG_ITEM_NAT_STATUS.c_str(), 15);
    }
    if (ItemCode == TAG_ITEM_NAT_NAT) {
        strncpy_s(sItemString, 16, CSiTRadar::mAcData[FlightPlan.GetCallsign()].TAG_ITEM_NAT_NAT.c_str(), 15);
    }
    if (ItemCode == TAG_ITEM_NAT_FIX) {
        strncpy_s(sItemString, 16, CSiTRadar::mAcData[FlightPlan.GetCallsign()].TAG_ITEM_NAT_FIX.c_str(), 15);
    }
    if (ItemCode == TAG_ITEM_NAT_LEVEL) {
        strncpy_s(sItemString, 16, CSiTRadar::mAcData[FlightPlan.GetCallsign()].TAG_ITEM_NAT_LEVEL.c_str(), 15);
    }
    if (ItemCode == TAG_ITEM_NAT_MACH) {
        strncpy_s(sItemString, 16, CSiTRadar::mAcData[FlightPlan.GetCallsign()].TAG_ITEM_NAT_MACH.c_str(), 15);
    }
    if (ItemCode == TAG_ITEM_NAT_ESTTIME) {
       strncpy_s(sItemString, 16, CSiTRadar::mAcData[FlightPlan.GetCallsign()].TAG_ITEM_NAT_ESTTIME.c_str(), 15);
    }
    if (ItemCode == TAG_ITEM_NAT_CLR) {
        string clearedTime;
        if (CSiTRadar::mAcData[FlightPlan.GetCallsign()].TAG_ITEM_NAT_CLR.length() > 16) {
            clearedTime = CSiTRadar::mAcData[FlightPlan.GetCallsign()].TAG_ITEM_NAT_CLR.substr(11, 5);
        }
        strncpy_s(sItemString, 16, clearedTime.c_str(), 15);
    }
    if (ItemCode == TAG_ITEM_NAT_EXTRA) {
        strncpy_s(sItemString, 16, CSiTRadar::mAcData[FlightPlan.GetCallsign()].TAG_ITEM_NAT_EXTRA.c_str(), 15);
    }
}