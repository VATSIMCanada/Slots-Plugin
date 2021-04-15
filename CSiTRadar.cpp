#include "pch.h"
#include "CSiTRadar.h"
#include "TopMenu.h"
#include "SituPlugin.h"
#include "vatsimAPI.h"

map<string, ACData> CSiTRadar::mAcData;
map<int, string> CSiTRadar::slotTime;
bool CSiTRadar::canAmend;
int CSiTRadar::refreshStatus;
int CSiTRadar::amendStatus;
string CSiTRadar::eventCode;
POINT CSiTRadar::menu{ 10, 40 };
int CSiTRadar::tagLocation{ 2 };

CSiTRadar::CSiTRadar()
{	
	CSiTRadar::eventCode = "Enter Code";
	CDataHandler::GetVatsimAPIurlData();

	time = clock();
	oldTime = clock();

	amendStatus = 1;
}

void CSiTRadar::OnRefresh(HDC hdc, int phase)
{


	if (phase != REFRESH_PHASE_AFTER_TAGS && phase != REFRESH_PHASE_BEFORE_TAGS) {
		return;
	}

	// set up the drawing renderer
	CDC dc;
	dc.Attach(hdc);

	if (phase == REFRESH_PHASE_AFTER_TAGS) {

		for (CRadarTarget radarTarget = GetPlugIn()->RadarTargetSelectFirst(); radarTarget.IsValid();
			radarTarget = GetPlugIn()->RadarTargetSelectNext(radarTarget))
		{
			POINT p = ConvertCoordFromPositionToPixel(radarTarget.GetPosition().GetPosition());
			string callSign = radarTarget.GetCallsign();

			if (radarTarget.GetPosition().GetRadarFlags() != 0) {

				if (mAcData[callSign].hasCTP) {
					CFont font;
					LOGFONT lgfont;

					memset(&lgfont, 0, sizeof(LOGFONT));
					lgfont.lfWeight = 500;
					strcpy_s(lgfont.lfFaceName, _T("EuroScope"));
					lgfont.lfHeight = 12;
					font.CreateFontIndirect(&lgfont);

					dc.SetTextColor(RGB(255,132,0));

					RECT rectPAM;
					
					// Toggle between location of tag, right, bottom or left
					switch (tagLocation) {
					case 0:
						continue;
					case 1:
						rectPAM.left = p.x + 10;
						rectPAM.right = p.x + 90; rectPAM.top = p.y - 6;	rectPAM.bottom = p.y + 30;
						break;
					case 2:
						rectPAM.left = p.x - 9;
						rectPAM.right = p.x + 75; rectPAM.top = p.y + 8;	rectPAM.bottom = p.y + 30;
						break;
					case 3:
						rectPAM.left = p.x - 30;
						rectPAM.right = p.x + 60; rectPAM.top = p.y - 6;	rectPAM.bottom = p.y + 30;
						break;
					case 4:
						rectPAM.left = p.x - 9;
						rectPAM.right = p.x + 60; rectPAM.top = p.y - 18;	rectPAM.bottom = p.y + 30;
						break;
					}

					dc.DrawText(CDataHandler::tagLabel.c_str(), &rectPAM, DT_LEFT);

					DeleteObject(font);
				}
			}
		}


		RECT but;

		but = TopMenu::DrawButton(&dc, menu, 60, 23, "Refresh", autoRefresh);
		ButtonToScreen(this, but, "Refresh Slot Data", BUTTON_MENU_REFRESH);

		POINT m2 = menu;
		m2.x = menu.x + 65;
		but = TopMenu::DrawButton(&dc, m2, 60, 23, CSiTRadar::eventCode.c_str(), 0);
		ButtonToScreen(this, but, "Settings", BUTTON_MENU_SETTINGS);

		// if correct event code, draw a green square:

		COLORREF targetPenColor = RGB(0, 150, 50);
		if (amendStatus == 1) {
			targetPenColor = RGB(255, 188, 5);
		}
		if (amendStatus == 2) {
			targetPenColor = RGB(153, 36, 0);
		}
			InflateRect(&but, 1, 1);
			HPEN targetPen = CreatePen(PS_SOLID, 2, targetPenColor);
			dc.SelectObject(targetPen);
			dc.SelectStockObject(NULL_BRUSH);
			dc.Rectangle(&but);

			// Indicator to show tag is hidden
			if (tagLocation == 0) {
				dc.MoveTo(but.right - 5, but.top + 3);
				dc.LineTo(but.right - 5, but.top + 5);
			}

			DeleteObject(targetPen);		
	}

	if (autoRefresh) {
		time = clock();
		if ((time - oldTime) / CLOCKS_PER_SEC > CDataHandler::refreshInterval) {
			
			CAsync* data = new CAsync();
			data->Plugin = GetPlugIn();
			_beginthread(CDataHandler::GetVatsimAPIData, 0, (void*) data);
			oldTime = clock();
		}
	}
	dc.Detach();
}

void CSiTRadar::OnClickScreenObject(int ObjectType,
	const char* sObjectId,
	POINT Pt,
	RECT Area,
	int Button)
{
	if (ObjectType == BUTTON_MENU_REFRESH) {
		if (Button == BUTTON_LEFT) { 
			
			CSiTRadar::canAmend = FALSE;
			CSiTRadar::amendStatus = 1;
			CAsync* data = new CAsync();
			data->Plugin = GetPlugIn();
			_beginthread(CDataHandler::GetVatsimAPIData, 0, (void*) data);
				
			oldTime = clock(); }
		if (Button == BUTTON_RIGHT) { autoRefresh = !autoRefresh; }
	}

	if (ObjectType == BUTTON_MENU_SETTINGS) {
		if (Button == BUTTON_LEFT) {
			GetPlugIn()->OpenPopupEdit(Area, FUNCTION_SET_URL, CSiTRadar::eventCode.c_str());
		}

		if (Button == BUTTON_RIGHT) {
			// toggle between tag locations
			if (tagLocation < 4) { tagLocation++; }
			else { tagLocation = 0; }
		}
	}
}

void CSiTRadar::OnMoveScreenObject(int ObjectType,
	const char* sObjectId,
	POINT Pt,
	RECT Area,
	bool Released) {

	
	if (ObjectType == BUTTON_MENU_REFRESH) {
		menu.x = Pt.x - 30;
		menu.y = Pt.y - 12;
	}
}

void CSiTRadar::ButtonToScreen(CSiTRadar* radscr, RECT rect, string btext, int itemtype) {
	AddScreenObject(itemtype, btext.c_str(), rect, 1, "");
}

void CSiTRadar::OnFlightPlanDisconnect(CFlightPlan FlightPlan) {
	string callSign = FlightPlan.GetCallsign();

	mAcData.erase(callSign);
}

void CSiTRadar::OnFunctionCall(int FunctionId,
	const char* sItemString,
	POINT Pt,
	RECT Area) {
	if (FunctionId == FUNCTION_SET_URL) {
		try {
			CSiTRadar::eventCode = sItemString;
		}
		catch (...) {}
	}
}

CSiTRadar::~CSiTRadar()
{
}