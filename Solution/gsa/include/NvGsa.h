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
#ifndef GFSDK_GSA_GSA_H
#define GFSDK_GSA_GSA_H

////////////////////////////////////////////////////////////////////////////////
/// @file NvGsa.h
/// @brief The GFE Settings API C/C++ header
////////////////////////////////////////////////////////////////////////////////

#include <NvFoundation.h>

#ifndef __cplusplus
#	include <stdbool.h>
#endif

NV_PUSH_PACK_DEFAULT

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined(GFSDK_GSA_API)
#	if !defined(GFSDK_GSA_STATIC)
#		define GFSDK_GSA_API __declspec(dllimport)
#	else
#		define GFSDK_GSA_API
#	endif
#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief Contains the library version number.
////////////////////////////////////////////////////////////////////////////////
typedef struct NvGsaVersion
{
	uint32_t major;    //!< Major version of the product, changed manually with every product release with a large new feature set.
	uint32_t minor;    //!< Minor version of the product, changed manually with every minor product release containing some features.
	uint32_t revision; //!< Revision, changed manually with every revision for bug fixes.
	uint32_t build;    //!< Build number of the same revision. Typically 0.
} NvGsaVersion;

////////////////////////////////////////////////////////////////////////////////
/// @brief Contains the current library version number.
////////////////////////////////////////////////////////////////////////////////
static const NvGsaVersion NvGsaCurrentVersion = { 1, 0, 2, 0 };

////////////////////////////////////////////////////////////////////////////////
/// @brief Status codes that may be returned by functions in the library.
////////////////////////////////////////////////////////////////////////////////
typedef enum NvGsaStatus
{
	NV_GSA_STATUS_OK                      =  0,   //!< Success. Request is completed.
	NV_GSA_STATUS_ERROR                   = -1,   //!< Generic error
	NV_GSA_STATUS_FILENOTFOUND            = -2,   //!< The config file cannot be found in the specified path
	NV_GSA_STATUS_SDK_VERSION_MISMATCH    = -3,   //!< Mismatch of dll vs header file+lib
	NV_GSA_STATUS_INVALID_ARGUMENT        = -4,   //!< Invalid argument (such as a null pointer)
	NV_GSA_STATUS_OPTION_NOT_FOUND        = -20,  //!< The Named Option cannot be found
	NV_GSA_STATUS_MULTIPLE_OPTIONS_FOUND  = -21,  //!< Multiple optins with the same name found
	NV_GSA_STATUS_OPTION_VALUE_OUTOFSTEP  = -22,  //!< The data value provided doesn't fit the step size
	NV_GSA_STATUS_OPTION_VALUE_OUTOFRANGE = -23,  //!< The data value provided is outside the valid range
	NV_GSA_STATUS_OPTION_FOUND            = -24,  //!< The Named Option already exists
	NV_GSA_STATUS_OPTION_VALUE_WRONG_TYPE = -25,  //!< The value destination structure has the wrong data type
	NV_GSA_STATUS_DIRTY_OPTIONS_FOUND     = -30,  //!< One or more options are dirty
	NV_GSA_STATUS_SDK_NOT_INITIALIZED     = -100, //!< Trying to use without being initalized
	NV_GSA_STATUS_SDK_ALREADY_INITIALIZED = -101  //!< Trying to initialize more than once
} NvGsaStatus;

////////////////////////////////////////////////////////////////////////////////
/// @brief Type of data in an associated NvGsaValue.
////////////////////////////////////////////////////////////////////////////////
typedef enum NvGsaDataType
{
	NV_GSA_TYPE_INT   = 0, //!< 32-bit integer type
	NV_GSA_TYPE_FLOAT = 1, //!< 32-bit floating point type
	NV_GSA_TYPE_ENUM  = 2, //!< string enumeration type
	NV_GSA_TYPE_BOOL  = 3  //!< boolean type
} NvGsaDataType;

////////////////////////////////////////////////////////////////////////////////
/// @brief Flag values used by the GFSDK_GSA_SaveConfigFile function.
////////////////////////////////////////////////////////////////////////////////
typedef enum NvGsaSaveFlagValues
{
	NV_GSA_SAVE_ALL               = 0, //!< Save all options, including those that have not been registered by the application
	NV_GSA_SAVE_SKIP_UNREGISTERED = 1  //!< Skip saving unregistered options
} NvGsaSaveFlagValues;

////////////////////////////////////////////////////////////////////////////////
/// @brief Flags containing NvGsaSaveFlagValues bitflags.
////////////////////////////////////////////////////////////////////////////////
typedef uint32_t NvGsaSaveFlags;

////////////////////////////////////////////////////////////////////////////////
/// @brief Structure for passing a dynamically typed option value.
////////////////////////////////////////////////////////////////////////////////
typedef struct NvGsaVariant
{
	NvGsaDataType type;  //!< type of the value field
	union
	{
		int32_t        asInt;	//!< int value
		float          asFloat; //!< float value
		const wchar_t *asEnum;	//!< string enum value
		bool           asBool;	//!< bool value
	};
} NvGsaVariant;

////////////////////////////////////////////////////////////////////////////////
/// @brief Structure containing the valid values for an enum option.
////////////////////////////////////////////////////////////////////////////////
typedef struct NvGsaEnumRange
{
	const wchar_t **enums;    //!< Array of strings that will contain all the enum names
	uint32_t        numEnums; //!< Number of strings in the array
} NvGsaEnumRange;

////////////////////////////////////////////////////////////////////////////////
/// @brief Union of option values.
////////////////////////////////////////////////////////////////////////////////
typedef union NvGsaValue
{
	int32_t        asInt;	//!< int value
	float          asFloat; //!< float value
	const wchar_t *asEnum;	//!< string enum value
	bool           asBool;	//!< bool value
} NvGsaValue;

////////////////////////////////////////////////////////////////////////////////
/// @brief Structure containing the valid range of numeric (int or float) value.
////////////////////////////////////////////////////////////////////////////////
typedef struct NvGsaNumericRange
{
	NvGsaValue minValue; //!< Minimum value allowed
	NvGsaValue maxValue; //!< Maximum value allowed

	/// @brief determines the allowable values for an option given min/max              
	///   numSteps == -1  range is [-inf, inf]                                   
	///   numSteps ==  0  range is continuous within [MinValue, MaxValue]        
	///   numSteps >   0  assumes NumSteps uniform increments between min/max    
	///                   eg, if min = 0, max = 8, and NumSteps = 4, then our    
	///                   option can accept any value in the set {0, 2, 4, 6, 8} 
	int32_t numSteps;  
} NvGsaNumericRange;

////////////////////////////////////////////////////////////////////////////////
/// @brief Structure containing value and range info for a named option.
////////////////////////////////////////////////////////////////////////////////
typedef struct NvGsaNamedOption
{
	const wchar_t *name; //!< Name of the option. Universal, non-localized name 
	NvGsaVariant   value; //!< Dynamically typed value of the option

	union
	{
		NvGsaNumericRange  numRange;  //!< Valid range for numeric types (float and int)
		NvGsaEnumRange     enumRange; //!< All possible values that an enum option can get; treated as strings
	};
} NvGsaNamedOption;

////////////////////////////////////////////////////////////////////////////////
/// @brief Structure containing display resolution data.
////////////////////////////////////////////////////////////////////////////////
typedef struct NvGsaResolution
{
	uint32_t width;       //!< Width of the screen in pixels
	uint32_t height;      //!< Height of the screen in pixels
	float    refreshRate; //!< Monitor refresh rate in Hz
} NvGsaResolution;

////////////////////////////////////////////////////////////////////////////////
/// @brief Structure containing information about a registered application.
///
/// The configuration file name and location will be as follows, with words
/// in <> substituting the value of the specified field:
/// <configPath>/GFXSettings.<executable>.xml
////////////////////////////////////////////////////////////////////////////////
typedef struct NvGsaApplication
{
	const wchar_t *displayName; //!< Contains default application name     
	const wchar_t *versionName; //!< Contains application version information      

	const wchar_t *installPath; //!< Absolute path pointing to the directory where the game has been installed
	const wchar_t *configPath;  //!< Absolute path pointing to the location where the settings file will be stored and read from
	const wchar_t *executable;  //!< Executable name for this configuration. This allows to have multiple GFX config files
	const wchar_t *execCmd;     //!< Direct command line used to launch the game. This should be compatible with Steam, Origin, Uplay, etc    
} NvGsaApplication;

////////////////////////////////////////////////////////////////////////////////
/// @brief Initializes the SDK.
///
/// If the SDK has already been initialized, it will skip any work and return.
/// If the SDK is not initialized, it will proceed with the initalization steps.
/// First the supplied version will be checked against the dll internal version
/// to avoid potential mismatchs and crashes due to incompatibility of dll's
/// and the version compiled in the application. Second the application data
/// will be copied to the internal storage.
///
/// @param[in] appRegInfo Pointer to an NvGsaApplication structure with information
///            about the application initializing the library; should not be null
///
/// @param[in] version Pointer to an NvGsaVersion structure; the application should
///            pass the address of the NvGsaCurrentVersion constant
///
/// @returns NV_GSA_STATUS_INVALID_ARGUMENT if appRegInfo or version is null;
///          NV_GSA_STATUS_SDK_ALREADY_INITIALIZED if the library has already
///          been initialized;
///		     NV_GSA_STATUS_VERSION_MISMATCH if the application was compiled
///          against an incompatible version of the library;
///          NV_GSA_STATUS_OK if initialization succeeded
////////////////////////////////////////////////////////////////////////////////
GFSDK_GSA_API NvGsaStatus GFSDK_GSA_InitializeSDK(const NvGsaApplication *appRegInfo, const NvGsaVersion *version);

////////////////////////////////////////////////////////////////////////////////
/// @brief Releases the library and all resources allocated by the library.
///
/// Data is not saved or preserved when calling this function. If there are
/// configuration changes that need to be saved, GFSDK_GSA_SaveConfigFile should
/// be called before calling this function.
///
/// @returns NV_GSA_STATUS_SDK_NOT_INITIALIZED if the library has not been
///          initialized with a call to GFSDK_GSA_InitializeSDK;
///          NV_GSA_STATUS_OK if the library has been fully released
////////////////////////////////////////////////////////////////////////////////
GFSDK_GSA_API NvGsaStatus GFSDK_GSA_ReleaseSDK();

////////////////////////////////////////////////////////////////////////////////
/// @brief Loads the GSA config file specified during library initialization.
/// 
/// Options should be registered by the application using
/// GFSDK_GSA_RegisterOption before calling this function. Any options in the
/// read from the configuration file that aren't registered will be inaccessible
/// to the application, and the GeForce Experience client will be responsible
/// for cleaning up unregistered options from the file.
///
/// @returns NV_GSA_STATUS_SDK_NOT_INITIALIZED if the library has not been
///          initialized with a call to GFSDK_GSA_InitializeSDK;
///          NV_GSA_STATUS_FILENOTFOUND if unable to find the settings file;
///          NV_GSA_STATUS_ERROR if the file was unable to be parsed;
///          NV_GSA_STATUS_OK if the file was loaded and options were updated
////////////////////////////////////////////////////////////////////////////////
GFSDK_GSA_API NvGsaStatus GFSDK_GSA_LoadConfigFile();

////////////////////////////////////////////////////////////////////////////////
/// @brief Saves registered option values to the GSA config file specified
///        during library initialization.
///
/// If the NV_GSA_SAVE_SKIP_UNREGISTERED flag is specified, only registered
/// options will be saved to the GSA config file.
///
/// @param[in] flags Set of NvGsaSaveFlagValues specifying how to
///            save the file; should not be null
///
/// @returns NV_GSA_STATUS_SDK_NOT_INITIALIZED if the library has not been
///          initialized with a call to GFSDK_GSA_InitializeSDK;
///          NV_GSA_STATUS_ERROR if the file could not be saved;
///          NV_GSA_STATUS_OK if the file was successfully saved
////////////////////////////////////////////////////////////////////////////////
GFSDK_GSA_API NvGsaStatus GFSDK_GSA_SaveConfigFile(NvGsaSaveFlags flags);

////////////////////////////////////////////////////////////////////////////////
/// @brief Registers an option used by the application.
///
/// This function checks for duplicate options with the same name and type, but
/// no verification of option bounds is performed. Duplicate options are
/// skipped.
///
/// @param[in] currentOption Pointer to the option to register; should not be
///            null
///
/// @returns NV_GSA_STATUS_ILLEGAL_ARGUMENT if currentOption is null;
///          NV_GSA_STATUS_SDK_NOT_INITIALIZED if the library has not been
///          initialized with a call to GFSDK_GSA_InitializeSDK;
///          NV_GSA_STATUS_OPTION_FOUND if the specified option is a duplicate;
///          NV_GSA_STATUS_OK if the option was successfully registered
////////////////////////////////////////////////////////////////////////////////
GFSDK_GSA_API NvGsaStatus GFSDK_GSA_RegisterOption(const NvGsaNamedOption *currentOption);

////////////////////////////////////////////////////////////////////////////////
/// @brief Registers resolutions available to the application.
///
/// This function should only been used when the application wants to restrict
/// the set of available resolutions.
///
/// @param[in] availableResolutions Array of NvGsaResolution structures
///            that enumerate the available resolutions; should not be null
///
/// @param[in] numResolutions Number of elements in the availableResolutions
///            array
///
/// @returns NV_GSA_STATUS_INVALID_ARGUMENT if availableResolutions is null;
///          NV_GSA_STATUS_SDK_NOT_INITIALIZED if the library has not been
///          initialized with a call to GFSDK_GSA_InitializeSDK;
///          NV_GSA_STATUS_OK if the available resolutions were successfully
///          registered
////////////////////////////////////////////////////////////////////////////////
GFSDK_GSA_API NvGsaStatus GFSDK_GSA_RegisterResolutions(const NvGsaResolution *availableResolutions, uint32_t numResolutions);

////////////////////////////////////////////////////////////////////////////////
/// @brief Releases memory used by a variant that was returned by the
///        library.
///
/// @param[in] variant Variant to be released
////////////////////////////////////////////////////////////////////////////////
GFSDK_GSA_API void GFSDK_GSA_ReleaseVariant(NvGsaVariant *variant);

////////////////////////////////////////////////////////////////////////////////
/// @brief Returns the current value of the specified registered option.
///
/// The returned value should be freed using GFSDK_GSA_ReleaseVariant when
/// it is no longer needed.
///
/// @param[out] value Address of NvGsaVariant that will be filled in with the
///             value of the specified option; should not be null
///
/// @param[in] name Name of the option to retrieve; should not be null
///
/// @returns NV_GSA_STATUS_INVALID_ARGUMENT if value or name is null;
///          NV_GSA_STATUS_SDK_NOT_INITIALIZED if the library has not been
///          initialized with a call to GFSDK_GSA_InitializeSDK;
///          NV_GSA_STATUS_OPTION_NOT_FOUND if a registered option with the
///          specified name and type is not found;
///          NV_GSA_STATUS_OK if option was found, and value returned
////////////////////////////////////////////////////////////////////////////////
GFSDK_GSA_API NvGsaStatus GFSDK_GSA_GetOptionValue(NvGsaVariant *value, const wchar_t *name);

////////////////////////////////////////////////////////////////////////////////
/// @brief Updates the current value of the specified registered option.
///
/// The current option value will be overwritten by the new one. Verification
/// is done to ensure that the value provided has the correct type and is in
/// the within the allowed range of the registered option.
///
/// @param[in] value Pointer to new value of option; should not be null
///
/// @param[in] name Name of the option to retrieve; should not be null
///
/// @returns NV_GSA_STATUS_INVALID_ARGUMENT if value or name is null;
///          NV_GSA_STATUS_SDK_NOT_INITIALIZED if the library has not been
///          initialized with a call to GFSDK_GSA_InitializeSDK;
///          NV_GSA_STATUS_OPTION_NOT_FOUND if a registered option with the
///          specified name and type is not found;
///          NV_GSA_STATUS_OPTION_VALUE_WRONG_TYPE if the registered option
///          type does not match the specified option type;
///			 NV_GSA_VALUE_OUTOFRANGE if the value provided is outside the range
///          specified at registration;
///          NV_GSA_VALUE_OUTOFSTEP if the value provided does not match the
///          step size specified at registration;
///          NV_GSA_STATUS_OK if option was found, and value updated
////////////////////////////////////////////////////////////////////////////////
GFSDK_GSA_API NvGsaStatus GFSDK_GSA_SetOptionValue(const NvGsaVariant *value, const wchar_t *name);

////////////////////////////////////////////////////////////////////////////////
/// @brief Returns the current resolution setting.
///
/// @param[out] value Address of NvGsaResolution structure to be filled with
///             information about the current resolution; should not be null
///
/// @returns NV_GSA_STATUS_INVALID_ARGUMENT if value is null;
///          NV_GSA_STATUS_SDK_NOT_INITIALIZED if the library has not been
///          initialized with a call to GFSDK_GSA_InitializeSDK;
///          NV_GSA_STATUS_OK if the current resolution was retrieved
///          successfully
////////////////////////////////////////////////////////////////////////////////
GFSDK_GSA_API NvGsaStatus GFSDK_GSA_GetResolution(NvGsaResolution *value);

////////////////////////////////////////////////////////////////////////////////
/// @brief Updates the current resolution value stored in the library.
///
/// If a range of valid resolutions has been specified, the input value will be
/// validated against the range.
///
/// @param[in] value Pointer to the new resolution; should not be null
///
/// @returns NV_GSA_STATUS_INVALID_ARGUMENT if value is null;
///          NV_GSA_STATUS_SDK_NOT_INITIALIZED if the library has not been
///          initialized with a call to GFSDK_GSA_InitializeSDK;
///          NV_GSA_STATUS_OPTION_VALUE_OUTOFRANGE if the specified resolution
///          is not available;
///          NV_GSA_STATUS_OK if the current resolution was updated
////////////////////////////////////////////////////////////////////////////////
GFSDK_GSA_API NvGsaStatus GFSDK_GSA_SetResolution(const NvGsaResolution *value);

////////////////////////////////////////////////////////////////////////////////
/// @brief Checks if there are options that have changed from the values in
///        the configuration file.
///
/// This function compares the application side registered options with their 
/// counterparts from the config file. If there are dirty options, it means that 
/// some values have changed and the config file might need updating. A use case 
/// of this function is to verify whether it is needed to re-save the config file or
/// not when the user applys changes that are identical to the previous settings. 
///
/// @returns NV_GSA_STATUS_SDK_NOT_INITIALIZED if the library has not been
///          initialized with a call to GFSDK_GSA_InitializeSDK;
///          NV_GSA_STATUS_DIRTY_OPTIONS_FOUND if one more options are dirty
///          NV_GSA_STATUS_OK if there are no dirty options
////////////////////////////////////////////////////////////////////////////////
GFSDK_GSA_API NvGsaStatus GFSDK_GSA_CheckForDirtyOptions();

////////////////////////////////////////////////////////////////////////////////
/// @brief Returns the runtime SDK version of the library.
///
/// Note that the version returned may be different from NvGsaCurrentVersion,
/// if the library dll being used has a different version than the SDK that
/// the application was compiled agains.
///
/// @returns the runtime SDK version of the library.
////////////////////////////////////////////////////////////////////////////////
GFSDK_GSA_API NvGsaVersion GFSDK_GSA_GetVersion();

#ifdef __cplusplus
}
#endif

NV_POP_PACK

#endif
