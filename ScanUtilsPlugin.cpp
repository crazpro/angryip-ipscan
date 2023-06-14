/*********************************************************************
 * This is a part of Angry IP Scanner source code                    *
 * http://www.angryziber.com/ipscan/                                 *
 *                                                                   *
 * Written by Angryziber                                             *
 *                                                                   *
 * You may distribute this code as long as this message is not       *
 * removed and it is clear who has written it.                       *
 * You may not rename the program and distribute it.                 *
 *********************************************************************/

// ScanUtilsPlugin.cpp: implementation of the CScanUtilsPlugin class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ipscan.h"
#include "ScanUtilsPlugin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScanUtilsPlugin::CScanUtilsPlugin()
{
}

CScanUtilsPlugin::~CScanUtilsPlugin()
{
}

void CScanUtilsPlugin::loadFromDir(CArray<TScannerColumn, TScannerColumn&> &columns, int &nColumnCount, CString &szDir)
{
	// This function loads plugins from the specified directory

	WIN32_FIND_DATA fileData;
	HANDLE hFind;

	hFind = FindFirstFile(szDir + "\\*.dll", &fileData);
	
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			CString szFileName = fileData.cFileName;

			// Load plugin
			HMODULE hPlugin = LoadLibrary(szDir + "\\" + szFileName);
			
			if (hPlugin != INVALID_HANDLE_VALUE)
			{
				// Let's check the type of plugin
				TInfoFunction *pInfoFunction = (TInfoFunction*) GetProcAddress(hPlugin, "Info");
				TInfoStruct infoStruct;
				
				if (pInfoFunction == NULL)
				{
					MessageBox(0, "No \"Info\" function exported from plugin:\n" + szDir + "\\" + szFileName + "\nPlease move it to another directory if this is not a plugin for Angry IP Scanner.", NULL, MB_ICONHAND | MB_OK);
					continue;
				}
				
				// Initialize info structure
				memset(&infoStruct, 0, sizeof(infoStruct));
				infoStruct.nStructSize = sizeof(infoStruct);
				infoStruct.nUniqueIndex = 0;	// TODO: a true index must be set here
		
				// Call plugin's info function
				pInfoFunction(&infoStruct);

				// Verify target Angry IP Scanner versiion
				if (infoStruct.nAngryIPScannerVersion > 218)	// TODO: 218 is hardcoded
				{
					MessageBox(0, "Plugin " + szFileName + " requires a newer version of Angry IP Scanner!", NULL, MB_ICONHAND | MB_OK);
					continue;
				}

				if (infoStruct.nPluginType != PLUGIN_TYPE_COLUMN)
					continue;	// Skip unknown types
								
				columns[nColumnCount].pInfoFunction = pInfoFunction;
				
				// Lookup optional functions
				columns[nColumnCount].pInitFunction = (TInitFunction*) GetProcAddress(hPlugin, "Init");
				columns[nColumnCount].pFinalizeFunction = (TFinalizeFunction*) GetProcAddress(hPlugin, "Finalize");				
				columns[nColumnCount].pOptionsFunction = (TOptionsFunction*) GetProcAddress(hPlugin, "Options");

				// This is a mandatory function for PLUGIN_TYPE_COLUMN
				if (infoStruct.nPluginType == PLUGIN_TYPE_COLUMN)
				{
					columns[nColumnCount].pScanFunction = (TScanFunction*) GetProcAddress(hPlugin, "Scan");

					if (columns[nColumnCount].pScanFunction == NULL)
					{
						MessageBox(0, "No \"Scan\" function exported from plugin: " + szFileName, NULL, MB_ICONHAND | MB_OK);
						continue;
					}
				}

				columns[nColumnCount].bBuiltinColumn = FALSE;				

				// Accept this plugin
				nColumnCount++;
			}
		}
		while (FindNextFile(hFind, &fileData));
	}

	FindClose(hFind);
}

void CScanUtilsPlugin::load(CArray<TScannerColumn, TScannerColumn&> &columns, int &nColumnCount)
{
	// This function loads plugins from the HDD and initializes structures passed as parameters

	// Load from default directory
	loadFromDir(columns, nColumnCount, g_options->m_szExecutablePath);

	// Load from dedicated directory
	loadFromDir(columns, nColumnCount, g_options->m_szExecutablePath + "\\" + PLUGINS_DIR);
}
