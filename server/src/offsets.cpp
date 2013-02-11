/*
 *  offsets.cpp
 *  WebSpec project
 *  
 *  Copyright (c) 2013 Matthew McNamara
 *  BSD License
 *  http://opensource.org/licenses/BSD-3-Clause
 *
 */

#include "offsets.h"

int WSOffsets::pCTFPlayer__m_iClass = 0;
int WSOffsets::pCWeaponMedigun__m_flChargeLevel = 0;

void WSOffsets::PrepareOffsets() {
	WSOffsets::pCTFPlayer__m_iClass = WSOffsets::FindOffsetOfClassProp("CTFPlayer", "m_iClass");
	WSOffsets::pCWeaponMedigun__m_flChargeLevel = WSOffsets::FindOffsetOfClassProp("CWeaponMedigun", "m_flChargeLevel");
}

int WSOffsets::FindOffsetOfClassProp(const char *className, const char *propName) {
	ServerClass *sc = serverGameDLL->GetAllServerClasses();
	while (sc) {
		if (Q_strcmp(sc->GetName(), className) == 0) {
			SendTable *sTable = sc->m_pTable;
			if (sTable) {
				int offset = 0;
				bool found = WSOffsets::CrawlForPropOffset(sTable, propName, offset);
				if (!found)
					offset = 0;
				return offset;
			}
		}
		sc = sc->m_pNext;
	}
	return 0;
}

bool WSOffsets::CrawlForPropOffset(SendTable *sTable, const char *propName, int &offset) {
	for (int i=0; i < sTable->GetNumProps(); i++) {
		SendProp *sProp = sTable->GetProp(i);
		if (strcmp(sProp->GetName(),"000") == 0) //End of an array
			continue;

		SendTable *sChildTable = sProp->GetDataTable();

		//Check if it is an array, don't care for these atm so skip them
		bool isArray = false;
		if (sChildTable && sChildTable->GetNumProps() > 0) {
			if (   !strcmp(sChildTable->GetProp(0)->GetName(), "000")
				|| !strcmp(sChildTable->GetProp(0)->GetName(), "lengthproxy"))
				isArray = true;
		}

		if (!isArray) {
			//If we have our property, add to the offset and start returning
			if (strcmp(sProp->GetName(), propName) == 0) {
				offset += sProp->GetOffset();
				return true;
			}
			
			//If we find a subtable, search it for the property, 
			//but keep current offset in case it isn't found here
			if (sProp->GetType() == DPT_DataTable) {
				int origOffset = offset;
				offset += sProp->GetOffset();
				bool found = WSOffsets::CrawlForPropOffset(sChildTable, propName, offset);
				if (found) {
					return true;
				} else {
					offset = origOffset;
				}
			}
		} else {
			continue;
		}

		if (strcmp(sProp->GetName(), "000") == 0) //More array stuff from dumping function, may not be needed here
			break;
	}
	return false;
}
