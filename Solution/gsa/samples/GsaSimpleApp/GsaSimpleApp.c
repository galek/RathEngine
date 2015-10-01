// Copyright (c) 2014, NVIDIA CORPORATION. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

////////////////////////////////////////////////////////////////////////////////
/// @file GsaSimpleApp.h
/// @brief Simple GFE Settings API application.
///
/// This application registers a small set of options, and each time it is run
/// it queries and prints the current value of each option. The options are
/// each incremented and saved so that the next time the app is run, you should
/// see different values for each option.
////////////////////////////////////////////////////////////////////////////////

#include <NvGsa.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

////////////////////////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////////////////////////
void printGsaVersion(const NvGsaVersion *version);

bool initApplication(NvGsaApplication *app, const wchar_t *exePath);
void releaseApplication(NvGsaApplication *app);

bool initOptions(const NvGsaNamedOption **options, size_t *numOptions);
void releaseOptions(const NvGsaNamedOption *options);

bool initResolutions(const NvGsaResolution **resolutions, size_t *numResolutions);
void releaseResolutions(const NvGsaResolution *resolutions);

void printOptionValue(const NvGsaVariant *value, const NvGsaNamedOption *option);
void cycleOptionValue(NvGsaVariant *result, const NvGsaVariant *value, const NvGsaNamedOption *option);

void printResolution(const NvGsaResolution *resolution);
void cycleResolution(NvGsaResolution *resolution, const NvGsaResolution *availableResolutions, size_t numResolutions);

////////////////////////////////////////////////////////////////////////////////
// wmain()
////////////////////////////////////////////////////////////////////////////////
int wmain(int argc, wchar_t **argv)
{
	NvGsaApplication app;
	NvGsaStatus status;

	const NvGsaNamedOption *options = NULL;
	size_t numOptions = 0;

	const NvGsaResolution *resolutions = NULL;
	size_t numResolutions = 0;

	NvGsaVersion runtimeVersion = GFSDK_GSA_GetVersion();

	// Print the GSA compile time version
	wprintf(L"Compiled against GSA version: ");
	printGsaVersion(&NvGsaCurrentVersion);
	wprintf(L"\n");

	// Print the GSA run time version
	wprintf(L"Running against GSA version: ");
	printGsaVersion(&runtimeVersion);
	wprintf(L"\n");

	// Initialize the application
	if (argc < 1 || !initApplication(&app, argv[0])) {
		wprintf(L"Failed to initialize the application, exiting!");
		return -1;
	}

	// Initialize the GSA SDK
    status = GFSDK_GSA_InitializeSDK(&app, &NvGsaCurrentVersion);
	releaseApplication(&app);
	if (status != NV_GSA_STATUS_OK) {
		wprintf(L"Failed to initialize the GSA SDK, exiting!");
		return -2;
	}

	if (initOptions(&options, &numOptions) &&
		initResolutions(&resolutions, &numResolutions)) {

		NvGsaResolution currentResolution;
		size_t i;
	
		// Register the options
		for (i = 0; i < numOptions; ++i) {
			GFSDK_GSA_RegisterOption(&options[i]);
		}

		// Register the resolutions
		GFSDK_GSA_RegisterResolutions(resolutions, (int)numResolutions);

		// Load the config file
        status = GFSDK_GSA_LoadConfigFile();
		if (status == NV_GSA_STATUS_FILENOTFOUND) {
			GFSDK_GSA_SetResolution(&resolutions[0]);
		}

		// Print and cycle the options
		for (i = 0; i < numOptions; ++i) {
			NvGsaVariant value;
			value.type = options[i].value.type;
			status = GFSDK_GSA_GetOptionValue(&value, options[i].name);
			if (status == NV_GSA_STATUS_OK) {
				NvGsaVariant newValue;
				printOptionValue(&value, &options[i]);
				cycleOptionValue(&newValue, &value, &options[i]);
				status = GFSDK_GSA_SetOptionValue(&newValue, options[i].name);
				GFSDK_GSA_ReleaseVariant(&value);
			}
		}

		// Print and increment the resolution
		status = GFSDK_GSA_GetResolution(&currentResolution);
		printResolution(&currentResolution);
		cycleResolution(&currentResolution, resolutions, numResolutions);
		status = GFSDK_GSA_SetResolution(&currentResolution);

		// Save the config file if options have changed
		if (GFSDK_GSA_CheckForDirtyOptions() == NV_GSA_STATUS_DIRTY_OPTIONS_FOUND) {
			GFSDK_GSA_SaveConfigFile(NV_GSA_SAVE_SKIP_UNREGISTERED);
		}
	}

	// Cleanup
	releaseOptions(options);
	releaseResolutions(resolutions);
	GFSDK_GSA_ReleaseSDK();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// printGsaVersion()
////////////////////////////////////////////////////////////////////////////////
void printGsaVersion(const NvGsaVersion *version)
{
	if (version != NULL) {
		wprintf(
			L"%d.%d.%d.%d",
			version->major,
			version->minor,
			version->revision,
			version->build);
	}
}

////////////////////////////////////////////////////////////////////////////////
// initApplication()
////////////////////////////////////////////////////////////////////////////////
bool initApplication(NvGsaApplication *app, const wchar_t *exePath)
{
	wchar_t fullExePath[_MAX_PATH];
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t exe[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];
	wchar_t fullExeDir[_MAX_PATH];
	wchar_t fullExeName[_MAX_PATH];

	// Get the full executable path.
	if (_wfullpath(fullExePath, exePath, _MAX_PATH) == NULL) {
		return false;
	}

	// Split the executable path
	_wsplitpath_s(
		fullExePath,
		drive, _MAX_DRIVE,
		dir, _MAX_DIR,
		exe, _MAX_FNAME,
		ext, _MAX_EXT);

	// Get the executable directory
	if (0 > swprintf_s(fullExeDir, _MAX_PATH, L"%s%s", drive, dir)) {
		return false;
	}

	// Get the executable name with extension
	if (0 > swprintf_s(fullExeName, _MAX_PATH, L"%s%s", exe, ext)) {
		return false;
	}

	// Setup the application fields
	app->displayName = _wcsdup(L"GsaSimpleApp");
    app->versionName = _wcsdup(L"1.0.0.0");
	app->installPath = _wcsdup(fullExeDir);
    app->configPath = _wcsdup(fullExeDir);
	app->executable = _wcsdup(exe);
	app->execCmd = _wcsdup(fullExeName);

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// releaseApplication()
////////////////////////////////////////////////////////////////////////////////
void releaseApplication(NvGsaApplication *app)
{
	if (app != NULL) {
		// Free the application fields
		free((void *)app->displayName);
		app->displayName = NULL;
		free((void *)app->versionName);
		app->versionName = NULL;
		free((void *)app->installPath);
		app->installPath = NULL;
		free((void *)app->configPath);
		app->configPath = NULL;
		free((void *)app->executable);
		app->executable = NULL;
		free((void *)app->execCmd);
		app->execCmd = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////
// initOptions()
////////////////////////////////////////////////////////////////////////////////
bool initOptions(const NvGsaNamedOption **options, size_t *numOptions)
{
	static const wchar_t *testEnums[] = {
		L"Value1",
		L"Value2",
		L"Value3"
	};

	static size_t numTestEnums = sizeof(testEnums) / sizeof(const wchar_t *);

	NvGsaNamedOption *results = NULL;

	if (options == NULL) {
		return false;
	}

	results = (NvGsaNamedOption *)malloc(4 * sizeof(NvGsaNamedOption));
	if (results == NULL) {
		return false;
	}

	results[0].name = L"Test_INT";
	results[0].value.type = NV_GSA_TYPE_INT;
	results[0].value.asInt = 35;
	results[0].numRange.maxValue.asInt = 0;
	results[0].numRange.minValue.asInt = 0;
	results[0].numRange.numSteps = -1;

	results[1].name = L"Test_FLOAT";
	results[1].value.type = NV_GSA_TYPE_FLOAT;
	results[1].value.asFloat = 35.5f;
	results[1].numRange.maxValue.asFloat = 0.0f;
	results[1].numRange.minValue.asFloat = 0.0f;
	results[1].numRange.numSteps = -1;
		

	results[2].name = L"Test_ENUM";
	results[2].value.type = NV_GSA_TYPE_ENUM;
	results[2].value.asEnum = L"Value1";
	results[2].enumRange.enums = testEnums;
	results[2].enumRange.numEnums = (uint32_t)numTestEnums;
	
	results[3].name = L"Test_BOOL";
	results[3].value.type = NV_GSA_TYPE_BOOL;
	results[3].value.asBool = true;

	*options = results;
	if (numOptions != NULL) {
		*numOptions = 4;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// releaseOptions()
////////////////////////////////////////////////////////////////////////////////
void releaseOptions(const NvGsaNamedOption *options)
{
	free((void *)options);
}

////////////////////////////////////////////////////////////////////////////////
// initResolutions()
////////////////////////////////////////////////////////////////////////////////
bool initResolutions(const NvGsaResolution **resolutions, size_t *numResolutions)
{
	NvGsaResolution *results = NULL;

	if (resolutions == NULL) {
		return false;
	}

	results = (NvGsaResolution *)malloc(4 * sizeof(NvGsaResolution));
	if (results == NULL) {
		return false;
	}

	results[0].width = 800;
	results[0].height = 600;
	results[0].refreshRate = 60;

	results[1].width = 1024;
	results[1].height = 768;
	results[1].refreshRate = 60;

	results[2].width = 1600;
	results[2].height = 1200;
	results[2].refreshRate = 60;

	results[3].width = 1920;
	results[3].height = 1440;
	results[3].refreshRate = 60;

	*resolutions = results;
	if (numResolutions != NULL) {
		*numResolutions = 4;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// destroyResolutions()
////////////////////////////////////////////////////////////////////////////////
void releaseResolutions(const NvGsaResolution *resolutions)
{
	free((void *)resolutions);
}

////////////////////////////////////////////////////////////////////////////////
// printOptionValue()
////////////////////////////////////////////////////////////////////////////////
void printOptionValue(const NvGsaVariant *value, const NvGsaNamedOption *option)
{
	if (value != NULL && option != NULL) {
		switch (value->type) {
			case NV_GSA_TYPE_INT:
				wprintf(L"int %s = %d\n", option->name, value->asInt);
				break;

			case NV_GSA_TYPE_FLOAT:
				wprintf(L"float %s = %g\n", option->name, value->asFloat);
				break;

			case NV_GSA_TYPE_ENUM:
				wprintf(L"enum %s = %s\n", option->name, value->asEnum);
				break;

			case NV_GSA_TYPE_BOOL:
				wprintf(L"bool %s = %s\n", option->name, value->asBool ? L"true" : L"false");
				break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// cycleOptionValue()
////////////////////////////////////////////////////////////////////////////////
void cycleOptionValue(NvGsaVariant *result, const NvGsaVariant *value, const NvGsaNamedOption *option)
{
	if (result != NULL && value != NULL && option != NULL) {
		result->type = value->type;
		switch (value->type) {
			case NV_GSA_TYPE_INT:
				result->asInt = value->asInt + 1;
				return;

			case NV_GSA_TYPE_FLOAT:
				result->asFloat = value->asFloat + 1.0f;
				return;

			case NV_GSA_TYPE_ENUM: {
				uint32_t i;
				for (i = 0; i < option->enumRange.numEnums; ++i) {
					if (0 == wcscmp(value->asEnum, option->enumRange.enums[i])) {
						result->asEnum = option->enumRange.enums[(i + 1) % option->enumRange.numEnums];
						return;
					}
				}
				if (option->enumRange.numEnums > 0 && option->enumRange.enums != NULL) {
					result->asEnum = option->enumRange.enums[0];
				} else {
					result->asEnum = L"<error>";
				}
				return;
			}

			case NV_GSA_TYPE_BOOL:
				result->asBool = !value->asBool;
				return;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// printResolution()
////////////////////////////////////////////////////////////////////////////////
void printResolution(const NvGsaResolution *resolution)
{
	if (resolution != NULL) {
		wprintf(
			L"resolution = %d x %d pixels @ %g Hz\n",
			resolution->width,
			resolution->height,
			resolution->refreshRate);
	}
}

////////////////////////////////////////////////////////////////////////////////
// cycleResolution()
////////////////////////////////////////////////////////////////////////////////
void cycleResolution(NvGsaResolution *resolution, const NvGsaResolution *resolutions, size_t numResolutions)
{
	if (resolution != NULL && resolutions != NULL && numResolutions > 0) {
		uint32_t i;
		for (i = 0; i < numResolutions; ++i) {
			if (resolution->width == resolutions[i].width &&
				resolution->height == resolutions[i].height &&
				resolution->refreshRate == resolutions[i].refreshRate) {
				*resolution = resolutions[(i + 1) % numResolutions];
				return;
			}
		}
		*resolution = resolutions[0];
	}
}
