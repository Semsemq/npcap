/***********************IMPORTANT NPCAP LICENSE TERMS***********************
 *
 * Npcap (https://npcap.com) is a Windows packet sniffing driver and library
 * and is copyright (c) 2013-2022 by Nmap Software LLC ("The Nmap Project").
 * All rights reserved.
 *
 * Even though Npcap source code is publicly available for review, it
 * is not open source software and may not be redistributed or used in
 * other software without special permission from the Nmap
 * Project. The standard (free) version is usually limited to
 * installation on five systems. For more details, see the LICENSE
 * file included with Npcap and also avaialble at
 * https://github.com/nmap/npcap/blob/master/LICENSE. This header file
 * summarizes a few important aspects of the Npcap license, but is not
 * a substitute for that full Npcap license agreement.
 *
 * We fund the Npcap project by selling two types of commercial licenses to a
 * special Npcap OEM edition:
 *
 * 1) The Npcap OEM Redistribution License allows companies distribute Npcap
 * OEM within their products. Licensees generally use the Npcap OEM silent
 * installer, ensuring a seamless experience for end users. Licensees may
 * choose between a perpetual unlimited license or a quarterly term license,
 * along with options for commercial support and updates. Prices and details:
 * https://npcap.com/oem/redist.html
 *
 * 2) The Npcap OEM Internal-Use License is for organizations that wish to
 * use Npcap OEM internally, without redistribution outside their
 * organization. This allows them to bypass the 5-system usage cap of the
 * Npcap free edition. It includes commercial support and update options, and
 * provides the extra Npcap OEM features such as the silent installer for
 * automated deployment. Prices and details:
 * https://npcap.com/oem/internal.html
 *
 * Both of these licenses include updates and support as well as a
 * warranty. Npcap OEM also includes a silent installer for unattended
 * installation. Further details about Npcap OEM are available from
 * https://npcap.com/oem/, and you are also welcome to contact us at
 * sales@nmap.com to ask any questions or set up a license for your
 * organization.
 *
 * Free and open source software producers are also welcome to contact us for
 * redistribution requests. However, we normally recommend that such authors
 * instead ask your users to download and install Npcap themselves. It will
 * be free for them if they need 5 or fewer copies.
 *
 * If the Nmap Project (directly or through one of our commercial
 * licensing customers) has granted you additional rights to Npcap or
 * Npcap OEM, those additional rights take precedence where they
 * conflict with the terms of the license agreement.
 *
 * Since the Npcap source code is available for download and review, users
 * sometimes contribute code patches to fix bugs or add new features.  By
 * sending these changes to the Nmap Project (including through direct email
 * or our mailing lists or submitting pull requests through our source code
 * repository), it is understood unless you specify otherwise that you are
 * offering the Nmap Project the unlimited, non-exclusive right to reuse,
 * modify, and relicense your code contribution so that we may (but are not
 * obligated to) incorporate it into Npcap.  If you wish to specify special
 * license conditions or restrictions on your contributions, just say so when
 * you send them.
 *
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. Warranty rights and commercial
 * support are available for the OEM Edition described above.
 *
 * Other copyright notices and attribution may appear below this license
 * header. We have kept those for attribution purposes, but any license terms
 * granted by those notices apply only to their original work, and not to any
 * changes made by the Nmap Project or to this entire file.
 *
 ***************************************************************************/
/*
 * Copyright (c) 1999 - 2005 NetGroup, Politecnico di Torino (Italy)
 * Copyright (c) 2005 - 2010 CACE Technologies, Davis (California)
 * Copyright (c) 2010 - 2013 Riverbed Technology, San Francisco (California), Yang Luo (China)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Politecnico di Torino, CACE Technologies
 * nor the names of its contributors may be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "stdafx.h"

#include "Loopback.h"
#include "Packet.h"
#include "win_bpf.h"
#include "ioctls.h"

#include "..\..\..\version.h"
#include "..\..\..\Common\WpcapNames.h"

#include <windef.h>

#ifdef ALLOC_PRAGMA
#pragma NDIS_INIT_FUNCTION(DriverEntry)
#endif // ALLOC_PRAGMA

SINGLE_LIST_ENTRY g_arrFiltMod = {0}; //Adapter filter module list head
NDIS_SPIN_LOCK g_FilterArrayLock; //The lock for adapter filter module list.

#ifdef HAVE_WFP_LOOPBACK_SUPPORT
//
// Global variables used by WFP
//
NDIS_STRING g_LoopbackAdapterName;
NDIS_STRING g_LoopbackRegValueName = NDIS_STRING_CONST("LoopbackAdapter");
NDIS_STRING g_LoopbackSupportRegValueName = NDIS_STRING_CONST("LoopbackSupport");
ULONG g_LoopbackSupportMode = 0;
#endif

#ifdef HAVE_RX_SUPPORT

NDIS_STRING g_SendToRxAdapterName;
NDIS_STRING g_SendToRxRegValueName = NDIS_STRING_CONST("SendToRxAdapters");
NDIS_STRING g_BlockRxAdapterName;
NDIS_STRING g_BlockRxRegValueName = NDIS_STRING_CONST("BlockRxAdapters");

#endif

UNICODE_STRING deviceSymLink = RTL_CONSTANT_STRING(L"\\DosDevices\\" NPF_DRIVER_NAME_WIDECHAR);

NDIS_STRING g_AdminOnlyRegValueName = NDIS_STRING_CONST("AdminOnly");
NDIS_STRING g_DltNullRegValueName = NDIS_STRING_CONST("DltNull");
NDIS_STRING g_Dot11SupportRegValueName = NDIS_STRING_CONST("Dot11Support");
NDIS_STRING g_VlanSupportRegValueName = NDIS_STRING_CONST("VlanSupport");
NDIS_STRING g_TimestampRegValueName = NDIS_STRING_CONST("TimestampMode");
NDIS_STRING g_TestModeRegValueName = NDIS_STRING_CONST("TestMode");

ULONG g_AdminOnlyMode = 0;
ULONG g_DltNullMode = 0;
ULONG g_Dot11SupportMode = 0;
ULONG g_VlanSupportMode = 0;
ULONG g_TimestampMode = DEFAULT_TIMESTAMPMODE;
ULONG g_TestMode = 0;

//
// Global variables
//
NDIS_HANDLE         FilterDriverHandle = NULL;			// NDIS handle for filter driver
NDIS_HANDLE         FilterDriverHandle_WiFi = NULL;		// NDIS handle for WiFi filter driver
NDIS_HANDLE         FilterDriverObject;					// Driver object for filter driver
PDEVICE_OBJECT pNpcapDeviceObject = NULL;

#ifdef KeQuerySystemTime
// On Win x64, KeQuerySystemTime is defined as a macro,
// this function wraps the macro execution.
void
KeQuerySystemTimeWrapper(
	_Out_ PLARGE_INTEGER CurrentTime
)
{
	KeQuerySystemTime(CurrentTime);
}
PQUERYSYSTEMTIME g_ptrQuerySystemTime = &KeQuerySystemTimeWrapper;
#else
PQUERYSYSTEMTIME g_ptrQuerySystemTime = &KeQuerySystemTime;
#endif

#ifdef NPCAP_READ_ONLY
// For read-only Npcap, we want an explicit denial function for the Write call.
// The IOCTLs will be rejected as "invalid request"
_Dispatch_type_(IRP_MJ_WRITE)
DRIVER_DISPATCH NPF_Deny;

_Use_decl_annotations_
NTSTATUS NPF_Deny(
		IN PDEVICE_OBJECT DeviceObject,
		IN PIRP Irp
		)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	TRACE_ENTER();
	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	TRACE_EXIT();
	return STATUS_UNSUCCESSFUL;
}
#endif

/*!
  \brief The initialization routine of the LWF data structure.
  \param pFChars The LWF data structure.
  \param bWiFiOrNot Whether the LWF is registered as a WiFi one or standard one.
  \return NULL
*/
VOID
NPF_registerLWF(
	_Out_ PNDIS_FILTER_DRIVER_CHARACTERISTICS pFChars,
	_In_ BOOLEAN bWiFiOrNot
	);

/*!
  \brief read Npcap software's registry, get the option.

  If the registry key doesn't exist, we view the result as 0.
*/
_IRQL_requires_(PASSIVE_LEVEL)
ULONG
NPF_GetRegistryOption_Integer(
	_In_ PUNICODE_STRING RegistryPath,
	_In_ PUNICODE_STRING RegValueName
	);

/*!
  \brief read Npcap software's registry, get the option

  If NPF_GetLoopbackAdapterName() fails, g_LoopbackAdapterName will be NULL.
*/
_IRQL_requires_(PASSIVE_LEVEL)
VOID
NPF_GetRegistryOption_String(
	_In_ PUNICODE_STRING RegistryPath,
	_In_ PUNICODE_STRING RegValueName,
	_Inout_ PNDIS_STRING g_OutputString
	);

// This will get a list of adapter names, strip out any \Device\ prefix.
_IRQL_requires_(PASSIVE_LEVEL)
static VOID
NPF_GetRegistryOption_AdapterName(
	_In_ PUNICODE_STRING pRegistryPath,
	_In_ PUNICODE_STRING pRegValueName,
	_Inout_ PNDIS_STRING pOutputString
	)
{
	USHORT i=0, j=0;
	NPF_GetRegistryOption_String(pRegistryPath, pRegValueName, pOutputString);
	if (pOutputString->Buffer == NULL)
	{
		// Not found, that's fine.
		pOutputString->Length = 0;
		pOutputString->MaximumLength = 0;
		return;
	}
	// We don't actually want the "\\Device\\" prefix.
	j = DEVICE_PATH_CCH;
	while (j < BYTES2CCH(pOutputString->Length))
	{
		pOutputString->Buffer[i] = pOutputString->Buffer[j];
		if(pOutputString->Buffer[i] == L';') {
			// Separator found, need to jump over another prefix
			j += DEVICE_PATH_CCH;
		}
		i++;
		j++;
	}
	// Fix up the length to the number of bytes we actually copied.
	pOutputString->Length = CCH2BYTES(i);
}
//-------------------------------------------------------------------
//
//  Packet Driver's entry routine.
//
_Use_decl_annotations_
NTSTATUS
DriverEntry(
	IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath
	)
{
	NDIS_FILTER_DRIVER_CHARACTERISTICS FChars; // The specification for the filter.
	NDIS_FILTER_DRIVER_CHARACTERISTICS FChars_WiFi; // The specification for the WiFi filter.
	UNICODE_STRING parametersPath;
	NTSTATUS Status = STATUS_SUCCESS;
	PDEVICE_OBJECT devObjP;
	// https://docs.microsoft.com/en-us/windows-hardware/drivers/kernel/sddl-for-device-objects
#define SDDL_ALLOW_ALL_SYSTEM_ADMIN L"D:P(A;;GA;;;SY)(A;;GA;;;BA)"
#ifndef NPCAP_READ_ONLY
	UNICODE_STRING sddl = RTL_CONSTANT_STRING(SDDL_ALLOW_ALL_SYSTEM_ADMIN L"(A;;GRGW;;;WD)");
#else
	// For convenience and clarity, deny write access (GW) here. In reality, we
	// remove any code that injects packets in this configuration
	UNICODE_STRING sddl = RTL_CONSTANT_STRING(SDDL_ALLOW_ALL_SYSTEM_ADMIN L"(A;;GR;;;WD)");
#endif
	UNICODE_STRING sddl_admin_only = RTL_CONSTANT_STRING(SDDL_ALLOW_ALL_SYSTEM_ADMIN);
	const GUID guidClassNPF = { 0x26e0d1e0L, 0x8189, 0x12e0, { 0x99, 0x14, 0x08, 0x00, 0x22, 0x30, 0x19, 0x04 } };
	UNICODE_STRING AdapterName = RTL_CONSTANT_STRING(DEVICE_PATH_PREFIX NPF_DRIVER_NAME_WIDECHAR);

	NDIS_STRING strKeQuerySystemTimePrecise;

	UNREFERENCED_PARAMETER(RegistryPath);

	TRACE_ENTER();
	FilterDriverObject = DriverObject;

	RtlInitUnicodeString(&parametersPath, NULL);
	parametersPath.MaximumLength=RegistryPath->Length+sizeof(L"\\Parameters");
	parametersPath.Buffer=ExAllocatePoolWithTag(PagedPool, parametersPath.MaximumLength, NPF_UNICODE_BUFFER_TAG);
	if (!parametersPath.Buffer) {
		ERROR_DBG("Paged alloc of parametersPath failed.\n");
		TRACE_EXIT();
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	RtlZeroMemory(parametersPath.Buffer, parametersPath.MaximumLength);
	RtlCopyUnicodeString(&parametersPath, RegistryPath);
	RtlAppendUnicodeToString(&parametersPath, L"\\Parameters");

	Status = RtlCheckRegistryKey(RTL_REGISTRY_ABSOLUTE,
			parametersPath.Buffer);
	if (NT_SUCCESS(Status)) {
		// Get the AdminOnly option, if AdminOnly=1, devices will be created with the safe SDDL, to make sure only Administrators can use Npcap driver.
		// If the registry key doesn't exist, we view it as AdminOnly=0, so no protect to the driver access.
		g_AdminOnlyMode = NPF_GetRegistryOption_Integer(&parametersPath, &g_AdminOnlyRegValueName);
		INFO_DBG("g_AdminOnlyMode = %lu\n", g_AdminOnlyMode);
		// Get the DltNull option, if DltNull=1, loopback traffic will be DLT_NULL/DLT_LOOP style, including captured and sent packets.
		// If the registry key doesn't exist, we view it as DltNull=0, so loopback traffic are Ethernet packets.
		g_DltNullMode = NPF_GetRegistryOption_Integer(&parametersPath, &g_DltNullRegValueName);
		INFO_DBG("g_DltNullMode = %lu\n", g_DltNullMode);
		// Get the Dot11Support option, if Dot11Support=1, Npcap driver will enable the raw 802.11 functions.
		// If the registry key doesn't exist, we view it as Dot11Support=1, so has raw 802.11 support.
		g_Dot11SupportMode = NPF_GetRegistryOption_Integer(&parametersPath, &g_Dot11SupportRegValueName);
		INFO_DBG("g_Dot11SupportMode = %lu\n", g_Dot11SupportMode);
		// Get the VlanSupport option, if VlanSupport=1, Npcap driver will try to recognize 802.1Q VLAN tag when capturing and sending data.
		// If the registry key doesn't exist, we view it as VlanSupport=0, so no VLAN support.
		g_VlanSupportMode = NPF_GetRegistryOption_Integer(&parametersPath, &g_VlanSupportRegValueName);
		INFO_DBG("g_VlanSupportMode = %lu\n", g_VlanSupportMode);
		// Get the TimestampMode option. The meanings of its values is described in time_calls.h.
		// If the registry key doesn't exist, we view it as TimestampMode=0, so the default "QueryPerformanceCounter" timestamp gathering method.
		g_TimestampMode = NPF_GetRegistryOption_Integer(&parametersPath, &g_TimestampRegValueName);
		INFO_DBG("g_TimestampMode = %lu\n", g_TimestampMode);
		if (!NPF_TimestampModeSupported(g_TimestampMode)) {
			g_TimestampMode = DEFAULT_TIMESTAMPMODE;
		}
		// Get the TestMode option, if TestMode!=0, WFP callbacks will be registered regardless of whether any open instance needs it.
		// This is for WHQL testing.
		g_TestMode = NPF_GetRegistryOption_Integer(&parametersPath, &g_TestModeRegValueName);
		INFO_DBG("g_TestMode = %lu\n", g_TestMode);

#ifdef HAVE_WFP_LOOPBACK_SUPPORT
		g_LoopbackSupportMode = NPF_GetRegistryOption_Integer(&parametersPath, &g_LoopbackSupportRegValueName);
		INFO_DBG("g_LoopbackSupportMode = %lu\n", g_LoopbackSupportMode);
		if (g_LoopbackSupportMode) {
			NPF_GetRegistryOption_AdapterName(&parametersPath, &g_LoopbackRegValueName, &g_LoopbackAdapterName);
		}
#endif
#ifdef HAVE_RX_SUPPORT
		NPF_GetRegistryOption_AdapterName(&parametersPath, &g_SendToRxRegValueName, &g_SendToRxAdapterName);
		NPF_GetRegistryOption_AdapterName(&parametersPath, &g_BlockRxRegValueName, &g_BlockRxAdapterName);
#endif
	}
	if (parametersPath.Buffer) ExFreePool(parametersPath.Buffer);

	//
	// Initialize system-time function pointer.
	//
	RtlInitUnicodeString(&strKeQuerySystemTimePrecise, L"KeQuerySystemTimePrecise");
	g_ptrQuerySystemTime = (PQUERYSYSTEMTIME) MmGetSystemRoutineAddress(&strKeQuerySystemTimePrecise);
	// If KeQuerySystemTimePrecise is not available,
	// use KeQuerySystemTime function (Win32) or a wrapper to the KeQuerySystemTime macro (x64).
	if (g_ptrQuerySystemTime == NULL) {
#ifdef KeQuerySystemTime
		g_ptrQuerySystemTime = &KeQuerySystemTimeWrapper;
#else
		g_ptrQuerySystemTime = &KeQuerySystemTime;
#endif
	}

	//
	// Register as a service with NDIS
	//
	NPF_registerLWF(&FChars, FALSE);
	if (g_Dot11SupportMode)
		NPF_registerLWF(&FChars_WiFi, TRUE);

	DriverObject->DriverUnload = NPF_Unload;

	//
	// Standard device driver entry points stuff.
	//
	DriverObject->MajorFunction[IRP_MJ_CREATE] = NPF_OpenAdapter;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = NPF_CloseAdapter;
	DriverObject->MajorFunction[IRP_MJ_CLEANUP] = NPF_Cleanup;
	DriverObject->MajorFunction[IRP_MJ_READ] = NPF_Read;
#ifndef NPCAP_READ_ONLY
	DriverObject->MajorFunction[IRP_MJ_WRITE] = NPF_Write;
#else
	// Explicitly reject write calls
	DriverObject->MajorFunction[IRP_MJ_WRITE] = NPF_Deny;
#endif
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = NPF_IoControl;

	// Create the "NPCAP" device itself:
	Status = IoCreateDeviceSecure(DriverObject, sizeof(DEVICE_EXTENSION), &AdapterName, FILE_DEVICE_UNKNOWN,
			FILE_DEVICE_SECURE_OPEN, FALSE, (g_AdminOnlyMode ? &sddl_admin_only : &sddl), (LPCGUID)&guidClassNPF, &devObjP);
	if (!NT_SUCCESS(Status))
	{
		ERROR_DBG("IoCreateDevice failed: %#08x\n", Status);

		TRACE_EXIT();
		return Status;
	}

	PDEVICE_EXTENSION devExtP = (PDEVICE_EXTENSION)devObjP->DeviceExtension;

	devObjP->Flags |= DO_DIRECT_IO;

	Status = IoCreateSymbolicLink(&deviceSymLink, &AdapterName);
	if (!NT_SUCCESS(Status))
	{
		ERROR_DBG("IoCreateSymbolicLink(%ws) failed: %#08x\n", deviceSymLink.Buffer, Status);

		IoDeleteDevice(devObjP);

		TRACE_EXIT();
		return Status;
	}

	/* Have to set this up before NdisFRegisterFilterDriver, since we can get Attach calls immediately after that! */
	NdisAllocateSpinLock(&g_FilterArrayLock);

	// Register the filter to NDIS.
	Status = NdisFRegisterFilterDriver(DriverObject,
		(NDIS_HANDLE) FilterDriverObject,
		&FChars,
		&FilterDriverHandle);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		FilterDriverHandle = NULL;
		NdisFreeSpinLock(&g_FilterArrayLock);
		ERROR_DBG("NdisFRegisterFilterDriver failed: %#08x\n", Status);
		IoDeleteSymbolicLink(&deviceSymLink);
		IoDeleteDevice(devObjP);

		TRACE_EXIT();
		return Status;
	}
	INFO_DBG("FilterDriverHandle = %p\n", FilterDriverHandle);

	// Initialize DEVICE_EXTENSION
	do {
		Status = STATUS_INSUFFICIENT_RESOURCES; // Status for any of the below failures
		devExtP->FilterDriverHandle = FilterDriverHandle;
		InitializeListHead(&devExtP->AllOpens);
		devExtP->AllOpensLock = NdisAllocateRWLock(FilterDriverHandle);
		if (devExtP->AllOpensLock == NULL)
		{
			ERROR_DBG("Failed to allocate AllOpensLock\n");
			break;
		}

		Status = ExInitializeLookasideListEx(&devExtP->BufferPool, NULL, NULL, NPF_NONPAGED, 0, sizeof(BUFCHAIN_ELEM), NPF_PACKET_DATA_TAG, 0);
		if (Status != STATUS_SUCCESS)
		{
			ERROR_DBG("Failed to allocate BufferPool\n");
			break;
		}
		devExtP->bBufferPoolInit = 1;

		Status = ExInitializeLookasideListEx(&devExtP->NBLCopyPool, NULL, NULL, NPF_NONPAGED, 0, sizeof(NPF_NBL_COPY), NPF_NBLC_POOL_TAG, 0);
		if (Status != STATUS_SUCCESS)
		{
			ERROR_DBG("Failed to allocate NBLCopyPool\n");
			break;
		}
		devExtP->bNBLCopyPoolInit = 1;

		Status = ExInitializeLookasideListEx(&devExtP->NBCopiesPool, NULL, NULL, NPF_NONPAGED, 0, sizeof(NPF_NB_COPIES), NPF_NBC_POOL_TAG, 0);
		if (Status != STATUS_SUCCESS)
		{
			ERROR_DBG("Failed to allocate NBCopiesPool\n");
			break;
		}
		devExtP->bNBCopiesPoolInit = 1;

		Status = ExInitializeLookasideListEx(&devExtP->SrcNBPool, NULL, NULL, NPF_NONPAGED, 0, sizeof(NPF_SRC_NB), NPF_SRCNB_POOL_TAG, 0);
		if (Status != STATUS_SUCCESS)
		{
			ERROR_DBG("Failed to allocate SrcNBPool\n");
			break;
		}
		devExtP->bSrcNBPoolInit = 1;

		Status = ExInitializeLookasideListEx(&devExtP->InternalRequestPool, NULL, NULL, NPF_NONPAGED, 0, sizeof(INTERNAL_REQUEST), NPF_REQ_POOL_TAG, 0);
		if (Status != STATUS_SUCCESS)
		{
			ERROR_DBG("Failed to allocate InternalRequestPool\n");
			break;
		}
		devExtP->bInternalRequestPoolInit = 1;

		Status = ExInitializeLookasideListEx(&devExtP->CapturePool, NULL, NULL, NPF_NONPAGED, 0, sizeof(NPF_CAP_DATA), NPF_CAP_POOL_TAG, 0);
		if (Status != STATUS_SUCCESS)
		{
			ERROR_DBG("Failed to allocate CapturePool\n");
			break;
		}
		devExtP->bCapturePoolInit = 1;

#ifdef HAVE_DOT11_SUPPORT
		if (g_Dot11SupportMode)
		{
			Status = ExInitializeLookasideListEx(&devExtP->Dot11HeaderPool, NULL, NULL, NPF_NONPAGED, 0, SIZEOF_RADIOTAP_BUFFER, NPF_DOT11_POOL_TAG, 0);
			if (Status != STATUS_SUCCESS)
			{
				ERROR_DBG("Failed to allocate Dot11HeaderPool\n");
				break;
			}
			devExtP->bDot11HeaderPoolInit = 1;
		}
#endif

#ifdef HAVE_WFP_LOOPBACK_SUPPORT
		KeInitializeMutex(&devExtP->WFPInitMutex, 0);
		Status = NPF_WFPCalloutRegister(devObjP);
		if (!NT_SUCCESS(Status))
		{
			ERROR_DBG("NPF_WFPCalloutRegister failed: %#08x\n", Status);
			break;
		}
#endif

		Status = STATUS_SUCCESS;
	} while (0);

	if (!NT_SUCCESS(Status))
	{
#ifdef HAVE_DOT11_SUPPORT
		if (devExtP->bDot11HeaderPoolInit)
			ExDeleteLookasideListEx(&devExtP->Dot11HeaderPool);
#endif
		if (devExtP->bCapturePoolInit)
			ExDeleteLookasideListEx(&devExtP->CapturePool);
		if (devExtP->bInternalRequestPoolInit)
			ExDeleteLookasideListEx(&devExtP->InternalRequestPool);
		if (devExtP->bNBCopiesPoolInit)
			ExDeleteLookasideListEx(&devExtP->NBCopiesPool);
		if (devExtP->bNBLCopyPoolInit)
			ExDeleteLookasideListEx(&devExtP->NBLCopyPool);
		if (devExtP->bSrcNBPoolInit)
			ExDeleteLookasideListEx(&devExtP->SrcNBPool);
		if (devExtP->bBufferPoolInit)
			ExDeleteLookasideListEx(&devExtP->BufferPool);
		if (devExtP->AllOpensLock)
			NdisFreeRWLock(devExtP->AllOpensLock);
		NdisFDeregisterFilterDriver(FilterDriverHandle);
		NdisFreeSpinLock(&g_FilterArrayLock);
		IoDeleteSymbolicLink(&deviceSymLink);
		IoDeleteDevice(devObjP);

		TRACE_EXIT();
		return Status;
	}


#ifdef HAVE_WFP_LOOPBACK_SUPPORT
	if (g_LoopbackSupportMode) {
		do {
			// Create the fake "filter module" for loopback capture
			// This is a hack to let NPF_CreateFilterModule create "\Device\NPCAP\Loopback" just like it usually does with a GUID
			NDIS_STRING LoopbackDeviceName = NDIS_STRING_CONST("\\Device\\Loopback");
			PNPCAP_FILTER_MODULE pFiltMod = NPF_CreateFilterModule(FilterDriverHandle, &LoopbackDeviceName);
			if (pFiltMod == NULL)
			{
				WARNING_DBG("Could not create filter module for loopback.\n");
				break;
			}
			pFiltMod->Loopback = TRUE;
			pFiltMod->AdapterBindingStatus = FilterRunning;
			pFiltMod->MaxFrameSize = NPF_LOOPBACK_INTERFACR_MTU + ETHER_HDR_LEN;
			devExtP->pLoopbackFilter = pFiltMod;

			// No need to mess with SendToRx/BlockRx, packet filters, NDIS filter characteristics, Dot11, etc.
			NPF_AddToFilterModuleArray(pFiltMod);
		} while (0);

	}
#endif

	if (g_Dot11SupportMode)
	{
		// Register the WiFi filter to NDIS.
		Status = NdisFRegisterFilterDriver(DriverObject,
			(NDIS_HANDLE)FilterDriverObject,
			&FChars_WiFi,
			&FilterDriverHandle_WiFi);
		if (Status != NDIS_STATUS_SUCCESS)
		{
			ERROR_DBG("NdisFRegisterFilterDriver(WiFi) failed: %#08x\n", Status);
			FilterDriverHandle_WiFi = NULL;
			// We still run the driver even with the 2nd filter doesn't work.
		}
		INFO_DBG("FilterDriverHandle_WiFi = %p\n", FilterDriverHandle_WiFi);
	}

	pNpcapDeviceObject = devObjP;
	TRACE_EXIT();
	return STATUS_SUCCESS;
}

//-------------------------------------------------------------------
_Use_decl_annotations_
VOID
NPF_registerLWF(
	PNDIS_FILTER_DRIVER_CHARACTERISTICS pFChars,
	BOOLEAN bWiFiOrNot
	)
{
	NDIS_STRING FriendlyName = RTL_CONSTANT_STRING(NPF_SERVICE_DESC_WIDECHAR); // display name
	NDIS_STRING UniqueName = RTL_CONSTANT_STRING(FILTER_UNIQUE_NAME); // unique name, quid name
	NDIS_STRING ServiceName = RTL_CONSTANT_STRING(NPF_DRIVER_NAME_SMALL_WIDECHAR); // this to match the service name in the INF
	NDIS_STRING FriendlyName_WiFi = RTL_CONSTANT_STRING(NPF_SERVICE_DESC_WIDECHAR_WIFI); // display name
	NDIS_STRING UniqueName_WiFi = RTL_CONSTANT_STRING(FILTER_UNIQUE_NAME_WIFI); // unique name, quid name
	NDIS_STRING ServiceName_WiFi = RTL_CONSTANT_STRING(NPF_DRIVER_NAME_SMALL_WIDECHAR_WIFI); // this to match the service name in the INF

	NdisZeroMemory(pFChars, sizeof(NDIS_FILTER_DRIVER_CHARACTERISTICS));
	pFChars->Header.Type = NDIS_OBJECT_TYPE_FILTER_DRIVER_CHARACTERISTICS;
	pFChars->Header.Size = sizeof(NDIS_FILTER_DRIVER_CHARACTERISTICS);
#if NDIS_SUPPORT_NDIS680
	pFChars->Header.Revision = NDIS_FILTER_CHARACTERISTICS_REVISION_3;
#elif NDIS_SUPPORT_NDIS61
	pFChars->Header.Revision = NDIS_FILTER_CHARACTERISTICS_REVISION_2;
#else
	pFChars->Header.Revision = NDIS_FILTER_CHARACTERISTICS_REVISION_1;
#endif

	pFChars->MajorNdisVersion = NDIS_FILTER_MAJOR_VERSION;
	pFChars->MinorNdisVersion = NDIS_FILTER_MINOR_VERSION;
	// WINPCAP_MAJOR is 5 for Npcap
	pFChars->MajorDriverVersion = WINPCAP_MINOR;
	pFChars->MinorDriverVersion = WINPCAP_REV;
	pFChars->Flags = 0;

	// Use different names for the WiFi driver.
	if (bWiFiOrNot)
	{
		pFChars->FriendlyName = FriendlyName_WiFi;
		pFChars->UniqueName = UniqueName_WiFi;
		pFChars->ServiceName = ServiceName_WiFi;
	}
	else
	{
		pFChars->FriendlyName = FriendlyName;
		pFChars->UniqueName = UniqueName;
		pFChars->ServiceName = ServiceName;
	}

	pFChars->SetOptionsHandler = NPF_RegisterOptions;
	pFChars->AttachHandler = NPF_AttachAdapter;
	pFChars->DetachHandler = NPF_DetachAdapter;
	pFChars->RestartHandler = NPF_Restart;
	pFChars->PauseHandler = NPF_Pause;
	pFChars->SetFilterModuleOptionsHandler = NPF_SetModuleOptions;
	pFChars->OidRequestHandler = NPF_OidRequest;
	pFChars->OidRequestCompleteHandler = NPF_OidRequestComplete;
	pFChars->CancelOidRequestHandler = NPF_CancelOidRequest;

	pFChars->SendNetBufferListsHandler = NPF_SendEx;
	pFChars->ReturnNetBufferListsHandler = NPF_ReturnEx;
	pFChars->SendNetBufferListsCompleteHandler = NPF_SendCompleteEx;
	pFChars->ReceiveNetBufferListsHandler = NPF_TapEx;
	pFChars->DevicePnPEventNotifyHandler = NPF_DevicePnPEventNotify;
	pFChars->NetPnPEventHandler = NPF_NetPnPEvent;
	pFChars->StatusHandler = NPF_Status;
	pFChars->CancelSendNetBufferListsHandler = NPF_CancelSendNetBufferLists;

#if NDIS_SUPPORT_NDIS61
	pFChars->DirectOidRequestHandler = NULL;
	pFChars->DirectOidRequestCompleteHandler = NULL;
	pFChars->CancelDirectOidRequestHandler = NULL;
#endif

#if NDIS_SUPPORT_NDIS680
	pFChars->SynchronousOidRequestHandler = NULL;
	pFChars->SynchronousOidRequestCompleteHandler = NULL;
#endif
}

//-------------------------------------------------------------------

#define ABSOLUTE(wait)				(wait)
#define RELATIVE(wait)				(-(wait))
#define NANOSECONDS(nanos)			(((signed __int64)(nanos)) / 100L)
#define MICROSECONDS(micros)		(((signed __int64)(micros)) * NANOSECONDS(1000L))
#define MILLISECONDS(milli)			(((signed __int64)(milli)) * MICROSECONDS(1000L))
#define SECONDS(seconds)			(((signed __int64)(seconds)) * MILLISECONDS(1000L))


_Ret_maybenull_
static PKEY_VALUE_PARTIAL_INFORMATION
NPF_GetRegistryOption(
	_In_ PUNICODE_STRING RegistryPath,
	_In_ PUNICODE_STRING RegValueName
	)
{
	OBJECT_ATTRIBUTES objAttrs;
	NTSTATUS status;
	HANDLE keyHandle;
	PKEY_VALUE_PARTIAL_INFORMATION valueInfoP = NULL;
	SHORT retries = 2;

	TRACE_ENTER();
	INFO_DBG("\nRegistryPath: %ws, RegValueName: %ws\n", RegistryPath->Buffer, RegValueName->Buffer);

	InitializeObjectAttributes(&objAttrs, RegistryPath, OBJ_CASE_INSENSITIVE, NULL, NULL);
	status = ZwOpenKey(&keyHandle, KEY_READ, &objAttrs);
	if (!NT_SUCCESS(status))
	{
		WARNING_DBG("ZwOpenKey failed: %#08x\n", status);
	}
	else //OK
	{
		ULONG resultLength;
		do
		{
			status = ZwQueryValueKey(keyHandle,
				RegValueName,
				KeyValuePartialInformation,
				NULL,
				0,
				&resultLength);

			if (NT_SUCCESS(status) || status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL)
			{

				valueInfoP = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePoolWithTag(PagedPool, resultLength, NPF_SHORT_TERM_TAG);
				if (valueInfoP != NULL)
				{
					status = ZwQueryValueKey(keyHandle,
						RegValueName,
						KeyValuePartialInformation,
						valueInfoP,
						resultLength,
						&resultLength);
					if (!NT_SUCCESS(status))
					{
						WARNING_DBG("ZwQueryValueKey failed: %#08x\n", status);
					}
					else
					{
						break;
					}
					ExFreePool(valueInfoP);
					valueInfoP = NULL;
				}
				else
				{
					WARNING_DBG("Paged alloc of valueInfoP failed.\n");
				}
			}
			else
			{
				WARNING_DBG("ZwQueryValueKey(NULL buffer) failed: %#08x\n", status);
				break;
			}
		} while (--retries > 0 && (status == STATUS_BUFFER_TOO_SMALL || status == STATUS_BUFFER_OVERFLOW));

		ZwClose(keyHandle);
	}

	TRACE_EXIT();
	return valueInfoP;
}

//-------------------------------------------------------------------
_Use_decl_annotations_
ULONG
NPF_GetRegistryOption_Integer(
	PUNICODE_STRING RegistryPath,
	PUNICODE_STRING RegValueName
	)
{
	ULONG returnValue = 0;
	PKEY_VALUE_PARTIAL_INFORMATION valueInfoP = NULL;

	TRACE_ENTER();

	valueInfoP = NPF_GetRegistryOption(RegistryPath, RegValueName);

	if (valueInfoP != NULL)
	{
		if (valueInfoP->Type == REG_DWORD && valueInfoP->DataLength == 4)
		{
			returnValue = *((ULONG *) valueInfoP->Data);
			INFO_DBG("\"%ws\" Key = %08x\n", RegValueName->Buffer, *((ULONG *)valueInfoP->Data));
		}
		else
		{
			WARNING_DBG("\"%ws\" Key invalid type (%lu) or length (%lu)\n", RegValueName->Buffer, valueInfoP->Type, valueInfoP->DataLength);
		}
		ExFreePool(valueInfoP);
	}

	TRACE_EXIT();
	return returnValue;
}

//-------------------------------------------------------------------
_Use_decl_annotations_
VOID
NPF_GetRegistryOption_String(
	PUNICODE_STRING RegistryPath,
	PUNICODE_STRING RegValueName,
	PNDIS_STRING g_OutputString
	)
{
	PKEY_VALUE_PARTIAL_INFORMATION valueInfoP = NULL;

	TRACE_ENTER();

	valueInfoP = NPF_GetRegistryOption(RegistryPath, RegValueName);

	if (valueInfoP != NULL)
	{
		if (valueInfoP->Type == REG_SZ && valueInfoP->DataLength > 1)
		{
			INFO_DBG("\"%ws\" Key = %ws\n", RegValueName->Buffer, (PWSTR)valueInfoP->Data);

			g_OutputString->Length = (USHORT)(valueInfoP->DataLength - sizeof(UNICODE_NULL));
			g_OutputString->MaximumLength = (USHORT)(valueInfoP->DataLength);
			g_OutputString->Buffer = ExAllocatePoolWithTag(NPF_NONPAGED, g_OutputString->MaximumLength, NPF_UNICODE_BUFFER_TAG);

			if (g_OutputString->Buffer)
			{
				RtlCopyMemory(g_OutputString->Buffer, valueInfoP->Data, valueInfoP->DataLength);
			}
			else
			{
				WARNING_DBG("Nonpaged alloc of g_OutputString failed\n");
				g_OutputString->Length = g_OutputString->MaximumLength = 0;
			}
		}
		else
		{
			WARNING_DBG("\"%ws\" Key invalid type (%lu) or length (%lu)\n", RegValueName->Buffer, valueInfoP->Type, valueInfoP->DataLength);
		}
		ExFreePool(valueInfoP);
	}

	TRACE_EXIT();
}

//-------------------------------------------------------------------

_Use_decl_annotations_
VOID
NPF_Unload(
	IN PDRIVER_OBJECT      DriverObject
)
/*++

Routine Description:

Filter driver's unload routine.
	Deregister the driver from NDIS.

Arguments:

	DriverObject - pointer to the system's driver object structure
				   for this driver

Return Value:

	NONE

--*/
{
	PLIST_ENTRY CurrEntry = NULL;
	PDEVICE_OBJECT DeviceObject;
	PDEVICE_OBJECT OldDeviceObject;
	PDEVICE_EXTENSION DeviceExtension;
	PNPCAP_FILTER_MODULE pFiltMod = NULL;
	PSINGLE_LIST_ENTRY Prev = NULL;
	PSINGLE_LIST_ENTRY Curr = NULL;
	NDIS_EVENT Event;
	LOCK_STATE_EX lockState, lockState2;

	TRACE_ENTER();

	NdisInitializeEvent(&Event);
	NdisResetEvent(&Event);

#ifdef HAVE_WFP_LOOPBACK_SUPPORT
	// Free the loopback adapter name
	if (g_LoopbackAdapterName.Buffer != NULL)
	{
		ExFreePool(g_LoopbackAdapterName.Buffer);
		g_LoopbackAdapterName.Buffer = NULL;
	}

	// Release WFP resources.
	NPF_ReleaseWFP(pNpcapDeviceObject, TRUE);
	NPF_WFPCalloutUnregister(pNpcapDeviceObject);
#endif

#ifdef HAVE_RX_SUPPORT
	// Free the send-to-Rx adapter name
	if (g_SendToRxAdapterName.Buffer != NULL)
	{
		ExFreePool(g_SendToRxAdapterName.Buffer);
		g_SendToRxAdapterName.Buffer = NULL;
	}
	if (g_BlockRxAdapterName.Buffer != NULL)
	{
		ExFreePool(g_BlockRxAdapterName.Buffer);
		g_BlockRxAdapterName.Buffer = NULL;
	}
#endif

	DeviceObject = DriverObject->DeviceObject;

	while (DeviceObject != NULL)
	{
		OldDeviceObject = DeviceObject;

		DeviceObject = DeviceObject->NextDevice;

		DeviceExtension = OldDeviceObject->DeviceExtension;

		// Must still acquire this lock because NDIS could still be doing things with filter modules
		// Specifically, NPF_AttachAdapter acquires this for Read.
		NdisAcquireRWLockWrite(DeviceExtension->AllOpensLock, &lockState, 0);
		CurrEntry = RemoveHeadList(&DeviceExtension->AllOpens);
		while (CurrEntry != &DeviceExtension->AllOpens)
		{
			POPEN_INSTANCE pOpen = CONTAINING_RECORD(CurrEntry, OPEN_INSTANCE, AllOpensEntry);
			// Pretty sure we don't get here unless all this is already closed up, but better to be thorough.
			OPEN_STATE OldState = NPF_DemoteOpenStatus(pOpen, OpenClosed);
			// Just unlink it; other cleanup is handled already.
			if (NULL != (pFiltMod = pOpen->pFiltMod))
			{
				NdisAcquireRWLockWrite(pFiltMod->OpenInstancesLock, &lockState2, NDIS_RWL_AT_DISPATCH_LEVEL);
				Prev = &(pFiltMod->OpenInstances);
				Curr = Prev->Next;
				while (Curr != NULL)
				{
					if (Curr == &(pOpen->OpenInstancesEntry)) {
						Prev->Next = Curr->Next;
						Curr->Next = NULL;
						break;
					}
					Prev = Curr;
					Curr = Prev->Next;
				}
				NdisReleaseRWLock(pFiltMod->OpenInstancesLock, &lockState2);
			}
			NPF_ReleaseOpenInstanceResources(pOpen);
			ExFreePool(pOpen);
			CurrEntry = RemoveHeadList(&DeviceExtension->AllOpens);
		}

		NdisReleaseRWLock(DeviceExtension->AllOpensLock, &lockState);

		IoDeleteSymbolicLink(&deviceSymLink);

		ExDeleteLookasideListEx(&DeviceExtension->BufferPool);
		ExDeleteLookasideListEx(&DeviceExtension->NBLCopyPool);
		ExDeleteLookasideListEx(&DeviceExtension->NBCopiesPool);
		ExDeleteLookasideListEx(&DeviceExtension->SrcNBPool);
		ExDeleteLookasideListEx(&DeviceExtension->InternalRequestPool);
		ExDeleteLookasideListEx(&DeviceExtension->CapturePool);
#ifdef HAVE_DOT11_SUPPORT
		if (DeviceExtension->bDot11HeaderPoolInit)
			ExDeleteLookasideListEx(&DeviceExtension->Dot11HeaderPool);
#endif

		NdisFreeRWLock(DeviceExtension->AllOpensLock);
		IoDeleteDevice(OldDeviceObject);
	}

	if (FilterDriverHandle)
	{
		INFO_DBG("NdisFDeregisterFilterDriver: Deleting Filter Handle = %p\n", FilterDriverHandle);
		NdisFDeregisterFilterDriver(FilterDriverHandle);
		FilterDriverHandle = NULL;
	}
	else
	{
		INFO_DBG("NdisFDeregisterFilterDriver: Filter Handle = NULL, no need to delete.\n");
	}

	if (FilterDriverHandle_WiFi)
	{
		INFO_DBG("NdisFDeregisterFilterDriver: Deleting Filter Handle (WiFi) = %p\n", FilterDriverHandle_WiFi);
		NdisFDeregisterFilterDriver(FilterDriverHandle_WiFi);
		FilterDriverHandle_WiFi = NULL;
	}
	else
	{
		INFO_DBG("NdisFDeregisterFilterDriver: Filter Handle (WiFi) = NULL, no need to delete.\n");
	}
	// NdisFDeregisterFilterDriver ought to have called FilterDetach, but something is leaking. Let's force a wait:
	NdisAcquireSpinLock(&g_FilterArrayLock);
	while (g_arrFiltMod.Next != NULL) {
#ifdef HAVE_WFP_LOOPBACK_SUPPORT
		pFiltMod = CONTAINING_RECORD(g_arrFiltMod.Next, NPCAP_FILTER_MODULE, FilterModulesEntry);
		if (pFiltMod->Loopback) {
			// NDIS doesn't manage this, so we "detach" it ourselves.
			NdisReleaseSpinLock(&g_FilterArrayLock);
			NPF_DetachAdapter(pFiltMod);
		}
		else
#endif
		{
			// Wait for NDIS to release it
			NdisReleaseSpinLock(&g_FilterArrayLock);
			NdisWaitEvent(&Event, 1);
		}
		NdisAcquireSpinLock(&g_FilterArrayLock);
		NdisResetEvent(&Event);
	}
	NdisReleaseSpinLock(&g_FilterArrayLock);

	NdisFreeSpinLock(&g_FilterArrayLock);

	TRACE_EXIT();
}

#define SET_RESULT_SUCCESS(__a__) do{\
	Information = __a__;	\
	Status = STATUS_SUCCESS;	\
} while(FALSE)

#define SET_FAILURE_BUFFER(__len__) do {\
	Information = __len__; \
	Status = STATUS_BUFFER_TOO_SMALL; \
} while(FALSE)

#define SET_FAILURE(__STATUS_CODE) do{\
	Information = 0; \
	Status = __STATUS_CODE; \
} while(FALSE)

#define SET_FAILURE_CUSTOM(__b__) do{\
	Information = 0; \
	Status = __b__; \
	Status |= 1 << 29; \
} while(FALSE)

//-------------------------------------------------------------------

/* DO_DIRECT_IO */
_Use_decl_annotations_
NTSTATUS NPF_ValidateIoIrp(
		PIRP pIrp,
		POPEN_INSTANCE *ppOpen,
		PVOID* ppBuf,
		PULONG pBufLen)
{
	PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(pIrp);
	POPEN_INSTANCE pOpen = IrpSp->FileObject->FsContext;
	PVOID pBuf = NULL;
	ULONG BufLen = 0;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	TRACE_ENTER();

	// Validation done for Direct IO only
	NT_ASSERT(!(pIrp->Flags & IRP_BUFFERED_IO));

	do /* Validate */
	{
		/* Context is an Open instance (also checks for NULL) */
		if (!NPF_IsOpenInstance(pOpen))
		{
			Status = STATUS_INVALID_HANDLE;
			break;
		}

		/* output buffer exists (If buffer is 0-length, I/O manager passes NULL here) */
		if (!pIrp->MdlAddress)
		{
			Status = STATUS_INVALID_PARAMETER;
			break;
		}

		// Has this IRP been canceled?
		if (pIrp->Cancel)
		{
			Status = STATUS_CANCELLED;
			break;
		}

		NdisQueryMdl(pIrp->MdlAddress, &pBuf, &BufLen, NormalPagePriority | MdlMappingNoExecute);
		if (pBuf == NULL)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		// The subsequent assertions only make sense for both Read and Write if
		// the Length field is in the same place in the Parameters union.
		// We'll verify that with a compile-time assertion here.
		C_ASSERT(FIELD_OFFSET(IO_STACK_LOCATION, Parameters.Read.Length) == FIELD_OFFSET(IO_STACK_LOCATION, Parameters.Write.Length));

		// Make sure the buffer length is correct.
		// This should be guaranteed by the I/O manager, but it'd be bad if we're wrong about that.
		NT_ASSERT(BufLen == IrpSp->Parameters.Read.Length);
		NT_ASSERT(BufLen == IrpSp->Parameters.Write.Length);

		// Success! Fill out the output parameters.
		Status = STATUS_SUCCESS;
	} while (FALSE);

	INFO_DBG("IRP %p status %#08x; pOpen = %p, pBuf = %p, BufLen = %lu\n", pIrp, Status, pOpen, pBuf, BufLen);
	if (Status != STATUS_SUCCESS)
	{
		// Ensure output param is NULL on failure
		pOpen = NULL;
		pBuf = NULL;
		BufLen = 0;
	}

	if (ppOpen)
		*ppOpen = pOpen;
	if (ppBuf)
		*ppBuf = pBuf;
	if (pBufLen)
		*pBufLen = BufLen;

	TRACE_EXIT();
	return Status;
}

_Must_inspect_result_
static NTSTATUS funcBIOCGSTATS(_In_ POPEN_INSTANCE pOpen,
	       	_Out_writes_bytes_(ulBufLen) PVOID pBuf,
	       	_In_ ULONG ulBufLen,
	       	_Out_ PULONG_PTR Info)
{
	static const ULONG uNeeded = 4 * sizeof(UINT);

	*Info = 0;
	if (ulBufLen < uNeeded)
	{
		return STATUS_BUFFER_TOO_SMALL;
	}

	if (!NPF_StartUsingOpenInstance(pOpen, OpenDetached, NPF_IRQL_UNKNOWN))
	{
		return STATUS_CANCELLED;
	}

	((PUINT)pBuf)[0] = pOpen->Received;
	((PUINT)pBuf)[1] = pOpen->Dropped + pOpen->ResourceDropped;
	((PUINT)pBuf)[2] = 0;		// Not yet supported
	((PUINT)pBuf)[3] = pOpen->Accepted;

	NPF_StopUsingOpenInstance(pOpen, OpenDetached, NPF_IRQL_UNKNOWN);

	*Info = uNeeded;
	return STATUS_SUCCESS;
}

_Must_inspect_result_
static NTSTATUS funcBIOCSETF(_In_ POPEN_INSTANCE pOpen,
	       	_In_reads_bytes_(ulBufLen) LPCVOID pBuf,
	       	_In_ ULONG ulBufLen,
	       	_Out_ PULONG_PTR Info)
{
	LOCK_STATE_EX lockState;
	static const ULONG uNeeded = sizeof(struct bpf_insn);
	struct bpf_insn *NewBpfProgram = (struct bpf_insn *) pBuf;
	*Info = 0;

	if (ulBufLen < uNeeded)
	{
		return STATUS_BUFFER_TOO_SMALL;
	}

	// Validate the new program (valid instructions, only forward jumps, etc.)
	ULONG insns = ulBufLen / sizeof(struct bpf_insn);
	if (insns > BPF_MAXINSNS || !bpf_validate(NewBpfProgram, insns))
	{
		WARNING_DBG("BPF filter invalid.\n");
		return STATUS_INVALID_DEVICE_REQUEST;
	}

	// Truncate buf to last entire instruction
	ulBufLen = insns * sizeof(struct bpf_insn);

	// Allocate the memory to contain the new filter program
	PUCHAR TmpBPFProgram = (PUCHAR)ExAllocatePoolWithTag(NPF_NONPAGED, ulBufLen, NPF_BPF_TAG);
	if (TmpBPFProgram == NULL)
	{
		WARNING_DBG("Failed to alloc TmpBPFProgram.\n");
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	//copy the program in the new buffer
	RtlCopyMemory(TmpBPFProgram, NewBpfProgram, ulBufLen);

	if (!NPF_StartUsingOpenInstance(pOpen, OpenDetached, NPF_IRQL_UNKNOWN))
	{
		ExFreePool(TmpBPFProgram);
		return STATUS_CANCELLED;
	}

	// Lock the BPF engine for writing. 
	NdisAcquireRWLockWrite(pOpen->MachineLock, &lockState, 0);

	// Free the previous buffer if it was present
	if (pOpen->bpfprogram != NULL)
	{
		ExFreePool(pOpen->bpfprogram);
		pOpen->bpfprogram = NULL;
	}
	pOpen->bpfprogram = TmpBPFProgram;

	// release the machine lock and then reset the buffer
	NdisReleaseRWLock(pOpen->MachineLock, &lockState);

	NPF_ResetBufferContents(pOpen, TRUE);

	NPF_StopUsingOpenInstance(pOpen, OpenDetached, NPF_IRQL_UNKNOWN);

	return STATUS_SUCCESS;
}

_Must_inspect_result_
static NTSTATUS funcBIOCSULONG(_In_ POPEN_INSTANCE pOpen,
	    _In_reads_bytes_(ulBufLen) PULONG pBuf,
	    _In_ ULONG ulBufLen,
	    _Out_ PULONG_PTR Info,
		_In_ OPEN_STATE MaxState,
	    _Out_ PULONG pulOut)
{
	static const ULONG uNeeded = sizeof(ULONG);

	*Info = 0;
	if (ulBufLen < uNeeded)
	{
		return STATUS_BUFFER_TOO_SMALL;
	}

	if (!NPF_StartUsingOpenInstance(pOpen, MaxState, NPF_IRQL_UNKNOWN))
	{
		return (pOpen->OpenStatus == OpenDetached
				? STATUS_DEVICE_REMOVED
				: STATUS_CANCELLED);
	}

	*pulOut = *pBuf;

	NPF_StopUsingOpenInstance(pOpen, MaxState, NPF_IRQL_UNKNOWN);
	return STATUS_SUCCESS;
}

_Must_inspect_result_
static NTSTATUS funcBIOCSMODE(_In_ POPEN_INSTANCE pOpen,
	       _In_reads_bytes_(ulBufLen) PULONG pBuf,
	       _In_ ULONG ulBufLen,
	       _Out_ PULONG_PTR Info)
{
	static const ULONG uNeeded = sizeof(ULONG);
	ULONG mode = 0;

	*Info = 0;
	if (ulBufLen < uNeeded)
	{
		return STATUS_BUFFER_TOO_SMALL;
	}

	mode = *pBuf;

	// Compile-time assertion to ensure that MODE_CAPT and MODE_STAT are mutually exclusive.
	C_ASSERT(MODE_CAPT == 0 && MODE_STAT == 1);

	// If any bits except the least-significant are set, it's an invalid mode.
	if (mode > 1) 
	{
		return STATUS_INVALID_DEVICE_REQUEST;
	}

	if (!NPF_StartUsingOpenInstance(pOpen, OpenRunning, NPF_IRQL_UNKNOWN))
	{
		return (pOpen->OpenStatus <= OpenDetached
				? STATUS_DEVICE_REMOVED
				: STATUS_CANCELLED);
	}

	if (mode == MODE_STAT)
	{
		pOpen->bModeCapt = 0;
		NdisAcquireSpinLock(&pOpen->CountersLock);
		pOpen->Nbytes.QuadPart = 0;
		pOpen->Npackets.QuadPart = 0;
		NdisReleaseSpinLock(&pOpen->CountersLock);
	}
	else // MODE_CAPT
	{
		pOpen->bModeCapt = 1;
	}

	NPF_StopUsingOpenInstance(pOpen, OpenRunning, NPF_IRQL_UNKNOWN);
	return STATUS_SUCCESS;
}


_Must_inspect_result_
static NTSTATUS funcBIOCISETLOBBEH(_In_ POPEN_INSTANCE pOpen,
	       _In_reads_bytes_(ulBufLen) PULONG pBuf,
	       _In_ ULONG ulBufLen,
	       _Out_ PULONG_PTR Info)
{
	static const ULONG uNeeded = sizeof(ULONG);
	BOOLEAN SkipSent = FALSE;

	*Info = 0;
	if (ulBufLen < uNeeded)
	{
		return STATUS_BUFFER_TOO_SMALL;
	}

	switch (*pBuf)
	{
		case NPF_DISABLE_LOOPBACK:
			SkipSent = TRUE;
			break;
		case NPF_ENABLE_LOOPBACK:
			SkipSent = FALSE;
			break;
		default:
			return STATUS_INVALID_DEVICE_REQUEST;
			break;
	}

	if (!NPF_StartUsingOpenInstance(pOpen, OpenDetached, NPF_IRQL_UNKNOWN))
	{
		return STATUS_CANCELLED;
	}

	// Set the new value. If it's different than before,
	if (pOpen->SkipSentPackets != SkipSent)
	{
		pOpen->SkipSentPackets = SkipSent;
		// and they want to start skipping packets,
		if (SkipSent)
		{
			// clear out any packets from the buffer that would have been skipped
			// TODO: Change this to actually traverse the PacketQueue and
			// remove only injected packets. Will need to extend
			// NPF_CAP_DATA to hold this info.
			NPF_ResetBufferContents(pOpen, TRUE);
		}
	}

	NPF_StopUsingOpenInstance(pOpen, OpenDetached, NPF_IRQL_UNKNOWN);
	return STATUS_SUCCESS;
}

_Must_inspect_result_
static NTSTATUS funcBIOCSETEVENTHANDLE(_In_ POPEN_INSTANCE pOpen,
	       _In_reads_bytes_(ulBufLen) PVOID pBuf,
	       _In_ ULONG ulBufLen,
	       _In_ BOOLEAN is32bit,
	       _In_ KPROCESSOR_MODE kMode,
	       _Out_ PULONG_PTR Info)
{
	HANDLE hUserEvent = INVALID_HANDLE_VALUE;
	PKEVENT pKernelEvent = NULL;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	ULONG uNeeded = is32bit ? sizeof(VOID * POINTER_32) : sizeof(VOID * POINTER_64);

	*Info = 0;
	// We don't currently support overwriting the existing event.
	if (pOpen->ReadEvent != NULL)
	{
		return STATUS_OBJECT_NAME_EXISTS;
	}

	if (ulBufLen < uNeeded)
	{
		return STATUS_BUFFER_TOO_SMALL;
	}
	else if (ulBufLen > uNeeded)
	{
		return STATUS_INVALID_PARAMETER;
	}

#ifndef _WIN64
	// WIN32
	NT_ASSERT(is32bit);
#else
	// WIN64
	if (!is32bit)
	{
	       	// 64-bit process
		// HANDLE is 64-bit address
		hUserEvent = *(PHANDLE) pBuf;
	}
	else
#endif
	{
		// 32-bit process
		// Convert to native handle if necessary
		hUserEvent = Handle32ToHandle(*(VOID * POINTER_32 *) pBuf);
	}

	Status = ObReferenceObjectByHandle(hUserEvent,
			EVENT_MODIFY_STATE,
			*ExEventObjectType,
			kMode,
			(PVOID *) &pKernelEvent,
			NULL);

	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	if (!NPF_StartUsingOpenInstance(pOpen, OpenDetached, NPF_IRQL_UNKNOWN))
	{
		ObDereferenceObject(pKernelEvent);
		return STATUS_CANCELLED;
	}

	// If the event is NULL, replace it. This returns the previous value, so if it's not NULL,
	// we don't replace it and need to bail.
	if (InterlockedCompareExchangePointer(&pOpen->ReadEvent, pKernelEvent, NULL) != NULL)
	{
		// dereference the new pointer
		ObDereferenceObject(pKernelEvent);
		NPF_StopUsingOpenInstance(pOpen, OpenDetached, NPF_IRQL_UNKNOWN);
		return STATUS_OBJECT_NAME_EXISTS;
	}

	KeResetEvent(pOpen->ReadEvent);

	NPF_StopUsingOpenInstance(pOpen, OpenDetached, NPF_IRQL_UNKNOWN);
	return STATUS_SUCCESS;
}

_Must_inspect_result_
static NTSTATUS funcBIOCSETBUFFERSIZE(_In_ POPEN_INSTANCE pOpen,
	       _In_reads_bytes_(ulBufLen) PULONG pBuf,
	       _In_ ULONG ulBufLen,
	       _Out_ PULONG_PTR Info)
{
	static const ULONG uNeeded = sizeof(ULONG);
	LOCK_STATE_EX lockState;
	ULONG dim = 0;

	*Info = 0;
	if (ulBufLen < uNeeded)
	{
		return STATUS_BUFFER_TOO_SMALL;
	}

	dim = *(PULONG) pBuf;

	// verify that the provided size value is sensible
	if (dim > NPF_MAX_BUFFER_SIZE)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	} 

	if (!NPF_StartUsingOpenInstance(pOpen, OpenRunning, NPF_IRQL_UNKNOWN))
	{
		return (pOpen->OpenStatus <= OpenDetached
				? STATUS_DEVICE_REMOVED
				: STATUS_CANCELLED);
	}

	// Acquire buffer lock
	NdisAcquireRWLockWrite(pOpen->BufferLock, &lockState, 0);

	do
	{
		// If there's no change, we're done!
		if ((LONG)dim == pOpen->Size) {
			break;
		}

		// TODO: Could we avoid clearing the buffer but instead allow a
		// negative Free count or maybe just clear out the amount that
		// exceeds Size?
		pOpen->Size = dim;
		NPF_ResetBufferContents(pOpen, FALSE);
	} while (FALSE);

	NdisReleaseRWLock(pOpen->BufferLock, &lockState);

	NPF_StopUsingOpenInstance(pOpen, OpenRunning, NPF_IRQL_UNKNOWN);
	return STATUS_SUCCESS;
}

_Must_inspect_result_
static NTSTATUS funcBIOC_OID(_In_ POPEN_INSTANCE pOpen,
	       _Inout_updates_bytes_(ulBufLenIn) PPACKET_OID_DATA OidData,
	       _In_ ULONG ulBufLenIn,
	       _In_ ULONG ulBufLenOut,
	       _In_ BOOLEAN bSetOid,
	       _Out_ PULONG_PTR Info)
{
	PINTERNAL_REQUEST pRequest = NULL;
	PVOID OidBuffer = NULL;
	LOCK_STATE_EX lockState;
	ULONG ulTmp = 0;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	*Info = 0;
	// NDIS OID requests use the same buffer for in/out, so the caller must supply the same size buffers, too.
	if (ulBufLenIn != ulBufLenOut ||
			ulBufLenIn < sizeof(PACKET_OID_DATA) || // check before dereferencing OidData
			ulBufLenIn < (FIELD_OFFSET(PACKET_OID_DATA, Data) + OidData->Length) ||
			OidData->Length == 0
		)
	{
		return STATUS_BUFFER_TOO_SMALL;
	}

	INFO_DBG("%s Request: Oid=%08lx, Length=%08lx\n", bSetOid ? "BIOCSETOID" : "BIOCQUERYOID", OidData->Oid, OidData->Length);

	if (!NPF_StartUsingOpenInstance(pOpen, OpenAttached, NPF_IRQL_UNKNOWN))
	{
		return (pOpen->OpenStatus == OpenDetached
				? STATUS_DEVICE_REMOVED
				: STATUS_CANCELLED);
	}

		// We don't really support OID requests on our fake loopback
		// adapter, but we can pretend.
	if (bSetOid && (pOpen->pFiltMod->Loopback || pOpen->pFiltMod->Fragile))
	{
		switch (OidData->Oid) {
			// Backwards compatibility: libpcap can't handle adapters that do not support setting packet filter.
			case OID_GEN_CURRENT_PACKET_FILTER:
				Status = STATUS_SUCCESS;
				break;
			default:
				INFO_DBG("pFiltMod(%p) BIOCSETOID not supported. Loopback: %u, Fragile: %u\n",
						pOpen->pFiltMod, pOpen->pFiltMod->Loopback, pOpen->pFiltMod->Fragile);
				Status = STATUS_INVALID_DEVICE_REQUEST;
				break;
		}
		goto OID_REQUEST_DONE;
	}
#ifdef HAVE_WFP_LOOPBACK_SUPPORT
	if (!bSetOid && pOpen->pFiltMod->Loopback)
	{
		switch (OidData->Oid)
		{
			case OID_GEN_MAXIMUM_TOTAL_SIZE:
			case OID_GEN_TRANSMIT_BUFFER_SPACE:
			case OID_GEN_RECEIVE_BUFFER_SPACE:
				if (OidData->Length < sizeof(UINT))
				{
					Status = STATUS_BUFFER_TOO_SMALL;
					break;
				}
				*Info = FIELD_OFFSET(PACKET_OID_DATA, Data) + sizeof(UINT);
				*((PUINT)OidData->Data) = pOpen->pFiltMod->MaxFrameSize;
				OidData->Length = sizeof(UINT);
				INFO_DBG("Loopback: get MTU = %u\n", *((PUINT)OidData->Data));
				Status = STATUS_SUCCESS;
				break;

			case OID_GEN_TRANSMIT_BLOCK_SIZE:
			case OID_GEN_RECEIVE_BLOCK_SIZE:
				if (OidData->Length < sizeof(UINT))
				{
					Status = STATUS_BUFFER_TOO_SMALL;
					break;
				}
				*Info = FIELD_OFFSET(PACKET_OID_DATA, Data) + sizeof(UINT);
				*((PUINT)OidData->Data) = 1;
				OidData->Length = sizeof(UINT);
				INFO_DBG("Loopback: get OID_GEN_*_BLOCK_SIZE = %u\n", *((PUINT)OidData->Data));
				Status = STATUS_SUCCESS;
				break;
			case OID_GEN_MEDIA_IN_USE:
			case OID_GEN_MEDIA_SUPPORTED:
				if (OidData->Length < sizeof(UINT))
				{
					Status = STATUS_BUFFER_TOO_SMALL;
					break;
				}
				*Info = FIELD_OFFSET(PACKET_OID_DATA, Data) + sizeof(UINT);
				*((PUINT)OidData->Data) = g_DltNullMode ? NdisMediumNull : NdisMedium802_3;
				OidData->Length = sizeof(UINT);
				INFO_DBG("Loopback: get OID_GEN_MEDIA_IN_USE = %u\n", *((PUINT)OidData->Data));
				Status = STATUS_SUCCESS;
				break;
			case OID_GEN_LINK_STATE:
				if (OidData->Length < sizeof(NDIS_LINK_STATE))
				{
					Status = STATUS_BUFFER_TOO_SMALL;
					break;
				}
				*Info = FIELD_OFFSET(PACKET_OID_DATA, Data) + sizeof(NDIS_LINK_STATE);
				PNDIS_LINK_STATE pLinkState = (PNDIS_LINK_STATE) OidData->Data;
				pLinkState->MediaConnectState = MediaConnectStateConnected;
				pLinkState->MediaDuplexState = MediaDuplexStateFull;
				pLinkState->XmitLinkSpeed = NDIS_LINK_SPEED_UNKNOWN;
				pLinkState->RcvLinkSpeed = NDIS_LINK_SPEED_UNKNOWN;
				pLinkState->PauseFunctions = NdisPauseFunctionsUnsupported;
				OidData->Length = sizeof(NDIS_LINK_STATE);
				Status = STATUS_SUCCESS;
				break;
			default:
				WARNING_DBG("Unsupported BIOCQUERYOID for Loopback\n");
				Status = STATUS_INVALID_DEVICE_REQUEST;
				break;
		}
		goto OID_REQUEST_DONE;
	}
#endif
	NT_ASSERT(!pOpen->pFiltMod->Loopback);
	NT_ASSERT(!(pOpen->pFiltMod->Fragile && bSetOid));

#ifdef HAVE_DOT11_SUPPORT
	if (pOpen->pFiltMod->Dot11 && (OidData->Oid == OID_GEN_MEDIA_IN_USE || OidData->Oid == OID_GEN_MEDIA_SUPPORTED))
	{
		if (bSetOid)
		{
			INFO_DBG("Dot11: AdapterName=%ws, OID_GEN_MEDIA_IN_USE & BIOCSETOID, fail it\n", pOpen->pFiltMod->AdapterName.Buffer);
			Status = STATUS_UNSUCCESSFUL;
		}
		else
		{
			INFO_DBG("Dot11: AdapterName=%ws, OID_GEN_MEDIA_IN_USE & BIOCGETOID, OidData->Data = %d\n", pOpen->pFiltMod->AdapterName.Buffer, NdisMediumRadio80211);
			if (OidData->Length < sizeof(UINT))
			{
				Status = STATUS_BUFFER_TOO_SMALL;
			}
			else
			{
				*Info = FIELD_OFFSET(PACKET_OID_DATA, Data) + sizeof(UINT);
				OidData->Length = sizeof(UINT);
				*((PUINT)OidData->Data) = (UINT)NdisMediumRadio80211;
				Status = STATUS_SUCCESS;
			}
		}

		goto OID_REQUEST_DONE;
	}
#endif

	// OID_GEN_CURRENT_PACKET_FILTER requires additional checks
	if (bSetOid && OidData->Oid == OID_GEN_CURRENT_PACKET_FILTER)
	{
		NT_ASSERT(pOpen->pFiltMod != NULL);

		if (OidData->Length != sizeof(ULONG))
		{
			Status = STATUS_BUFFER_TOO_SMALL;
			goto OID_REQUEST_DONE;
		}
		*Info = FIELD_OFFSET(PACKET_OID_DATA, Data) + sizeof(ULONG);

#ifdef HAVE_DOT11_SUPPORT
		// Disable setting Packet Filter for wireless adapters, because this will cause limited connectivity.
		if (pOpen->bDot11)
		{
			INFO_DBG("pFiltMod(%p) (Dot11) does not support OID_GEN_CURRENT_PACKET_FILTER\n", pOpen->pFiltMod);
			Status = STATUS_SUCCESS;
			goto OID_REQUEST_DONE;
		}
#endif

		// Stash the old packet filter...
		ulTmp = pOpen->MyPacketFilter;
		// Store the requested packet filter for *this* Open instance
		pOpen->MyPacketFilter = *(ULONG*)OidData->Data;

		/* We don't want NDIS_PACKET_TYPE_ALL_LOCAL, since that may cause NDIS to loop
		 * packets back that shouldn't be. WinPcap had to do this as a protocol driver,
		 * but Npcap sees outgoing packets from all protocols already.  We'll clear this
		 * bit, but instead turn on the other aspects that it covers: packets that would
		 * be indicated by the NIC anyway.
		 */
		if (pOpen->MyPacketFilter & NDIS_PACKET_TYPE_ALL_LOCAL) {
			pOpen->MyPacketFilter ^= NDIS_PACKET_TYPE_ALL_LOCAL;
			pOpen->MyPacketFilter |= NDIS_PACKET_TYPE_DIRECTED | NDIS_PACKET_TYPE_MULTICAST | NDIS_PACKET_TYPE_BROADCAST;
		}

		// If the new packet filter is the same as the old one
		if (pOpen->MyPacketFilter == ulTmp)
		{
			// Nothing left to do!
			Status = STATUS_SUCCESS;
			goto OID_REQUEST_DONE;
		}

		// Start clean
		ulTmp = 0;
		// Set the filter module's packet filter to the union of all instances' filters
		NdisAcquireRWLockRead(pOpen->pFiltMod->OpenInstancesLock, &lockState, 0);
		for (PSINGLE_LIST_ENTRY Curr = pOpen->pFiltMod->OpenInstances.Next; Curr != NULL; Curr = Curr->Next)
		{
			ulTmp |= CONTAINING_RECORD(Curr, OPEN_INSTANCE, OpenInstancesEntry)->MyPacketFilter;
		}
		NdisReleaseRWLock(pOpen->pFiltMod->OpenInstancesLock, &lockState);

		// Now ulTmp is the new effective filter
		// NPF_SetPacketFilter will take care of any short-circuits and all the request stuff
		Status = NPF_SetPacketFilter(pOpen->pFiltMod, ulTmp);
		goto OID_REQUEST_DONE;
	}
	else if (bSetOid && OidData->Oid == OID_GEN_CURRENT_LOOKAHEAD)
	{
		// Stash the old lookahead
		ulTmp = pOpen->MyLookaheadSize;
		pOpen->MyLookaheadSize = *(PULONG) OidData->Data;
		// If it didn't change, or
		if (pOpen->MyLookaheadSize == ulTmp ||
			// if it got bigger but would not increase the current value, or
			(pOpen->MyLookaheadSize > ulTmp && pOpen->MyLookaheadSize <= pOpen->pFiltMod->MyLookaheadSize) ||
			// if it got smaller, but the old value was smaller than the current max,
			(pOpen->MyLookaheadSize < ulTmp && ulTmp < pOpen->pFiltMod->MyLookaheadSize))
		{
			// Nothing left to do!
			Status = STATUS_SUCCESS;
			goto OID_REQUEST_DONE;
		}
		// Remaining possibilities:
		// 1. It got smaller and will decrease the current value
		if (pOpen->MyLookaheadSize < ulTmp) {
			// Figure out the max of all open instances' lookaheads
			NdisAcquireRWLockRead(pOpen->pFiltMod->OpenInstancesLock, &lockState, 0);
			// Start clean
			ulTmp = 0;
			for (PSINGLE_LIST_ENTRY Curr = pOpen->pFiltMod->OpenInstances.Next; Curr != NULL; Curr = Curr->Next)
			{
				ulTmp = max(pOpen->pFiltMod->MyLookaheadSize,
						CONTAINING_RECORD(Curr, OPEN_INSTANCE, OpenInstancesEntry)->MyLookaheadSize);
			}
			NdisReleaseRWLock(pOpen->pFiltMod->OpenInstancesLock, &lockState);
		}
		// 2. It got bigger and will increase the current value.
		else {
			ulTmp = pOpen->MyLookaheadSize;
		}
		// Now ulTmp is the new max value. Let NPF_SetLookaheadSize handle request if needed
		Status = NPF_SetLookaheadSize(pOpen->pFiltMod, ulTmp);
		goto OID_REQUEST_DONE;
	}

	//  The buffer is valid

	// Extract a request from the list of free ones
	pRequest = (PINTERNAL_REQUEST) ExAllocateFromLookasideListEx(&pOpen->DeviceExtension->InternalRequestPool);
	if (pRequest == NULL)
	{
		INFO_DBG("pRequest=NULL\n");
		Status = STATUS_INSUFFICIENT_RESOURCES;
		goto OID_REQUEST_DONE;
	}
	// This also zeroes the NDIS_OID_REQUEST structure.
	RtlZeroMemory(pRequest, sizeof(INTERNAL_REQUEST));

	pRequest->Request.Header.Type = NDIS_OBJECT_TYPE_OID_REQUEST;
	pRequest->Request.Header.Revision = NDIS_OID_REQUEST_REVISION_1;
	pRequest->Request.Header.Size = NDIS_SIZEOF_OID_REQUEST_REVISION_1;

	/* NDIS_OID_REQUEST.InformationBuffer must be non-paged */
	// TODO: Test whether this copy needs to happen. Buffered I/O ought to
	// mean AssociatedIrp.SystemBuffer is non-paged already and is not
	// freed until we complete the IRP.
	OidBuffer = ExAllocatePoolWithTag(NPF_NONPAGED, OidData->Length, NPF_USER_OID_TAG);
	if (OidBuffer == NULL)
	{
		INFO_DBG("Failed to allocate OidBuffer\n");
		Status = STATUS_INSUFFICIENT_RESOURCES;
		goto OID_REQUEST_DONE;
	}
	RtlCopyMemory(OidBuffer, OidData->Data, OidData->Length);

	if (bSetOid)
	{
		pRequest->Request.RequestType = NdisRequestSetInformation;
		pRequest->Request.DATA.SET_INFORMATION.Oid = OidData->Oid;

		pRequest->Request.DATA.SET_INFORMATION.InformationBuffer = OidBuffer;
		pRequest->Request.DATA.SET_INFORMATION.InformationBufferLength = OidData->Length;
	}
	else
	{
		pRequest->Request.RequestType = NdisRequestQueryInformation;
		pRequest->Request.DATA.QUERY_INFORMATION.Oid = OidData->Oid;

		pRequest->Request.DATA.QUERY_INFORMATION.InformationBuffer = OidBuffer;
		pRequest->Request.DATA.QUERY_INFORMATION.InformationBufferLength = OidData->Length;
	}

	NdisInitializeEvent(&pRequest->InternalRequestCompletedEvent);
	NdisResetEvent(&pRequest->InternalRequestCompletedEvent);

	if (*((PVOID *) pRequest->Request.SourceReserved) != NULL)
	{
		*((PVOID *) pRequest->Request.SourceReserved) = NULL;
	}

	//
	//  submit the request
	//
	pRequest->Request.RequestId = (PVOID) NPF_REQUEST_ID;
	pRequest->Request.RequestHandle = pOpen->pFiltMod->AdapterHandle;
	// ASSERT(pOpen->pFiltMod->AdapterHandle != NULL);

	Status = NdisFOidRequest(pOpen->pFiltMod->AdapterHandle, &pRequest->Request);

	if (Status == NDIS_STATUS_PENDING)
	{
		NdisWaitEvent(&pRequest->InternalRequestCompletedEvent, 0);
		Status = pRequest->RequestStatus;
	}

	//
	// Complete the request
	//
	if (bSetOid)
	{
		OidData->Length = pRequest->Request.DATA.SET_INFORMATION.BytesRead;
		INFO_DBG("BIOCSETOID completed, BytesRead = %u\n", OidData->Length);
		*Info = FIELD_OFFSET(PACKET_OID_DATA, Data);
	}
	else
	{
		ulTmp = pRequest->Request.DATA.QUERY_INFORMATION.BytesWritten;

		// check for the stupid bug of the Nortel driver ipsecw2k.sys v. 4.10.0.0 that doesn't set the BytesWritten correctly
		// The driver is the one shipped with Nortel client Contivity VPN Client V04_65.18, and the MD5 for the buggy (unsigned) driver
		// is 3c2ff8886976214959db7d7ffaefe724 *ipsecw2k.sys (there are multiple copies of this binary with the same exact version info!)
		//
		// The (certified) driver shipped with Nortel client Contivity VPN Client V04_65.320 doesn't seem affected by the bug.
		//
		//if (pRequest->Request.DATA.QUERY_INFORMATION.BytesWritten > pRequest->Request.DATA.QUERY_INFORMATION.InformationBufferLength)
		if (ulTmp > OidData->Length)
		{
			INFO_DBG("Bogus return from NdisRequest (query): Bytes Written (%u) > InfoBufferLength (%u)!!\n",
					pRequest->Request.DATA.QUERY_INFORMATION.BytesWritten, pRequest->Request.DATA.QUERY_INFORMATION.InformationBufferLength);
			ulTmp = OidData->Length; // truncate
			Status = NDIS_STATUS_INVALID_DATA;
		}

		// Don't trust that the length fits in the output buffer
		if (FIELD_OFFSET(PACKET_OID_DATA, Data) + ulTmp > ulBufLenOut)
		{
			ulTmp = ulBufLenOut - FIELD_OFFSET(PACKET_OID_DATA, Data);
			Status = NDIS_STATUS_INVALID_DATA;
		}
		RtlCopyMemory(OidData->Data, OidBuffer, ulTmp);
		OidData->Length = ulTmp;

		INFO_DBG("BIOCQUERYOID completed, BytesWritten = %u\n", OidData->Length);
		*Info = (ULONG_PTR) FIELD_OFFSET(PACKET_OID_DATA, Data) + ulTmp;
	}


	if (Status == NDIS_STATUS_SUCCESS)
	{
		Status = STATUS_SUCCESS;
	}
	else
	{
		// Return the error code of NdisFOidRequest() to the application.
		INFO_DBG("Original NdisFOidRequest() Status = %#x\n", Status);
		// Why do we set this custom bit? Unfortunately now libpcap relies on it.
		Status = (1 << 29) | Status;
		INFO_DBG("Custom NdisFOidRequest() Status = %#x\n", Status);
	}

OID_REQUEST_DONE:

	if (OidBuffer != NULL)
	{
		ExFreePoolWithTag(OidBuffer, NPF_USER_OID_TAG);
	}

	if (pRequest)
	{
		ExFreeToLookasideListEx(&pOpen->DeviceExtension->InternalRequestPool, pRequest);
		pRequest = NULL;
	}

	NPF_StopUsingOpenInstance(pOpen, OpenAttached, NPF_IRQL_UNKNOWN);
	return Status;
}

_Must_inspect_result_
static NTSTATUS funcBIOCSTIMESTAMPMODE(_In_ POPEN_INSTANCE pOpen,
	       _In_reads_bytes_(ulBufLen) PULONG pBuf,
	       _In_ ULONG ulBufLen,
	       _Out_ PULONG_PTR Info)
{
	static const ULONG uNeeded = sizeof(ULONG);
	ULONG mode = 0;

	*Info = 0;
	if (ulBufLen < uNeeded)
	{
		return STATUS_BUFFER_TOO_SMALL;
	}

	mode = *(PULONG) pBuf;

	// verify that the provided mode is supported
	if (!NPF_TimestampModeSupported(mode))
	{
		return STATUS_INVALID_DEVICE_REQUEST;
	} 

	if (!NPF_StartUsingOpenInstance(pOpen, OpenDetached, NPF_IRQL_UNKNOWN))
	{
		return STATUS_CANCELLED;
	}

	if (InterlockedExchange(&pOpen->TimestampMode, mode) != mode)
	{
		/* Reset buffer, since contents have differing timestamps */
		NPF_ResetBufferContents(pOpen, TRUE);
	}

	NPF_StopUsingOpenInstance(pOpen, OpenDetached, NPF_IRQL_UNKNOWN);
	return STATUS_SUCCESS;
}

_Must_inspect_result_
static NTSTATUS funcBIOCGTIMESTAMPMODES(_In_ POPEN_INSTANCE pOpen,
	       _Out_writes_bytes_(ulBufLen) PVOID pBuf,
	       _In_ ULONG ulBufLen,
	       _Out_ PULONG_PTR Info)
{
	// Need to at least deliver the number of modes
	ULONG uNeeded = sizeof(ULONG);
	static ULONG SupportedModes[] = {
		0, // count of modes, 0 means not initialized yet
		TIMESTAMPMODE_SINGLE_SYNCHRONIZATION,
		TIMESTAMPMODE_QUERYSYSTEMTIME,
		// This is last and is not reported if not different than QST
		TIMESTAMPMODE_QUERYSYSTEMTIME_PRECISE
	};

	// Initialize the count if not already done.
	if (SupportedModes[0] == 0)
	{
		// If all modes are supported, Count is length minus 1 for the count element.
		SupportedModes[0] = sizeof(SupportedModes) / sizeof(ULONG) - 1;

		// If KeQuerySystemTimePrecise is available, g_ptrQuerySystemTime will point to it.
		// If it points to KeQuerySystemTime instead, it's not available.
		if (g_ptrQuerySystemTime ==
#ifdef KeQuerySystemTime
				&KeQuerySystemTimeWrapper
#else
				&KeQuerySystemTime
#endif
		   )
		{
			// Precise not supported. Count is as before, but minus 1 for QST Precise.
			SupportedModes[0] -= 1;
		}
	}

	*Info = 0;
	if (ulBufLen < uNeeded)
	{
		return STATUS_BUFFER_TOO_SMALL;
	}

	uNeeded = (SupportedModes[0] + 1) * sizeof(ULONG);
	if (ulBufLen < uNeeded)
	{
		*Info = ulBufLen;
		RtlCopyMemory(pBuf, SupportedModes, ulBufLen);
		return STATUS_BUFFER_OVERFLOW;
	}
	else
	{
		*Info = uNeeded;
		RtlCopyMemory(pBuf, SupportedModes, uNeeded);
		return STATUS_SUCCESS;
	}
}

_Must_inspect_result_
static NTSTATUS funcBIOCGETPIDS(_In_ POPEN_INSTANCE pOpen,
	       _Out_writes_bytes_(ulBufLen) PULONG pBuf,
	       _In_ ULONG ulBufLen,
	       _Out_ PULONG_PTR Info)
{
	LOCK_STATE_EX lockState;
	ULONG cnt = 0;
	ULONG ulWritten = 0;
	// Need to at least deliver the number of PIDS
	static const ULONG uNeeded = sizeof(ULONG);

	*Info = 0;
	if (ulBufLen < uNeeded)
	{
		return STATUS_BUFFER_TOO_SMALL;
	}

	if (!NPF_StartUsingOpenInstance(pOpen, OpenDetached, NPF_IRQL_UNKNOWN))
	{
		return STATUS_CANCELLED;
	}

	ulWritten = sizeof(ULONG);
	NdisAcquireRWLockRead(pOpen->DeviceExtension->AllOpensLock, &lockState, 0);

	for (PLIST_ENTRY CurrEntry = pOpen->DeviceExtension->AllOpens.Flink;
			CurrEntry != &pOpen->DeviceExtension->AllOpens;
			CurrEntry = CurrEntry->Flink)
	{
		POPEN_INSTANCE pOpenTmp = CONTAINING_RECORD(CurrEntry, OPEN_INSTANCE, AllOpensEntry);
		cnt++;
		if (ulWritten <= ulBufLen - sizeof(ULONG))
		{
			pBuf[cnt] = pOpenTmp->UserPID;
			ulWritten += sizeof(ULONG);
		}
	}
	NdisReleaseRWLock(pOpen->DeviceExtension->AllOpensLock, &lockState);

	NPF_StopUsingOpenInstance(pOpen, OpenDetached, NPF_IRQL_UNKNOWN);
	pBuf[0] = cnt;
	*Info = ulWritten;
	if (ulWritten / (ULONG) sizeof(ULONG) < (1 + cnt))
	{
		return STATUS_BUFFER_OVERFLOW;
	}
	return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS
NPF_IoControl(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	)
{
	ULONG_PTR Information = 0;
	PVOID pBuf = Irp->AssociatedIrp.SystemBuffer;
	PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
	POPEN_INSTANCE Open = IrpSp->FileObject->FsContext;
	NTSTATUS Status = STATUS_SUCCESS;
	ULONG InputBufferLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
	ULONG OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
	ULONG FunctionCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;

	// general flag for a couple of ioctls
	BOOLEAN bFlag = FALSE;

	UNREFERENCED_PARAMETER(DeviceObject);
	TRACE_ENTER();

	if (!NPF_IsOpenInstance(Open))
	{
		Status = STATUS_INVALID_HANDLE;
		goto NPF_IoControl_End;
	}

	// Has this IRP been canceled?
	if (Irp->Cancel)
	{
		Status = STATUS_CANCELLED;
		goto NPF_IoControl_End;
	}

	// Make sure at least one buffer is valid and not 0-length.
	if (pBuf == NULL || InputBufferLength + OutputBufferLength == 0)
	{
		Status = STATUS_INVALID_PARAMETER;
		goto NPF_IoControl_End;
	}

	INFO_DBG(
		"Function code is %08lx Input size=%08lx Output size %08lx",
		FunctionCode,
		InputBufferLength,
		OutputBufferLength);

	switch (FunctionCode)
	{
		// OPEN_STATE must be OpenRunning:
#ifndef NPCAP_READ_ONLY
		case BIOCSENDPACKETSSYNC:
			bFlag = TRUE;
		case BIOCSENDPACKETSNOSYNC:
			Status = NPF_BufferedWrite(Open, pBuf, InputBufferLength, bFlag, &Information);
			break;
		case BIOCSWRITEREP:
			Status = funcBIOCSULONG(Open, pBuf, InputBufferLength, &Information, OpenRunning, &Open->Nwrites);
			break;
#endif // NPCAP_READ_ONLY

		// BIOCSETBUFFERSIZE and BIOCSMODE do not technically require
		// an attached adapter, but NPF_StartUsingOpenInstance(x, OpenRunning)
		// does some initialization that is needed to start actually
		// processing packets
		case BIOCSETBUFFERSIZE:
			Status = funcBIOCSETBUFFERSIZE(Open, pBuf, InputBufferLength, &Information);
			break;
		case BIOCSMODE:
			Status = funcBIOCSMODE(Open, pBuf, InputBufferLength, &Information);
			break;

		// OPEN_STATE must be OpenAttached
		case BIOCSETOID:
			bFlag = TRUE;
		case BIOCQUERYOID:
			Status = funcBIOC_OID(Open, pBuf, InputBufferLength, OutputBufferLength, bFlag, &Information);
			break;

		// OPEN_STATE can be OpenDetached
		case BIOCGSTATS:
			Status = funcBIOCGSTATS(Open, pBuf, OutputBufferLength, &Information);
			break;

		case BIOCSETF:
			Status = funcBIOCSETF(Open, pBuf, InputBufferLength, &Information);
			break;
		case BIOCSMINTOCOPY:
			Status = funcBIOCSULONG(Open, pBuf, InputBufferLength, &Information, OpenDetached, &Open->MinToCopy);
			break;
		case BIOCISETLOBBEH:
			Status = funcBIOCISETLOBBEH(Open, pBuf, InputBufferLength, &Information);
			break;
		case BIOCSETEVENTHANDLE:
#ifdef _WIN64
			bFlag = IoIs32bitProcess(Irp);
#else
			bFlag = TRUE;
#endif
			Status = funcBIOCSETEVENTHANDLE(Open, pBuf, InputBufferLength, bFlag, Irp->RequestorMode, &Information);
			break;
		case BIOCSTIMESTAMPMODE:
			Status = funcBIOCSTIMESTAMPMODE(Open, pBuf, InputBufferLength, &Information);
			break;
		case BIOCGETPIDS:
			Status = funcBIOCGETPIDS(Open, pBuf, OutputBufferLength, &Information);
			break;

		// OPEN_STATE doesn't matter for now, since it's global for the whole driver.
		case BIOCGTIMESTAMPMODES:
			Status = funcBIOCGTIMESTAMPMODES(Open, pBuf, OutputBufferLength, &Information);
			break;

#if DBG
		/* Deprecated codes */
		case BIOCGEVNAME:
		case BIOCSETDUMPFILENAME:
		case BIOCSETDUMPLIMITS:
		case BIOCISDUMPENDED:
		case BIOCSRTIMEOUT:
#endif
		default:
			WARNING_DBG("Unknown IOCTL code: %#08x\n", FunctionCode);
			Status = STATUS_INVALID_DEVICE_REQUEST;
			break;
	}

NPF_IoControl_End:
	//
	// complete the IRP
	//
	Irp->IoStatus.Information = Information;
	Irp->IoStatus.Status = Status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	INFO_DBG("Status = %#08x\n", Status);
	TRACE_EXIT();
	return Status;
}
