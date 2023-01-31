#include "Globals.h"

#define IOCONTROL_DoProtect CTL_CODE(FILE_DEVICE_UNKNOWN, 0x20000, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCONTROL_DisableProtect CTL_CODE(FILE_DEVICE_UNKNOWN, 0x20001, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define Log(format, ...) DbgPrintEx(DPFLTR_IHVDRIVER_ID,DPFLTR_ERROR_LEVEL,"[Microcosm Protect]->" format "\n",##__VA_ARGS__)

HANDLE h_ProcessCallBack = NULL;
BOOL DoProtect = FALSE;
DWORD beProtectProcessId = 0;
//Fuck Axing

typedef struct _LDR_DATA                         // 24 elements, 0xE0 bytes (sizeof)
{
	struct _LIST_ENTRY InLoadOrderLinks;                     // 2 elements, 0x10 bytes (sizeof)
	struct _LIST_ENTRY InMemoryOrderLinks;                   // 2 elements, 0x10 bytes (sizeof)
	struct _LIST_ENTRY InInitializationOrderLinks;           // 2 elements, 0x10 bytes (sizeof)
	VOID* DllBase;
	VOID* EntryPoint;
	ULONG32      SizeOfImage;
	UINT8        _PADDING0_[0x4];
	struct _UNICODE_STRING FullDllName;                      // 3 elements, 0x10 bytes (sizeof)
	struct _UNICODE_STRING BaseDllName;                      // 3 elements, 0x10 bytes (sizeof)
	ULONG32      Flags;
	UINT16       LoadCount;
	UINT16       TlsIndex;
	union
	{
		struct _LIST_ENTRY HashLinks;
		struct
		{
			VOID* SectionPointer;
			ULONG32      CheckSum;
			UINT8        _PADDING1_[0x4];
		};
	};

	union
	{
		ULONG32      TimeDateStamp;
		VOID* LoadedImports;
	};
	struct _ACTIVATION_CONTEXT* EntryPointActivationContext;
	VOID* PatchInformation;
	struct _LIST_ENTRY ForwarderLinks;
	struct _LIST_ENTRY ServiceTagLinks;
	struct _LIST_ENTRY StaticLinks;
	VOID* ContextInformation;
	UINT64       OriginalBase;
	union _LARGE_INTEGER LoadTime;
	}LDR_DATA, * PLDR_DATA;



OB_PREOP_CALLBACK_STATUS ProcessCallBack(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION pOperationInformation)
{
	if (DoProtect && beProtectProcessId != 0) {
		HANDLE CatchPid = PsGetProcessId((PEPROCESS)pOperationInformation->Object);
		if (CatchPid == beProtectProcessId) {
			if (pOperationInformation->Operation == OB_OPERATION_HANDLE_CREATE
				|| pOperationInformation->Operation == OB_OPERATION_HANDLE_DUPLICATE) {
				
				pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess = NULL;
				pOperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess = NULL;
			}

		}


	}

	return OB_PREOP_SUCCESS;
}




//Driver Service
VOID DriverUnload(PDRIVER_OBJECT pDriverObj)
{
	UNICODE_STRING strLink;

	//DbgPrint("[Microcosm Protect]DriverUnload\n");

	RtlInitUnicodeString(&strLink, LINK_NAME);

	IoDeleteSymbolicLink(&strLink);
	IoDeleteDevice(pDriverObj->DeviceObject);
}
NTSTATUS DispatchCreate(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
//	DbgPrint("[Microcosm Protect]DispatchCreate\n");
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	
	return STATUS_SUCCESS;
}
NTSTATUS DispatchClose(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{

	//DbgPrint("[Microcosm Protect]DispatchClose\n");
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}
NTSTATUS DispatchIoControl(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
	PIO_STACK_LOCATION pIrpStack;
	ULONG uIoControlCode;
	PVOID pIoBuffer;
	ULONG uInSize;
	ULONG uOutSize;
	//DbgPrint("[Microcosm Protect]DispatchIoctl\n");
	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	//控制码
	uIoControlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;
	//输入输出缓冲区
	pIoBuffer = pIrp->AssociatedIrp.SystemBuffer;
	//输入区域大小
	uInSize = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
	//输出区域大小
	uOutSize = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;
	switch (uIoControlCode)
	{

	case IOCONTROL_DoProtect:
	{
		DoProtect = TRUE;
		int Temp = 0;
		memcpy(&Temp, pIoBuffer, sizeof(int));
		beProtectProcessId = Temp;
		Log("Protect At ProcessId:%d", beProtectProcessId);
		break;
	}
	case IOCONTROL_DisableProtect:
	{
		DoProtect = FALSE;
		beProtectProcessId = 0;
		Log("unProtect");
		ObUnRegisterCallbacks(h_ProcessCallBack);
		break;
	}

	default:
		break;
	}


	if (status == STATUS_SUCCESS)
		pIrp->IoStatus.Information = uOutSize;
	else
		pIrp->IoStatus.Information = 0;
	pIrp->IoStatus.Status = status;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObj, PUNICODE_STRING pRegistryString)
{
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING ustrLinkName;
	UNICODE_STRING ustrDevName;
	PDEVICE_OBJECT pDevObj;
	//初始化驱动例程
	pDriverObj->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
	pDriverObj->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;
	pDriverObj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchIoControl;
	pDriverObj->DriverUnload = DriverUnload;

	// 创建驱动设备
	RtlInitUnicodeString(&ustrDevName, DEVICE_NAME);
	status = IoCreateDevice(pDriverObj, NULL, &ustrDevName, FILE_DEVICE_UNKNOWN,NULL, FALSE, &pDevObj);
	if (!NT_SUCCESS(status)) return status;
	if (IoIsWdmVersionAvailable(1, 0x10))
		RtlInitUnicodeString(&ustrLinkName, LINK_GLOBAL_NAME);
	else
		RtlInitUnicodeString(&ustrLinkName, LINK_NAME);
	//创建符号链接

	status = IoCreateSymbolicLink(&ustrLinkName, &ustrDevName);

	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(pDevObj);
		return status;
	}


	((PLDR_DATA)pDriverObj->DriverSection)->Flags |= 0x20;
	Log("DriverEntry");

	OB_CALLBACK_REGISTRATION pReg_CallBack;
	OB_OPERATION_REGISTRATION pReg_Operation;

	memset(&pReg_CallBack, 0, sizeof(pReg_CallBack));
	pReg_CallBack.Version = ObGetFilterVersion();
	pReg_CallBack.OperationRegistrationCount = 1;
	pReg_CallBack.RegistrationContext = NULL;
	RtlInitUnicodeString(&pReg_CallBack.Altitude, L"21028");
	memset(&pReg_Operation, 0, sizeof(pReg_Operation));

	pReg_Operation.ObjectType = PsProcessType;
	pReg_Operation.Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;

	pReg_Operation.PreOperation = (POB_PRE_OPERATION_CALLBACK)ProcessCallBack;

	pReg_CallBack.OperationRegistration = &pReg_Operation;

	status =ObRegisterCallbacks(&pReg_CallBack, &h_ProcessCallBack);

	return STATUS_SUCCESS;
}
