/*
This file is part of CanFestival, a library implementing CanOpen Stack. 

Copyright (C): Edouard TISSERANT and Francis DUPIN

See COPYING file for copyrights details.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "objacces.h"
#include "objdictdef.h"
#include "def.h"

extern const dict_cste dict_cstes;
extern const indextable CommunicationProfileArea[];
extern const indextable receivePDOParameter[];
extern const indextable transmitPDOParameter[];
extern const indextable RxPDOMappingTable[];
extern const indextable TxPDOMappingTable[];
extern const indextable serverSDOParameter[];
extern const indextable clientSDOParameter[];
extern const indextable manufacturerProfileTable[];
extern const indextable newadd1ProfileTable[];
extern const indextable digitalInputTable[];
extern const indextable digitalOutputTable[];

const indextable *scanOD(UNS16 wIndex, UNS8  bSubindex, UNS8  dwSize, UNS32 *errorCode);

UNS8 accessDictionaryError(UNS16 index, UNS8 subIndex, UNS8 sizeDataDict, UNS8 sizeDataGiven, UNS32 code)
{
#ifdef DEBUG_WAR_CONSOLE_ON
	MSG_WAR(0x2B09,"Dictionary index : ", index);
	MSG_WAR(0X2B10,"           subindex : ", subIndex);
	switch (code) {
	case  OD_NO_SUCH_OBJECT: 
		MSG_WAR(0x2B11,"Index not found ", index);
		break;
	case OD_NO_SUCH_SUBINDEX :
		MSG_WAR(0x2B12,"SubIndex not found ", subIndex);
		break;
	case OD_WRITE_NOT_ALLOWED :
		MSG_WAR(0x2B13,"Write not allowed, data is read only ", index);
		break;
	case OD_LENGTH_DATA_INVALID :    
		MSG_WAR(0x2B14,"Conflict size data. Should be (bytes)  : ", sizeDataDict);
		MSG_WAR(0x2B15,"But you have given the size  : ", sizeDataGiven);
		break;
	case OD_NOT_MAPPABLE :
		MSG_WAR(0x2B16,"Not mappable data in a PDO at index    : ", index);
		break;
	case OD_VALUE_TOO_LOW :
		MSG_WAR(0x2B17,"Value range error : value too low. SDOabort : ", code);
		break;
	case OD_VALUE_TOO_HIGH :
		MSG_WAR(0x2B18,"Value range error : value too high. SDOabort : ", code);
		break;
	default:
		MSG_WAR(0x2B20, "Unknown error code : ", code);
	}
#endif
	return 0; 
}

UNS32 getODentry(UNS16 wIndex,
		  UNS8  bSubindex,
		  void  *pDestData,
		  UNS8  *pExpectedSize,
		  UNS8  *pDataType,
		  UNS8  checkAccess)
{
	/* DO NOT USE MSG_ERR because the macro may send a PDO -> infinite loop if it fails. */
	UNS32 errorCode;
	UNS8 szData = 0;
	const indextable *ptrTable;

	ptrTable = scanOD(wIndex, bSubindex, szData, &errorCode);

	if (errorCode != OD_SUCCESSFUL)
		return errorCode;
	if(ptrTable->bSubCount <= bSubindex){
		/* Subindex not found */
		accessDictionaryError(wIndex, bSubindex, 0, 0, OD_NO_SUCH_SUBINDEX);
		return OD_NO_SUCH_SUBINDEX;
	}

	if(checkAccess && !(ptrTable->pSubindex[bSubindex].bAccessType & WO))
	{
		MSG_WAR(0x2B30, "Access Type : ", ptrTable->pSubindex[bSubindex].bAccessType);
		accessDictionaryError(wIndex, bSubindex, 0, 0, OD_WRITE_NOT_ALLOWED);
		return OD_READ_NOT_ALLOWED;
	}

	*pDataType = ptrTable->pSubindex[bSubindex].bDataType;
	szData = ptrTable->pSubindex[bSubindex].size;

	/* 获取数据指针 */
	*(UNS32 *)pDestData = (UNS32)ptrTable->pSubindex[bSubindex].pObject;

	if(	*pExpectedSize == 0 ||
		*pExpectedSize == szData ||
		(*pDataType == visible_string && *pExpectedSize < szData))
	/* We allow to fetch a shorter string than expected */
	{
#if 0
#ifdef CANOPEN_BIG_ENDIAN
		if(*pDataType > boolean && *pDataType < visible_string){
			/* data must be transmited with low byte first */
			UNS8 i, j = 0;
			for ( i = szData ; i > 0 ; i--) {
				((UNS8*)pDestData)[j++] = ((UNS8*)ptrTable->pSubindex[bSubindex].pObject)[i-1];
			}
		}
		else /* It it is a visible string no endianisation to perform */
			memcpy(pDestData, ptrTable->pSubindex[bSubindex].pObject,szData);
#else
			memcpy(pDestData, ptrTable->pSubindex[bSubindex].pObject,szData);
#endif
#endif
        *pExpectedSize = szData;
		return OD_SUCCESSFUL;
	}
	else
	/* Error ! */
	{
		*pExpectedSize = szData;
		accessDictionaryError(wIndex, bSubindex, szData, *pExpectedSize, OD_LENGTH_DATA_INVALID);
		return OD_LENGTH_DATA_INVALID;
	}
}

UNS32 setODentry(UNS16 wIndex,
		  UNS8  bSubindex, 
		  void  *pSourceData, 
		  UNS8  *pExpectedSize, 
		  UNS8  checkAccess)
{
	UNS8 szData = 0;
	UNS8 dataType;
	UNS32 errorCode;
	const indextable *ptrTable;

	ptrTable = scanOD(wIndex, bSubindex, szData, &errorCode);
	if (errorCode != OD_SUCCESSFUL)
		return errorCode;

	if( ptrTable->bSubCount <= bSubindex ) {
		/* Subindex not found */
		accessDictionaryError(wIndex, bSubindex, 0, *pExpectedSize, OD_NO_SUCH_SUBINDEX);
		return OD_NO_SUCH_SUBINDEX;
	}
	if (checkAccess && (ptrTable->pSubindex[bSubindex].bAccessType == RO)) {
		accessDictionaryError(wIndex, bSubindex, 0, *pExpectedSize, OD_WRITE_NOT_ALLOWED);
		return OD_WRITE_NOT_ALLOWED;
	}

	dataType = ptrTable->pSubindex[bSubindex].bDataType;
	szData = ptrTable->pSubindex[bSubindex].size;

	if( *pExpectedSize == 0 || 
		*pExpectedSize <= szData || 
		(dataType == visible_string && *pExpectedSize < szData)) /* We allow to store a shorter string than entry size */
	{
#ifdef CANOPEN_BIG_ENDIAN
		if(dataType > boolean && dataType < visible_string)
		{
			/* we invert the data source directly. This let us do range testing without */
			/* additional temp variable */
			UNS8 i;
			for ( i = 0 ; i < ( ptrTable->pSubindex[bSubindex].size >> 1)  ; i++) 
			{
				UNS8 tmp =((UNS8 *)pSourceData)[(ptrTable->pSubindex[bSubindex].size - 1) - i];

				((UNS8 *)pSourceData) [(ptrTable->pSubindex[bSubindex].size - 1) - i] = ((UNS8 *)pSourceData)[i];
				((UNS8 *)pSourceData)[i] = tmp;
			}
		}
#endif
#if 0 /* TODO,后续需要增加该部分代码 */
		errorCode = (*d->valueRangeTest)(dataType, pSourceData);
		if (errorCode)
		{
			accessDictionaryError(wIndex, bSubindex, szData, *pExpectedSize, errorCode);
			return errorCode;
		}
#endif
		memcpy(ptrTable->pSubindex[bSubindex].pObject, pSourceData, *pExpectedSize);
		*pExpectedSize = szData;

		/* TODO : Store dans NVRAM */     
		if (ptrTable->pSubindex[bSubindex].bAccessType & TO_BE_SAVED)
		{
            SaveProcFun func;

			func = ptrTable->pSubindex[bSubindex].pSaveProc;
			if (func != ((SaveProcFun) 0))
			{
				func();
			}
		}
		return OD_SUCCESSFUL;
	}
#if 0
	else if (negt_procduce == dataType)
	{
		uint16_t temp;
        uint8_t *pdata = (uint8_t *)pSourceData;

		temp = pdata[1];
		temp = (temp << 8) + pdata[0];
		if (ptrTable->pSubindex[bSubindex].pObject != NULL )
		{
			typedef void (*adjust_func)(uint16_t);

			adjust_func func = (adjust_func )ptrTable->pSubindex[bSubindex].pObject;

			func(temp);
		}
		return OD_SUCCESSFUL;
	}
#endif
	else
	{
		*pExpectedSize = szData;
		accessDictionaryError(wIndex, bSubindex, szData, *pExpectedSize, OD_LENGTH_DATA_INVALID);
		return OD_LENGTH_DATA_INVALID;
	}
}

const indextable *scanOD(UNS16 wIndex,
				   UNS8  bSubindex, 
				   UNS8  dwSize,
				   UNS32 *errorCode)
{
	UNS16 offset = 0;
	const indextable *ptrTable;

	// A propos des memcpy, comme la source est toujours un tableau de char 
	// (cf canOpenMain.c), il n'y a pas de pb d'alignement, et ceux-ci devraient
	// tre portables. (FD)

	*errorCode = OD_SUCCESSFUL;

	if((wIndex >= (UNS16)0x1000 ) && (wIndex <= (UNS16)0x11FF)) {
		/***********************************************/
		/* we are in the communication profile area... */
		/***********************************************/
		offset = wIndex - (UNS16)0x1000;
		if( (wIndex > dict_cstes.comm_profile_last) || (CommunicationProfileArea[offset].pSubindex == NULL)){
			// This index is not defined. 
			accessDictionaryError(wIndex, bSubindex, 0, dwSize, OD_NO_SUCH_OBJECT);
			*errorCode = OD_NO_SUCH_OBJECT;
			return NULL;
		}
		ptrTable = CommunicationProfileArea + offset;
		return ptrTable;
	}
	else if( (wIndex >= (UNS16)0x1400) && (wIndex <= (UNS16)0x15FF) ) { 
		/***************************************/
		/* Receive PDO Communication Parameter */
		/***************************************/
		offset = wIndex - (UNS16)0x1400;
		if( (wIndex > dict_cstes.receive_PDO_last) || (receivePDOParameter[offset].pSubindex == NULL)) {
			accessDictionaryError(wIndex, bSubindex, 0, dwSize, OD_NO_SUCH_OBJECT);
			*errorCode = OD_NO_SUCH_OBJECT;
			return NULL;
		}
		ptrTable = receivePDOParameter + offset;
		return ptrTable;
	}
	else if( ( wIndex >= (UNS16)0x1800) && ( wIndex <= (UNS16)0x19FF) ) { 
		/****************************************/
		/* Transmit PDO Communication Parameter */
		/****************************************/
		offset = wIndex - (UNS16)0x1800;
		if( (wIndex > dict_cstes.transmit_PDO_last) || (transmitPDOParameter[offset].pSubindex == NULL)) {
			accessDictionaryError(wIndex, bSubindex, 0, dwSize, OD_NO_SUCH_OBJECT);
			*errorCode = OD_NO_SUCH_OBJECT;
			return NULL;
		}
		ptrTable = transmitPDOParameter + offset;
		return ptrTable;
	}
	else if( ( wIndex >= (UNS16)0x1600) && (wIndex <= (UNS16)0x17FF) ) { 
		/****************************************/
		/* Receive mapping Parameters           */
		/****************************************/
		offset = wIndex - (UNS16)0x1600;
		if( (wIndex > dict_cstes.receive_PDO_mapping_last) || (RxPDOMappingTable[offset].pSubindex == NULL)) {
			accessDictionaryError(wIndex, bSubindex, 0, dwSize, OD_NO_SUCH_OBJECT);
			*errorCode = OD_NO_SUCH_OBJECT;
			return NULL;
		}
		ptrTable = RxPDOMappingTable + offset;
		return ptrTable;
	}
	else if( (wIndex >= (UNS16)0x1A00) && (wIndex <= (UNS16)0x1BFF) ) { 
		/****************************************/
		/* Transmit mapping Parameters          */
		/****************************************/
		offset = wIndex - (UNS16)0x1A00;
		if( (wIndex > dict_cstes.transmit_PDO_mapping_last) || (TxPDOMappingTable[offset].pSubindex == NULL)) {
			accessDictionaryError(wIndex, bSubindex, 0, dwSize, OD_NO_SUCH_OBJECT);
			*errorCode = OD_NO_SUCH_OBJECT;
			return NULL;
		}
		ptrTable = TxPDOMappingTable + offset;
		return ptrTable;
	}
	else if( (wIndex >= (UNS16)0x1200) && (wIndex <= (UNS16)0x127F) ) { 
		/****************************************/
		/* Server SDO Parameter                 */
		/****************************************/
		offset = wIndex - (UNS16)0x1200;
		if( (wIndex > dict_cstes.server_SDO_last) || (serverSDOParameter[offset].pSubindex == NULL)) {
			accessDictionaryError(wIndex, bSubindex, 0, dwSize, OD_NO_SUCH_OBJECT);
			*errorCode = OD_NO_SUCH_OBJECT;
			return NULL;
		}
		ptrTable = serverSDOParameter + offset;
		return ptrTable;
	}
	else if( (wIndex >= (UNS16)0x1280) && (wIndex <= (UNS16)0x12FF) ) { 
		/****************************************/
		/* Client SDO Parameter                 */
		/****************************************/
		offset = wIndex - (UNS16)0x1280;
		if( (wIndex > dict_cstes.client_SDO_last) || (clientSDOParameter[offset].pSubindex == NULL)) {
			accessDictionaryError(wIndex, bSubindex, 0, dwSize, OD_NO_SUCH_OBJECT);
			*errorCode = OD_NO_SUCH_OBJECT;
			return NULL;
		}
		ptrTable = clientSDOParameter + offset;
		return ptrTable;
	}
	else if( (wIndex >= (UNS16)0x2000) && (wIndex <= (UNS16)0x4FFF) ) { 
		/****************************************/
		/* Manufacturer specific profile        */
		/****************************************/
		offset = wIndex - (UNS16)0x2000;
      
		if( (wIndex > dict_cstes.manufacturerSpecificLastIndex) || (manufacturerProfileTable[offset].pSubindex == NULL)) {
			accessDictionaryError(wIndex, bSubindex, 0, dwSize, OD_NO_SUCH_OBJECT);
			*errorCode = OD_NO_SUCH_OBJECT;
			return NULL;
		}
		ptrTable = manufacturerProfileTable + offset;
		return ptrTable;
	}
    else if( (wIndex >= (UNS16)0x5000) && (wIndex <= (UNS16)0x5FFF) ) { 
		/****************************************/
		/* Manufacturer specific profile        */
		/****************************************/
		offset = wIndex - (UNS16)0x5000;
      
		if( (wIndex > 0x5FFF ) || (newadd1ProfileTable[offset].pSubindex == NULL)) {
			accessDictionaryError(wIndex, bSubindex, 0, dwSize, OD_NO_SUCH_OBJECT);
			*errorCode = OD_NO_SUCH_OBJECT;
			return NULL;
		}
		ptrTable = newadd1ProfileTable + offset;
		return ptrTable;
	}
	else if( (wIndex >= (UNS16)0x6000) && (wIndex <= (UNS16)0x61FF) ) { 
		/****************************************/
		/* Digital Input specific profile       */
		/****************************************/
		offset = wIndex - (UNS16)0x6000;
		if( (wIndex > dict_cstes.digitalInputTableLastIndex) || (digitalInputTable[offset].pSubindex == NULL)) {
			accessDictionaryError(wIndex, bSubindex, 0, dwSize, OD_NO_SUCH_OBJECT);
			*errorCode = OD_NO_SUCH_OBJECT;
			return NULL;
		}
		ptrTable = digitalInputTable + offset;
		return ptrTable;
	}
	else if( (wIndex >= (UNS16)0x6200) && (wIndex <= (UNS16)0x9FFF) ) { 
		/****************************************/
		/* Digital Output specific profile      */
		/****************************************/
		offset = wIndex - (UNS16)0x6200;
		if( (wIndex > dict_cstes.digitalOutputTableLastIndex) || (digitalOutputTable[offset].pSubindex == NULL)) {
			accessDictionaryError(wIndex, bSubindex, 0, dwSize, OD_NO_SUCH_OBJECT);
			*errorCode = OD_NO_SUCH_OBJECT;
			return NULL;
		}
		ptrTable = digitalOutputTable + offset;
		return ptrTable;
	}
	else
	{
		accessDictionaryError(wIndex, bSubindex, 0, dwSize, OD_NO_SUCH_OBJECT);
		*errorCode = OD_NO_SUCH_OBJECT;
		return NULL;
	} 
}

