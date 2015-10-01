#include "pch.h"
#include "EngineState.h"

#include <shellapi.h>
#include <NvGsa.h>

#ifdef _WIN64
#pragma comment( lib, "..\\gsa\\lib\\win64\\NvGsa.x64.lib" )
#else
#pragma comment( lib, "..\\gsa\\lib\\win32\\NvGsa.x86.lib" )
#endif

constexpr uint64 str2int(const wchar* str, uint64 h = 0)
{
	return !str[h] ? 5381 : (str2int(str, h + 1) * 7) ^ str[h];
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
	NvGsaNamedOption *results = NULL;

	if (options == NULL) {
		return false;
	}

	results = (NvGsaNamedOption *)malloc(13 * sizeof(NvGsaNamedOption));
	if (results == NULL) {
		return false;
	}

	results[0].name = L"Field_Of_View";
	results[0].value.type = NV_GSA_TYPE_FLOAT;
	results[0].value.asFloat = XM_PI / 3.f;
	results[0].numRange.maxValue.asFloat = XM_PI / 2.f;
	results[0].numRange.minValue.asFloat = XM_PI / 5.f;
	results[0].numRange.numSteps = -1;

	results[1].name = L"Near_Plane";
	results[1].value.type = NV_GSA_TYPE_FLOAT;
	results[1].value.asFloat = 0.1f;
	results[1].numRange.maxValue.asFloat = 0.01f;
	results[1].numRange.minValue.asFloat = 1.f;
	results[1].numRange.numSteps = -1;

	results[2].name = L"Far_Plane";
	results[2].value.type = NV_GSA_TYPE_FLOAT;
	results[2].value.asFloat = 200.f;
	results[2].numRange.maxValue.asFloat = 1000.f;
	results[2].numRange.minValue.asFloat = 1.f;
	results[2].numRange.numSteps = -1;

	results[3].name = L"LightShafts";
	results[3].value.type = NV_GSA_TYPE_BOOL;
	results[3].value.asBool = true;

	results[4].name = L"DepthOfField";
	results[4].value.type = NV_GSA_TYPE_BOOL;
	results[4].value.asBool = true;

	results[5].name = L"Bloom";
	results[5].value.type = NV_GSA_TYPE_BOOL;
	results[5].value.asBool = true;

	results[6].name = L"AmbientOcclusion";
	results[6].value.type = NV_GSA_TYPE_BOOL;
	results[6].value.asBool = true;

	results[7].name = L"Fxaa";
	results[7].value.type = NV_GSA_TYPE_BOOL;
	results[7].value.asBool = true;

	results[8].name = L"FullScreen";
	results[8].value.type = NV_GSA_TYPE_BOOL;
	results[8].value.asBool = false;

	results[9].name = L"V-Sync";
	results[9].value.type = NV_GSA_TYPE_BOOL;
	results[9].value.asBool = true;

	static const wchar_t *ShadowMapSize[] = 
	{
		L"512",
		L"1024",
		L"2048",
		L"4096"
	};

	results[10].name = L"ShadowMapSize";
	results[10].value.type = NV_GSA_TYPE_ENUM;
	results[10].value.asEnum = L"2048";
	results[10].enumRange.enums = ShadowMapSize;
	results[10].enumRange.numEnums = ARRAYSIZE(ShadowMapSize);

	results[11].name = L"ShadowMapCount";
	results[11].value.type = NV_GSA_TYPE_INT;
	results[11].value.asInt = 3;
	results[11].numRange.maxValue.asInt = 4;
	results[11].numRange.minValue.asInt = 1;
	results[11].numRange.numSteps = -1;

	static const wchar_t *ShadowMapQuality[] =
	{
		L"Low_Hard",
		L"Medium_Hard",
		L"High_Hard",
		L"Low_PCF",
		L"Medium_PCF",
		L"High_PCF",
		L"Low_PCSS",
		L"Medium_PCSS",
		L"High_PCSS",
	};

	results[12].name = L"ShadowMapQuality";
	results[12].value.type = NV_GSA_TYPE_ENUM;
	results[12].value.asEnum = L"Medium_PCSS";
	results[12].enumRange.enums = ShadowMapQuality;
	results[12].enumRange.numEnums = ARRAYSIZE(ShadowMapQuality);

	*options = results;
	if (numOptions != NULL) {
		*numOptions = 13;
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

	results = (NvGsaResolution *)malloc(42 * sizeof(NvGsaResolution));
	if (results == NULL) {
		return false;
	}

	results[0].width = 800;
	results[0].height = 600;
	results[0].refreshRate = 60;

	results[1].width = 1024;
	results[1].height = 600;
	results[1].refreshRate = 60;

	results[2].width = 1024;
	results[2].height = 768;
	results[2].refreshRate = 60;

	results[3].width = 1152;
	results[3].height = 864;
	results[3].refreshRate = 60;

	results[4].width = 1280;
	results[4].height = 720;
	results[4].refreshRate = 60;

	results[5].width = 1280;
	results[5].height = 768;
	results[5].refreshRate = 60;

	results[6].width = 1280;
	results[6].height = 800;
	results[6].refreshRate = 60;

	results[7].width = 1280;
	results[7].height = 960;
	results[7].refreshRate = 60;

	results[8].width = 1280;
	results[8].height = 1024;
	results[8].refreshRate = 60;

	results[9].width = 1360;
	results[9].height = 768;
	results[9].refreshRate = 60;

	results[10].width = 1366;
	results[10].height = 768;
	results[10].refreshRate = 60;

	results[11].width = 1400;
	results[11].height = 1050;
	results[11].refreshRate = 60;

	results[12].width = 1440;
	results[12].height = 900;
	results[12].refreshRate = 60;

	results[13].width = 1600;
	results[13].height = 900;
	results[13].refreshRate = 60;

	results[14].width = 1600;
	results[14].height = 1200;
	results[14].refreshRate = 60;

	results[15].width = 1680;
	results[15].height = 1050;
	results[15].refreshRate = 60;

	results[16].width = 1920;
	results[16].height = 1080;
	results[16].refreshRate = 60;

	results[17].width = 1920;
	results[17].height = 1200;
	results[17].refreshRate = 60;

	results[18].width = 2048;
	results[18].height = 1152;
	results[18].refreshRate = 60;

	results[19].width = 2560;
	results[19].height = 1440;
	results[19].refreshRate = 60;

	results[20].width = 2560;
	results[20].height = 1600;
	results[20].refreshRate = 60;

	memcpy(&results[21], &results[0], 21 * sizeof(NvGsaResolution));
	for (size_t i = 21; i < 42; i++)
		results[i].refreshRate = 30;

	*resolutions = results;
	if (numResolutions != NULL) {
		*numResolutions = 42;
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
			}
			else {
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


namespace Rath
{
	EngineState::EngineState() :
#if defined(_PROFILE) | defined(_DEBUG)
		m_displayShadowMaps(false),
		m_displayShadowFrustum(false),
		m_displayShadowBuffer(false),
#endif
		m_BackBufferDesc({ 1280, 720, 
			DXGI_FORMAT_R16G16B16A16_FLOAT, 
			{ 1, 0 } }),
		m_Viewport({ 0.0f, 0.0f, 
			static_cast<float>(m_BackBufferDesc.Width), 
			static_cast<float>(m_BackBufferDesc.Height), 
			0.0f, 1.0f}),
		m_Light(-0.5f, -0.5f, 0.5f, 0.5f, 0.0f, 1.0f),
		m_LightDirection(XMVector3Normalize(XMVectorSet(1.f, 0.5f, 0.3f, 0.f))),

		m_fovy(XM_PI / 3.f),
		m_aspect(m_Viewport.Width / m_Viewport.Height),
		m_znear(0.1f),
		m_zfar(200.f),

		m_LightShafts(true),
		m_DepthOfField(true),
		m_Bloom(true),
		m_AmbientOcclusion(true),
		m_Fxaa(true),

		m_ShadowMapSize(2048),
		m_ShadowMapCount(3),
		m_ShadowMapQuality(1),
		m_ShadowMapType(2),

		m_fullScreen(false),
		m_vsync(true),
		m_refreshRate({60,1})
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
		LPWSTR *szArglist;
		int nArgs = 0;
		szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
		if (nArgs < 1 || !initApplication(&app, szArglist[0])) {
			wprintf(L"Failed to initialize the application, exiting!");
			return;
		}
		GlobalFree(szArglist);

		// Initialize the GSA SDK
		status = GFSDK_GSA_InitializeSDK(&app, &NvGsaCurrentVersion);
		releaseApplication(&app);
		if (status != NV_GSA_STATUS_OK) {
			wprintf(L"Failed to initialize the GSA SDK, exiting!");
			return;
		}

		if (initOptions(&options, &numOptions) &&
			initResolutions(&resolutions, &numResolutions)) {

			//status = GFSDK_GSA_SaveConfigFile(NV_GSA_SAVE_ALL);

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

					switch (str2int(options[i].name))
					{
					case str2int(L"Field_Of_View"):
						m_fovy = value.asFloat;
						break;
					case str2int(L"Near_Plane"):
						m_znear = value.asFloat;
						break;
					case str2int(L"Far_Plane"):
						m_zfar = value.asFloat;
						break;
					case str2int(L"LightShafts"):
						m_LightShafts = value.asBool;
						break;
					case str2int(L"DepthOfField"):
						m_DepthOfField = value.asBool;
						break;
					case str2int(L"Bloom"):
						m_Bloom = value.asBool;
						break;
					case str2int(L"AmbientOcclusion"):
						m_AmbientOcclusion = value.asBool;
						break;
					case str2int(L"Fxaa"):
						m_Fxaa = value.asBool;
						break;
					case str2int(L"FullScreen"):
						m_fullScreen = value.asBool;
						break;
					case str2int(L"V-Sync"):
						m_vsync = value.asBool;
						break;
					case str2int(L"ShadowMapSize"):
						switch (str2int(value.asEnum))
						{
						case str2int(L"512"):
							m_ShadowMapSize = 512;
							break;
						case str2int(L"1024"):
							m_ShadowMapSize = 1024;
							break;
						case str2int(L"2048"):
							m_ShadowMapSize = 2048;
							break;
						case str2int(L"4096"):
							m_ShadowMapSize = 4096;
							break;
						}
						break;
					case str2int(L"ShadowMapCount"):
						m_ShadowMapCount = value.asInt;
						break;
					case str2int(L"ShadowMapQuality"):
						switch (str2int(value.asEnum))
						{
						case str2int(L"Low_Hard"):
							m_ShadowMapQuality = 0;
							m_ShadowMapType = 0;
							break;
						case str2int(L"Medium_Hard"):
							m_ShadowMapQuality = 1;
							m_ShadowMapType = 0;
							break;
						case str2int(L"High_Hard"):
							m_ShadowMapQuality = 2;
							m_ShadowMapType = 0;
							break;
						case str2int(L"Low_PCF"):
							m_ShadowMapQuality = 0;
							m_ShadowMapType = 1;
							break;
						case str2int(L"Medium_PCF"):
							m_ShadowMapQuality = 1;
							m_ShadowMapType = 1;
							break;
						case str2int(L"High_PCF"):
							m_ShadowMapQuality = 2;
							m_ShadowMapType = 1;
							break;
						case str2int(L"Low_PCSS"):
							m_ShadowMapQuality = 0;
							m_ShadowMapType = 2;
							break;
						case str2int(L"Medium_PCSS"):
							m_ShadowMapQuality = 1;
							m_ShadowMapType = 2;
							break;
						case str2int(L"High_PCSS"):
							m_ShadowMapQuality = 2;
							m_ShadowMapType = 2;
							break;
						}
						break;
					}
					GFSDK_GSA_ReleaseVariant(&value);
				}
			}

			// Print and increment the resolution
			status = GFSDK_GSA_GetResolution(&currentResolution);
			if (status == NV_GSA_STATUS_OK) {
				m_refreshRate.Numerator = (UINT)round(currentResolution.refreshRate);
				WindowSizeChanged(currentResolution.width, currentResolution.height);
			}
		}

		// Cleanup
		releaseOptions(options);
		releaseResolutions(resolutions);

		m_Camera.SetProjParams(m_fovy, m_aspect, m_znear, m_zfar);
		m_Camera.SetViewParams(XMVectorSet(0.f, 0.f, 0.f, 1.f), XMVectorSet(0.f, 0.f, 1.f, 1.f));
	}

	EngineState::~EngineState()
	{
		// Save the config file if options have changed
		if (GFSDK_GSA_CheckForDirtyOptions() == NV_GSA_STATUS_DIRTY_OPTIONS_FOUND) {
			GFSDK_GSA_SaveConfigFile(NV_GSA_SAVE_SKIP_UNREGISTERED);
		}
		GFSDK_GSA_ReleaseSDK();
	}

	void EngineState::WindowSizeChanged(uint32 width, uint32 height)
	{
		m_BackBufferDesc.Width = width;
		m_BackBufferDesc.Height = height;

		m_Viewport.Width = (FLOAT)width;
		m_Viewport.Height = (FLOAT)height;

		m_aspect = m_Viewport.Width / m_Viewport.Height;

		m_Camera.SetProjParams(m_fovy, m_aspect, m_znear, m_zfar);
	}
}