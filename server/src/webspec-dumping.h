/*
 *  webspec-dumping.h
 *  WebSpec project
 *  
 *  Based on code from GameDeception poster Xero|Hawk
 *
 */

#ifndef WEBSPEC_DUMPING_H
#define WEBSPEC_DUMPING_H

#include "webspec.h"
#include "server_class.h"

#ifdef DEBUG

//=================================================================================
// Functions to dump all properties & offsets from all classes in the server DLL
// Will spam the console with them, so use -condebug launch option to have it
// printed to tf/console.log for searching in a notepad.
// 
// To use, just call DumpClasses from the plugin's Load function after the Valve
// interfaces have been loaded.
// 
// Example output, with other console messages removed: 
//   https://dl.dropbox.com/u/6635591/links/tf-srcds-ClassDump.txt
//=================================================================================

static void DumpSendTable(SendTable *pTable, int Offset = 0, int SpaceCount = 0, bool FirstArrayPropOnly = false) {
	char *Spaces = new char[SpaceCount+1];
	memset((void *)Spaces, (int)' ', SpaceCount);
	Spaces[SpaceCount] = 0;
	Msg("%s SendTable: %s\n", Spaces, pTable->GetName());
	for (int i=0; i < pTable->GetNumProps(); i++) {
		SendProp *pProp = pTable->GetProp(i);
		if (FirstArrayPropOnly && strcmp(pProp->GetName(),"000") != 0) 
			continue;

		SendTable *pChildTable = pProp->GetDataTable();
		bool IsArray = false;
		if (pChildTable && pChildTable->GetNumProps() > 0) {
			if (   !strcmp(pChildTable->GetProp(0)->GetName(), "000")
				|| !strcmp(pChildTable->GetProp(0)->GetName(), "lengthproxy"))
				IsArray = true;
		}

		if (!IsArray) {
			if (!Offset)
				Msg("%s Prop %s with offset %X\n", Spaces, pProp->GetName(), pProp->GetOffset());
			else
				Msg("%s Prop %s with offset %X (absolute offset %X)\n", Spaces, pProp->GetName(), pProp->GetOffset(), pProp->GetOffset()+Offset);
			if (pProp->GetType() == DPT_DataTable)
				DumpSendTable(pChildTable, pProp->GetOffset()+Offset, SpaceCount+1);
		} else {
			if (!Offset)
				Msg("%s Array %s of %i elements with offset %X\n", Spaces, pProp->GetName(), pTable->GetNumProps(), pProp->GetOffset());
			else
				Msg("%s Array %s of %i elements with offset %X (absolute offset %X)\n", Spaces, pProp->GetName(), pTable->GetNumProps(), pProp->GetOffset(), pProp->GetOffset()+Offset);
		}

		if (FirstArrayPropOnly && !strcmp(pProp->GetName(), "000"))
			break;
	}
	delete Spaces;
}

static void DumpClasses() {
	ServerClass *pClass = serverGameDLL->GetAllServerClasses();
	while(pClass) {
		Msg("Class %s\n", pClass->GetName());
		SendTable *pSendTable = pClass->m_pTable;
		if (pSendTable)
			DumpSendTable(pSendTable);

		pClass = pClass->m_pNext;
	}
}

#endif //DEBUG

#endif
